/*
 * Group members: Tan You Liang, Nicholas Adrian, Rahul Nambiar, Lee Ee Wei
 * Maintainer of "Main.c": Lee Ee Wei
 * Compile line(on QNX): cc -o The_G_Code Main.c System.c Print.c Input.c wave.c -lm
 *  
 * Note:
 * - When done remove these from System.h and System.c
 *      void* hardware_handle_func(void*);
 *      void* output_osc_func(void*);
 * - Change wave.c to Wave.c
 * - Check which header files are not needed and remove, and try to only include C headers in our own header files instead of source code
 */

#include "Global.h"
#include "System.h"

int main(int argc, char *argv[])
{
    // check arguments
	// this part is coded in this way because checking for arguments that are not there will raise segmentation fault
    if (argc != 2 && argc != 3)	// check that arg count is either 2 or 3
		print_arg_parse_error();
	else if (argc == 2 && strcmp(argv[1], "0")) // 
		print_arg_parse_error();
	else if (argc == 3 && (!strcmp(argv[1], "0") || (strcmp(argv[2], "0") && strcmp(argv[2], "1")) ) )
		print_arg_parse_error();

    // parse calibration flag if available
    if (argc == 3)
        parse_calibration_flag(argv[2]);

    // initialize system and threads
    if ( system_init(argv[1]) == -1 )
    {
        printf("Path of parameter file is invalid. Please enter a valid path if you would like to reuse old parameters.\n");
        return 0;
    }

    printInit();
    printf("---------- Process Officially Starts!!!---------\n");
    
    // spin main thread
    while(1)
    {
        if ( !reuse_param )
            check_info_switch(); //check info switch to prompt saving and/or quit,
            
        //printf("main after check info switch\n");
        printCurrent();          //print current frequency and amplitude 
        delay(100);
        //sleep(1);                //change to 1 milisecond in QNX
    }  

    return 0;
}