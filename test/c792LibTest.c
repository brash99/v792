/*
 * File:
 *    c792LibTest.c
 *
 * Description:
 *    Test c792 library
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

  c792Init(addr,0,1,0);

  c792Status(0,0,0);


  vmeBusUnlock();

 CLOSE:
  vmeCloseDefaultWindows();

  return 0;
}
