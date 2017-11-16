#include "Global.h"
#include "System.h"

double local_frequency;
double local_amplitude;
double previous_local_frequency;
double previous_local_amplitude;


void printInit(){
    printf("---------- Welcome! This program outputs waveform to the oscilloscope. ----------\n\n");
    printf("      __________                                                  _ _                   \n");
    printf("    /           \\                                               |   |                   \n");
    printf("   /     ________|                                              |   |                     \n");
    printf("  |     /                            ________    _____       __/    |   _________            \n");
    printf("  |    |      ____    _________     /   _____|  /  __  \\   /  __    |  /   ____  \\             \n");
    printf("  |    |    /_    \\  |         |   |   /       |  |  |  |  |  |  |  |  |  |___|  |                 \n");
    printf("  |    \\______|   |  |_________|   |   |       |  |  |  |  |  |  |  |  |   ______/               \n");
    printf("  \\               |                |   \\_____  |   --   |  |   \\/   |  \\  \\______           \n");
    printf("    \\____________/                  \\________|  \\______/   \\_______/    \\________|          \n\n");
    printf("  -Instructions:\n");
    printf("    -Toggle switches(from left to right):\n");
    printf("       a. info switch      (toggling it will display instructions)\n");
    printf("       b. waveform         (change between sine wave and square wave)\n");
    printf("       c. frequency/offset (to choose the parameter for analog input a)\n");
    printf("    -Analog input(from left to right):\n");
    printf("       a. D/A 0            (changes frequency/offset depending on toggle switch c)\n");
    printf("       b. D/A 1            (changes amplitude of wave)\n");
    printf("    -The program outputs to A/D 0 port.\n");
    printf("    -Number of LED lit up shows the amplitude level.\n");
    printf("    -The program will update the terminal screen with the latest values of frequency, amplitude, and offset.\n");
    printf("    -Instructions to save the parameters will be displayed after \"ctrl+c\" is entered.\n\n");
    printf("    -If you would like to see the instructions again and/or save the parameter, please enter \"ctrl+c\"\n");
    printf("    -After \"ctrl+c\" is detected, the hardware will stop updating the values.\n\n");
}

void printSave(){ // display save instructions and current value info
    char input[8];
    char *outputPath = "./output.txt";
    
    // current values, no need mutex, system_pause == true will stop writing of these
    printf("  Current frequency: %lf\n", global_frequency);
    printf("  Current amplitude: %lf\n", global_amplitude);
    // save instructions/info
    printf("Enter 's' to save, 'q' to quit!, other inputs to continue\n");
    scanf("%[^\n]s", input);
    if ( !strcmp(input, "q") || !strcmp(input, "Q") )
    {
        printf("Shutting down program!\n");
        system_shutdown();
    }
    else if( !strcmp(input, "s") || !strcmp(input, "S") )
    {
        printf("Saving param!\n");
        if ( !outputFile(outputPath) )
        {
            printf("OUTPUT PARAM FAILED!!\n");
        }
    }
    else
    {
        printf("Continuing program....\n");
    }
    pthread_mutex_lock( &global_stop_mutex );
    system_pause = false;
    pthread_mutex_unlock( &global_stop_mutex );
    printf("----------  Resuming The G Code ----------\n");
    sleep(1);//delay(1000); // stop 1 second to display the previous line's printf()
}

void printCurrent(){


    pthread_mutex_lock(&global_var_mutex);
    local_amplitude = global_amplitude;
    local_frequency = global_frequency;
    pthread_mutex_unlock(&global_var_mutex);

    if(abs(local_frequency-previous_local_frequency)>1e-6 || abs(local_frequency-previous_local_frequency)>1e-6){ //global var changed
        pthread_mutex_lock(&print_mutex);
        printf("\33[1A"); //move cursor up
        printf("%c[1K", 27); //clear line
        printf("\33[1A"); //move cursor up
        printf("%c[1K", 27); //clear line
        printf("  Current frequency: %lf\n", local_frequency);
        printf("  Current amplitude: %lf\n", local_amplitude);
        pthread_mutex_unlock(&print_mutex);
    }
}