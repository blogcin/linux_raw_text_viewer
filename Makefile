CFLAGS= -static -Wall -O3
CROSS_COMPILE = /usr/local/arm/4.2.2-eabi/usr/bin/arm-linux-

CC      =$(CROSS_COMPILE)gcc #--verbose
STRIP   =$(CROSS_COMPILE)strip
LD      =$(CROSS_COMPILE)ld #-m elf32arm26
AR      =$(CROSS_COMPILE)ar
RANLIB  =$(CROSS_COMPILE)ranlib

CPP		= $(CC) -E
NM		= $(CROSS_COMPILE)nm
DEP		= $(CROSS_COMPILE)gcc
CPP		= $(CROSS_COMPILE)g++
ARFLAGS = rv
LINKER  = $(CROSS_COMPILE)g++ --verbose


CFLAGS += -I./libhdmi/include -I./libhdmi

default: fontTest

clean:
	rm -f fontTest *.o

fontTest: test.c font.c display.c file.c
	$(CC) test.c font.c display.c file.c -o fontTest -L. -lhdmi -lm $(CFLAGS) -L./libhdmi
