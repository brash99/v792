#
# File:
#    Makefile
#
# Description:
#    Makefile for the v792 library test programs
#
#
DEBUG	?= 1
QUIET	?= 1
#
ifeq ($(QUIET),1)
        Q = @
else
        Q =
endif

ARCH	?= $(shell uname -m)
OS	?= LINUX

# Check for CODA 3 environment
ifdef CODA_VME

INC_CODA	= -I${CODA_VME}/include
LIB_CODA	= -L${CODA_VME_LIB}

endif

# linuxvme defaults, if they're not already defined
LINUXVME_INC	?= .
LINUXVME_LIB	?= .
LINUXVME_BIN	?= .

CROSS_COMPILE		=
CC			= $(CROSS_COMPILE)gcc
AR                      = ar
RANLIB                  = ranlib
INCS			= -I. -I../ -I${LINUXVME_INC} ${INC_CODA}
CFLAGS			= -L. -L../ -L${LINUXVME_LIB} ${LIB_CODA} -lrt -ljvme -lc792
ifeq ($(DEBUG),1)
	CFLAGS		+= -Wall -g
endif

SRC			= $(wildcard *.c)
DEPS			= $(SRC:.c=.d)
OBJ			= $(SRC:.c=.o)
PROGS			= $(SRC:.c=)

all: echoarch $(PROGS)

clean distclean:
	@rm -f $(PROGS) *~ $(OBJS) $(DEPS)

%: %.c
	@echo " CC     $@"
	${Q}$(CC) $(CFLAGS) $(INCS) -o $@ $<

%.d: %.c
	@echo " DEP    $@"
	@set -e; rm -f $@; \
	$(CC) -MM -shared $(INCS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEPS)

.PHONY: all clean distclean

echoarch:
	@echo "Make for $(OS)-$(ARCH)"
