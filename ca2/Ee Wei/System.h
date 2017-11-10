/* Maintainer: Lee Ee Wei */

#pragma once
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

// #include <string.h>
// #include <stdlib.h>
// #include <stdint.h>
#include <stdbool.h>

// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <signal.h>
// #include <errno.h>
// #include <pthread.h>

// system init
int system_init(const char *D2A_port_selection, const char *file_param );

// setup signal handling variables
void signal_handling_setup();

// remove when all is done by respective coder
void* hardware_handle_func(void*); // Nicholas
void* output_osc_func(void*); // Rahul
void save_state(const bool *save_param); // You Liang

int system_shutdown(const bool *save_param);

//int system_shutdown(); // probably need arg

#endif