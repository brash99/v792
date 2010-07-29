#
# Description:  Makefile for c792Lib.o
#   This driver is specific to VxWorks BSPs and must be compiled
#   with access to vxWorks headers.
#

#Check Operating system we are using
ifndef OSNAME
  OSNAME := $(subst -,_,$(shell uname))
endif

ifndef ARCH
  ARCH = VXWORKSPPC
endif

ifeq ($(ARCH),VXWORKSPPC)
INCDIR=/site/vxworks/5.5/ppc/target/h
CC = ccppc
LD = ldppc
DEFS = -mcpu=604 -DCPU=PPC604 -DVXWORKS -D_GNU_TOOL -DVXWORKSPPC
INCS = -fno-for-scope -fno-builtin -fvolatile -fstrength-reduce -mlongcall -I. -I$(INCDIR)
CFLAGS = -O $(DEFS)
endif

ifeq ($(ARCH),VXWORKS68K51)
INCDIR=/site/vxworks/5.3/68k/target/h
CC = cc68k
DEFS = -DCPU=MC68040 -DVXWORKS -DVXWORKS68K51
INCS = -Wall -mc68020 -fvolatile -fstrength-reduce -nostdinc -I. -I$(INCDIR)
CFLAGS = -O $(DEFS)
endif

ifeq ($(ARCH),Linux)

ifndef LINUXVME_LIB
	LINUXVME_LIB	= $CODA/extensions/linuxvme/libs
endif
ifndef LINUXVME_INC
	LINUXVME_INC	= $CODA/extensions/linuxvme/include
endif
CC = gcc
DEFS = -DJLAB
INCS = -I. -I${LINUXVME_INC}
CFLAGS = -O ${DEFS} -O2  -L. -L${LINUXVME_LIB}
ifdef DEBUG
CFLAGS += -Wall -g
endif

endif


all: echoarch c792Lib.o

c792Lib.o: caen792Lib.c c792Lib.h
	$(CC) -c $(CFLAGS) $(INCS) -o $@ caen792Lib.c

clean:
	rm -f *.o

echoarch:
	echo "Make for $(ARCH)"
