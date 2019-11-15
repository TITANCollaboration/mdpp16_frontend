#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "mdpp16.h"

//=========================================================================
//========================== Midas wrapper ================================
#include "mvmestd.h"

int vme_A32D32_read (int p, unsigned int module_addr, unsigned int *data);
int vme_A32D32_write(int p, unsigned int module_addr, unsigned int  data);
int vme_A32D16_read (int p, unsigned int module_addr, unsigned int *data);
int vme_A32D16_write(int p, unsigned int module_addr, unsigned int  data);
int vme_A32MBLT64_read(int p, unsigned int addr, unsigned int *dest, unsigned int  dma_req_length, unsigned int *dma_length);

int vme_A32D32_read(int p, unsigned int module_addr, unsigned int *data) {
  MVME_INTERFACE *mvme;

  mvme = (MVME_INTERFACE *) p;
  mvme_set_am(mvme, MVME_AM_A24);
  mvme_set_dmode(mvme, MVME_DMODE_D32);
  *data = mvme_read_value(mvme, module_addr);
  return 0;
}
int vme_A32D32_write(int p, unsigned int module_addr, unsigned int data) {
  MVME_INTERFACE *mvme;

  mvme = (MVME_INTERFACE *) p;
  mvme_set_am(mvme, MVME_AM_A24);
  mvme_set_dmode(mvme, MVME_DMODE_D32);
  return mvme_write_value(mvme, module_addr, data);
}

int vme_A32D16_read(int p, unsigned int module_addr, unsigned int *data) {
  MVME_INTERFACE *mvme;

  mvme = (MVME_INTERFACE *) p;
  mvme_set_am(mvme, MVME_AM_A24);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  *data = mvme_read_value(mvme, module_addr);
  return 0;
}
int vme_A32D16_write(int p, unsigned int module_addr, unsigned int data) {
  MVME_INTERFACE *mvme;

  mvme = (MVME_INTERFACE *) p;
  mvme_set_am(mvme, MVME_AM_A24);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  return mvme_write_value(mvme, module_addr, data);
}


//  called as: vme_A32MBLT64_read(hVme, vme_addr, dma_buffer, end_addr, &dma_bufend);
int vme_A32MBLT64_read(int p, unsigned int addr, unsigned *dest, unsigned dma_req_length, unsigned *dma_length) {
  MVME_INTERFACE *mvme;
  mvme = (MVME_INTERFACE *) p;

  mvme_set_am(mvme, MVME_AM_A24);
  mvme_set_dmode(mvme, MVME_DMODE_D32);
  *dma_length=dma_req_length;
  return mvme_read(mvme,dest,addr,dma_req_length*4);
 // let size decide whether a DMA is done
}

///////////////////////////////////////////////////////////////////////////
///////////////////////      General Registers      ///////////////////////

int mdpp16_reset(int h, unsigned module_addr)
{
   vme_A32D16_write(h, module_addr+MDPP16_SOFT_RESET, 0);
   return(0);
}

int mdpp16_version(int h, unsigned module_addr)
{
   printf("Address %p\n", module_addr+MDPP16_FIRMWARE_VSN);
   unsigned version;
   vme_A32D16_read(h, module_addr+MDPP16_FIRMWARE_VSN, &version);
   return((int)version);
}

int mdpp16_read_fifo_words(int h, unsigned module_addr)
{
   unsigned words;
   vme_A32D16_read(h, module_addr+MDPP16_BUFFER_DATALEN, &words);
   return((int)words);
}

int mdpp16_acq_start(int h, unsigned module_addr)
{
   vme_A32D16_write(h, module_addr+MDPP16_START_ACQ,  0);
   vme_A32D16_write(h, module_addr+MDPP16_FIFO_RESET, 0);
   vme_A32D16_write(h, module_addr+MDPP16_START_ACQ,  1);
   vme_A32D16_write(h, module_addr+MDPP16_READOUT_RESET, 1);
   return(0);
}
int mdpp16_acq_stop(int h, unsigned module_addr)
{
   vme_A32D16_write(h, module_addr+MDPP16_START_ACQ,  0);
   return(0);
}
