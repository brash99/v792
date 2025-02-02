#
# File:
#    Makefile
#
# Description:
#    caen792 Driver Makefile
#
#
BASENAME=c792
#
# Uncomment DEBUG line, to include some debugging info ( -g and -Wall)
DEBUG	?= 1
#QUIET	?= 1
#
ifeq ($(QUIET),1)
        Q = @
else
        Q =
endif

ARCH	?= $(shell uname -m)

# Check for CODA 3 environment
ifdef CODA_VME

INC_CODA	= -I${CODA_VME}/include
LIB_CODA	= -L${CODA_VME_LIB}

endif

# Defs and build for PPC using VxWorks
ifeq (${ARCH}, PPC)
OS			= VXWORKS
VXWORKS_ROOT		?= /site/vxworks/5.5/ppc/target

ifdef LINUXVME_INC
VME_INCLUDE             ?= -I$(LINUXVME_INC)
endif

CC			= ccppc
LD			= ldppc
DEFS			= -mcpu=604 -DCPU=PPC604 -DVXWORKS -D_GNU_TOOL -mlongcall \
				-fno-for-scope -fno-builtin -fvolatile -DVXWORKSPPC
INCS			= -I. -I$(VXWORKS_ROOT)/h  \
				$(VME_INCLUDE) ${INC_CODA}
CFLAGS			= $(INCS) $(DEFS)
else
OS			= LINUX
endif

# Defs and build for i686, x86_64 Linux
ifeq ($(OS),LINUX)

# Safe defaults
LINUXVME_LIB		?= ../lib
LINUXVME_INC		?= ../include

CC			= gcc
ifeq ($(ARCH),i686)
CC			+= -m32
endif
AR                      = ar
RANLIB                  = ranlib
CFLAGS			= -L. -L${LINUXVME_LIB} ${LIB_CODA}
INCS			= -I. -I${LINUXVME_INC} ${INC_CODA}

LIBS			= lib${BASENAME}.a lib${BASENAME}.so
endif #OS=LINUX#

ifdef DEBUG
CFLAGS			+= -Wall -g
else
CFLAGS			+= -O2
endif
SRC			= caen792Lib.c
HDRS			= c792Lib.h
OBJ			= $(SRC:.c=.o)
DEPS			= $(SRC:.c=.d)

ifeq ($(OS),LINUX)
all: echoarch ${LIBS}
else
all: echoarch $(OBJ)
endif

%.o: %.c
	@echo " CC     $@"
	${Q}$(CC) $(CFLAGS) $(INCS) -c -o $@ $(SRC)

%.so: $(SRC)
	@echo " CC     $@"
	${Q}$(CC) -fpic -shared $(CFLAGS) $(INCS) -o $(@:%.a=%.so) $(SRC)

%.a: $(OBJ)
	@echo " AR     $@"
	${Q}$(AR) ru $@ $<
	@echo " RANLIB $@"
	${Q}$(RANLIB) $@

ifeq ($(OS),LINUX)
install: $(LIBS)
	@echo " CP     $<"
	${Q}cp $(PWD)/$< $(LINUXVME_LIB)/$<
	@echo " CP     $(<:%.a=%.so)"
	${Q}cp $(PWD)/$(<:%.a=%.so) $(LINUXVME_LIB)/$(<:%.a=%.so)
	@echo " CP     ${HDRS}"
	${Q}cp ${HDRS} $(LINUXVME_INC)

coda_install: $(LIBS)
	@echo " CP     $<"
	${Q}cp $(PWD)/$< $(CODA_VME_LIB)/$<
	@echo " CP     $(<:%.a=%.so)"
	${Q}cp $(PWD)/$(<:%.a=%.so) $(CODA_VME_LIB)/$(<:%.a=%.so)
	@echo " CP     ${HDRS}"
	${Q}cp ${HDRS} $(CODA_VME)/include

%.d: %.c
	@echo " DEP    $@"
	@set -e; rm -f $@; \
	$(CC) -MM -shared $(INCS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEPS)

endif

clean:
	@rm -vf ${OBJ} lib${BASENAME}.{a,so} ${DEPS}

echoarch:
	@echo "Make for $(OS)-$(ARCH)"

.PHONY: clean echoarch
