/* Maintainer: Lee Ee Wei */

#pragma once
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define _XOPEN_SOURCE 700
#include <stdio.h>

// #define _XOPEN_SOURCE 700
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// data types
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#define DEFAULT_FREQUENCY 100
#define DEFAULT_AMPLITUDE 10
#define DEFAULT_OFFSET 1

extern double global_frequency;
extern double global_amplitude;
extern double global_offset;
extern sigset_t all_sig_mask_set;
extern bool kill_switch;

#endif