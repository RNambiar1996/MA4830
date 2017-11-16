/* Maintainer: Lee Ee Wei */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdio.h>
#include <stdbool.h>

#include <signal.h>
#include <pthread.h>
#include <stdint.h>

#define DEFAULT_FREQUENCY 100
#define DEFAULT_AMPLITUDE  10

// under global_var_mutex
extern double global_frequency;
extern double global_amplitude;
extern bool var_update;
extern bool waveform;
extern bool calibration_done; // only in use if user wants to calibrate
extern bool hardware_ready;

// under global_stop_mutex
extern bool kill_switch;
extern bool info_switch;
extern bool system_pause;

// mutexes
extern pthread_mutex_t print_mutex;
extern pthread_mutex_t global_var_mutex;
extern pthread_mutex_t global_stop_mutex;

// for hardware
extern int badr[5];
extern uintptr_t iobase[6];
extern struct pci_dev_info info;
extern void *hdl;

// etc
extern bool reuse_param;
extern sigset_t all_sig_mask_set;

#endif