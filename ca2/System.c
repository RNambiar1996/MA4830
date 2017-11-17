/* Maintainer: Lee Ee Wei */

#include "Global.h"
#include "System.h"
#include "hardware.h"

#include <string.h>
#include <stdlib.h>
// #include <stdint.h>
#include <stdbool.h>
//#include <atomic.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <sys/neutrino.h>
#include <process.h>
#include <sched.h>

// under global_var_mutex
uint8_t global_frequency;
uint8_t global_amplitude;
bool var_update = 0;
bool waveform = 0;
bool calibration_done;
bool hardware_ready;

// under global_stop_mutex
bool kill_switch;
bool info_switch = 0;
bool system_pause;

// shared across threads, but constant after system_init()
bool reuse_param;          // bool to check whether param file is used, if yes, do not catch ctrl + s signal, and do not save backup, will only write once, no need atomic
sigset_t all_sig_mask_set; // set of signals to block all signals for all except 1 thread, the 1 thread will do signal handling

// Mutexes
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;       // for printing to terminal screen
pthread_mutex_t global_var_mutex = PTHREAD_MUTEX_INITIALIZER;  // for global frequency, amplitude, var_update, waveform, calibration_done
pthread_mutex_t global_stop_mutex = PTHREAD_MUTEX_INITIALIZER; // for kill_switch, info_switch, system_pause

// To keep track of thread, for joining when terminating threads
pthread_t oscilloscope_thread_handle; // output to oscilloscope thread
pthread_t hardware_thread_handle;     // handles analog/digital hardware

// Convar to synchronise hardware and system, make sure hardware is ready before spawning oscilloscope output
pthread_cond_t hardware_ready_cond = PTHREAD_COND_INITIALIZER;

// for hardware
int badr[5];
uintptr_t iobase[6];
struct pci_dev_info info;
void *hdl;

// global variable for only this source code
bool info_switch_prev;         // for debounce
bool calibration_flag = false; // to check whether user wants to calibrate potentiometer

int system_init(const char *file_param)
{
	struct sched_param  params;
	int policy;

    // local variables
    bool calibrate;

    // Variables to read file_param
    FILE *fp;            // file pointer
    char str_buffer[64];
    char *temp_str;      // temp string variable to help parse file
    int line_length = 0; // size of line
    int count;           // for loop counter

    // Just to make it a little bit more robust, instead of assuming they are in order
    const char *freq_str = "Scaled Frequency: ";
    const char *amp_str  = "Scaled Amplitude: ";
    const char *waveform_str  = "Waveform: ";

    // pthread attribute
    pthread_attr_t joinable_attr;

    // initializations
    kill_switch = false;  // for ctrl + c
    waveform = 0;         // waveform defaults to 0, which is sine wave

	//sched_getparam(0, &params);
	//params.sched_priority+=1;
	//sched_setscheduler(0,SCHED_RR,&params);
		
    // Check validity of file_param
    if ( strcmp(file_param, "0") ) // if file_param is not "0"
    {
        if ( ( fp = fopen(file_param, "r") ) == NULL ) // check whether able to open file according to path given, if not, return -1
            return -1;
        else
            reuse_param = true;

        // malloc to temp string pointer
        temp_str = (char *) malloc(64);

        for ( count = 0; count < 3; ++count ) // 1 info line + 2 variables
        {
            // get entire line first
            while( (str_buffer[line_length] = getc(fp)) != '\n' )
                ++line_length;
            
            if ( !strncmp(str_buffer, freq_str, strlen(freq_str) ) )
            {
                memset( temp_str, '\0', sizeof(temp_str)); // clear string
                strncpy( temp_str, &str_buffer[strlen(freq_str)], (line_length - strlen(freq_str)) ); // get value
                global_frequency = strtod( temp_str, NULL); // set value
            }
            else if ( !strncmp(str_buffer, amp_str, strlen(amp_str) ) )
            {
                memset( temp_str, '\0', sizeof(temp_str)); // clear string
                strncpy( temp_str, &str_buffer[strlen(amp_str)], (line_length - strlen(amp_str)) ); // get value
                global_amplitude = strtod( temp_str, NULL); // set value
            }
            else if ( !strncmp(str_buffer, waveform_str, strlen(waveform_str) ) )
            {
                memset( temp_str, '\0', sizeof(temp_str)); // clear string
                strncpy( temp_str, &str_buffer[strlen(amp_str)], (line_length - strlen(amp_str)) ); // get value
                waveform = strcmp(temp_str, "Sine"); // set value
            }

            // clear string buffer, and line length variables
            memset(str_buffer, 0, sizeof(str_buffer));
            line_length = 0;
        }

        free(temp_str);     // free temp_str memory that we malloc earlier
        fclose(fp);         // close the stream, because we are good people
    }
    else // if file_param is "0"
        reuse_param = false;

    // setup signal handling
    signal_handling_setup();

    // init and set pthread attributes to be joinable
    if( pthread_attr_init(&joinable_attr) ) // returns 0 on success
    {
        perror("pthread_attr_init");
        exit(EXIT_FAILURE);
    }
    if( pthread_attr_setdetachstate(&joinable_attr, PTHREAD_CREATE_JOINABLE) ) // returns 0 on success
    {
        perror("pthread_attr_setdetachstate");
        exit(EXIT_FAILURE);
    }

    // Spawn all wanted threads
    if (reuse_param)
    {
        if( pthread_create( &hardware_thread_handle, &joinable_attr, &read_param, hw_struct ) ) // returns 0 on success
        //if( pthread_create( &hardware_thread_handle, &joinable_attr, &output_osc_func, NULL ) ) // returns 0 on success
        {
            perror("pthread_create for read_param thread");
            exit(EXIT_FAILURE);
        }
    }
    else // reuse_param == false
    {
        if( pthread_create( &hardware_thread_handle, &joinable_attr, &read_input, NULL ) ) // returns 0 on success
        //if( pthread_create( &hardware_thread_handle, &joinable_attr, &output_osc_func, NULL ) ) // returns 0 on success
        {
            perror("pthread_create for read_input thread");
            exit(EXIT_FAILURE);
        }
    }
    
    // convar to make sure frequency and amplitude has been mapped to current potentiometer value
    pthread_mutex_lock(&global_var_mutex);
    while( hardware_ready == false ) pthread_cond_wait( &hardware_ready_cond, &global_var_mutex );
    pthread_mutex_unlock(&global_var_mutex);
    
    // get the current info_switch state (at this point, read_input() thread has updated it)
    pthread_mutex_lock(&global_stop_mutex);
    info_switch_prev = info_switch; // for debounce
    //printf("\n\n\n\n\n\n\n first infos: %d infos_p: %d \n\n\n\n\n\n", info_switch, info_switch_prev);
    pthread_mutex_lock(&global_stop_mutex);

    if( pthread_create( &oscilloscope_thread_handle, &joinable_attr, &generateWave, NULL ) ) // returns 0 on success
    //if( pthread_create( &oscilloscope_thread_handle, &joinable_attr, &hardware_handle_func, NULL ) ) // returns 0 on success
    {
        perror("pthread_create for generateWave thread");
        exit(EXIT_FAILURE);
    } 

    // Destroys pthread convar object before leaving this function, not needed after this
    if( pthread_cond_destroy(&hardware_ready_cond) ) // returns 0 on success
    {
        perror("pthread_cond_destroy for hardware_ready_cond");
        exit(EXIT_FAILURE);
    }

    // Destroys pthread attribute object before leaving this function, not needed after this
    if( pthread_attr_destroy(&joinable_attr) ) // returns 0 on success
    {
        perror("pthread_attr_destroy");
        exit(EXIT_FAILURE);
    }

	//printf("\n\n\n\n\n\n\n endsysteminit infos: %d infos_p: %d \n\n\n\n\n\n", info_switch, info_switch_prev);

    return 0; // successfully init all threads
}

void parse_calibration_flag(const char *calib_arg)
{
    calibration_flag = strcmp(calib_arg, "0");
}

void print_arg_parse_error()
{
	printf("Please enter only up to 2 arguments in the following format:\n");
	printf("Arg1: [0 to use analog/digital inputs, or path of parameter file, to reuse old parameters]\n");
	printf("Arg2: [1 to undergo calibration procedure for potentiometer if Arg1 is not 0]\n");
    printf("If Arg1 is 0, Arg2 is not needed.\n");
    exit(EXIT_FAILURE);
}

void signal_handling_setup()
{
    // empty the signal set first
    sigemptyset(&all_sig_mask_set);

    // Mask all signals, since 1 thread is dedicated to handle signals
    sigfillset (&all_sig_mask_set);

    signal(SIGINT, INThandler); // main thread catches SIGINT
}

void* hardware_handle_func(void* arg)
{

}
void* output_osc_func(void* arg)
{
	
}

// basically just wait all threads to join, and check whether to save the param
void system_shutdown()
{
    void *status;
    
    // initiating all threads shutdown
    pthread_mutex_lock( &global_stop_mutex );
    kill_switch = true;
    pthread_mutex_unlock( &global_stop_mutex );

    // waiting for all threads to join
    // Wait for oscilloscope output thread to join
    if( pthread_join(oscilloscope_thread_handle, &status) ) // returns 0 on success
    {
        printf("ERROR; return code from pthread_join() is %d\n", status);
        exit(EXIT_FAILURE);
    }

    // Wait for hardware thread to join
    if( pthread_join(hardware_thread_handle, &status) ) // returns 0 on success
    {
        printf("ERROR; return code from pthread_join() is %d\n", status);
        exit(EXIT_FAILURE);
    }

    // destory all mutexes, all threads have joined, nobody is locking any mutex
    if( pthread_mutex_destroy(&print_mutex) ) // returns 0 on success
    {
        printf("ERROR; return code from pthread_mutex_destroy is %d\n", status);
        exit(EXIT_FAILURE);
    }

    if( pthread_mutex_destroy(&global_var_mutex) ) // returns 0 on success
    {
        printf("ERROR; return code from pthread_mutex_destroy is %d\n", status);
        exit(EXIT_FAILURE);
    }

    if( pthread_mutex_destroy(&global_stop_mutex) ) // returns 0 on success
    {
        printf("ERROR; return code from pthread_mutex_destroy is %d\n", status);
        exit(EXIT_FAILURE);
    }

    // no need mutex, all threads have terminated
    printf("All threads terminated. Ending The G Code. Good bye.\n");

    exit(EXIT_SUCCESS);
}

void INThandler(int sig) // handles SIGINT
{
    // alerts ctrl+c detection to user
    pthread_mutex_lock( &print_mutex );
    printf("\"ctrl+c\" detected, ending program\n");
    pthread_mutex_unlock( &print_mutex );

    system_shutdown();
}

//output user's current param to file 
int outputFile(){
    //for output of time in file
    time_t Time = time(NULL);
    struct tm tme = *localtime(&Time);
    char *path = "./output.txt";
      
	FILE *fptr;
    fptr = fopen(path, "w");
    
	if( fptr == NULL )
	{
        pthread_mutex_lock( &print_mutex );
        printf("Error with writing! Invalid Path\n");
        pthread_mutex_unlock( &print_mutex );
        return 0;
	}

    fprintf(fptr,"##Output Param at: %d-%d-%d %d:%d\n", tme.tm_year-100, tme.tm_mon+1, tme.tm_mday, tme.tm_hour, tme.tm_min);
    fprintf(fptr,"Frequency: %lfHz\nAmplitude: %lfV\n Waveform: ",global_frequency*FREQUENCY_MAX/255.0, global_amplitude*AMPLITUDE_MAX/255.0);
    fprintf(fptr,"Waveform: %s\n\n-for program, value in 8 bits\n",waveform?"Square":"Sine");
    fprintf(fptr,"Scaled frequency: %d\nScaled Amplitude: %d\n",global_frequency, global_amplitude);
	fclose(fptr);

    pthread_mutex_lock( &print_mutex );
    printf("Output Path is %s\n", path);
    printf("File saved!\n");
    pthread_mutex_unlock( &print_mutex );

	return 1;
}

void flush_input()
{
    char flush_ch;
    while ( (flush_ch = getchar()) != '\n' );//&& flush_ch != EOF );
}

void check_info_switch()
{
    pthread_mutex_lock( &global_stop_mutex );

	//printf("a");

    if ( info_switch ) // checks for info_switch toggle
    {
    	//printf("b");
    	//printf("\n\n\n\n\n info switch if infos: %d, infos: %d \n\n\n\n\n", info_switch, info_switch_prev);
        //info_switch_prev = info_switch;
        
        system_pause = true;
    }

    pthread_mutex_unlock( &global_stop_mutex );

	//printf("c");

    // no need mutex, only main thread writes to system_pause variable, and only main thread calls this function
    if ( system_pause )
    {
    	//printf("d");
        printSave();
        //printf("e");
     }
}