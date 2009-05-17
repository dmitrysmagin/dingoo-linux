/*
 *  Copyright (C) 2009 Ignacio Garcia Perez <iggarpe@gmail.com>
 *
 *  Author: <iggarpe@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "jz4740.h"
#include "configs.h"
#include "board.h"
#include "serial.h"
#include "delay.h"

volatile u32 CPU_ID;
volatile u32 UART_BASE;
volatile u32 CONFIG_BAUDRATE;
volatile u8 SDRAM_BW16;
volatile u8 SDRAM_BANK4;
volatile u8 SDRAM_ROW;
volatile u8 SDRAM_COL;
volatile u8 CONFIG_MOBILE_SDRAM;
volatile u32 CFG_CPU_SPEED;
volatile u32 CFG_EXTAL;
volatile u8 PHM_DIV;
volatile u8 IS_SHARE;

void test_load_args(void)
{
	CPU_ID			= 0x4740 ;
	CFG_EXTAL		= 12000000 ;
	CFG_CPU_SPEED		= 336000000 ;
	PHM_DIV			= 4;
	UART_BASE		= UART0_BASE;
	CONFIG_BAUDRATE		= 57600;
	SDRAM_BW16		= 0;
	SDRAM_BANK4		= 1;
	SDRAM_ROW		= 12;
	SDRAM_COL		= 9;
	CONFIG_MOBILE_SDRAM	= 0;
	IS_SHARE		= 1;
}

void work (void);

void c_main(void)
{
	test_load_args();

	gpio_init();
	pll_init();
	serial_init();
	sdram_init();

	serial_puts("A320 hwinit by Ignacio Garcia Perez <iggarpe@gmail.com>\n");
}

