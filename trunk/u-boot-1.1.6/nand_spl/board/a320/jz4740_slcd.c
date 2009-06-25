/*
 * jz4740_slcd.c -- Ingenic On-Chip SLCD simple setup
 *
 * Copyright (C) 2005-2007, Ingenic Semiconductor Inc.
 * Copyright (C) 2009       Ignacio Garcia Perez <iggarpe@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <config.h>

#ifdef CONFIG_JZ_SLCD

#include <common.h>
#include <asm/jz4740.h>

extern void mdelay (unsigned long ms);
extern unsigned char logo [];
extern int lzw_decode (void *temp, const void *src, void *dst);

#define DELAY_FLAG		0x8000

struct slcd_reg_pair {
	unsigned short	index;
	unsigned short	value;
};

/*
 * SLCD related GPIO pins for the Dingoo A320
 */

#define PIN_BKLIGHT	(32*3+31)	/* Port 3 pin 31: Backlight PWM  */
#define PIN_RS_N	(32*2+19)	/* Port 2 pin 19: RS# (register select, active low) */
#define PIN_CS_N	(32*1+17)	/* Port 1 pin 17: CS# (chip select, active low) */
#define PIN_RESET_N	(32*1+18)	/* Port 1 pin 18: RESET# (reset, active low) */

/*
 * Dingoo A320 IL9325 specific stuff.
 * Reverse engineered from A320_PD27_ILI9325_RLS.DL
 * found in the first released unbricking tool.
 * Only 16 bit bus supported.
 */

#ifdef CONFIG_JZ_SLCD_A320_ILI9325

static const struct slcd_reg_pair slcd_reg_init_table [] = {
	{ 0xE3, 0x3008 },
	{ 0xE7, 0x0012 },
	{ 0xEF, 0x1231 },
	{ 0x01, 0x0100 },
	{ 0x02, 0x0700 },
	{ 0x03, 0x1098 },
	{ 0x04, 0x0000 },
	{ 0x08, 0x0207 },
	{ 0x09, 0x0000 },
	{ 0x0A, 0x0000 },
	{ 0x0C, 0x0000 },
	{ 0x0D, 0x0000 },
	{ 0x0F, 0x0000 },
	{ 0x10, 0x0000 },
	{ 0x11, 0x0007 },
	{ 0x12, 0x0000 },
	{ 0x13, 0x0000 },
	{ DELAY_FLAG | 100 },
	{ 0x10, 0x1290 },
	{ 0x11, 0x0227 },
	{ DELAY_FLAG | 20 },
	{ 0x12, 0x001B },
	{ DELAY_FLAG | 20 },
	{ 0x13, 0x0500 },
	{ 0x29, 0x000C },
	{ 0x2B, 0x000D },
	{ DELAY_FLAG | 20 },
	{ 0x20, 0x0000 },
	{ 0x21, 0x0000 },
	{ 0x30, 0x0000 },
	{ 0x31, 0x0204 },
	{ 0x32, 0x0200 },
	{ 0x35, 0x0007 },
	{ 0x36, 0x1404 },
	{ 0x37, 0x0705 },
	{ 0x38, 0x0305 },
	{ 0x39, 0x0707 },
	{ 0x3C, 0x0701 },
	{ 0x3D, 0x000E },
	{ 0x50, 0x0000 },
	{ 0x51, 0x00EF },
	{ 0x52, 0x0000 },
	{ 0x53, 0x013F },
	{ 0x60, 0xA700 },
	{ 0x61, 0x0001 },
	{ 0x6A, 0x0000 },
	{ 0x80, 0x0000 },
	{ 0x81, 0x0000 },
	{ 0x82, 0x0000 },
	{ 0x83, 0x0000 },
	{ 0x84, 0x0000 },
	{ 0x85, 0x0000 },
	{ 0x90, 0x0010 },
	{ 0x92, 0x0600 },
	{ 0x07, 0x0133 },
	{ DELAY_FLAG | 20 },
	{ DELAY_FLAG }		/* End of table marker */
};

#endif /* CONFIG_JZ_SLCD_A320_ILI9325 */

/*
 * Dingoo A320 IL9331 specific stuff.
 * Reverse engineered from CCPMP_CFG_A320_LCM_FAIR_ILI9331_320_240.DL
 * found in the second released unbricking tool.
 * Only 16 bit bus supported.
 */

#ifdef CONFIG_JZ_SLCD_A320_ILI9331

static const struct slcd_reg_pair slcd_reg_init_table [] = {
	{ 0xE7, 0x1014 },
	{ 0x01, 0x0000 },
	{ 0x02, 0x0200 },
	{ 0x03, 0x1048 },
	{ 0x08, 0x0202 },
	{ 0x09, 0x0000 },
	{ 0x0A, 0x0000 },
	{ 0x0C, 0x0000 },
	{ 0x0D, 0x0000 },
	{ 0x0F, 0x0000 },
	{ 0x10, 0x0000 },
	{ 0x11, 0x0007 },
	{ 0x12, 0x0000 },
	{ 0x13, 0x0000 },
	{ DELAY_FLAG | 100 },
	{ 0x10, 0x1690 },
	{ 0x11, 0x0224 },
	{ DELAY_FLAG | 20 },
	{ 0x12, 0x001F },
	{ DELAY_FLAG | 20 },
	{ 0x13, 0x0500 },
	{ 0x29, 0x000C },
	{ 0x2B, 0x000D },
	{ DELAY_FLAG | 20 },
	{ 0x30, 0x0000 },
	{ 0x31, 0x0106 },
	{ 0x32, 0x0000 },
	{ 0x35, 0x0204 },
	{ 0x36, 0x160A },
	{ 0x37, 0x0707 },
	{ 0x38, 0x0106 },
	{ 0x39, 0x0706 },
	{ 0x3C, 0x0402 },
	{ 0x3D, 0x0C0F },
	{ 0x50, 0x0000 },
	{ 0x51, 0x00EF },
	{ 0x52, 0x0000 },
	{ 0x53, 0x013F },
	{ 0x20, 0x0000 },
	{ 0x21, 0x0000 },
	{ 0x60, 0x2700 },
	{ 0x61, 0x0001 },
	{ 0x6A, 0x0000 },
	{ 0x80, 0x0000 },
	{ 0x81, 0x0000 },
	{ 0x82, 0x0000 },
	{ 0x83, 0x0000 },
	{ 0x84, 0x0000 },
	{ 0x85, 0x0000 },
	{ 0x20, 0x00EF },
	{ 0x21, 0x0190 },
	{ 0x90, 0x0010 },
	{ 0x92, 0x0600 },
	{ 0x07, 0x0133 },
	{ DELAY_FLAG | 20 },
	{ DELAY_FLAG }		/* End of table marker */
};

#endif /* CONFIG_JZ_SLCD_A320_ILI9331 */

/*
 * Write just a command
 */

static void slcd_cmd_write (unsigned long cmd)
{
	__gpio_clear_pin(PIN_RS_N);

	REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | (cmd & 0xffff);
	while (REG_SLCD_STATE & SLCD_STATE_BUSY);

	__gpio_set_pin(PIN_RS_N);
}

/*
 * Write just data
 */
static void slcd_data_write (unsigned long data) {
	REG_SLCD_DATA = SLCD_DATA_RS_DATA | (data & 0xffff);
	while (REG_SLCD_STATE & SLCD_STATE_BUSY);
}

/*
 * Initialize LCD registers from a table.
 */

static void slcd_init_regs (const struct slcd_reg_pair *p)
{
	unsigned long delay;
	for (;; p++) {
		if (p->index & DELAY_FLAG) {		/* If flag set, this is a delay */
			delay = p->index & ~DELAY_FLAG;
			if (delay == 0) break;		/* Zero delay means end of table */
			mdelay(delay);			
		} else {
			slcd_cmd_write(p->index);
			slcd_data_write(p->value);
		}
	}
}

static void slcd_hw_init (void)
{
	unsigned int val = 0;
	unsigned int pclk;
	int pll_div;

	/* Setting Control register */
	REG_LCD_CFG &= ~LCD_CFG_LCDPIN_MASK;
	REG_LCD_CFG |= LCD_CFG_LCDPIN_SLCD;

	/* Configure SLCD module for initialize smart lcd registers*/
	REG_SLCD_CFG = SLCD_CFG_BURST_8_WORD | SLCD_CFG_DWIDTH_16
		| SLCD_CFG_CWIDTH_16BIT | SLCD_CFG_CS_ACTIVE_LOW
		| SLCD_CFG_RS_CMD_LOW | SLCD_CFG_CLK_ACTIVE_FALLING
		| SLCD_CFG_TYPE_PARALLEL;

	REG_GPIO_PXFUNS(2) = 0x0014FFFF;	/* Dingoo A320 is a bit freaky */
	REG_GPIO_PXSELC(2) = 0x0014FFFF;	/* (don't use __gpio_as_lcd_16bit) */
	REG_GPIO_PXPES(2)  = 0x0014FFFF;

	REG_SLCD_CTRL = SLCD_CTRL_DMA_EN;

	/* Timing setting */
	__cpm_stop_lcd();

	pclk = 16000000;	/* Pixclk */

	pll_div = (REG_CPM_CPCCR & CPM_CPCCR_PCS);	/* clock source,0:pllout/2 1: pllout */
	pll_div = pll_div ? 1 : 2 ;
	val = (__cpm_get_pllout() / pll_div) / pclk;
	val--;
	if (val > 0x1ff)
		val = 0x1ff;

	__cpm_set_pixdiv(val);

	REG_CPM_CPCCR |= CPM_CPCCR_CE ; /* Update divide */

	__cpm_start_lcd();
	mdelay(10);

	/* Initialize LCD GPIO pins */
	__gpio_as_output(PIN_RS_N);
	__gpio_set_pin(PIN_RS_N);
	__gpio_as_output(PIN_CS_N);
	__gpio_set_pin(PIN_CS_N);
	__gpio_as_output(PIN_RESET_N);
	__gpio_clear_pin(PIN_RESET_N); mdelay(10);
	__gpio_set_pin(PIN_RESET_N); mdelay(10);
	__gpio_clear_pin(PIN_CS_N);

	slcd_init_regs(slcd_reg_init_table);
	slcd_cmd_write(0x22);

	/* Configure SLCD module for transfer data to smart LCD GRAM*/
	REG_SLCD_CFG &= ~SLCD_CFG_DWIDTH_MASK;
	REG_SLCD_CFG |= SLCD_CFG_DWIDTH_16;
}

void slcd_init (void)
{
	unsigned char *temp = (void *)0x80100000;
	unsigned char *dst  = (void *)0x80200000;
	int len, i; unsigned long c, r, g, b;

	slcd_hw_init();

	len = lzw_decode(temp, logo, dst);

	for (i = 0; i < len; i++) {
		c = *dst++;
		r = c >> 3;
		g = c >> 3;
		b = c >> 3;
		slcd_data_write((r << 11) | (g << 6) | b);
	}

	/* Enable backlight */
	__gpio_as_output(PIN_BKLIGHT);
	__gpio_set_pin(PIN_BKLIGHT);
}

#endif /* CONFIG_JZ_SLCD */
 
