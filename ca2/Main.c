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
    // Check arguments
    if( argc != 2 ) // Make sure only 1 argument , otherwise show message and exit
    {
        printf("Please enter only 1 argument in the following format:\n");
        printf("Arg: [0 to use analog/digital inputs, or path of parameter file, to reuse old parameters]\n");
        return 0;
    }

    // initialize system and threads
    if ( system_init(argv[1]) == -1 )
    {
        printf("Path of parameter file is invalid. Please enter a valid path if you would like to reuse old parameters.\n");
        return 0;
    }

    print_info();

    signal(SIGINT, INThandler); // catch SIGINT
    
    // spin main thread
    while(1)
    {
    	delay(1);
    	check_info_switch();
    }  

    return 0;
}