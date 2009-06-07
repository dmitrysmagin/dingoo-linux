#
# Copyright (C) 2006 Ingenic Semiconductor Inc.
#
# Makefile for the JZ4740 internal bootrom code.
#

CROSS	:= mipsel-linux-

CFLAGS	:= -Wall -O2 -mips32 -fno-pic -mno-abicalls
LDFLAGS	:= -nostdlib -EL -T target.ld 

OBJS	= head.o main.o board.o serial.o delay.o nand.o

all: hwinit.bin

hwinit.bin: hwinit.elf
	$(CROSS)objcopy -O binary $< $@
	$(CROSS)objdump -D $< > hwinit.dump
	$(CROSS)objdump -h $< > hwinit.map
	$(CROSS)nm -n $< > System.map

hwinit.elf: $(OBJS)
	$(CROSS)ld $(LDFLAGS) $(OBJS) -o $@
.c.o:
	$(CROSS)gcc $(CFLAGS) -c $< -o $@
.S.o:
	$(CROSS)gcc $(CFLAGS) -c $< -o $@
clean:
	rm -fr *.o *.elf *.bin *.dump *.map
