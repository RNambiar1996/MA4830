/* Maintainer: Lee Ee Wei */

#include "Global.h"
#include "System.h"

//#define _GNU_SOURCE
//#define _XOPEN_SOURCE 700
// #define _POSIX_C_SOURCE 200809L
// #define _XOPEN_SOURCE 700
// #define _XOPEN_SOURCE 700
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// data types
// #include <stdbool.h>

// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <signal.h>
// #include <errno.h>
// #include <pthread.h>

// Declaration of global variables for all source codes
double global_frequency;
double global_amplitude;
double global_offset;
bool kill_switch;
sigset_t all_sig_mask_set; // set of signals to block all signals for all except 1 thread, the 1 thread will do signal handling

// Initializing Mutexes
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t global_var_mutex = PTHREAD_MUTEX_INITIALIZER;

// To keep track of thread ids if necessary
pthread_t oscilloscope_thread_handle; // output to oscilloscope thread
pthread_t hardware_thread_handle;     // handles analog/digital hardware

// bool to check whether param file is used, if yes, do not catch ctrl + s signal, and do not save backup
bool reuse_param;

int system_init(char *D2A_port_selection, char *file_param )
{
    // Declaration of variables
    bool D2A_Port;

    // Variables to read file_param
    FILE *fp;                  // file pointer
    char *line_pointer = NULL; // line pointer for getline()
    char *temp_str;            // temp string variable to help parse file
    int length_of_line;        // size of line
    int count;                 // for loop counter
    size_t read_line_size = 0; // let getline() determine by itself
    // Just to make it a little bit more robust, instead of assuming they are in order
    const char *freq_str = "Frequency: ";
    const char *amp_str = "Amplitude: ";
    const char *offset_str = "Offset: ";

    // pthread attribute
    pthread_attr_t joinable_attr;

    // initializations
    kill_switch = false;

    // Check validity of file_param
    if ( strcmp(file_param, "0") ) // if file_param is not "0"
    {
        if ( ( fp = fopen(file_param, "r") ) == NULL ) // check whether able to open file according to path given, if not, return -1
            return -1;
        else
            reuse_param = true;

        // malloc to temp string pointer
        temp_str = (char *) malloc(64);
        // getline() automatically malloc() or realloc() to line_pointer, so no need to do it ourself

        for ( count = 0; count < 4; ++count ) // 1 info line + 3 variables
        {
            length_of_line = getline(&line_pointer, &read_line_size, fp);
            
            if ( !strncmp(line_pointer, freq_str, strlen(freq_str) ) )
            {
                memset( temp_str, '\0', sizeof(temp_str)); // clear string
                strncpy( temp_str, &line_pointer[strlen(freq_str)], (length_of_line - strlen(freq_str)) ); // get value
                global_frequency = strtod( temp_str, NULL); // set value
            }
            else if ( !strncmp(line_pointer, amp_str, strlen(amp_str) ) )
            {
                memset( temp_str, '\0', sizeof(temp_str)); // clear string
                strncpy( temp_str, &line_pointer[strlen(amp_str)], (length_of_line - strlen(amp_str)) ); // get value
                global_amplitude = strtod( temp_str, NULL); // set value
            }
            else if ( !strncmp(line_pointer, offset_str, strlen(offset_str) ) )
            {
                memset( temp_str, '\0', sizeof(temp_str)); // clear string
                strncpy( temp_str, &line_pointer[strlen(offset_str)], (length_of_line - strlen(offset_str)) ); // get value
                global_offset = strtod( temp_str, NULL); // set value
            }
        }

        free(temp_str);     // free temp_str memory that we malloc earlier
        free(line_pointer); // free line_pointer memory
        fclose(fp);         // closes the stream, because we are good people
    }
    else // if file_param is "0"
    {
        reuse_param = false;

        // set default global values
        global_frequency = DEFAULT_FREQUENCY;
        global_amplitude = DEFAULT_AMPLITUDE;
        global_offset    = DEFAULT_OFFSET;
    }

    D2A_Port = D2A_port_selection[0] - '0'; // since we have confirmed Arg 1 is either '0' or '1', can do this directly

    // setup signal handling mask set
    signal_handling_setup();

    // // set pthread attributes and relevant things
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

    // Spawn all necessary threads
    pthread_create( &oscilloscope_thread_handle, &joinable_attr, &output_osc_func, NULL );
    pthread_create( &hardware_thread_handle, &joinable_attr, &hardware_handle_func, &D2A_Port );
    //pthread_create( &sig_thandle, &joinable_attr, &sig_handle_func );

    // Mask all signals
    //pthread_sigmask (SIG_SIG_SETMASK, &signal_mask, NULL);

    return 0; // successfully init all threads
}

void signal_handling_setup()
{
    // empty the signal set first
    sigemptyset(&all_sig_mask_set);

    // Mask all signals, since 1 thread is dedicated to handle signals
    sigfillset (&all_sig_mask_set); //sigdelset() use this in the sig handle thread
}

void* hardware_handle_func(void* arg)
{

}
void* output_osc_func(void* arg)
{

}