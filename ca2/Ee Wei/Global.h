/* Maintainer: Lee Ee Wei */

#pragma once
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define DEFAULT_FREQUENCY 100
#define DEFAULT_AMPLITUDE 10
#define DEFAULT_OFFSET 1

extern double global_frequency;
extern double global_amplitude;
extern double global_offset;
extern sigset_t sig_mask_set;

#endif