/*
 * Group members: Tan You Liang, Nicholas Adrian, Rahul Nambiar, Lee Ee Wei
 * Maintainer of "Main.c": Lee Ee Wei
 * Compile line(on QNX): cc -o The_G_Code Main.c System.c Print.c Input.c wave.c -lm
 *  
 * FAQ:
 * bool info_switch variable is not called pause_switch due to legacy
 *
 */

#include "Global.h"
#include "System.h"

int main(int argc, char *argv[])
{
    // check arguments
	// this part is coded in this way because checking for arguments that are not there will raise segmentation fault
    if (argc != 2)	// check that arg count is only 2
		print_arg_parse_error();
	else if (argc == 2 && strcmp(argv[1], "0"))
		print_arg_parse_error();

    // initialize system and threads
    if ( system_init(argv[1]) == -1 )
    {
        printf("Path of parameter file is invalid. Please enter a valid path if you would like to reuse old parameters.\n");
        return 0;
    }

    printInit();
    
    // spin main thread
    if ( reuse_param )
    {
	    printCurrent();          //print current frequency and amplitude only once
    	while( true )
    		delay(10);
    }
    else // reuse_param == false
    {
    	while( true )
    	{
            check_info_switch(); //check info switch to prompt saving and/or quit,
        	printCurrent();          //print current frequency and amplitude 
       		delay(10);
    	}
    }
    
    return 0;
}