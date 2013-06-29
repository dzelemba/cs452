#
# Makefile for busy-wait IO tests
#
XCC = ./colorgcc
AS = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/as
LD = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ld
CFLAGS  = -O2 -c -fPIC -Wall -I. -mcpu=arm920t -msoft-float -fno-builtin

# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -Map main.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L../lib

ifdef TEST
  CFLAGS += -DTEST
  CFLAGS += -DDEBUG_1
  OBJECT_DIR = test
else
ifdef DBG
  CFLAGS += -DDEBUG
  OBJECT_DIR = dbg
else
ifdef DBG3
  CFLAGS += -DDEBUG_3
  OBJECT_DIR = dbg3
else
ifdef DBG2
  CFLAGS += -DDEBUG_2
  OBJECT_DIR = dbg2
else
ifdef DBG1
  CFLAGS += -DDEBUG_1
  OBJECT_DIR = dbg1
else
  OBJECT_DIR = obj
endif
endif
endif
endif
endif

TEST_SRC_FILES = $(wildcard tests/*.c)
TEST_OBJ_FILES = $(addprefix $(OBJECT_DIR)/,$(TEST_SRC_FILES:.c=.o))
KERNEL_SRC_FILES = $(wildcard *.c)
KERNEL_OBJ_FILES = $(addprefix $(OBJECT_DIR)/,$(KERNEL_SRC_FILES:.c=.o) context_switch.o tests.o)

.PHONY: dbg dbg1 dbg2 dbg3 test clean

all: stuff

stuff: export RTOS_COMPILER=/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc
stuff: directories main.elf

directories:
	mkdir -p obj/tests dbg/tests dbg1/tests dbg2/tests dbg3/tests test/tests

dbg:
	$(MAKE) install DBG=1

dbg1:
	$(MAKE) install DBG1=1

dbg2:
	$(MAKE) install DBG2=1

dbg3:
	$(MAKE) install DBG3=1

test:
	$(MAKE) install TEST=1

obj: install

# Assembly Sources

$(OBJECT_DIR)/context_switch.o: context_switch.s
	$(AS) $(ASFLAGS) -o $(OBJECT_DIR)/context_switch.o context_switch.s

# Kernel Sources

$(OBJECT_DIR)/%.o: %.c
	$(XCC) -S $(CFLAGS) $< -o $(OBJECT_DIR)/$(<:.c=.s)
	$(AS) $(ASFLAGS) -o $@ $(OBJECT_DIR)/$(<:.c=.s)

# Test Sources

$(OBJECT_DIR)/tests.o: $(TEST_OBJ_FILES)
	$(LD) -r $(LDFLAGS) -o $@ $^

tests/%.o: tests/%.c
	$(XCC) -S $(CFLAGS) $< -o $(OBJECT_DIR)/$(<:.c=.s)
	$(AS) $(ASFLAGS) -o $(OBJECT_DIR)/$@ $(OBJECT_DIR)/$(<:.c=.s)

main.elf: $(KERNEL_OBJ_FILES)
	$(LD) $(LDFLAGS) -o $(OBJECT_DIR)/$@ $(KERNEL_OBJ_FILES) -lgcc

install: stuff
	cp $(OBJECT_DIR)/main.elf /u/cs452/tftp/ARM/`id -u -n`/

# Unit Tests

TESTFLAGS = -g -Wall -DUNIT

unit: test.out
	./test.out

test.out: export RTOS_COMPILER=/usr/bin/gcc
test.out: unittests/* strings.* linked_array.* stdlib.* bitmask.* heap.*
	$(XCC) $(TESTFLAGS) unittests/all_tests.c unittests/test_helpers.c strings.c linked_array.c stdlib.c heap.c bitmask.c -o test.out

clean:
	rm -rf obj dbg dbg1 dbg2 dbg3 test test.out
