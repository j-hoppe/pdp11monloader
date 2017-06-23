#
# tu58fs emulator makefile
#

# UNIX comms model
PROG = pdp11monloader

# compiler flags and libraries
CC_DBG_FLAGS = -ggdb3 -O0 -Wall -Wextra

ifeq ($(MAKE_TARGET_ARCH),BBB)
	CC=$(BBB_CC) -std=c99 -U__STRICT_ANSI__
	LDFLAGS = -lpthread -lrt
	OBJDIR=bin-bbb
else ifeq ($(MAKE_TARGET_ARCH),RPI)
	CC=arm-linux-gnueabihf-gcc -std=c99 -U__STRICT_ANSI__
	LDFLAGS = -lrt
	OBJDIR=bin-rpi
else ifeq ($(MAKE_TARGET_ARCH),CYGWIN)
    # compiling under CYGWIN
    OS_CCDEFS=
	LDFLAGS =
	OBJDIR=bin-cygwin
#    OS_CCDEFS=-m32
    PROG = pdp11monloader.exe
else
    # compiling for X64
    OS_CCDEFS=-m64
	LDFLAGS =
	OBJDIR=bin-ubuntu-x64
endif

CCFLAGS= \
	-I.	\
	-c	\
	$(CCDEFS) $(CC_DBG_FLAGS) $(OS_CCDEFS)

OBJECTS = $(OBJDIR)/main.o \
		$(OBJDIR)/getopt2.o \
		$(OBJDIR)/serial.o \
		$(OBJDIR)/error.o \
		$(OBJDIR)/utils.o \
		$(OBJDIR)/memory.o \
		$(OBJDIR)/monitor.o 


$(OBJDIR)/$(PROG) : $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	file $@
#	mv $@ $(OBJDIR) ; file $(OBJDIR)/$@

all :   $(OBJDIR)/$(PROG)

clean :
	-rm -f $(OBJECTS)
#	-chmod a-x,ug+w,o-w *.c *.h makefile
#	-chmod a+rx $(OBJDIR)/$(PROG)
#	-chown `whoami` *

purge : clean
	-rm -f $(OBJDIR)/$(PROG)

$(OBJDIR)/main.o : main.c main.h
	$(CC) $(CCFLAGS) main.c -o $@

$(OBJDIR)/serial.o : serial.c serial.h
	$(CC) $(CCFLAGS) serial.c -o $@

$(OBJDIR)/getopt2.o : getopt2.c getopt2.h
	$(CC) $(CCFLAGS) getopt2.c -o $@

$(OBJDIR)/error.o : error.c error.h
	$(CC) $(CCFLAGS) error.c -o $@

$(OBJDIR)/utils.o : utils.c utils.h
	$(CC) $(CCFLAGS) utils.c -o $@

$(OBJDIR)/memory.o : memory.c memory.h
	$(CC) $(CCFLAGS) memory.c -o $@

$(OBJDIR)/monitor.o : monitor.c monitor.h
	$(CC) $(CCFLAGS) monitor.c -o $@

# the end
