/*
 * File:
 *    c792Status
 *
 * Description:
 *    Show status from specified Cean 792 ADC
 *
 *  c792Status 0x100000 0x80000 2
 *  c792Status 0x100000
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "jvme.h"
#include "c792Lib.h"

extern int Nc792;

int
main(int argc, char *argv[])
{

  int stat;
  uint32_t addr = 0, addr_inc = 0, nadc = 0;

  if (argc == 4)
    {
      addr = strtol(argv[1], NULL, 16);
      addr_inc = strtol(argv[2], NULL, 16);
      nadc = strtol(argv[3], NULL, 10);
    }
  else if(argc == 2)
    {
      addr = strtol(argv[1], NULL, 16);
    }
  else
    addr = 0x100000;

  printf("----------------------------\n");

  vmeSetQuietFlag(0);
  stat = vmeOpenDefaultWindows();
  if(stat == ERROR)
    goto CLOSE;

  vmeCheckMutexHealth(1);
  printf("\n");

  c792Init(addr, addr_inc, nadc, 0);
  c792Reset(0);

  if(Nc792==0)
    goto CLOSE;

  c792GStatus(0);

 CLOSE:

  vmeCloseDefaultWindows();

  exit(0);
}

/*
  Local Variables:
  compile-command: "make -k -B c792Status"
  End:
 */
