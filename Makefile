#
# Makefile for busy-wait IO tests
#
XCC = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc
AS = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/as
LD = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ld
CFLAGS  = -c -fPIC -Wall -I. -I../include -mcpu=arm920t -msoft-float

# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -Map main.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

all: main.elf 

context_switch.o: context_switch.s
	$(AS) $(ASFLAGS) -o context_switch.o context_switch.s

task.s: task.c
	$(XCC) -S $(CFLAGS) task.c

task.o: task.s
	$(AS) $(ASFLAGS) -o task.o task.s

syscall.s: syscall.c
	$(XCC) -S $(CFLAGS) syscall.c

syscall.o: syscall.s
	$(AS) $(ASFLAGS) -o syscall.o syscall.s

main.s: main.c
	$(XCC) -S $(CFLAGS) main.c

main.o: main.s
	$(AS) $(ASFLAGS) -o main.o main.s

main.elf: main.o syscall.o context_switch.o task.o
	$(LD) $(LDFLAGS) -o $@ main.o syscall.o context_switch.o task.o -lbwio -lgcc

install: main.elf
	cp main.elf /u/cs452/tftp/ARM/dzelemba/

#
# Tests
#

# GCC = /usr/bin/gcc
# TESTFLAGS = -g -Wall -I. -I../include

# test: test.out

#test.out: all_tests.c strings_test.c ring_buffer_test.c test_helpers.* strings.* ring_buffer.*
#	$(GCC) $(TESTFLAGS) all_tests.c test_helpers.c strings.c ring_buffer.c -o test.out 

clean:
	-rm -f *.elf *.o *.out main.map syscall.s main.s task.s
