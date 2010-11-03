/*
 *  Copyright (C) 2009 Ignacio Garcia Perez <iggarpe@gmail.com>
 *
 *  Author: <iggarpe@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "config.h"	/* Always first, defines CFG_EXTAL for jz4740.h */
#include "jz4740.h"

#include "board.h"
#include "serial.h"
#include "delay.h"
#include "jz4740_slcd.h"

void c_main(void)
{
	gpio_init();
	serial_init();
	pll_init();
	sdram_init();

	mdelay(10);

	serial_puts("A320 hwinit by Ignacio Garcia Perez <iggarpe@gmail.com>\n");

	slcd_init();
}

