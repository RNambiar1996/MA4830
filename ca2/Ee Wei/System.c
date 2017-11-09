/* Maintainer: Lee Ee Wei */

#include "Global.h"
#include "System.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

// Declaration of global variables for all source codes
double global_frequency;
double global_amplitude;
double global_offset;
sigset_t sig_mask_set; // set of signals to block all signals for all except 1 thread, the 1 thread will do signal handling

// Initializing Mutexes
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t global_var_mutex = PTHREAD_MUTEX_INITIALIZER;

// To keep track of threads
pthread_t output_osc;       // output to oscilloscope thread
pthread_t sig_handle;       // signal handle thread
pthread_t hardware_handle;  // handles analog/digital hardware

int system_init(char *D2A_port_selection, char *file_param )
{
    // Declaration of variables
    bool reuse_param;
    bool D2A_Port;
    int fd; // file descriptor

    // pthread attribute
    pthread_attr_t joinable_attr;
    
    // Check validity of file_param
    if ( strcmp(file_param, "0") ) // if file_param is not "0"
    {
        if ( fd = open(file_param, O_RDONLY) < 0 ) // check whether able to open file according to path given, if not, return -1
            return -1;
        else
            reuse_param = true;

        // use getline()
    }
    else
    {
        reuse_param = false;
        // set_default_params();
    }

    D2A_Port = D2A_port_selection[0] - '0'; // since we have confirmed Arg 1 is either '0' or '1', can do this directly

    // setup signal handling mask set
    signal_handling_init();

    // set pthread attributes and relevant things
    if( pthread_attr_init(&joinable_attr) )
    {
        perror("pthread_attr_init");
        exit(EXIT_FAILURE);
    }

    if( pthread_attr_setdetachstate(&joinable_attr, PTHREAD_CREATE_JOINABLE) )
    {
        perror("pthread_attr_setdetachstate");
        exit(EXIT_FAILURE);
    }

    // Spawn all necessary threads
    pthread_create( &output_osc, &joinable_attr, %output_osc_func );
    pthread_create( &sig_handle, &joinable_attr, %sig_handle_func );
    pthread_create( &hardware_handle, &joinable_attr, %hardware_handle_func, &D2A_Port );

    return 0; // successfully init all threads
}

void signal_handling_init()
{
    // empty the signal set, normal practice
    sigemptyset(&sig_mask_set);

    // Mask all signals, since 1 thread is dedicated to handle signals
    sigfillset (&sig_mask_set); //sigdelset() use this in the sig handle thread

    //rc = pthread_sigmask (SIG_SIG_SETMASK, &signal_mask, NULL);
}