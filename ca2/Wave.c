#include "hardware.h"

void generateWave(int resolution)
{

	float delta, T, delta_t, dummy, slope, frequency, frequency_old, offset, amplitude, amplitude_old, offset, offset_old;
	unsigned int i, delta_frequency, delta_amplitude, delta_offset;
	unsigned int data[1000];
	struct timespec t_delta;
	bool waveType, latest_waveType;
	bool die = 0;

	pthread_sigmask(SIG_SIG_SETMASK,&signal_mask,NULL);
	
	//find waveform points
	delta = (2.0*PI)/((float)resolution);
	printf("%d\n", delta);

	//locking the mutex
	pthread_mutex_lock(&global_var_mutex);
	frequency = global_frequency/255.0*10.0;
	
	if frequency < 0.1:
		frequency = 0.1

	frequency_old = frequency;

	amplitude = global_amplitude;
	amplitude_old = amplitude;

	offset = global_offset/255.0*2.0;
	offset_old = offset;

	waveType = waveForm
	latest_waveType = waveForm;
	//unlocking the mutex
	pthread_mutex_unlock(&global_var_mutex);

	while (1)
	{
		if (waveType == 0)
		{
			printf("Sine Wave\n");
			for(i=0; i<resolution;i++)
			{
				dummy= (global_amplitude*sinf((float)(i*delta))+global_amplitude);
				dummy = ((dummy)/(2.0*MAX_AMP) + offset)*0xffff;
				printf("Dummy: %f\n", dummy);
		  		data[i]= (unsigned) dummy;	
			}		

		}
		else if(waveType == 1)
		{
			printf("Square wave\n");
			for(i=0; i<resolution; i++)
			{	
				dummy= global_amplitude*sinf((float)(i*delta));
				printf("Dummy: %f\n", dummy);
		
				if (dummy > 0.0)
				{
					dummy = ((global_amplitude/MAX_AMP)+global_offset)*0xffff;
				}
				else 
				{
					dummy = 0.0+global_offset;
				}
		
				data[i] = (unsigned)dummy;
			}
		}

		//Triangular wave just in case
	 	/*

		else if(waveType[0]=='t')
		{
			printf("Triangular Wave\n");
		
			slope = 2.0*global_amplitude/PI;
	
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
					dummy = -slope*((float)(i*delta))+2*global_amplitude;
				}	
				else
				{
					printf("3rd seg\n");
					dummy = slope*((float)(i*delta))-4*global_amplitude;
				}
		
				dummy = dummy + global_amplitude;
				printf("dummy: %f\n", dummy);
		
				dummy = ((dummy)/(2.0*MAX_AMP))*0xffff;
		
				data[i] = (unsigned)dummy;
		
				printf("Data: %d\n", data[i]);
			}	
		}
		*/
	
		//*********************************************************************************************
		// Output wave
		//*********************************************************************************************

		//determing t_delta for sleep function to regulate frequency
		printf("F: %f\n", global_frequency);
		T = 1.0/global_frequency;
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
			if(pthread_mutex_trylock(&global_var_mutex)==0)
			{
				//pthred_mutex_lock(&global_var_mutex);
				amplitude = global_amplitude;
				frequency = global_frequency;
				offset = global_offset;
				pthread_mutex_unlock(&global_var_mutex);
			}
			
			delta_frequency = (int)(frequency-fruquency_old);
			delta_amplitude = (int)(amplitude-amplitude_old);
			delta_offset = (int)(offset-offset_old);
			latest_waveType = waveForm;

			if ((delta_frequency != 0) || (delta_amplitude != 0) || (delta_offset != 0) || (latest_waveType != waveType))
			{	
				waveType = latest_waveType;
				amplitude_old = amplitude;
				frequency_old = frequency;
				offset_old = offset;
				break;
			}
			
		}
		
		
		if(pthread_mutex_trylock(&global_stop_mutex)==0)
		{
			die = kill_switch;
			pthread_mutex_unlock(&global_stop_mutex);
		}
		if (die == 1)
		{
			break;
		}
	}

	// Reset DAC to 5V
	out16(DA_CTLREG,(short)0x0a23);	
	out16(DA_FIFOCLR,(short) 0);			
	out16(DA_Data, 0x8fff);
																																																																	
	printf("\n\nExit Demo Program\n");
	pci_detach_device(hdl);


}

