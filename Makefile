# Make file for epOS by Ramon Fried
# Last modifcation: 21/1/2013

.PHONY: all clean clean_dep dist check testdrivers todolist cscope cscope_update

TOOLCHAIN_PATH = toolchain/i686-elf-4.9.1-Linux-x86_64/bin
CC	= $(TOOLCHAIN_PATH)/i686-elf-gcc
LD	= $(TOOLCHAIN_PATH)/i686-elf-ld
ASM	= nasm
WARNINGS := -Wall -Wextra 

CFLAGS := -g -std=gnu99 -nostdlib -nostartfiles -nodefaultlibs $(WARNINGS)

PROJDIRS := inc src

INCLUDES := -Iinc
SRCFILES := $(shell find $(PROJDIRS) -type f -name "*.c")
SRCFILES += $(shell find $(PROJDIRS) -type f -name "*.s")

OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
OBJFILES := $(patsubst %.s,%.o,$(OBJFILES))
DEPFILES := $(OBJFILES)

-include $(dep)   # include all dep files in the makefile

%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

all: kernel.bin

test:
	echo "Source files: "
	echo $(SRCFILES)
	echo "Object files: "
	echo $(OBJFILES)
#kernel.bin: src/kernel/kernel.o src/kernel/loader.o
kernel.bin: $(OBJFILES)
	$(LD) -T make/linker.ld -o $@ $^

kernel.img: kernel.bin
	dd if=/dev/zero of=pad bs=1 count=750
	cat make/stage1 make/stage2 pad $< > $@
	
clean:
	$(RM) $(OBJFILES) kernel.bin kernel.img

cleandep:
	rm -f $(dep)
.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@
	
.s.o:
	$(ASM) -f elf -o $@ $<

cscope:
	@echo "Preparing cscope tags"
	@find $(PROJDIRS) -name *.[csh] > cscope.files
	@cscope -b -q -k

cscope_update:
	@echo "Updating cscope tags"
	@cscope -b -q -k
