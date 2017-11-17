#include "Global.h"
#include "hardware.h"

void *generateWave()
{
	int resolution = 80;

	uint8_t frequency_8bit, amplitude_8bit;

	float delta, T, delta_t, dummy, slope, frequency, amplitude;

	unsigned int i;
	unsigned int data[1000];

	struct timespec t_delta;
	
	bool waveType;
	
	pthread_sigmask(SIG_SETMASK, &all_sig_mask_set, NULL);
	
	if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
	  perror("Thread Control");
	  exit(1);
	  }	
	
	//find waveform points
	delta = (2.0*PI)/((float)resolution);
	
	while(1)
	{	
		//locking the mutex
		pthread_mutex_lock(&global_var_mutex);

		//updating variables for wave form 
		//frequency_8bit = global_frequency;
		//amplitude_8bit = global_amplitude;
		frequency = (float)global_frequency;
		amplitude = (float)global_amplitude;
		waveType = waveform;

		//unlocking the mutex
		pthread_mutex_unlock(&global_var_mutex);
	
		if (frequency == 0.0)
		    frequency = FREQUENCY_MIN;
		else if (frequency == 255)
		    frequency = FREQUENCY_MAX;
		else
		    frequency = (FREQUENCY_MAX-FREQUENCY_MIN)*frequency/245.0 + 1.0 - (FREQUENCY_MAX-FREQUENCY_MIN)*10.0/245.0;

		//sine wave
		if (waveType == 0)
		{
			//filling data points for sine wave
			for(i=0;i<=resolution;++i)
			{
				dummy = (amplitude*sin((float)(i*delta))+amplitude);
				dummy = ((dummy)/(2.0*MAX_AMP))*0xffff;
	  			data[i] = (unsigned) dummy;	
			}	
		}		
		//square wave
		else // if (waveType == 1)
		{
			//filling data points for square wave
			for(i=0;i<resolution;++i)
			{
				dummy= amplitude*sin((float)(i*delta));

				if (dummy > 0.0)
				{
					dummy = ((amplitude/MAX_AMP))*0xffff;
				}
				else 
				{
					dummy = 0.0;
				}
				data[i] = (unsigned)dummy;
			}
		}

		
		//determing time period for set frequency 
		T = 1.0/frequency;
		//time step for each data point in wave
		delta_t = T/((float)resolution);
		t_delta.tv_sec = 0;
		t_delta.tv_nsec = delta_t*1000000000;

		while(1)
		{

			//setting control register to unipolar 5V
			out16(DA_CTLREG,0x0a23);		
			//clearing buffer				
		   	out16(DA_FIFOCLR, 0);
			//outputting data to DA port
			
		    out16(DA_Data,data[i]);
		    
			//suspenduing CPU to adjust frequency of wave
			nanospin(&t_delta);
			//to adjust output frequency to oscilloscope		  
		
			//incrementing counter
			++i;

			//re adjusting counter to 0 when greater than no of points
			if (i>=resolution)
			{
				i=0;
			}

			//locking mutex to check if variable update flag has been raised
			if(pthread_mutex_trylock(&global_var_mutex)==0)
			{
				//if var has been updated
				if (var_update == 1)
				{
					//restting var update flag
					var_update = 0;
					//unlocking mutex before exiting
					pthread_mutex_unlock(&global_var_mutex);
					break;
				}
				//unlocking mutex
				pthread_mutex_unlock(&global_var_mutex);		
			}	
			if(pthread_mutex_trylock(&global_stop_mutex)==0)
			{
				if (kill_switch==1)
				{
					pthread_mutex_unlock(&global_stop_mutex);
					break;
				}
				pthread_mutex_unlock(&global_stop_mutex);
			}
		}

		//checking mutex to access state of global kill switch 
		
		if(pthread_mutex_trylock(&global_stop_mutex)==0)
		{
			if (kill_switch==1)
			{
				pthread_mutex_unlock(&global_stop_mutex);
				break;
			}
			pthread_mutex_unlock(&global_stop_mutex);
		}
	}

	// Reset DAC to 5V
	out16(DA_CTLREG,(short)0x0a23);	
	out16(DA_FIFOCLR,0);			
	out16(DA_Data, 0x0000);

	pci_detach_device(hdl);		
																																																															
}

