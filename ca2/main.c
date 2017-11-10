/*
[ ] remove unnecessary PCI setup printf
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include "hardware.h"
#include "input.h"

int badr[5];								// PCI 2.2 assigns 6 IO base addresses
int i=0;

int main(int argc, char *argv[]){
//==============================================================================
//PCI SETUP
//==============================================================================
struct pci_dev_info info;
void *hdl;

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

														// Modify thread control privity
if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
  perror("Thread Control");
  exit(1);
  }				

//==================================================================

//START HERE

//display instruction
printf("This program bla bla bla.. Following are instructions bla bla:\n");
printf("1) bla bla\n");
printf("2) more bla bla\n");


//instruction to press [ENTER] to start [INPUT READING MODE]
printf("\nPress [ENTER] when you are ready\n");

//loop wait for [ENTER] key to be pressed


//read and update digital & analog input
dio_setup(0x90);
sleep(1);
while(read_input());

}
