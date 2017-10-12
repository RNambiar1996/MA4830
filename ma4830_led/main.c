/*
  Author: Lee Ee Wei, Rahul Nambiar, Nicholas Adrian, Tan You Liang
  12 October 2017
  Program name: C Language Program for "Computing Trajectory of a Projectile"
  Code maintainer email: rahul010@e.ntu.edu.sg
  Compile line(on QNX): cc -o ca1 main.c printTrajectory.c -lm
  Execute: ./ca1
  
  Explanation for how we used the scanf functions:-
  
  How it is used:
    scanf("%[^\n]s", input); // input is the character array for string input
    flush_input();           // flush_input() is a function we made to flush the input
    
  The "[^\n]" in scanf is to change the delimiter, this is so that we can prevent such cases:
  e.g. When "a a a" is entered, the error message would be printed 3 times, because scanf accepts ' ', or space as a delimiter
  e.g. When "1 a a" is entered, "1" will be accepted, but 2 error messages would still be printed
  
  flush_input() function is to prevent scanf from accepting the '\n' from the previous input, if it does, error messages will be printed non-stop
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <unistd.h>
#include "printTrajectory.h"
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>

// defining the maximum and minimum angles for initial launch angle
#define MAX_ANGLE 90
#define MIN_ANGLE 0

#define DIO_PORTA     iobase[3] + 4       // Badr3 + 4
#define DIO_PORTB     iobase[3] + 5       // Badr3 + 5
#define DIO_PORTC     iobase[3] + 6       // Badr3 + 6
#define DIO_CTLREG    iobase[3] + 7       // Badr3 + 7

#define DEBUG  1

int badr[5];    // PCI 2.2 assigns 6 IO base addresses

// Parameter selection using bit operations
enum parameter_selection {
  ANGLE    = 0x01,
  VELOCITY = 0x02,
  HEIGHT   = 0x04,
};

// Projectile parameter structure
struct projectile
{
  float angle;
  float velocity;
  float height;
};

// Checks input string 
bool check_str_for_non_digit (char input[])
{
  int i;
  int check_return;
  bool dot = false; // whether dot has been found, only allow 1 dot
  for ( i = 0; i < strlen (input); ++i )
  {
    check_return = isdigit(input[i]);
    
    if (input[i] == ' ')
      return false;
    
    if ( !check_return && input[0] != '-' // check_return checks whether it is a valid number, if it is not, check whether it is a negative character, if negative character, ignore check_return
        || ( input[i] == '.' && dot )     // true if a second dot is found
        || input[0] == '.' )              // does not allow first character to be a dot
    {
      return false;
    }
    if ( input[i] == '.' && !dot ) // dot has been found
      dot = true;
  }
  return true;
}

// Checks input validity according to parameter selection, and saves it to "save_value"
bool check_input (char input[], float *save_value, uint8_t param)
{
  bool digit_check = false;
  float digit_buffer;
  
  digit_check = check_str_for_non_digit(input);
  
  if (!digit_check)
  {
    printf("Sorry, that is not a valid number. Please enter a valid number.\n");
    return false;
  }
  
  digit_buffer = strtod(input, NULL);
  
  if ( param == ANGLE && ( digit_buffer < MIN_ANGLE ||  digit_buffer > MAX_ANGLE ) ) // so that angle is within limit
  {
    printf("Please enter a value between %d and %d for angle.\n", MIN_ANGLE, MAX_ANGLE);
    return false;
  }
  else if ( param != ANGLE && digit_buffer < 0 ) // for positive velocity and height
  {
    printf("Please enter a value larger than 0.\n");
    return false;
  }
  else if ( param != ANGLE && digit_buffer >= FLT_MAX ) // for float data type upper limit
  {
    printf("Please enter a smaller value.\n");
    return false;
  }
  else if ( digit_buffer <= FLT_MIN ) // for float data type lower limit
  {
    printf("Please enter a larger value.\n");
    return false;
  }
  else if ( param == VELOCITY && pow(digit_buffer, 2) >= FLT_MAX ) // for velocity, float data type upper limit, because velocity will need to be squared
  {
    printf("Please enter a smaller value.\n");
    return false;
  }
  else if ( param == VELOCITY && pow(digit_buffer, 2) <= FLT_MIN ) // for velocity, float data type lower limit, because velocity will need to be squared
  {
    printf("Please enter a larger value.\n");
    return false;
  }
  
  *save_value = digit_buffer;
  return true;
}

void flush_input()
{
  char flush_ch;
  while ( (flush_ch = getchar()) != '\n' && flush_ch != EOF );
}

int main () {

  // Initializations
  struct pci_dev_info info;
  void *hdl;

  uintptr_t iobase[6];

  unsigned int i,count;
  unsigned short chan;

  int number_of_parameters;
  int str_length;
  
  bool success;
  uint8_t parameter_selection = 0;

  char input[100];
  float input_buffer; // for holding the floating number after being converted from string input

  const struct projectile proj_const = { 45.0, 100.0, 100.0 }; // For default parameter values
  struct projectile proj_initial = proj_const;

  float sqrtEq_main;
  float d_main;

  float guess;
  int tries;

  memset(&info,0,sizeof(info));
  if(pci_attach(0)<0) {
    perror("pci_attach");
    exit(EXIT_FAILURE);
  }
  
  // Vendor and Device ID
  info.VendorId=0x1307;
  info.DeviceId=0x01;

  //establishing pci bus connection
  if ((hdl=pci_attach_device(0, PCI_SHARE|PCI_INIT_ALL, 0, &info))==0) {
    perror("pci_attach_device");
    exit(EXIT_FAILURE);
  }
  
  //determining the different base addresses
  for(i=0;i<6;i++)
  {
    if(info.BaseAddressSize[i]>0)
    {
      printf("Aperture %d  Base 0x%x Length %d Type %s\n", i, 
      PCI_IS_MEM(info.CpuBaseAddress[i]) ?  (int)PCI_MEM_ADDR(info.CpuBaseAddress[i]) : 
      (int)PCI_IO_ADDR(info.CpuBaseAddress[i]),info.BaseAddressSize[i], 
      PCI_IS_MEM(info.CpuBaseAddress[i]) ? "MEM" : "IO");
    }
  }
  
  // Assign BADRn IO addresses for PCI-DAS1602
  if(DEBUG)
  {
    printf("\nDAS 1602 Base addresses:\n\n");
    for(i=0;i<5;i++)
    {
      badr[i]=PCI_IO_ADDR(info.CpuBaseAddress[i]);
      if(DEBUG)
        printf("Badr[%d] : %x\n", i, badr[i]);
    }

    printf("\nReconfirm Iobase:\n");  // map I/O base address to user space
    for(i=0;i<5;i++)                  // expect CpuBaseAddress to be the same as iobase for PC
    {
      iobase[i]=mmap_device_io(0x0f,badr[i]);
      printf("Index %d : Address : %x ", i,badr[i]);
      printf("IOBASE  : %x \n",iobase[i]);
    }
  }

  // Modify thread control privity
  if( ThreadCtl(_NTO_TCTL_IO,0) == -1 )
  {
    perror("Thread Control");
    exit(1);
  }

  //establishin PORTB as output
  out8(DIO_CTLREG, 0x90);
  //sending a signal to turn off all LED's connected on PORT B
  out8(DIO_PORTB, 0x00);
  
  // Clearing terminal screen
  system("clear");
  
  // Initialization statements
  printf("Hi! Welcome to the C Language Program for : \"Computing Trajectory of a Projectile\".\n\n");
  printf("This program will calculate the horizontal range (d) travelled by the projectile. To do so, it requires up to 3 input variable(s) of:\n");
  printf("(1) initial launch angle [theta]\n");
  printf("(2) initial launch velocity [v]\n");
  printf("(3) initial launch height [h]\n\n");

  printf("If you decide to  provide less than 3 input variables, the remaining variables will be fixed at the default values of:\n");

  printf("theta   = 45 degrees\n");
  printf("v       = 100 m/s\n");
  printf("h       = 100 m\n\n");
  
  // Prompting user to input number of parameters desired for input
  while (true)
  { 
    printf("Please enter the number of input variable(s) that you would like to enter [1/2/3]:\n");

    scanf("%[^\n]s", input);
    flush_input();

    // checking validity of input
    if ( (str_length = strlen (input)) == 1 && input[0] >= 49 && input[0] <= 51 )
      break;
    else
      printf("\nPlease enter either 1, 2, or 3. Thank you.\n");
  }
  
  // Recording selection
  number_of_parameters = input[0] - '0';

  if (number_of_parameters == 1) // If only 1 input parameter desired
  {
    printf("\nPlease enter the input variable that you would like to provide:\n");
    printf("[1] theta\n");
    printf("[2] v\n");
    printf("[3] h\n");
    printf("Please specify the variable that you would like to enter [1/2/3]:\n");

    // checks validity of selection
    while (true)
    {
      //flush_input();
      scanf("%[^\n]s", input);
      flush_input();

      // make sure selection is eiter 1, 2, or 3. 49 = '1', 50 = '2', 51 = '3'.
      if ( (str_length = strlen (input)) == 1 && input[0] >= 49 && input[0] <= 51 )
        break;
      else
        printf("\nPlease enter either 1, 2, or 3. Thank you.\n");
    }
    
    // clearing parameter selection variable
    parameter_selection = 0;
    
    // recording parameter selection
    if (input[0] == '1')
    {
      parameter_selection = parameter_selection | ANGLE;
    }
    else if (input[0] == '2')
    {
      parameter_selection = parameter_selection | VELOCITY;
    }
    else if (input[0] == '3')
    {
      parameter_selection = parameter_selection | HEIGHT;
    }
    
    printf("\nPlease enter the value for your desired parameter.\n");
    
    while (true)
    {
      printf("\nInitial ");
      if (parameter_selection == ANGLE)
        printf("angle (in degrees): \n");
      else if (parameter_selection == VELOCITY)
        printf("velocity (in meters per second): \n");
      else if (parameter_selection == HEIGHT)
        printf("height (in meters): \n");

      scanf("%[^\n]s", input);
      flush_input();

      // checks validity of selection, and records them if valid
      success = false;

      if (parameter_selection == ANGLE)
        success = check_input(input, &proj_initial.angle, ANGLE);
      else if (parameter_selection == VELOCITY)
        success = check_input(input, &proj_initial.velocity, VELOCITY);
      else if (parameter_selection == HEIGHT)
        success = check_input(input, &proj_initial.height, HEIGHT);

      if (!success)
        continue;
        
      break;
    }
  }
  else if (number_of_parameters == 2) // If only 2 input parameter desired
  {
    printf("\nPlease enter the input variables combination:\n");
    printf("[1] theta  and  v\n"); // 0x01 | 0x02 = 0x03
    printf("[2] v      and  h\n"); // 0x02 | 0x04 = 0x06
    printf("[3] theta  and  h\n"); // 0x01 | 0x04 = 0x05
    printf("Please specify the variables that you would like to enter [1/2/3]:\n");

    while (true)
    {
      scanf("%[^\n]s", input);
      flush_input();

      // make sure selection is eiter 1, 2, or 3. 49 = '1', 50 = '2', 51 = '3'.
      if ( (str_length = strlen (input)) == 1 && input[0] >= 49 && input[0] <= 51 )
        break;
      else
        printf("Please enter either 1, 2, or 3. Thank you.\n");
    }

    parameter_selection = 0; // clearing parameter selection
    
    // recording parameter selection
    if (input[0] == '1')
      parameter_selection = parameter_selection | ANGLE    | VELOCITY;
    else if (input[0] == '2')
      parameter_selection = parameter_selection | VELOCITY | HEIGHT;
    else if (input[0] == '3')
      parameter_selection = parameter_selection | ANGLE    | HEIGHT;

    printf("\nPlease enter the values for your desired parameters.\n");

    while (true)
    {
      if (parameter_selection == (ANGLE|VELOCITY) )
      {
        printf("Initial angle (in degrees): \n");
        
        scanf("%[^\n]s", input);
        flush_input();

        // checks validity of selection, and records them if valid
        success = false;
        success = check_input(input, &proj_initial.angle, ANGLE);

        if (!success)
          continue;
          
        while (true)
        {
          printf("Initial velocity (in meters per second): \n");
          
          scanf("%[^\n]s", input);
          flush_input();

          success = false;
          success = check_input(input, &proj_initial.velocity, VELOCITY);

          if (!success)
            continue;
          else
            break;
        }
      }
      else if (parameter_selection == (VELOCITY|HEIGHT) )
      {
        printf("Initial velocity (in meters per second): \n");
        
        scanf("%[^\n]s", input);
        flush_input();

        success = false;
        success = check_input(input, &proj_initial.velocity, VELOCITY);

        if (!success)
          continue;
          
        while (true)
        {
          printf("Initial height (in meters): \n");

          scanf("%[^\n]s", input);
          flush_input();

          success = false;
          success = check_input(input, &proj_initial.height, HEIGHT);

          if (!success)
            continue;
          else
            break;
        }
      }
      else if (parameter_selection == (ANGLE|HEIGHT) )
      {
        printf("Initial angle (in degrees): \n");
        
        scanf("%[^\n]s", input);
        flush_input();

        success = false;
        success = check_input(input, &proj_initial.angle, ANGLE);

        if (!success)
          continue;
          
        while (true)
        {
          printf("Initial height (in meters): \n");

          scanf("%[^\n]s", input);
          flush_input();

          success = false;
          success = check_input(input, &proj_initial.height, HEIGHT);

          if (!success)
            continue;
          else
            break;
        }
      }
      break;
    }
  }
  else if (number_of_parameters == 3) // If only 3 input parameter desired
  {
    printf("\n");
    while (true)
    {
      printf("theta (in degrees): \n");
      
      scanf("%[^\n]s", input);
      flush_input();

      // checks validity of selection, and records them if valid
      success = false;
      success = check_input(input, &proj_initial.angle, ANGLE);

      if (!success)
      continue;

      while (true)
      {
        printf("\nv (in meters per second): \n");
        
        scanf("%[^\n]s", input);
        flush_input();

        success = false;
        success = check_input(input, &proj_initial.velocity, VELOCITY);

        if (!success)
          continue;
          
        while (true)
        {
          printf("\nh (in meters): \n");

          scanf("%[^\n]s", input);
          flush_input();

          success = false;
          success = check_input(input, &proj_initial.height, HEIGHT);

          if (!success)
            continue;
          else
            break;
        }
        break;
      }
      break;
    }
  } // end of if(number_of_parameters == 3) condition

  sqrtEq_main = 1 + ( ( 2 * G_ACC * ( proj_initial.height ) ) / ( ( pow( sin( proj_initial.angle * PI/180 ), 2 ) * pow( proj_initial.velocity, 2 ) ) ) );
  d_main = ( pow( proj_initial.velocity, 2) / ( 2 * G_ACC ) ) * ( 1 +  sqrt( sqrtEq_main ) ) * sin( 2 * ( proj_initial.angle * PI/180 ) );
  
  printf("\nDo you want to play a game to guess the horizontal range (d) reached by the projectile?\n");

  //while loop to check for valid input from user
  while(true)
  {
    printf("\nPlease enter y or n\n");
    
    scanf("%[^\n]s", input);
    flush_input();

    if (input[0] == 'y' || input[0] == 'n')
      break;
    else
      printf("Invalid entry. Try again\n");
  }

  if (input[0] == 'y')
  {
    //initialising variable to be used to keep track of number of attempted tries by user
    tries=1;
    printf("\n\n\nYou have 4 chances to estimate the horizontal range (d) of the projectile\n");
    out8(DIO_PORTB, 0xff);

    while(true)
    {
      printf("\nChance no %d: \n\n", tries);

      scanf("%[^\n]s", input);
      flush_input();

      success = false;
      success = check_str_for_non_digit(input);

      //checking if entered number is valid
      if (!success)
      {
        printf("\n\nSorry, that is not a valid number. Please enter a valid number.\n\n");
        continue;
      }

      //converting string to number
      guess = strtod(input, NULL);

      //checking if guess is close to the actual distance travelled by the projectile
      if(abs(guess-d_main) < 0.01)
      {
        //turning on all LED's on PORTB to indicate success
        out8(DIO_PORTB, 0xff);
        printf("\n\n\nYou got it right. Congratulations!\n\n\n");
        break;
      }
      //if user enters incorrect number, incrementing tries variable to reflect incorrect attempt
      else
      {
        tries++;
        printf("\n\nWrong guess! No of tries so far: %d. You only have %d lives left\n", tries, (5-tries));
      }

      //switch statement to turn off one LED for each incorrect try
      switch((5-tries))
      {
        case 0:
          out8(DIO_PORTB, 0x00);
          break;
        case 1: 
          out8(DIO_PORTB, 0x08);
          break;
        case 2: 
          out8(DIO_PORTB, 0x0c);
          break;
        case 3:
          out8(DIO_PORTB, 0x0e);
          break;
        case 4:
          out8(DIO_PORTB, 0x0f);
          break;
      }

      //if user exhausts his 4 chances
      if(tries == 5)
      {
        printf("You have used up your 4 lives! Game over!\n\n\n");
        break;
      }
    }
  }

  //turning off all led's
  out8(DIO_PORTB, 0x00);

  //disconnecting from the pci bus
  pci_detach_device(hdl);

  //function to display the projectile motion of the object
  compute_trajectory(proj_initial.velocity, proj_initial.height, proj_initial.angle); 

  return 0;
}
