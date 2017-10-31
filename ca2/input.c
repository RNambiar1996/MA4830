/*
[ ] do masking for only one bit for dio_result
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
#include "define.h"
#include "input.h"

char* aio_source;

void dio_setup(uint8_t ctlreg){
	out8(DIO_CTLREG,ctlreg);
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

int read_input(){
dio_result = dio_read(DIO_PORTA);

if(dio_result==0xf4){		//change to do masking for only one bit
//AI0 = amplitude
channel = 0x00;
aio_source = "amplitude";
}
else{
//AI1 = frequency
channel = 0x01;
aio_source = "frequency";
}

aio_result = aio_read(channel);

//update LED
//led()

//print value to screen
printf("[%s]: %4x \n",aio_source,(unsigned int)aio_result);


}
