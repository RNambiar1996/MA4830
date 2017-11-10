/*
[X] read digital input switch 1: killswitch
[X] read digital input switch 2: sine vs square waveform
[X] read analog input switch 1: amplitude/frequency value
[X] read analog input switch 2: offset value
[X] rescaled analog uint16 bit to uint8 bit
[X] update LED
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
//#include "hardware.h"
#include "input.h"
#include "Global.h"

bool waveform;
bool af;
uintptr_t dio_result;
char* w_source;
char* aio_source;

uint16_t channel0 = 0x00;
uint16_t channel1 = 0x01;

void dio_setup(uint8_t ctlreg){
	out8(DIO_CTLREG,ctlreg);		//Digital CTLREG
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
	delay(1);

	out16(AD_DATA,0);
	while(!(in16(MUXCHAN) & 0x4000));
	return in16(AD_DATA);
}

void led(uint16_t offset){
	if(offset<0x0190){out8(DIO_PORTB,0x00);}				// <400
	else if(0x0190<=offset & offset<0x3fff){out8(DIO_PORTB,0x01);}		// 400<X<16383
        else if(0x3fff<=offset & offset<0x7fff) {out8(DIO_PORTB,0x03);}		// 16383<X<32767
	else if(0x7fff<=offset & offset<0xbfff) {out8(DIO_PORTB,0x07);}		// 32767<X<49151
	else {out8(DIO_PORTB,0x0f);}						// >49151
}

int read_input(){
  dio_result = dio_read(DIO_PORTA);

  if(dio_result & 0x08){
  //kill_switch on
  kill_switch = 1;
  }
  else{
  //kill_switch off
  kill_switch = 0;
  }

  if(dio_result & 0x04){
  //square waveform
  waveform = 1; w_source = "SQUARE";
  }
  else{
  //sine waveform
  waveform = 0; w_source = "SINE";
  }

  if(dio_result & 0x02){
  //Analog switch 1 = amplitude
  af = 1;aio_source="amplitude";
  }
  else{
  //Analog switch 1 = frequency
  af = 0;aio_source="frequency";
  }

  global_frequency = (1-af)*aio_read(channel0);
  global_amplitude = af*aio_read(channel0);
  global_offset = aio_read(channel1);
  //print value to screen | analog values are scaled to 8 bits by keeping the 8 MSB
  printf("[%6s] ",w_source);
  if(af) printf("[%s]: %4d    ",aio_source,(unsigned int)global_amplitude>>8);
  else printf("[%s] : %4d    ",aio_source,(unsigned int)global_frequency>>8);
  printf("[offset]: %4d \n",(unsigned int)global_offset>>8);
  
  //update LED
  led(global_offset);

}
