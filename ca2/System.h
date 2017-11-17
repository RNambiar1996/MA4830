/* Maintainer: Lee Ee Wei */

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

// system init
int system_init(const char *file_param);




// setup signal handling variables
void signal_handling_setup();

// SIGINT handler function callback
void INThandler(int sig);

// inititate systematic program shutdown
void system_shutdown();

// to flush '\n'
void flush_input();

// check whether pause switch is toggled
void check_info_switch();

// Print.c function prototypes
void printInit();               // print initial messages
void printSave();               // print this is pause switch is toggled
void printCurrent();            // print the current frequency and amplitude values
int  outputFile();              // function outputs file, returns 0 on success, return -1 on error
void print_arg_parse_error();   // for arg parsing error

#endif