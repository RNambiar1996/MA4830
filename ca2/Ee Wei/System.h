/* Maintainer: Lee Ee Wei */

#pragma once
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

int system_init(char *D2A_port_selection, char *file_param );

// setup signal handling variables
void signal_handling_init()

//int system_shutdown(); // probably need arg

// void set_init_params();
// void set_init_params(double* freq, double* amp, double*);

#endif