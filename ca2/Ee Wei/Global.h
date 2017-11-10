/* Maintainer: Lee Ee Wei */

#pragma once
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

// this is to prevent the getline() warning
//#define _XOPEN_SOURCE 700 // need to change depending on glibc version, run "ldd --version" on terminal
//#define _POSIX_C_SOURCE 200809L
//#define _GNU_SOURCE
#include <stdio.h>
//https://www.linuxquestions.org/questions/programming-9/getline-problem-4175485184/

// #define _XOPEN_SOURCE 700
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

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