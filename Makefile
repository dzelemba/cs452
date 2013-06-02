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
TEST_OBJ_FILES = $(TEST_SRC_FILES:.c=.o)
KERNEL_SRC_FILES = $(wildcard *.c)
KERNEL_OBJ_FILES = $(KERNEL_SRC_FILES:.c=.o) context_switch.o tests.o

OBJECT_DIR = obj

all: stuff

stuff: directories main.elf

directories:
	mkdir -p obj/tests dbg/tests

dbg: CFLAGS += -DDEBUG
dbg: OBJECT_DIR = dbg
dbg: stuff

# Assembly Sources

context_switch.o: context_switch.s
	$(AS) $(ASFLAGS) -o $(OBJECT_DIR)/context_switch.o context_switch.s

# Kernel Sources

%.o: %.c
	$(XCC) -S $(CFLAGS) $< -o $(OBJECT_DIR)/$(<:.c=.s)
	$(AS) $(ASFLAGS) -o $(OBJECT_DIR)/$@ $(OBJECT_DIR)/$(<:.c=.s)

# Test Sources

tests.o: $(TEST_OBJ_FILES)
	$(LD) -r $(LDFLAGS) -o $(OBJECT_DIR)/tests.o $(addprefix $(OBJECT_DIR)/,$^)

tests/%.o: tests/%.c
	$(XCC) -S $(CFLAGS) $< -o $(OBJECT_DIR)/$(<:.c=.s)
	$(AS) $(ASFLAGS) -o $(OBJECT_DIR)/$@ $(OBJECT_DIR)/$(<:.c=.s)

main.elf: $(KERNEL_OBJ_FILES)
	$(LD) $(LDFLAGS) -o $(OBJECT_DIR)/$@ $(addprefix $(OBJECT_DIR)/,$(KERNEL_OBJ_FILES)) -lbwio -lgcc

install: main.elf
	cp $(OBJECT_DIR)/main.elf /u/cs452/tftp/ARM/dzelemba/

# Unit Tests

GCC = /usr/bin/gcc
TESTFLAGS = -g -Wall

unittests: test.out
	./test.out

test.out: unittests/* strings.* linked_array.* stdlib.*
	$(GCC) $(TESTFLAGS) unittests/all_tests.c unittests/test_helpers.c strings.c linked_array.c stdlib.c -o test.out

clean:
	rm -rf obj dbg test.out
