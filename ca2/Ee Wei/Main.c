/*
 * Group members: Tan You Liang, Nicholas Adrian, Rahul Nambiar, Lee Ee Wei
 * Maintainer of "Main.c": Lee Ee Wei
 * Compile line: gcc -o The_G_Code Main.c System.c -pthread -std=c11
 * (might work without -std=c11)
 * 
 * Justification for use of atomic
 * https://stackoverflow.com/questions/15056237/which-is-more-efficient-basic-mutex-lock-or-atomic-integer
 */

#include "Global.h"
#include "System.h"
#include <string.h>

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

    // signal_handle_spin(); // check reuse_param, if true, do not catch ctrl + s // put kill switch to true here, if ctrl + c signal caught
    printf("starts");
    signal(SIGINT, INThandler); //ctrl-C
    
    // call system_shutdown
    while(1){
        pause();
    }
    return 0;
}