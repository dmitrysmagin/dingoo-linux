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
#include "config.h"
#include "board.h"
#include "serial.h"
#include "delay.h"

void c_main(void)
{
	int bsel;

	gpio_init();
	pll_init();
	serial_init();
	sdram_init();

	bsel = __gpio_get_pin(BOOT_SELECT_PIN);	

	delay(10);

	serial_puts("A320 hwinit by Ignacio Garcia Perez <iggarpe@gmail.com>\n");
}

