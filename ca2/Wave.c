#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <math.h>
#include <time.h>		
		
#define PI 3.14159
																
#define	INTERRUPT		iobase[1] + 0				// Badr1 + 0 : also ADC register
#define	MUXCHAN			iobase[1] + 2				// Badr1 + 2
#define	TRIGGER			iobase[1] + 4				// Badr1 + 4
#define	AUTOCAL			iobase[1] + 6				// Badr1 + 6
#define 	DA_CTLREG		iobase[1] + 8				// Badr1 + 8

#define	 AD_DATA			iobase[2] + 0				// Badr2 + 0
#define	 AD_FIFOCLR		iobase[2] + 2				// Badr2 + 2

#define	TIMER0				iobase[3] + 0				// Badr3 + 0
#define	TIMER1				iobase[3] + 1				// Badr3 + 1
#define	TIMER2				iobase[3] + 2				// Badr3 + 2
#define	COUNTCTL			iobase[3] + 3				// Badr3 + 3
#define	DIO_PORTA		iobase[3] + 4				// Badr3 + 4
#define	DIO_PORTB		iobase[3] + 5				// Badr3 + 5
#define	DIO_PORTC		iobase[3] + 6				// Badr3 + 6
#define	DIO_CTLREG		iobase[3] + 7				// Badr3 + 7
#define	PACER1				iobase[3] + 8				// Badr3 + 8
#define	PACER2				iobase[3] + 9				// Badr3 + 9
#define	PACER3				iobase[3] + a				// Badr3 + a
#define	PACERCTL			iobase[3] + b				// Badr3 + b

#define 	DA_Data			iobase[4] + 0				// Badr4 + 0
#define	DA_FIFOCLR		iobase[4] + 2				// Badr4 + 2
	
#define MAX_AMP            100
#define MAX_FREQ          5000

int badr[5];															// PCI 2.2 assigns 6 IO base addresses

int generateWave(float frequency, float amplitude, float offset, int resolution, char* waveType)
{

struct pci_dev_info info;
void *hdl;

uintptr_t iobase[6];
uintptr_t dio_in;
uint16_t adc_in;
	
unsigned int i,count;
unsigned short chan;

unsigned int data[1000];
float delta,dummy;

float slope = 0.0;

float tmp;
float T, delta_t;

memset(&info,0,sizeof(info));
if(pci_attach(0)<0) {
  perror("pci_attach");
  exit(EXIT_FAILURE);
  }
  																		/* Vendor and Device ID */
info.VendorId=0x1307;
info.DeviceId=0x01;

if ((hdl=pci_attach_device(0, PCI_SHARE|PCI_INIT_ALL, 0, &info))==0) {
  perror("pci_attach_device");
  exit(EXIT_FAILURE);
  }

// Determine assigned BADRn IO addresses for PCI-DAS1602			

for(i=0;i<5;i++) {
  badr[i]=PCI_IO_ADDR(info.CpuBaseAddress[i]);
  }
 
for(i=0;i<5;i++) {												// expect CpuBaseAddress to be the same as iobase for PC
  iobase[i]=mmap_device_io(0x0f,badr[i]);	
  }													
																		// Modify thread control privity
if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
  perror("Thread Control");
  exit(1);
  }																											
	
//find waveform points
delta = (2.0*PI)/((float)resolution);

printf("%d\n", delta);

if (waveType[0] == 's' && waveType[1] == 'i')
{
	printf("Sine Wave\n");
	for(i=0; i<resolution;i++)
	{
		dummy= (amplitude*sinf((float)(i*delta))+amplitude);
		dummy = ((dummy)/(2.0*MAX_AMP))*0xffff;
		printf("Dummy: %f\n", dummy);
  		data[i]= (unsigned) dummy;	
	}		

}
else if(waveType[0] == 's' && waveType[1] == 'q')
{
	printf("Square wave\n");
	for(i=0; i<resolution; i++)
	{	
		dummy= amplitude*sinf((float)(i*delta));
		printf("Dummy: %f\n", dummy);
		
		if (dummy > 0.0)
		{
			dummy = (amplitude/MAX_AMP)*0xffff;
		}
		else 
		{
			dummy = 0.0;
		}
		
		data[i] = (unsigned)dummy;
	}
}

else if(waveType[0]=='t')
{
	printf("Triangular Wave\n");
		
	slope = 2.0*amplitude/PI;
	
	for(i=0;i<resolution;i++)
	{
		if(i<=(resolution/4))
		{
			printf("1st seg\n");
			dummy = slope*((float)(i*delta));
		}
		else if((i>(resolution/4))&&(i<=((3*resolution)/4)))
		{
			printf("2nd seg\n");
			dummy = -slope*((float)(i*delta))+2*amplitude;
		}	
		else
		{
			printf("3rd seg\n");
			dummy = slope*((float)(i*delta))-4*amplitude;
		}
		
		dummy = dummy + amplitude;
		printf("dummy: %f\n", dummy);
		
		dummy = ((dummy)/(2.0*MAX_AMP))*0xffff;
		
		data[i] = (unsigned)dummy;
		
		printf("Data: %d\n", data[i]);
	}	
}

//*********************************************************************************************
// Output wave
//*********************************************************************************************

printf("F: %f\n", frequency);
T = 1.0/frequency;
T = T*1000.00;
printf("T: %f\n", T);
delta_t = (T)/((float)resolution);
printf("delta_t: %f\n", delta_t);

while(1) {
for(i=0;i<resolution;i++) {
	out16(DA_CTLREG,0x0523);			// DA Enable, #0, #1, SW 5V unipolar		2/6
   	out16(DA_FIFOCLR, 0);							// Clear DA FIFO  buffer
    out16(DA_Data,(short) data[i]);																												
  	delay(delta_t);																													
  	}
}

// Reset DAC to 5V
out16(DA_CTLREG,(short)0x0023);	
out16(DA_FIFOCLR,(short) 0);			
out16(DA_Data, 0x8fff);						// Mid range - Unipolar																											 		
																																						
printf("\n\nExit Demo Program\n");
pci_detach_device(hdl);

return 0;

}

int main(int argc, char* argv)
{
	float frequency = 10.0;
	float amplitude = 100.0;
	int resolution = 40;
	float offset = 1.0;
	
	if(frequency>MAX_FREQ)
	{
		frequency = MAX_FREQ;
	}
	else if(frequency<0.1)
	{
		frequency = 0.1;
	}
	
	generateWave(frequency, amplitude, offset, resolution, "si");
	
	return 0;
	
}