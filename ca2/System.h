/* Maintainer: Lee Ee Wei */

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
int system_init(const char *file_param );

// setup signal handling variables
void signal_handling_setup();

// remove when all is done by respective coder
void* hardware_handle_func(void*); // Nicholas
void* output_osc_func(void*); // Rahul
void save_state(const bool *save_param); // You Liang
void INThandler(int sig);
void system_shutdown();

int outputFile(const char *path);

void flush_input();

void print_info();

void check_info_switch();

#endif