/*
[ ] accept digital input for two types of waves
[ ] change frequency/amplitude value to the right ratio
[ ] update LED
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
//#include "define.h"
#include "input.h"

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
	if(offset<0x0060){out8(DIO_PORTB,0x00);}
	else if(0x0060<=offset & offset<0x0f00){out8(DIO_PORTB,0x01);}
        else if(0x0f00<=offset & offset<0x3000) {out8(DIO_PORTB,0x03);}
	else if(0x3000<=offset & offset<0xf700) {out8(DIO_PORTB,0x07);}
	else {out8(DIO_PORTB,0x0f);}
}

int read_input(){
dio_result = dio_read(DIO_PORTA);

if(dio_result & 0x02){
//Analog switch 1 = amplitude
aio_source = "amplitude";
}
//else if(dio_result==0xf4){
else{
//Analog switch 1 = frequency
aio_source = "frequency";
}

ai0_result = aio_read(channel0);
ai1_result = aio_read(channel1);
//print value to screen
printf("[%s]: %4x	",aio_source,(unsigned int)ai0_result);
printf("[offset]: %4x \n",(unsigned int)ai1_result);

//update LED
led(ai1_result);


}
