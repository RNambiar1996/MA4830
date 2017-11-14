#include "Wave.h"

int generateWave(int resolution, char* waveType)
{

	float delta, T, delta_t, dummy, slope;
	unsigned int i;
	unsigned int data[1000];
	struct timespec t_delta;
		
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
	t_delta.tv_nsec = 1000;
	printf("delta_t: %f\n", delta_t);
	printf("nsec: %ld\n", t_delta.tv_nsec);

	while(1) {
	for(i=0;i<resolution;i++) {
		out16(DA_CTLREG,0x0a23);			
	   	out16(DA_FIFOCLR, 0);							
	    out16(DA_Data,(short) data[i]);																												
	  	nanosleep(&t_delta, NULL);																												
	  	}
	}

	// Reset DAC to 5V
	out16(DA_CTLREG,(short)0x0a23);	
	out16(DA_FIFOCLR,(short) 0);			
	out16(DA_Data, 0x8fff);
																																																																	
	printf("\n\nExit Demo Program\n");
	pci_detach_device(hdl);

	return 0;

}

int main(int argc, char* argv)
{
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
	 
	for(i=0;i<5;i++) {												
	  iobase[i]=mmap_device_io(0x0f,badr[i]);	
	  }													
																			
	if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
	  perror("Thread Control");
	  exit(1);
	  }																											
	
	frequency = 10.0;
	amplitude = 100.0;
	
	generateWave(200, "si");

}


