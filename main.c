/*
  Author: Lee Ee Wei
  Compile line(in QNX): cc -o prog prog.c -lncurses
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "printTrajectory.h"
#include <unistd.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>

//#include <stdarg.h>
//#include <ncurses.h>

	
#define MAX_ANGLE 90
#define MIN_ANGLE -90

#define	INTERRUPT		iobase[1] + 0		// Badr1 + 0 : also ADC register
#define	MUXCHAN		iobase[1] + 2				// Badr1 + 2
#define	TRIGGER		iobase[1] + 4				// Badr1 + 4
#define	AUTOCAL		iobase[1] + 6				// Badr1 + 6
#define 	DA_CTLREG		iobase[1] + 8				// Badr1 + 8

#define 	AD_DATA		iobase[2] + 0				// Badr2 + 0
#define 	AD_FIFOCLR		iobase[2] + 2				// Badr2 + 2

#define	TIMER0			iobase[3] + 0				// Badr3 + 0
#define	TIMER1			iobase[3] + 1				// Badr3 + 1
#define	TIMER2			iobase[3] + 2				// Badr3 + 2
#define	COUNTCTL		iobase[3] + 3				// Badr3 + 3
#define	DIO_PORTA		iobase[3] + 4				// Badr3 + 4
#define	DIO_PORTB		iobase[3] + 5				// Badr3 + 5
#define	DIO_PORTC		iobase[3] + 6				// Badr3 + 6
#define	DIO_CTLREG		iobase[3] + 7				// Badr3 + 7
#define	PACER1			iobase[3] + 8				// Badr3 + 8
#define	PACER2			iobase[3] + 9				// Badr3 + 9
#define	PACER3			iobase[3] + a				// Badr3 + a
#define	PACERCTL		iobase[3] + b				// Badr3 + b

#define 	DA_Data		iobase[4] + 0				// Badr4 + 0
#define 	DA_FIFOCLR		iobase[4] + 2				// Badr4 + 2

#define	DEBUG						1
 	
int badr[5];			// PCI 2.2 assigns 6 IO base addresses

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


void get_values()
{

}
int main () {


	struct pci_dev_info info;
	void *hdl;

	uintptr_t iobase[6];
	uintptr_t dio_in;
	uint16_t adc_in;
	
	unsigned int i,count;
	unsigned short chan;

	// Initializations
	int number_of_parameters;
	int str_length;
	
	bool success;
	uint8_t parameter_selection = 0;

	char input[100];
	double input_buffer;

	const struct projectile proj_const = { 45.0, 10.0, 100.0 };
	struct projectile proj_initial = proj_const;

	float sqrtEq_main;
	float d_main;

	float guess;
	int tries;

	printf("\fDemonstration Routine for PCI-DAS 1602\n\n");

	memset(&info,0,sizeof(info));
	if(pci_attach(0)<0) {
		perror("pci_attach");
		exit(EXIT_FAILURE);
	}
		                                                     /* /*Vendor and Device ID */
	info.VendorId=0x1307;
	info.DeviceId=0x01;

	if ((hdl=pci_attach_device(0, PCI_SHARE|PCI_INIT_ALL, 0, &info))==0) {
		perror("pci_attach_device");
		exit(EXIT_FAILURE);
	}
	  
	for(i=0;i<6;i++) {		// Another printf BUG ? - Break printf to two statements
		if(info.BaseAddressSize[i]>0) {
			printf("Aperture %d  Base 0x%x Length %d Type %s\n", i, 
			PCI_IS_MEM(info.CpuBaseAddress[i]) ?  (int)PCI_MEM_ADDR(info.CpuBaseAddress[i]) : 
			(int)PCI_IO_ADDR(info.CpuBaseAddress[i]),info.BaseAddressSize[i], 
			PCI_IS_MEM(info.CpuBaseAddress[i]) ? "MEM" : "IO");
		}
	}  
	    													
	printf("IRQ %d\n",info.Irq); 		
						// Assign BADRn IO addresses for PCI-DAS1602			
	if(DEBUG) {
		printf("\nDAS 1602 Base addresses:\n\n");
		for(i=0;i<5;i++) {
		badr[i]=PCI_IO_ADDR(info.CpuBaseAddress[i]);
		if(DEBUG) printf("Badr[%d] : %x\n", i, badr[i]);
	}
	 
	printf("\nReconfirm Iobase:\n");  	// map I/O base address to user space						
		for(i=0;i<5;i++) {			// expect CpuBaseAddress to be the same as iobase for PC
			iobase[i]=mmap_device_io(0x0f,badr[i]);	
			printf("Index %d : Address : %x ", i,badr[i]);
			printf("IOBASE  : %x \n",iobase[i]);
		}													
	}
																			// Modify thread control privity
	if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
		perror("Thread Control");
		exit(1);
	}

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

		if ( (str_length = strlen (input)) == 1 && input[0] >= 49 && input[0] <= 51 )
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

			if ( (str_length = strlen (input)) == 1 && input[0] >= 49 && input[0] <= 51 )
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

			if ( (str_length = strlen (input)) == 1 && input[0] >= 49 && input[0] <= 51 )
			{
				break;
			}
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

	sqrtEq_main = 1 +  ((2*G_ACC*(h))/((pow(sin(theta),2)*pow(v,2))));
	d_main = (pow(v,2)/(2*G_ACC))  *  (1 +  sqrt(sqrtEq) )  *  sin(2*theta);

	out8(DIO_CTLREG, 0x90);

	while(true)
	{
		printf("You have 4 chances to estimate the landing point of the projectile\n");
		printf("Chance No: %d: ", tries);

		scanf("%f", &guess);
		
		if(abs(guess-d_main) < 0.001)
		{
			printf("You got it right!!!Congratulations\n");
			break;
		}
		else if(tries > 4)
		{
			printf("You have used up your 4 lives\n");
			break;
		}
		else
		{
			lives--;
			printf("No of tries so far: %d. You only have %d lives left\n", tries, (4-tries));
			
		}

		switch((4-tries))
		{
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
	}

	printf("This is how the projectile will move!!\n");
	compute_trajectory(proj_initial.velocity, proj_initial.height, proj_initial.angle); 

	return 0;
}
