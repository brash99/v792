/*
 * File:
 *    c792ReadoutTest.c
 *
 * Description:
 *    Test readout of c792
 *
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "jvme.h"
#include "c792Lib.h"

int
main(int argc, char *argv[])
{
  int stat;
  unsigned int addr;

  if (argc > 1)
    {
      addr = (unsigned int) strtoll(argv[1], NULL, 16) & 0xffffffff;
    }
  else
    {
      printf("ERROR: Must provide A24/A32 address\n");
      return -1;
    }

  vmeSetQuietFlag(1);

  stat = vmeOpenDefaultWindows();

  if(stat!=OK)
    goto CLOSE;

  vmeBusLock();

  extern int Nc792;
  c792Init(addr,0,1,0);

  if(Nc792 < 1)
    goto CLOSE;

  int iadc;

  for(iadc = 0; iadc < Nc792; iadc++)
    {
      c792Sparse(iadc,0,0);
      c792Clear(iadc);
      c792EnableBerr(iadc);
    }
  c792GStatus(0);


  /* Enable modules */
  for(iadc = 0; iadc < Nc792; iadc++)
    c792Enable(iadc);

  /* Set up readout buffer */
  DMA_MEM_ID vmeIN,vmeOUT;
  extern DMANODE *the_event;
  extern unsigned int *dma_dabufp;
  dmaPFreeAll();
  vmeIN  = dmaPCreate("vmeIN",1024,4,0);
  vmeOUT = dmaPCreate("vmeOUT",0,0,0);

  dmaPStatsAll();

  dmaPReInitAll();

  /* Trigger modules */
  for(iadc = 0; iadc < Nc792; iadc++)
    c792Gate(iadc);


  c792GStatus(0);

  GETEVENT(vmeIN,0);

  unsigned int scanmask = 0, datascan = 0;
  int dCnt;

  /* Check if an Event is available */
  scanmask = c792ScanMask();
  datascan = c792GDReady(scanmask, 1000);

  if(datascan==scanmask)
    {
      for(iadc = 0; iadc < Nc792; iadc++)
	{
	  vmeDmaConfig(1,3,0);
	  dCnt = c792ReadBlock(iadc,dma_dabufp,40);
	  if(dCnt <= 0)
	    {
	      printf("ERROR: ADC %2d Read Failed - Status 0x%x\n",
		     iadc, dCnt);
	      c792Clear(iadc);
	    }
	  else
	    {
	      dma_dabufp += dCnt;
	    }
	}

    }
  else
    {
      printf("ERROR: datascan != scanmask for ADC  (0x%08x != 0x%08x)\n",
	     datascan,scanmask);

      for(iadc = 0; iadc < Nc792; iadc++)
	c792Clear(iadc);
    }

  PUTEVENT(vmeOUT);
  unsigned long length =
    (((unsigned long)(dma_dabufp) - (unsigned long)(&the_event->length))) - 4;
  int size = the_event->part->size - sizeof(DMANODE);

  if(length>size)
    {
      printf("rocLib: ERROR: Event length > Buffer size (%ld > %d).  Event %ld\n",
	     length,size,the_event->nevent);
    }

  DMANODE *outEvent = dmaPGetItem(vmeOUT);
  length = outEvent->length;

  int idata;
  for(idata=0;idata<length;idata++)
    {
      if((idata%5)==0) printf("\n\t");
      printf("  0x%08x ",(unsigned int)LSWAP(outEvent->data[idata]));
    }
  printf("\n\n");
  dmaPFreeItem(outEvent);

  c792GStatus(0);

 CLOSE:
  dmaPFreeAll();
  vmeBusUnlock();

  vmeCloseDefaultWindows();

  return 0;
}


/*
  Local Variables:
  compile-command: "make -B c792ReadoutTest"
  End:
*/
