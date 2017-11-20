/* Maintainer: Lee Ee Wei */

#include "Global.h"
#include "System.h"
#include "Hardware.h"

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
bool reuse_param;           // bool to check whether param file is used, if yes, do not catch ctrl + s signal, and do not save backup, will only write once, no need atomic
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
pthread_cond_t info_switch_cond = PTHREAD_COND_INITIALIZER;

// for hardware
int badr[5];
uintptr_t iobase[6];
struct pci_dev_info info;
void *hdl;

int system_init(const char *file_param)
{
    // Variables to read file_param
    FILE *fp;                  // file pointer
    char str_buffer[64]; // buffer to read file
    char *temp_str;       // temp string variable to help parse file
    int line_length = 0;  // size of line
    int count;                // for loop counter
    
    char *end_str;

    // Just to make it a little bit more robust, instead of assuming they are in order
    const char *freq_str = "Scaled Frequency: ";
    const char *amp_str  = "Scaled Amplitude: ";
    const char *waveform_str  = "Waveform: ";

    // pthread attribute
    pthread_attr_t joinable_attr;

    // initializations
    kill_switch = false;  // for ctrl + c
		
    // Check validity of file_param
    if ( strcmp(file_param, "0") ) // if file_param is not "0"
    {
        if ( ( fp = fopen(file_param, "r") ) == NULL ) // check whether able to open file according to path given, if not, return -1
            return -1;
        else
            reuse_param = true;

        // malloc to temp string pointer
        temp_str = (char *) malloc(64);

        while ( true )
        {
            // get entire line first
            while( (str_buffer[line_length] = getc(fp)) != '\n' &&  str_buffer[line_length] != EOF)
                ++line_length;
            
            if ( !strncmp(str_buffer, waveform_str, strlen(waveform_str) ) )
            {
                memset( temp_str, '\0', sizeof(temp_str)); // clear string
                strncpy( temp_str, &str_buffer[strlen(waveform_str)], (line_length - strlen(waveform_str)) ); // get value
                waveform = strcmp(temp_str, "Sine");
            }
            else if ( !strncmp(str_buffer, freq_str, strlen(freq_str) ) )
            {
                memset( temp_str, '\0', sizeof(temp_str)); // clear string
				strncpy( temp_str, &str_buffer[strlen(freq_str)], (line_length - strlen(freq_str)) ); // get value
                global_frequency = strtol( temp_str, NULL, 10); // set value
            }
            else if ( !strncmp(str_buffer, amp_str, strlen(amp_str) ) )
            {
                memset( temp_str, '\0', sizeof(temp_str)); // clear string
                strncpy( temp_str, &str_buffer[strlen(amp_str)], (line_length - strlen(amp_str)) ); // get value
                global_amplitude = strtol( temp_str, NULL, 10); // set value
            }

            if ( str_buffer[line_length] == EOF ) // exit when EOF has been found
            	break;

            // clear string buffer, and line length variables for next iteration
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
        if( pthread_create( &hardware_thread_handle, &joinable_attr, &read_param, NULL ) ) // returns 0 on success
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

    return 0; // successfully init all threads
}



void signal_handling_setup()
{
    // empty the signal set first
    sigemptyset(&all_sig_mask_set);

    // Mask all signals, since 1 thread is dedicated to handle signals
    sigfillset (&all_sig_mask_set);

    signal(SIGINT, INThandler); // main thread catches SIGINT
}


// basically just wait all threads to join, and check whether to save the param
void system_shutdown()
{
    void *status;
    
    // initiating all threads shutdown
    pthread_mutex_lock( &global_stop_mutex );
    kill_switch = true;
    // read_input thread might be waiting for info switch to be false, make sure it wakes up
    system_pause = false;
    info_switch = false;
    pthread_cond_signal(&info_switch_cond);     // signal hardware thread to wake up
    pthread_mutex_unlock( &global_stop_mutex );

    // waiting for all threads to join
    // Wait for oscilloscope output thread to join
    if( pthread_join(oscilloscope_thread_handle, &status) ) // returns 0 on success
    {
        printf("ERROR; return code from wave pthread_join() is %d\n", status);
        exit(EXIT_FAILURE);
    }

    // Wait for hardware thread to join
    if( pthread_join(hardware_thread_handle, &status) ) // returns 0 on success
    {
        printf("ERROR; return code from hardware pthread_join() is %d\n", status);
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

    // Destroys pthread convar object
    if( pthread_cond_destroy(&info_switch_cond) ) // returns 0 on success
    {
        perror("pthread_cond_destroy for info_switch_cond");
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


void check_info_switch()
{
    pthread_mutex_lock( &global_stop_mutex );

    if ( info_switch )       // checks for info_switch toggle
        system_pause = true; // copies info_switch state to system_pause, to not hog mutex

    pthread_mutex_unlock( &global_stop_mutex );

    // no need mutex, only main thread writes to system_pause variable, and only main thread calls this function
    if ( system_pause )
        printSave();
}
