/*************************************************************************
 *
 *  c792_list.c - Library of routines for the user to write for
 *                      readout and buffering of events using
 *                      a Linux VME controller.
 *
 */

/* Event Buffer definitions */
#define MAX_EVENT_POOL     400
#define MAX_EVENT_LENGTH   1024*10      /* Size in Bytes */

/* Initialize TI as a master (TI_MASTER) or slave (TI_SLAVE) */
#define TI_MASTER

#ifdef TI_MASTER
/* EXTernal trigger source (e.g. front panel ECL input), POLL for available data */
#define TI_READOUT TI_READOUT_EXT_POLL
#else
/* TS trigger source (e.g. fiber), POLL for available data */
#define TI_READOUT TI_READOUT_TS_POLL
#endif

/* TI VME address, or 0 for Auto Initialize (search for TI by slot) */
#define TI_ADDR  0

/* Measured longest fiber length in system */
#define FIBER_LATENCY_OFFSET 0x4A

#include "dmaBankTools.h"   /* Macros for handling CODA banks */
#include "tiprimary_list.c" /* Source required for CODA readout lists using the TI */
#include "c792Lib.h"

/* Define initial blocklevel and buffering level */
#define BLOCKLEVEL 1
#define BUFFERLEVEL 1

/*
  enable triggers (random or fixed rate) from internal pulser
 */
/* #define INTRANDOMPULSER */
/* #define INTFIXEDPULSER */

#define C792_BANKID 792
#define MAX_ADC_DATA 34
extern int Nc792;


/* function prototype */
void rocTrigger(int arg);

/****************************************
 *  DOWNLOAD
 ****************************************/
void
rocDownload()
{
  /* Setup Address and data modes for DMA transfers
   *
   *  vmeDmaConfig(addrType, dataType, sstMode);
   *
   *  addrType = 0 (A16)    1 (A24)    2 (A32)
   *  dataType = 0 (D16)    1 (D32)    2 (BLK32) 3 (MBLK) 4 (2eVME) 5 (2eSST)
   *  sstMode  = 0 (SST160) 1 (SST267) 2 (SST320)
   */
  vmeDmaConfig(1,3,0);

  /* Define BLock Level */
  blockLevel = BLOCKLEVEL;

#ifdef TI_MASTER
  /*****************
   *   TI SETUP
   *****************/

  /*
   * Set Trigger source
   *    For the TI-Master, valid sources:
   *      TI_TRIGGER_FPTRG     2  Front Panel "TRG" Input
   *      TI_TRIGGER_TSINPUTS  3  Front Panel "TS" Inputs
   *      TI_TRIGGER_TSREV2    4  Ribbon cable from Legacy TS module
   *      TI_TRIGGER_PULSER    5  TI Internal Pulser (Fixed rate and/or random)
   */
#if (defined (INTFIXEDPULSER) | defined(INTRANDOMPULSER))
  tiSetTriggerSource(TI_TRIGGER_PULSER); /* TS Inputs enabled */
#else
  tiSetTriggerSource(TI_TRIGGER_TSINPUTS); /* TS Inputs enabled */
#endif

  /* Enable set specific TS input bits (1-6) */
  tiEnableTSInput( TI_TSINPUT_1 | TI_TSINPUT_2 );

  /* Load the trigger table that associates
   *  pins 21/22 | 23/24 | 25/26 : trigger1
   *  pins 29/30 | 31/32 | 33/34 : trigger2
   */
  tiLoadTriggerTable(0);

  tiSetTriggerHoldoff(1,10,0);
  tiSetTriggerHoldoff(2,10,0);

  /* Set initial number of events per block */
  tiSetBlockLevel(blockLevel);

  /* Set Trigger Buffer Level */
  tiSetBlockBufferLevel(BUFFERLEVEL);
#endif

  tiStatus(0);

  c792Init(0x200000,0x80000,2,0);
  int iadc;
  for(iadc = 0; iadc < Nc792; iadc++)
    {
      c792SetGeoAddress(iadc, iadc+3);
    }

  printf("rocDownload: User Download Executed\n");

}

/****************************************
 *  PRESTART
 ****************************************/
void
rocPrestart()
{

#ifdef TI_MASTER
  /* Set number of events per block (broadcasted to all connected TI Slaves)*/
  tiSetBlockLevel(blockLevel);
  printf("rocPrestart: Block Level set to %d\n",blockLevel);
#endif

  tiStatus(0);

  /* Program/Init VME Modules Here */
  /* Setup ADCs (no sparcification, enable berr for block reads) */
  int iadc;
  for(iadc = 0; iadc < Nc792; iadc++)
    {
      c792Sparse(iadc,0,0);
      c792Clear(iadc);
      c792EventCounterReset(iadc);
      c792EnableBerr(iadc);
    }
  c792GStatus(0);

  printf("rocPrestart: User Prestart Executed\n");

}

/****************************************
 *  GO
 ****************************************/
void
rocGo()
{

  /* Print out the Run Number and Run Type (config id) */
  printf("rocGo: Activating Run Number %d, Config id = %d\n",
	 rol->runNumber,rol->runType);

  /* Get the current Block Level */
  blockLevel = tiGetCurrentBlockLevel();
  printf("rocGo: Block Level set to %d\n",blockLevel);

  /* Enable/Set Block Level on modules, if needed, here */
  int iadc;
  for(iadc = 0; iadc < Nc792; iadc++)
    c792Enable(iadc);

#ifdef TI_MASTER
#if (defined (INTFIXEDPULSER) | defined(INTRANDOMPULSER))
  printf("************************************************************\n");
  printf("%s: TI Configured for Internal Pulser Triggers\n",
	 __func__);
  printf("************************************************************\n");
#endif

  /* Example: How to start internal pulser trigger */
#ifdef INTRANDOMPULSER
  /* Enable Random at rate 500kHz/(2^7) = ~3.9kHz */
  tiSetRandomTrigger(1,0x7);
#elif defined (INTFIXEDPULSER)
  /*
    Enable fixed rate with period (ns)
    120 +30*700*(1024^0) = 21.1 us (~47.4 kHz)
     - arg2 = 0xffff - Continuous
     - arg2 < 0xffff = arg2 times
  */
  tiSoftTrig(1,0xffff,700,0);
#endif
#endif

  /* Interrupts/Polling enabled after conclusion of rocGo() */
}

/****************************************
 *  END
 ****************************************/
void
rocEnd()
{

#ifdef TI_MASTER
  /* Example: How to stop internal pulser trigger */
#ifdef INTRANDOMPULSER
  /* Disable random trigger */
  tiDisableRandomTrigger();
#elif defined (INTFIXEDPULSER)
  /* Disable Fixed Rate trigger */
  tiSoftTrig(1,0,700,0);
#endif
#endif

  int iadc;
  for(iadc = 0; iadc < Nc792; iadc++)
    {
      c792Disable(iadc);
    }
  c792GStatus(0);


  tiStatus(0);

  printf("rocEnd: Ended after %d blocks\n",tiGetIntCount());

}

/****************************************
 *  TRIGGER
 ****************************************/
void
rocTrigger(int arg)
{
  int dCnt;

  /* Set TI output 1 high for diagnostics */
  tiSetOutputPort(1,0,0,0);

  /* Readout the trigger block from the TI
     Trigger Block MUST be readout first */
  dCnt = tiReadTriggerBlock(dma_dabufp);

  if(dCnt<=0)
    {
      printf("No TI Trigger data or error.  dCnt = %d\n",dCnt);
    }
  else
    { /* TI Data is already in a bank structure.  Bump the pointer */
      dma_dabufp += dCnt;
    }

  unsigned int scanmask = 0, datascan = 0;
  int iadc;

  /* Check if an Event is available */
  scanmask = c792ScanMask();
  datascan = c792GDReady(scanmask, 1000);

  BANKOPEN(C792_BANKID,BT_UI4,blockLevel);
  if(datascan==scanmask)
    {
      for(iadc = 0; iadc < Nc792; iadc++)
	{
	  dCnt = c792ReadBlock(iadc,dma_dabufp,MAX_ADC_DATA+40);
	  if(dCnt <= 0)
	    {
	      logMsg("%4d: ERROR: ADC %2d Read Failed - Status 0x%x\n",
		     tiGetIntCount(),
		     iadc, dCnt,0,0,0,0);
	      *dma_dabufp++ = LSWAP(iadc);
	      *dma_dabufp++ = LSWAP(0xda00bad1);
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
      logMsg("%4d: ERROR: datascan != scanmask for ADC  (0x%08x != 0x%08x)\n",
	     tiGetIntCount(),
	     datascan,scanmask,0,0,0,0);
      *dma_dabufp++ = LSWAP(datascan);
      *dma_dabufp++ = LSWAP(scanmask);
      *dma_dabufp++ = LSWAP(0xda00bad2);

      for(iadc = 0; iadc < Nc792; iadc++)
	c792Clear(iadc);
    }
  BANKCLOSE;

  /* Set TI output 0 low */
  tiSetOutputPort(0,0,0,0);

}

void
rocCleanup()
{

#ifdef TI_MASTER
  tiResetSlaveConfig();
#endif
}


/*
  Local Variables:
  compile-command: "make -k -B c792_list.so"
  End:
*/
