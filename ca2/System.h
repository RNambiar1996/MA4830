/* Maintainer: Lee Ee Wei */

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <stdbool.h>
#include <string.h>

// system init
int system_init(const char *file_param);

// for parsing calibration flag
void parse_calibration_flag(const char *calib_arg);

// for arg parsing error
void print_arg_parse_error();

// setup signal handling variables
void signal_handling_setup();

// remove when all is done by respective coder
void* hardware_handle_func(void*); // Nicholas
void* output_osc_func(void*); // Rahul
void save_state(const bool *save_param); // You Liang
void INThandler(int sig);
void INThandler2(int sig);
void system_shutdown();

int outputFile(const char *path);

void flush_input();

void print_info();

void check_info_switch();


//printout
void printInit();
void printSave();

#endif