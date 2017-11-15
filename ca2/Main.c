/*
 * Group members: Tan You Liang, Nicholas Adrian, Rahul Nambiar, Lee Ee Wei
 * Maintainer of "Main.c": Lee Ee Wei
 * Compile line: gcc -o The_G_Code Main.c System.c
 * 
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

    // the alternative to this would be to overload system_init(), but system_init(), is quite a long function
    if (argc == 3)
        parse_calibration_flag(argv[2]);

    // initialize system and threads
    if ( system_init(argv[1]) == -1 )
    {
        printf("Path of parameter file is invalid. Please enter a valid path if you would like to reuse old parameters.\n");
        return 0;
    }

    print_info();

    // spin main thread
    while(1)
    {
    	delay(1);
    	check_info_switch();
    }  

    return 0;
}