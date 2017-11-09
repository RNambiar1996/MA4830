/*
 * Group members: Tan You Liang, Nicholas Adrian, Rahul Nambiar, Lee Ee Wei
 * Maintainer of "Main.c": Lee Ee Wei
 * Compile line: g++ -o The_G_Code Main.c System.c
 * 
*/

#include "Global.h"
#include "System.h"


int main(int argc, char *argv[])
{
    // Variables to read file_param
    FILE *fp;           // file pointer
    char *line_pointer = NULL; // line pointer for getline()
    char *temp_str;     // temp string variable to help parse file
    int size_of_line;   // size of line
    int count;          // for loop counter
    size_t read_line_size = 64;
    const char *freq_str = "Frequency:";
    const char *amp_str = "Amplitude:";
    const char *offset_str = "Offset:";

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

    // signal_handle_spin();

    return 0;
}