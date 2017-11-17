/*
Maintainer	: Nicholas Adrian
*/
#include "Global.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include "Hardware.h"

bool info_switch_prev;
bool wavef;
bool infos;
uint16_t freq;
uint16_t amp;
uint8_t f_prev;
uint8_t a_prev;
uintptr_t dio_result;
uint16_t channel0 = 0x00;
uint16_t channel1 = 0x01;

void pci_setup(){
	int i;
	//set memory of the size of struct info
	memset(&info,0,sizeof(info));
	//attach to PCI Server
	if(pci_attach(0)<0) {
	  perror("pci_attach");
	  exit(EXIT_FAILURE);
	  }
	
	//vendor and device ID of the board
	info.VendorId=0x1307;
	info.DeviceId=0x01;
	
	//attaching to device
	if ((hdl=pci_attach_device(0, PCI_SHARE|PCI_INIT_ALL, 0, &info))==0) {
	  perror("pci_attach_device");
	  exit(EXIT_FAILURE);
	  }
	
	if (DEBUG){  
	  for(i=0;i<6;i++) {
	    if(info.BaseAddressSize[i]>0) {
	      printf("Aperture %d  Base 0x%x Length %d Type %s\n", i, 
	        PCI_IS_MEM(info.CpuBaseAddress[i]) ?  (int)PCI_MEM_ADDR(info.CpuBaseAddress[i]) : 
	        (int)PCI_IO_ADDR(info.CpuBaseAddress[i]),info.BaseAddressSize[i], 
	        PCI_IS_MEM(info.CpuBaseAddress[i]) ? "MEM" : "IO");
	      }
	  }  
	printf("IRQ %d\n",info.Irq); 		
	}
	
	//get base address
	if(DEBUG)printf("\nDAS 1602 Base addresses:\n\n");
	for(i=0;i<5;i++) {
	  badr[i]=PCI_IO_ADDR(info.CpuBaseAddress[i]);
	  if(DEBUG) printf("Badr[%d] : %x\n", i, badr[i]);
	  }
	 
	//map I/O base address to user space
	for(i=0;i<5;i++) {
	  iobase[i]=mmap_device_io(0x0f,badr[i]);	
	  if(DEBUG) printf("Index %d : Address : %x ", i,badr[i]);
	  if(DEBUG) printf("IOBASE  : %x \n",iobase[i]);
	  }													

	//get access to resources
        if(ThreadCtl(_NTO_TCTL_IO,0)==1){
        perror("Thread Control");
        exit(1);
        }
}

void dio_setup(){
	//digital control register
	out8(DIO_CTLREG,0x90);
	//clear LED
	out8(DIO_PORTB,0x00);
}

uintptr_t dio_read(uintptr_t dio_port){
	//return 8 bits value of the digital port
	return in8(dio_port);
}

uint16_t aio_read(uint16_t channel){
	out16(INTERRUPT,0x60c0);
	out16(TRIGGER,0x2081);
	out16(AUTOCAL,0x007f);

	out16(AD_FIFOCLR,0);
	out16(MUXCHAN,0x0D00|channel);

	out16(AD_DATA,0);
	//while(!(in16(MUXCHAN) & 0x4000));
	return in16(AD_DATA);
}

void led(uint16_t lvl){
	//segment the 16 bit ADC value to different stages for the LED display
	if(lvl<0x0258){out8(DIO_PORTB,0x00);}				// <600
	else if(0x0258<=lvl & lvl<0x3fff) out8(DIO_PORTB,0x01);		// 600<X<16383
        else if(0x3fff<=lvl & lvl<0x7fff) out8(DIO_PORTB,0x03);		// 16383<X<32767
	else if(0x7fff<=lvl & lvl<0xbfff) out8(DIO_PORTB,0x07);		// 32767<X<49151
	else out8(DIO_PORTB,0x0f);					// >49151
}

void *read_param(){
  //this function is called if user chose to read parameters from file

  //setup PCI and indicate hardware readiness
  pci_setup();
  pthread_mutex_lock(&global_var_mutex);
  hardware_ready = true;
  pthread_cond_signal(&hardware_ready_cond);
  pthread_mutex_unlock(&global_var_mutex);
  }

void *read_input(){
  //this function is responsible for reading digital and analog input from the hardware
  bool waveform_prev;

  //setup PCI and I/O port
  pci_setup();
  dio_setup();

  pthread_sigmask(SIG_SETMASK, &all_sig_mask_set, NULL);
  //acquire permission to resources
  if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
	  perror("Thread Control");
	  exit(1);
  }	

  //read input for initialization
  freq = aio_read(channel0);
  amp = aio_read(channel1);
  dio_result = dio_read(DIO_PORTA);
  wavef=(dio_result & 0x04);				// 0 - sine  1 - square
  
  pthread_mutex_lock(&global_var_mutex);
  global_frequency = freq>>8;
  global_amplitude = amp>>8;
  waveform=wavef;
  hardware_ready = true;

  //indicate hardware readiness
  pthread_cond_signal(&hardware_ready_cond);
  pthread_mutex_unlock(&global_var_mutex);

  waveform_prev=wavef;
  f_prev = freq;
  a_prev = amp;
  
  infos=(dio_result & 0x08);
  
  //for debouncing info_switch
  info_switch_prev=infos;
  
  while(1){
    delay(1);
    dio_result = dio_read(DIO_PORTA);
    infos=(dio_result & 0x08);
    
    //info switch toggle
    if(!!infos != !!info_switch_prev){
      pthread_mutex_lock(&global_stop_mutex);
      info_switch = 1;
      while(info_switch) pthread_cond_wait(&info_switch_cond, &global_stop_mutex);
      pthread_mutex_unlock(&global_stop_mutex);
      info_switch_prev=infos;
    }

    //ADC read
    freq = aio_read(channel0);
    amp = aio_read(channel1);
    //digital input read
    wavef=(dio_result & 0x04);

    //check for any update in values
    if(abs((freq>>8)-f_prev)>30 || abs((amp>>8)-a_prev)>30 || (waveform_prev!=wavef))
    {
     pthread_mutex_lock(&global_var_mutex);
       //global variables are scaled to 8 bits by keeping the 8 MSB
       global_frequency = freq>>8;
       global_amplitude = amp>>8;
       waveform = wavef;
       var_update=1; 
       pthread_mutex_unlock(&global_var_mutex);
       waveform_prev=wavef;
       f_prev = freq>>8;
       a_prev = amp>>8;
     }
      
    //update LED based on amplitude value
    led(amp);

    //check kill_switch and exit cleanly
    if(pthread_mutex_trylock(&global_stop_mutex)==0){
    if(kill_switch){
      pthread_mutex_unlock(&global_stop_mutex);
      return;}
    else {pthread_mutex_unlock(&global_stop_mutex);}
    }
  }
}
