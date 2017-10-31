#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include "define.h"

void dio_setup(uint8_t ctlreg);

uintptr_t dio_read(uintptr_t dio_port);

uint16_t aio_read(uint16_t channel);

int read_input();
