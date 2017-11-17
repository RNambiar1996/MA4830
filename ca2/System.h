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

// remove when final
void* hardware_handle_func(void*);
void* output_osc_func(void*);

void INThandler(int sig);
void system_shutdown();

int outputFile();

void flush_input();

void check_info_switch();

//Within Print.c
void printInit();
void printSave();
void printCurrent();
void outputFile();

#endif