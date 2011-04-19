#
# Copyright (C) 2006 Ingenic Semiconductor Inc.
#
# Makefile for the JZ4740 internal bootrom code.
#

CROSS	:= mipsel-linux-

CFLAGS	:= -Wall -O2 -mips32 -fno-pic -mno-abicalls
LDFLAGS	:= -nostdlib -EL -T target.ld 

OBJS	= head.o main.o board.o serial.o delay.o lzw.o logo.o

all: hwinit_ILI9325.bin hwinit_ILI9331.bin hwinit_ILI9338.bin



hwinit_ILI9325.elf: $(OBJS) jz4740_slcd_ILI9325.o
	$(CROSS)ld $(LDFLAGS) $^ -o $@

hwinit_ILI9331.elf: $(OBJS) jz4740_slcd_ILI9331.o
	$(CROSS)ld $(LDFLAGS) $^ -o $@

hwinit_ILI9338.elf: $(OBJS) jz4740_slcd_ILI9338.o
	$(CROSS)ld $(LDFLAGS) $^ -o $@



jz4740_slcd_ILI9325.o: jz4740_slcd.c
	$(CROSS)gcc -DCONFIG_JZ_SLCD_A320_ILI9325 $(CFLAGS) -c $< -o $@

jz4740_slcd_ILI9331.o: jz4740_slcd.c
	$(CROSS)gcc -DCONFIG_JZ_SLCD_A320_ILI9331 $(CFLAGS) -c $< -o $@

jz4740_slcd_ILI9338.o: jz4740_slcd.c
	$(CROSS)gcc -DCONFIG_JZ_SLCD_A320_ILI9338 $(CFLAGS) -c $< -o $@



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

