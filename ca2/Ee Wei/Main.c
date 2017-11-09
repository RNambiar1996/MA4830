/*
 * Group members: Tan You Liang, Nicholas Adrian, Rahul Nambiar, Lee Ee Wei
 * Maintainer of "Main.c": Lee Ee Wei
 * Compile line: g++ -o Program Main.c System.c
 * 
*/

#include "Global.h"
#include "System.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <stdbool.h>
// #include <float.h>
// #include <unistd.h>

int main(int argc, char *argv[])
{
    // Parse arguments
    if( argc != 3 ||                                      // Make sure only 2 arguments , otherwise show message and exit
        (strcmp(argv[1], "0") && strcmp(argv[1], "1")) )  // Check that Arg 1 is either '0' or '1'
    {
        printf("Please enter only up to 2 arguments in the following format:\n");
        printf("Arg 1: [0 or 1, D/A port selection]\n");
        printf("Arg 2: [0 to use analog/digital inputs, or path of parameter file, to reuse old parameters]\n");
        return 0;
    }

    // initialize system and threads
    if ( system_init(argv[1], argv[2]) == -1 )
    {
        printf("Path of parameter file is invalid. Please enter a valid path if you would like to reuse old parameters.\n");
        return 0;
    }

    return 0;
}