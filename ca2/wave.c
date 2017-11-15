#include "Global.h"
#include "hardware.h"

void *generateWave()
{
	int resolution = 100;

	float delta, T, delta_t, dummy, slope, frequency, amplitude;

	unsigned int i;
	unsigned int data;

	struct timespec t_delta;
	
	bool waveType, latest_waveType;
	bool die = 0;
	bool parameter_changed = 0;

	pthread_sigmask(SIG_SETMASK, &all_sig_mask_set, NULL);
	
	//find waveform points
	delta = (2.0*PI)/((float)resolution);
	printf("%d\n", delta);

	
	while(1)
	{	
		//locking the mutex
		pthread_mutex_lock(&global_var_mutex);

		frequency = (global_frequency/255.0)*3000.0;
	
		if (frequency < 0.1)
			frequency = 0.1;

		amplitude = global_amplitude;

		waveType = waveform;

		//unlocking the mutex
		pthread_mutex_unlock(&global_var_mutex);
		
		if (waveType == 0)
		{
			for(i=0;i<resolution;i++)
			{
				dummy = (amplitude*sin((float)(i*delta))+amplitude);
				dummy = ((dummy)/(2.0*MAX_AMP))*0x8000;
				//printf("Dummy: %f\n", dummy);
	  			data[i] = (unsigned) dummy;	
			}	
		}		
		else if(waveType == 1)
		{
			printf("Square wave\n");
			for(i=0;i<resolution;i++)
			{
				dummy= amplitude*sin((float)(i*delta));
				//printf("Dummy: %f\n", dummy);	

				if (dummy > 0.0)
				{
					dummy = ((amplitude/MAX_AMP))*0x8000;
				}
				else 
				{
					dummy = 0.0;
				}
				data[i] = (unsigned)dummy;
			}
		}

		T = 1.0/frequency;
		//printf("T: %f\n", T);
		delta_t = (T)/((float)resolution);
		t_delta.tv_sec = 0;
		t_delta.tv_nsec = delta_t*1000000000;
	

		while(1)
		{
			out16(DA_CTLREG,0x0a23);						
		   	out16(DA_FIFOCLR, 0);							
		    	out16(DA_Data,(short) data[i]);																												
		  	nanospin(&t_delta);

			++i;
			if (i>=resolution)
			{
				i=0;
			}
			pthread_mutex_lock(&global_var_mutex);
			if (var_update == 1)
			{
				var_update = 0;
				pthread_mutex_unlock(&global_var_mutex);
				break;
			}
			pthread_mutex_unlock(&global_var_mutex);		
		}

		pthread_mutex_lock(&global_stop_mutex);
		if (kill_switch==1)
		{
			pthread_mutex_unlock(&global_stop_mutex);
			break;
		}
		pthread_mutex_unlock(&global_stop_mutex);

	}

	// Reset DAC to 5V
	out16(DA_CTLREG,(short)0x0a23);	
	out16(DA_FIFOCLR,(short) 0);			
	out16(DA_Data, 0x8fff);

	pci_detach_device(hdl);		
																																																															
}

