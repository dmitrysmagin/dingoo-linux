#
# Copyright (C) 2006 Ingenic Semiconductor Inc.
#
# Makefile for the JZ4740 internal bootrom code.
#

CROSS	:= mipsel-linux-

CFLAGS	:= -Wall -O2 -mips32 -fno-pic -mno-abicalls
LDFLAGS	:= -nostdlib -EL -T target.ld 

OBJS	= head.o main.o board.o serial.o delay.o

all: hwinit.bin

hwinit.bin: hwinit.elf

hwinit.elf: $(OBJS)
	$(CROSS)ld $(LDFLAGS) $(OBJS) -o $@

%.bin: %.elf
	$(CROSS)objcopy -O binary $< $@
	$(CROSS)objdump -D $< > $(basename $@).dump
	$(CROSS)objdump -h $< > $(basename $@).map
	$(CROSS)nm -n $< > System.map

%.o: %.c
	$(CROSS)gcc $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CROSS)gcc $(CFLAGS) -c $< -o $@

clean:
	rm -fr *.o *.elf *.bin *.dump *.map

