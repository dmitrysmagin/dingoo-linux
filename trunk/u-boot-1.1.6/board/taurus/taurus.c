/*
 * (C) Copyright 2005
 * Ingenic Semiconductor, <jlwei@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/jz5730.h>

static void gpio_init(void)
{
	/* board led pins */
	__gpio_as_uart0();
	__gpio_as_uart1();
	__gpio_as_emc(0);
	__gpio_as_dma();
	__gpio_as_eth();
	__gpio_as_pci();
	__gpio_as_usbclk();

	__gpio_as_output(67);
	__gpio_as_output(68);
	__gpio_as_output(69);
	__gpio_as_output(70);
}

//----------------------------------------------------------------------
// board early init routine

void board_early_init(void)
{
	gpio_init();
}

void board_led(char ch)
{
	REG32(0xa8000000) = (ch << 24) | (ch << 16) | (ch << 8) | ch;
}

//----------------------------------------------------------------------
// U-Boot common routines

int checkboard(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	printf("Board: Ingenic Taurus (CPU Speed %d MHz)\n", gd->cpu_clk/1000000);

	return 0; /* success */
}

