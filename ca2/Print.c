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

void printInit(){
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
    printf("       a. pause switch (toggling it will pause the hardware input, and prompt user to save, quit or continue.)\n");
    printf("       b. waveform     (change between sine wave and square wave)\n");
    printf("    -Analog input(from left to right):\n");
    printf("       a. D/A 0        (changes frequency of wave)\n");
    printf("       b. D/A 1        (changes amplitude of wave)\n");
    printf("  -Other information:\n");
    printf("    -The program outputs to A/D 0 port.\n");
    printf("    -Number of LED lit up shows the amplitude level.\n");
    printf("    -The program will update the terminal screen with the latest values of frequency, and amplitude.\n");
    printf("    -Toggle switch 'a' will not be available if parameters are loaded from a file.\n");
    printf("    -After \"ctrl+c\" is detected, the program will shutdown all threads systematically.\n\n");
    printf("\n\n\n"); // for printSave(), so that the current values will remain even after pause
}

void printSave(){ // display save instructions and current value info
    char input[8];

	int count, lines_to_remove = 0;

    // save instructions/info
    pthread_mutex_lock( &print_mutex );
    printf("\nEnter 's' to save, 'q' to quit, other enter to continue\n");
    pthread_mutex_unlock( &print_mutex );
    
    ++lines_to_remove;
    
    scanf("%[^\n]s", input);
    ++lines_to_remove; // scanf takes 1 line too
    flush_input();
    
    pthread_mutex_lock( &print_mutex );
    printf("\33[1A");    //move cursor up 1 line
    printf("%c[2K", 27); //clear entire line
    pthread_mutex_unlock( &print_mutex );
    
    if ( !strcmp(input, "q") || !strcmp(input, "Q") )
    {
        pthread_mutex_lock( &print_mutex );
        printf("Shutting down program!\n");
        pthread_mutex_unlock( &print_mutex );
        system_shutdown();
    }
    else if( !strcmp(input, "s") || !strcmp(input, "S") )
    {
        pthread_mutex_lock( &print_mutex );
        printf("Saving parameters!\n");
        pthread_mutex_unlock( &print_mutex );

        if ( !outputFile() )
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
    pthread_cond_signal(&info_switch_cond);
    pthread_mutex_unlock( &global_stop_mutex );
    
    pthread_mutex_lock(&print_mutex);
    printf("----------  Resuming The G-code ----------\n");
    pthread_mutex_unlock(&print_mutex);
    lines_to_remove += 2;

    sleep(2); // stop 2 seconds to display the lines
    
    pthread_mutex_lock(&print_mutex);
    for ( count = 0; count < lines_to_remove; ++ count )
    {
        printf("\33[1A");     //move cursor up 1 line
    	printf("%c[2K", 27); //clear entire line
    }
    printf("\n"); // just to refresh the screen, otherwise would only refresh if have update from potentiometer
    pthread_mutex_unlock(&print_mutex);
}

void printCurrent()
{
	int count = 0;

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
        //printf("\n\n\n\n\n");

    	for ( count = 0; count < 3; ++ count )
    	{
        	printf("\33[1A");     //move cursor up 1 line
    		printf("%c[2K", 27); //clear entire line
    	}

        printf("  Current frequency : %lf\n", real_frequency);
        printf("  Current amplitude : %lf\n", real_amplitude);
        printf("  Current waveform  : %s\n", local_waveform? "Square wave" : "Sine wave");

        //printf("  real frequency: %d\n", local_frequency);
        //printf("  real amplitude: %d\n", local_amplitude);

        pthread_mutex_unlock(&print_mutex);
        
		previous_local_frequency = local_frequency;
		previous_local_amplitude = local_amplitude;
		previous_local_waveform = local_waveform;
    }
}

#/** PhEDIT attribute block
#-11:16777215
#0:2984:default:-3:-3:0
#2984:3003:TextFont9:0:-1:0
#3003:3004:FixedFont9:0:-1:0
#3004:3115:TextFont9:0:-1:0
#3115:4505:default:-3:-3:0
#4505:4524:TextFont9:0:-1:0
#4524:4526:FixedFont9:0:-1:0
#4526:4754:TextFont9:0:-1:0
#4754:5470:default:-3:-3:0
#5470:5471:FixedFont9:-3:-3:0
#5471:5577:TextFont9:-3:-3:0
#5577:5596:TextFont9:0:-1:0
#5596:5598:FixedFont9:0:-1:0
#5598:5676:TextFont9:0:-1:0
#5676:5812:TextFont9:-3:-3:0
#5812:5894:TextFont9:0:-1:0
#5894:6017:TextFont9:-3:-3:0
#6017:6216:default:-3:-3:0
#**  PhEDIT attribute block ends (-0000594)**/
