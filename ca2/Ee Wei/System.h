/* Maintainer: Lee Ee Wei */

#pragma once
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
//#include <float.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

// system init
int system_init(char *D2A_port_selection, char *file_param );

// setup signal handling variables
void signal_handling_setup();

// remove when all is done by respective coder
void* hardware_handle_func(void*); // Nicholas
void* output_osc_func(void*); // Rahul
// void* save_state(); // You Liang

int system_shutdown();

//int system_shutdown(); // probably need arg

// void set_init_params();
// void set_init_params(double* freq, double* amp, double*);

#endif