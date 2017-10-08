/*
  Author: Lee Ee Wei
  Compile line(in QNX): cc -o prog prog.c -lncurses
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
//#include <stdarg.h>
//#include <ncurses.h>

#define G_ACC -9.81 // gravitational acceleration
#define PI 3.14159265359	
#define MAX_ANGLE 90
#define MIN_ANGLE -90

enum parameter_selection {
  ANGLE = 1,
  VELOCITY = 2,
  HEIGHT = 4,
};

char clear_buffer_ch;

struct projectile
{
  double angle;    // horizontal distance
  double velocity; // vertical distance
  double height;
};

void flush()
{
  while ((clear_buffer_ch = getchar()) != '\n' && clear_buffer_ch != EOF);
}

bool check_str_for_non_digit (char str[])
{
  int i;
  int check_return;
  for ( i = 0; i < strlen (str); ++i )
  {
    check_return = isdigit(str[i]);
    if ( !check_return && str[i] != '.' && str[i] != '-')
      return false;
  }
  return true;
}

bool check_input (char str[], double *save_value, uint8_t param)
{
  bool digit_check = false;
  double digit_buffer;
  
  digit_check = check_str_for_non_digit(str);
  
  if (!digit_check)
  {
    printf("Sorry, that is not a valid number. Please enter a valid number.\n");
    return false;
  }
  
  digit_buffer = strtod(str, NULL);
  
  if ( param == ANGLE && ( digit_buffer < MIN_ANGLE ||  digit_buffer > MAX_ANGLE ) )
  {
    printf("Please enter a value between %d and %d for angle.\n", MIN_ANGLE, MAX_ANGLE);
    return false;
  }
  else if ( param != ANGLE && digit_buffer < 0 ) // for positive velocity and height
  {
    printf("Please enter a value larger than 0.\n");
    return false;
  }
  
  *save_value = digit_buffer;
  return true;
}

int main () {

  // Initializations
  int max_screen_height, screen_width; // TODO, remove if don't need
  int number_of_parameters;
  int str_length;
  int i; // for loop variable
  bool exit_flag = false; // TODO, remove if don't need
  bool success;
  uint8_t parameter_selection = 0;
  
  char input[100];
  double input_buffer;
  
  const struct projectile proj_const = { 45.0, 10.0, 100.0 };
  struct projectile proj_initial = proj_const;

  system("clear");
  
  printf("Hi! Welcome to a C Language Program to : \"Compute the Trajectory of A Projectile\".\n\n");
  printf("In order to compute the trajectory of a projectile, we would need 3 parameters. They are:-\n");
  printf("\t- Initial launch angle\n");
  printf("\t- Initial launch velocity\n");
  printf("\t- Initial launch height\n\n");
  
  printf("You can input either 1, 2, or 3 parameters. For options other than 3, the other parameter(s) will be kept constant.\n");
  
  while (true)
  {
    printf("How many parameters would you like to input?\n");
  
    scanf("%s", input);
    
    if ( str_length = strlen (input) == 1 && input[0] >= 49 && input[0] <= 51 )
      break;
    else
      printf("\nPlease enter either 1, 2, or 3. Thank you.\n");
  }
  
  number_of_parameters = input[0] - '0';
  
  if (number_of_parameters == 1)
  {
    printf("\nWhich parameter would you like to input?\n");
    printf("\t1. Initial launch angle\n");
    printf("\t2. Initial launch velocity\n");
    printf("\t3. Initial launch height\n");
    printf("Please enter either 1, 2, or 3. Thank you.\n");
    
    while (true)
    {
      scanf("%s", input);
    
      if ( str_length = strlen (input) == 1 && input[0] >= 49 && input[0] <= 51 )
        break;
      else
        printf("\nPlease enter either 1, 2, or 3. Thank you.\n");
    }
    
    parameter_selection = 0;
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
      printf("Initial ");
      if (parameter_selection == ANGLE)
        printf("angle (in degrees): ");
      else if (parameter_selection == VELOCITY)
        printf("velocity (in meters per second): ");
      else if (parameter_selection == HEIGHT)
        printf("height (in meters): ");
        
      scanf("%s", input);
      
      success = false;
      success = check_str_for_non_digit(input);
      
      if (!success)
      {
        printf("Sorry, that is not a valid number. Please enter a valid number.\n");
        continue;
      }
      
      input_buffer = strtod(input, NULL);
      
      if (parameter_selection == ANGLE)
      {
        if ( input_buffer < MIN_ANGLE ||  input_buffer > MAX_ANGLE )
        {
          printf("Please enter a value between %d and %d for angle.\n", MIN_ANGLE, MAX_ANGLE);
          continue;
        }
        proj_initial.angle = input_buffer;
      }
      else if (parameter_selection == VELOCITY)
      {
        if ( input_buffer < 0 )
        {
          printf("Please enter a value larger than 0.\n");
          continue;
        }
        proj_initial.velocity = input_buffer;
      }
      else if (parameter_selection == HEIGHT)
      {
        if ( input_buffer < 0 )
        {
          printf("Please enter a value larger than 0.\n");
          continue;
        }
        proj_initial.height = input_buffer;
      }
      break;
    }
  }
  else if (number_of_parameters == 2)
  {
    printf("\nWhich parameters would you like to input?\n");
    printf("\t1. Initial launch angle     and  initial launch velocity\n"); // 12 3
    printf("\t2. Initial launch velocity  and  initial launch height\n");   // 24 6
    printf("\t3. Initial launch angle     and  initial launch height\n");    // 14 5
    printf("Please enter either 1, 2, or 3. Thank you.\n");
    
    while (true)
    {
      scanf("%s", input);
    
      if ( str_length = strlen (input) == 1 && input[0] >= 49 && input[0] <= 51 )
        break;
      else
        printf("Please enter either 1, 2, or 3. Thank you.\n");
    }
    
    parameter_selection = 0;
    if (input[0] == '1')
      parameter_selection = parameter_selection | ANGLE    | VELOCITY;
    else if (input[0] == '2')
      parameter_selection = parameter_selection | VELOCITY | HEIGHT;
    else if (input[0] == '3')
      parameter_selection = parameter_selection | ANGLE    | HEIGHT;
      
    printf("\nPlease enter the values for your desired parameters.\n");
    //printf("%d %d", parameter_selection, ANGLE    | VELOCITY);
    
    while (true)
    {
      if (parameter_selection == (ANGLE|VELOCITY) )
      {
        printf("Initial angle (in degrees): ");
        scanf("%s", input);
        
        success = false;
        success = check_input(input, &proj_initial.angle, ANGLE);
        
        if (!success)
          continue;
          
        while (true)
        {
          printf("Initial velocity (in meters per second): ");
          scanf("%s", input);
          
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
        printf("Initial velocity (in meters per second): ");
        scanf("%s", input);
        
        success = false;
        success = check_input(input, &proj_initial.velocity, VELOCITY);
        
        if (!success)
          continue;
          
        while (true)
        {
          printf("Initial height (in meters): ");
          
          scanf("%s", input);
          
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
        printf("Initial angle (in degrees): ");
        scanf("%s", input);
        
        success = false;
        success = check_input(input, &proj_initial.angle, ANGLE);
        
        if (!success)
          continue;
          
        while (true)
        {
          printf("Initial height (in meters): ");
          
          scanf("%s", input);
          
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
  else if (number_of_parameters == 3)
  {
    printf("\n");
    while (true)
    {
      printf("Initial angle (in degrees): ");
      scanf("%s", input);
      
      success = false;
      success = check_input(input, &proj_initial.angle, ANGLE);
      
      if (!success)
        continue;
      
      while (true)
      {
        printf("Initial velocity (in meters per second): ");
        scanf("%s", input);
        
        success = false;
        success = check_input(input, &proj_initial.velocity, VELOCITY);
        
        if (!success)
          continue;
          
        while (true)
        {
          printf("Initial height (in meters): ");
          
          scanf("%s", input);
          
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
  }
  
  printf("\nEND\n%lf %lf %lf\n", proj_initial.angle, proj_initial.velocity, proj_initial.height);
  
  return 0;
}
