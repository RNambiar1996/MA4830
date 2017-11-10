/* Maintainer: Lee Ee Wei */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdio.h>
#include <stdbool.h>

// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <unistd.h>
#include <signal.h>
// #include <errno.h>
// #include <pthread.h>
//#include <atomic.h>

#define DEFAULT_FREQUENCY 100
#define DEFAULT_AMPLITUDE 10
#define DEFAULT_OFFSET 1

extern double global_frequency;
extern double global_amplitude;
extern double global_offset;
extern bool kill_switch;
extern bool waveform;
extern bool reuse_param;
extern sigset_t all_sig_mask_set;

#endif