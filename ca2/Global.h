/* Maintainer: Lee Ee Wei */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>

#define FREQUENCY_MAX 600.0
#define FREQUENCY_MIN 1.0
#define AMPLITUDE_MAX 5.0
#define AMPLITUDE_MIN 0.0

// under global_var_mutex
extern uint8_t global_frequency;
extern uint8_t global_amplitude;
extern bool var_update;
extern bool waveform;
extern bool hardware_ready;

// under global_stop_mutex
extern bool kill_switch;
extern bool info_switch;
extern bool system_pause;

// mutexes
extern pthread_mutex_t print_mutex;
extern pthread_mutex_t global_var_mutex;
extern pthread_mutex_t global_stop_mutex;

// convar
extern pthread_cond_t hardware_ready_cond;
extern pthread_cond_t info_switch_cond;

// for hardware
extern int badr[5];
extern uintptr_t iobase[6];
extern struct pci_dev_info info;
extern void *hdl;

// etc
extern bool reuse_param;          // for choosing main function's spinning conditions
extern sigset_t all_sig_mask_set;

#endif