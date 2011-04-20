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
#define DATA_COUNT_MSK	0x7f00

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

static const unsigned short slcd_reg_init_table [] = {
	0x01E3, 0x3008,
	0x01E7, 0x0012,
	0x01EF, 0x1231,
	0x0101, 0x0100,
	0x0102, 0x0700,
	0x0103, 0x1098,
	0x0104, 0x0000,
	0x0108, 0x0207,
	0x0109, 0x0000,
	0x010A, 0x0000,
	0x010C, 0x0000,
	0x010D, 0x0000,
	0x010F, 0x0000,
	0x0110, 0x0000,
	0x0111, 0x0007,
	0x0112, 0x0000,
	0x0113, 0x0000,
	DELAY_FLAG | 100,
	0x0110, 0x1290,
	0x0111, 0x0227,
	DELAY_FLAG | 20,
	0x0112, 0x001B,
	DELAY_FLAG | 20,
	0x0113, 0x0500,
	0x0129, 0x000C,
	0x012B, 0x000D,
	DELAY_FLAG | 20,
	0x0120, 0x0000,
	0x0121, 0x0000,
	0x0130, 0x0000,
	0x0131, 0x0204,
	0x0132, 0x0200,
	0x0135, 0x0007,
	0x0136, 0x1404,
	0x0137, 0x0705,
	0x0138, 0x0305,
	0x0139, 0x0707,
	0x013C, 0x0701,
	0x013D, 0x000E,
	0x0150, 0x0000,
	0x0151, 0x00EF,
	0x0152, 0x0000,
	0x0153, 0x013F,
	0x0160, 0xA700,
	0x0161, 0x0001,
	0x016A, 0x0000,
	0x0180, 0x0000,
	0x0181, 0x0000,
	0x0182, 0x0000,
	0x0183, 0x0000,
	0x0184, 0x0000,
	0x0185, 0x0000,
	0x0190, 0x0010,
	0x0192, 0x0600,
	DELAY_FLAG | 20,
	0x0107, 0x0133,
	DELAY_FLAG | 20,
	0x0022,
	DELAY_FLAG		/* End of table marker */
};

#endif /* CONFIG_JZ_SLCD_A320_ILI9325 */

/*
 * Dingoo A320 IL9331 specific stuff.
 * Reverse engineered from CCPMP_CFG_A320_LCM_FAIR_ILI9331_320_240.DL
 * found in the second released unbricking tool.
 * Only 16 bit bus supported.
 */

#ifdef CONFIG_JZ_SLCD_A320_ILI9331

static const unsigned short slcd_reg_init_table [] = {
	0x01E7, 0x1014,
	0x0101, 0x0000,
	0x0102, 0x0200,
	0x0103, 0x1048,
	0x0108, 0x0202,
	0x0109, 0x0000,
	0x010A, 0x0000,
	0x010C, 0x0000,
	0x010D, 0x0000,
	0x010F, 0x0000,
	0x0110, 0x0000,
	0x0111, 0x0007,
	0x0112, 0x0000,
	0x0113, 0x0000,
	DELAY_FLAG | 100,
	0x0110, 0x1690,
	0x0111, 0x0224,
	DELAY_FLAG | 20,
	0x0112, 0x001F,
	DELAY_FLAG | 20,
	0x0113, 0x0500,
	0x0129, 0x000C,
	0x012B, 0x000D,
	DELAY_FLAG | 20,
	0x0130, 0x0000,
	0x0131, 0x0106,
	0x0132, 0x0000,
	0x0135, 0x0204,
	0x0136, 0x160A,
	0x0137, 0x0707,
	0x0138, 0x0106,
	0x0139, 0x0706,
	0x013C, 0x0402,
	0x013D, 0x0C0F,
	0x0150, 0x0000,
	0x0151, 0x00EF,
	0x0152, 0x0000,
	0x0153, 0x013F,
	0x0120, 0x0000,
	0x0121, 0x0000,
	0x0160, 0x2700,
	0x0161, 0x0001,
	0x016A, 0x0000,
	0x0180, 0x0000,
	0x0181, 0x0000,
	0x0182, 0x0000,
	0x0183, 0x0000,
	0x0184, 0x0000,
	0x0185, 0x0000,
	0x0120, 0x00EF,
	0x0121, 0x0190,
	0x0190, 0x0010,
	0x0192, 0x0600,
	DELAY_FLAG | 20,
	0x0107, 0x0133,
	DELAY_FLAG | 20,
	0x0022,
	DELAY_FLAG		/* End of table marker */
};

#endif /* CONFIG_JZ_SLCD_A320_ILI9331 */


/*
 * Dingoo A320 IL9338 specific stuff. Provided by ChinaChip.
 */

#ifdef CONFIG_JZ_SLCD_A320_ILI9338

static const unsigned short slcd_reg_init_table [] = {
	0x0011,
	DELAY_FLAG | 100,
	0x01cb, 0x0001,
	0x02c0, 0x0026, 0x0001,
	0x01c1, 0x0010,
	0x02c5, 0x0010, 0x0052,
	0x0126, 0x0001,
	0x0fe0, 0x0010, 0x0010, 0x0010, 0x0008, 0x000e, 0x0006, 0x0042, 0x0028, 0x0036, 0x0003, 0x000e, 0x0004, 0x0013, 0x000e, 0x000c,
	0x0fe1, 0x000c, 0x0023, 0x0026, 0x0004, 0x000c, 0x0004, 0x0039, 0x0024, 0x004b, 0x0003, 0x000b, 0x000b, 0x0033, 0x0037, 0x000f,
	0x042a, 0x0000, 0x0000, 0x0001, 0x003f,
	0x042b, 0x0000, 0x0000, 0x0000, 0x00ef,
	0x0136, 0x00e8,
	0x013a, 0x0005,
	0x0029,
	0x002c,
	DELAY_FLAG		/* End of table marker */
};

#endif /* CONFIG_JZ_SLCD_A320_ILI9338 */

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

static void slcd_init_regs (const unsigned short *p)
{
	unsigned long delay, data_cnt, tmp;
	while (1) {
		tmp = *p++;
		if (tmp & DELAY_FLAG) {		/* If flag set, this is a delay */
			delay = tmp & ~DELAY_FLAG;
			if (0 == delay) break;		/* Zero delay means end of table */
			mdelay(delay);
		} else {
			data_cnt = (tmp & DATA_COUNT_MSK) >> 8;
			slcd_cmd_write(tmp & 0xff);
			while (data_cnt--) slcd_data_write(*p++);
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
	__gpio_as_output(PIN_BKLIGHT);
	__gpio_clear_pin(PIN_BKLIGHT);
	__gpio_as_output(PIN_RS_N);
	__gpio_set_pin(PIN_RS_N);
	__gpio_as_output(PIN_CS_N);
	__gpio_set_pin(PIN_CS_N);
	__gpio_as_output(PIN_RESET_N);
	__gpio_clear_pin(PIN_RESET_N); mdelay(10);
	__gpio_set_pin(PIN_RESET_N); mdelay(10);
	__gpio_clear_pin(PIN_CS_N);

	slcd_init_regs(slcd_reg_init_table);

	/* Configure SLCD module for transfer data to smart LCD GRAM*/
	REG_SLCD_CFG &= ~SLCD_CFG_DWIDTH_MASK;
	REG_SLCD_CFG |= SLCD_CFG_DWIDTH_16;
}

void slcd_off (void)
{
	__gpio_clear_pin(PIN_BKLIGHT); mdelay(10);
	__gpio_clear_pin(PIN_RESET_N);
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

	mdelay(10);

	__gpio_set_pin(PIN_BKLIGHT);	/* Enable backlight */
}

#endif /* CONFIG_JZ_SLCD */
 
