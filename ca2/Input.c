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
#include "hardware.h"

bool info_switch_prev;
uint16_t amp;
//bool waveform;
//bool fo;
uintptr_t dio_result;
char* w_source;
char* aio_source;
uint8_t f_prev;
uint8_t a_prev;

uint16_t channel0 = 0x00;
uint16_t channel1 = 0x01;

void pci_setup(){
	int i;
	memset(&info,0,sizeof(info));
	if(pci_attach(0)<0) {
	  perror("pci_attach");
	  exit(EXIT_FAILURE);
	  }
	
	info.VendorId=0x1307;								// Vendor and Device ID
	info.DeviceId=0x01;
	
	if ((hdl=pci_attach_device(0, PCI_SHARE|PCI_INIT_ALL, 0, &info))==0) {
	  perror("pci_attach_device");
	  exit(EXIT_FAILURE);
	  }
	
	if (DEBUG){  
	  for(i=0;i<6;i++) {							// Another printf BUG ? - Break printf to two statements
	    if(info.BaseAddressSize[i]>0) {
	      printf("Aperture %d  Base 0x%x Length %d Type %s\n", i, 
	        PCI_IS_MEM(info.CpuBaseAddress[i]) ?  (int)PCI_MEM_ADDR(info.CpuBaseAddress[i]) : 
	        (int)PCI_IO_ADDR(info.CpuBaseAddress[i]),info.BaseAddressSize[i], 
	        PCI_IS_MEM(info.CpuBaseAddress[i]) ? "MEM" : "IO");
	      }
	  }  
	
	    														
	printf("IRQ %d\n",info.Irq); 		
	}
	
	if(DEBUG)printf("\nDAS 1602 Base addresses:\n\n");
	for(i=0;i<5;i++) {
	  badr[i]=PCI_IO_ADDR(info.CpuBaseAddress[i]);
	  if(DEBUG) printf("Badr[%d] : %x\n", i, badr[i]);
	  }
	 
		printf("\nReconfirm Iobase:\n");  			// map I/O base address to user space						
	for(i=0;i<5;i++) {								// expect CpuBaseAddress to be the same as iobase for PC
	  iobase[i]=mmap_device_io(0x0f,badr[i]);	
	  if(DEBUG) printf("Index %d : Address : %x ", i,badr[i]);
	  if(DEBUG) printf("IOBASE  : %x \n",iobase[i]);
	  }													
	
	  if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
	  perror("Thread Control");
	  exit(1);
  	}														// Modify thread control privity
			
}

void dio_setup(){
	out8(DIO_CTLREG,0x90);		//Digital CTLREG
	out8(DIO_PORTB,0x00);			//clear LED
}

uintptr_t dio_read(uintptr_t dio_port){
	return in8(dio_port);
}

uint16_t aio_read(uint16_t channel){
	out16(INTERRUPT,0x60c0);
	out16(TRIGGER,0x2081);
	out16(AUTOCAL,0x007f);

	out16(AD_FIFOCLR,0);
	out16(MUXCHAN,0x0D00|channel);
	//delay(1);

	out16(AD_DATA,0);
	//printf("before while muxchan\n");
	//while(!(in16(MUXCHAN) & 0x4000));
	//printf("after while muxchan\n");
	return in16(AD_DATA);
}

void led(uint16_t lvl){
	if(lvl<0x0190){out8(DIO_PORTB,0x00);}				// <400
	else if(0x0190<=lvl & lvl<0x3fff){out8(DIO_PORTB,0x01);}	// 400<X<16383
        else if(0x3fff<=lvl & lvl<0x7fff) {out8(DIO_PORTB,0x03);}	// 16383<X<32767
	else if(0x7fff<=lvl & lvl<0xbfff) {out8(DIO_PORTB,0x07);}	// 32767<X<49151
	else {out8(DIO_PORTB,0x0f);}					// >49151
}

void *read_input(){

  pthread_sigmask(SIG_SETMASK, &all_sig_mask_set, NULL);
  if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
	  perror("Thread Control");
	  exit(1);
  }	
  pthread_mutex_lock(&global_stop_mutex);
  info_switch_prev=info_switch;
  pthread_mutex_unlock(&global_stop_mutex);
  //pthread_mutex_lock(&global_var_mutex);
  f_prev = aio_read(channel0)>>8; ;
  a_prev =  aio_read(channel1)>>8;;
  //pthread_mutex_unlock(&global_var_mutex);
  //printf("Before read_input while loop\n");
  while(1){
  	delay(1);
    pthread_mutex_lock(&global_var_mutex);
    dio_result = dio_read(DIO_PORTA);
  
    //info switch toggle
    pthread_mutex_lock(&global_stop_mutex);
    if(info_switch!=info_switch_prev){
      info_switch=!info_switch;
      info_switch_prev=info_switch;
    }
    pthread_mutex_unlock(&global_stop_mutex); 
    if(dio_result & 0x04){
    //square waveform
    waveform = 1; w_source = "SQUARE";
    }
    else{
    //sine waveform
    waveform = 0; w_source = "SINE";
    }
  
    //if(dio_result & 0x02){
    ////Read Analog switch 1 for frequency
    //fo = 1;aio_source="offset";
    //}
  
    //if(dio_result & 0x01){
    ////Read Analog switch 2 for amplitude
    //fo = 1;aio_source="offset";
    //}

    global_frequency = aio_read(channel0)>>8;
    amp = aio_read(channel1);
    global_amplitude = amp>>8;
    hardware_ready = 1;
    if(abs(global_frequency-f_prev)>2 || abs(global_amplitude-a_prev)>2) {var_update=1; printf("nic var update\n");} 
    pthread_mutex_unlock(&global_var_mutex);
    //print value to screen | analog values are scaled to 8 bits by keeping the 8 MSB
    pthread_mutex_lock(&print_mutex);
    printf("[frequency]: %4d     ",(unsigned int)global_frequency);
    printf("[amplitude]: %4d \n",(unsigned int)global_amplitude);
    pthread_mutex_unlock(&print_mutex);
    
    //update LED
    led(amp);
  }
}
