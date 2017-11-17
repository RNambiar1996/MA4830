#include "Global.h"
#include "System.h"

// local relative to this source code
uint8_t local_frequency;
uint8_t local_amplitude;
uint8_t previous_local_frequency;
uint8_t previous_local_amplitude;

// to store value after conversion
double real_frequency;
double real_amplitude;

void printInit(){
	//printf("\33[2J"); // clears screen
    printf("---------- Welcome to the G-code. This program outputs waveform to the oscilloscope. ----------\n");
    printf("       __________                                                  _ _  \n");
    printf("     /           \\                                               |   | \n");
    printf("    /     ________|                                              |   |  \n");
    printf("   |     /                            ________    _____       __/    |  \n");
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
}

void printSave(){ // display save instructions and current value info
    char input[8];
    	//printf("f");
    //flush_input();
    	//printf("g");
    // save instructions/info
    pthread_mutex_lock( &print_mutex );
    	//printf("h");
    printf("Enter 's' to save, 'q' to quit, other enter to continue\n");
    pthread_mutex_unlock( &print_mutex );
    scanf("%[^\n]s", input);
    flush_input();
    
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
        }

    }
    else
    {
        pthread_mutex_lock( &print_mutex );
        printf("Continuing program...\n");
        pthread_mutex_unlock( &print_mutex );
    }

    pthread_mutex_lock( &global_stop_mutex );
    system_pause = false;
    info_switch = false;
    pthread_cond_signal(&info_switch_cond);
    pthread_mutex_unlock( &global_stop_mutex );
    
    pthread_mutex_lock(&print_mutex);
    printf("----------  Resuming The G Code ----------\n");
    pthread_mutex_unlock(&print_mutex);

    sleep(1);//delay(1000); // stop 1 second to display the previous printf()
}

void printCurrent()
{
    pthread_mutex_lock(&global_var_mutex);
    local_amplitude = global_amplitude;
    local_frequency = global_frequency;
    pthread_mutex_unlock(&global_var_mutex);

    //if global var changed, then reprint current value
    if( abs(local_frequency-previous_local_frequency)>1 || abs(local_frequency-previous_local_frequency)>1 )
    {
        if (local_frequency == 0)
            real_frequency = FREQUENCY_MIN;
        else if (local_frequency == 255)
            real_frequency = FREQUENCY_MAX;
        else
            real_frequency = local_frequency/255.0 * FREQUENCY_MAX;

        real_amplitude = local_amplitude/255.0 * AMPLITUDE_MAX;

        pthread_mutex_lock(&print_mutex);
        //printf("\n\n\n\n\n");

        printf("  Current frequency: %lf\n", real_frequency);
        printf("  Current amplitude: %lf\n", real_amplitude);

        printf("  real frequency: %d\n", local_frequency);
        printf("  real amplitude: %d\n", local_amplitude);

        printf("\33[1A");    //move cursor up 1 line
        printf("%c[2K", 27); //clear entire line
        printf("\33[1A");    //move cursor up 1 line
        printf("%c[2K", 27); //clear entire line
        
        printf("\33[1A");    //move cursor up 1 line
        printf("%c[2K", 27); //clear entire line
        printf("\33[1A");    //move cursor up 1 line
        printf("%c[2K", 27); //clear entire line

        pthread_mutex_unlock(&print_mutex);
    }
}

#/** PhEDIT attribute block
#-11:16777215
#0:4553:default:-3:-3:0
#4553:4554:FixedFont9:-3:-3:0
#4554:4871:TextFont9:-3:-3:0
#4871:4872:FixedFont9:-3:-3:0
#4872:5343:default:-3:-3:0
#**  PhEDIT attribute block ends (-0000229)**/
