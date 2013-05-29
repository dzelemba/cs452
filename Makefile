#
# Makefile for busy-wait IO tests
#
XCC = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc
AS = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/as
LD = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ld
CFLAGS  = -O2 -c -fPIC -Wall -I. -I../include -mcpu=arm920t -msoft-float

# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -Map main.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

TEST_SRC_FILES = $(wildcard tests/*.c)
TEST_OBJ_FILES = $(addprefix tests/,$(notdir $(TEST_SRC_FILES:.c=.o)))

all: main.elf

dbg: CFLAGS += -DDEBUG
dbg: main.elf

# Purely Assembly Files

context_switch.o: context_switch.s
	$(AS) $(ASFLAGS) -o context_switch.o context_switch.s

# Building Test Files

tests.o: $(TEST_OBJ_FILES)
	$(LD) -r $(LDFLAGS) -o tests.o $^

tests/%.o: tests/%.c
	$(XCC) -S $(CFLAGS) $< -o $(<:.c=.s); $(AS) $(ASFLAGS) -o $@ $(<:.c=.s)

# Normal C Files

stdlib.s: stdlib.c stdlib.h
	$(XCC) -S $(CFLAGS) stdlib.c

stdlib.o: stdlib.s
	$(AS) $(ASFLAGS) -o stdlib.o stdlib.s

test_helpers.s: test_helpers.c test_helpers.h
	$(XCC) -S $(CFLAGS) test_helpers.c

test_helpers.o: test_helpers.s
	$(AS) $(ASFLAGS) -o test_helpers.o test_helpers.s

run_tests.s: run_tests.c run_tests.h
	$(XCC) -S $(CFLAGS) run_tests.c

run_tests.o: run_tests.s
	$(AS) $(ASFLAGS) -o run_tests.o run_tests.s

kernel.s: kernel.c kernel.h
	$(XCC) -S $(CFLAGS) kernel.c

kernel.o: kernel.s
	$(AS) $(ASFLAGS) -o kernel.o kernel.s

task.s: task.c task.h
	$(XCC) -S $(CFLAGS) task.c

task.o: task.s
	$(AS) $(ASFLAGS) -o task.o task.s

syscall.s: syscall.c syscall.h
	$(XCC) -S $(CFLAGS) syscall.c

syscall.o: syscall.s
	$(AS) $(ASFLAGS) -o syscall.o syscall.s

queue.s: queue.c queue.h
	$(XCC) -S $(CFLAGS) queue.c

queue.o: queue.s
	$(AS) $(ASFLAGS) -o queue.o queue.s

scheduler.s: scheduler.c scheduler.h
	$(XCC) -S $(CFLAGS) scheduler.c

scheduler.o: scheduler.s
	$(AS) $(ASFLAGS) -o scheduler.o scheduler.s

messenger.s: messenger.c messenger.h
	$(XCC) -S $(CFLAGS) messenger.c

messenger.o: messenger.s
	$(AS) $(ASFLAGS) -o messenger.o messenger.s

debug.s: debug.c debug.h
	$(XCC) -S $(CFLAGS) debug.c

debug.o: debug.s
	$(AS) $(ASFLAGS) -o debug.o debug.s

timer.s: timer.c timer.h
	$(XCC) -S $(CFLAGS) timer.c

timer.o: timer.s
	$(AS) $(ASFLAGS) -o timer.o timer.s

main.s: main.c
	$(XCC) -S $(CFLAGS) main.c

main.o: main.s
	$(AS) $(ASFLAGS) -o main.o main.s

main.elf: main.o run_tests.o tests.o kernel.o test_helpers.o syscall.o context_switch.o task.o queue.o scheduler.o messenger.o stdlib.o debug.o timer.o
	$(LD) $(LDFLAGS) -o $@ main.o run_tests.o tests.o kernel.o test_helpers.o syscall.o context_switch.o task.o queue.o scheduler.o messenger.o stdlib.o debug.o timer.o -lbwio -lgcc

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
	-rm -f *.elf *.o tests/*.o *.out main.map tests/*.s `find ./*.s ! -name context_switch.s`
