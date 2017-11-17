#include "Global.h"
#include "System.h"

// local relative to this source code
uint8_t local_frequency;
uint8_t local_amplitude;
bool local_waveform;
uint8_t previous_local_frequency;
uint8_t previous_local_amplitude;
bool previous_local_waveform;

// to store value after conversion
double real_frequency;
double real_amplitude;

//print parse error
void print_arg_parse_error()
{
	printf("Please enter only 1 argument in the following format:\n");
	printf("Arg1: [0 to use analog/digital inputs, or path of parameter file, to reuse old parameters]\n");
    exit(EXIT_FAILURE);
}


//print Initial lines
void printInit()
{
	printf("\33[2J"); // clears screen
    printf("---------- Welcome to the G-code. This program outputs waveform to the oscilloscope. ----------\n");
    printf("       __________                                                  _ _  \n");
    printf("     /           \\                                               |   | \n");
    printf("    /     ________|                                              |   |  \n");
    printf("   |     /                            ________    _____       __/    |   ________ \n");
    printf("   |    |      ____    _________     /   _____|  /  __  \\   /  __    |  /   ____  \\    \n");
    printf("   |    |    /_    \\  |         |   |   /       |  |  |  |  |  |  |  |  |  |___|  |     \n");
    printf("   |    \\______|   |  |_________|   |   |       |  |  |  |  |  |  |  |  |   ______/     \n");
    printf("   \\               |                |   \\_____  |   --   |  |   \\/   |  \\  \\______  \n");
    printf("     \\____________/                  \\________|  \\______/   \\_______/    \\________| \n\n");
    printf("  -Instructions:\n");
    printf("    -Toggle switches(from left to right):\n");
    printf("       a. pause switch (Toggling it pauses hardware input and prompt user to save, quit or continue.)\n");
    printf("       b. waveform     (Change between sine wave and square wave)\n");
    printf("    -Analog input(from left to right):\n");
    printf("       a. D/A 0        (Changes frequency of wave)\n");
    printf("       b. D/A 1        (Changes amplitude of wave)\n");
    printf("  -Other information:\n");
    printf("    -The program outputs wave function to A/D port '0'.\n");
    printf("    -Number of LED lit up shows the amplitude level.\n");
    printf("    -The program will update the terminal screen with the latest values of frequency, and amplitude.\n");
    printf("    -If parameters are loaded from file, you can only review it, hardware input is disabled.\n");
    printf("    -After \"ctrl+c\" is detected, the program will shutdown all threads systematically.\n\n");
    printf("\n\n\n"); // for printSave(), so that the current values will remain even after pause
}


// display save instructions
void printSave(){  
    char input[8]; // buffer for scanf
    char flush_ch; // to flush '\n'
	int count, lines_to_remove = 0;

    // save instructions/info
    pthread_mutex_lock( &print_mutex );
    printf("\nEnter 's' to save, 'q' to quit, other enter to continue\n");
    pthread_mutex_unlock( &print_mutex );
    ++lines_to_remove;
    
    scanf("%[^\n]8s", input);
    ++lines_to_remove;                          // scanf takes 1 line too
    while ( (flush_ch = getchar()) != '\n' );   //flush input 
    
    pthread_mutex_lock( &print_mutex );
    printf("\33[1A");    // move cursor up 1 line
    printf("%c[2K", 27); // clear entire line
    pthread_mutex_unlock( &print_mutex );
    
    if ( !strcmp(input, "q") || !strcmp(input, "Q") )
    {
        pthread_mutex_lock( &print_mutex );
        printf("Shutting down program!\n");
        pthread_mutex_unlock( &print_mutex );
        
        // wakes hardware input thread up to check kill_switch and terminate
        pthread_mutex_lock( &global_stop_mutex );
    	system_pause = false;
    	info_switch = false;
    	pthread_cond_signal(&info_switch_cond);     // signal hardware thread to wake up
    	pthread_mutex_unlock( &global_stop_mutex );
        
        system_shutdown();
    }
    else if( !strcmp(input, "s") || !strcmp(input, "S") )
    {
        pthread_mutex_lock( &print_mutex );
        printf("Saving parameters!\n");
        pthread_mutex_unlock( &print_mutex );

        if ( outputFile() )
        {
            pthread_mutex_lock( &print_mutex );
            printf("Output failed!\n");
            pthread_mutex_unlock( &print_mutex );
            ++lines_to_remove;
        }
        lines_to_remove += 3;
    }
    else
    {
        pthread_mutex_lock( &print_mutex );
        printf("Continuing program...\n");
        pthread_mutex_unlock( &print_mutex );
        ++lines_to_remove;
    }

    pthread_mutex_lock( &global_stop_mutex );
    system_pause = false;
    info_switch = false;
    pthread_cond_signal(&info_switch_cond);     // signal hardware thread to wake up
    pthread_mutex_unlock( &global_stop_mutex );
    
    pthread_mutex_lock(&print_mutex);
    printf("----------  Resuming The G-code ----------\n");
    pthread_mutex_unlock(&print_mutex);
    lines_to_remove += 2;

    sleep(2); // stop 2 seconds to display the lines
    
    pthread_mutex_lock(&print_mutex);
    for ( count = 0; count < lines_to_remove; ++ count )
    {
        printf("\33[1A");    //move cursor up 1 line
    	printf("%c[2K", 27); //clear entire line
    }
    printf("\n"); // just to refresh the screen, otherwise would only refresh if have update from hardware input
    pthread_mutex_unlock(&print_mutex);
}


//print current frequency and amplitude on screen
void printCurrent() 
{
    // for loop counter
	int count = 0;

    // grabs latest values and release mutex
    pthread_mutex_lock(&global_var_mutex);
    local_amplitude = global_amplitude;
    local_frequency = global_frequency;
    local_waveform = waveform;
    pthread_mutex_unlock(&global_var_mutex);

    //if global var changed, then reprint current value
    if( abs(local_frequency-previous_local_frequency)>1 || abs(local_frequency-previous_local_frequency)>1 || !!local_waveform != !!previous_local_waveform )
    {
        if (local_frequency == 0)
            real_frequency = FREQUENCY_MIN;
        else
            real_frequency = local_frequency/255.0 * FREQUENCY_MAX;

        real_amplitude = local_amplitude/255.0 * AMPLITUDE_MAX;

        pthread_mutex_lock(&print_mutex);

    	for ( count = 0; count < 3; ++ count )
    	{
        	printf("\33[1A");    //move cursor up 1 line
    		printf("%c[2K", 27); //clear entire line
    	}

        printf("  Current frequency : %lf\n", real_frequency);
        printf("  Current amplitude : %lf\n", real_amplitude);
        printf("  Current waveform  : %s\n", local_waveform? "Square wave" : "Sine wave");

        pthread_mutex_unlock(&print_mutex);
        
        // update previous values
		previous_local_frequency = local_frequency;
		previous_local_amplitude = local_amplitude;
		previous_local_waveform = local_waveform;
    }
}


//output user's current param to file 
int outputFile(){
    //for output of time in file, and file path
    char *path = "./output.txt";
    time_t Time = time(NULL);
    struct tm tme = *localtime(&Time);
      
	FILE *fptr;
    fptr = fopen(path, "w");
    
	if( fptr == NULL )
	{
        printf("Error with writing! Invalid Path\n");
        return -1;
	}

    fprintf(fptr,"##Output Param at: %d-%d-%d %d:%d\n", tme.tm_year-100, tme.tm_mon+1, tme.tm_mday, tme.tm_hour, tme.tm_min);
    fprintf(fptr,"-Human readable form\n");
    fprintf(fptr,"Frequency: %lfHz\n", global_frequency*FREQUENCY_MAX/255.0);
    fprintf(fptr,"Amplitude: %lfV\n", global_amplitude*AMPLITUDE_MAX/255.0);
    fprintf(fptr,"Waveform: %s\n", waveform?"Square":"Sine");
    fprintf(fptr,"-For program, value in 8 bits\n");
    fprintf(fptr,"Scaled Frequency: %d\n", global_frequency);
    fprintf(fptr,"Scaled Amplitude: %d", global_amplitude);
	fclose(fptr);

    printf("Output Path is %s\n", path);
    printf("File saved!\n");

	return 0;
}