/*
 * Copyright (C) 2007 Ingenic Semiconductor Inc.
 * Copyright (C) 2009 Ignacio Garcia Perez
 *
 * Author:   Peter <jlwei@ingenic.cn>
 * Modified: Ignacio Garcia Perez <iggarpe@gmail.com>
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
#include <nand.h>

#include <asm/io.h>
#include <asm/jz4740.h>

/*
 * Image loading parameters
 */

typedef void (*image_start_t)(int);

int		image_weird_ecc;
int		image_offs;
int		image_size;
uchar *		image_dst;
image_start_t	image_start;

/*
 * NAND flash definitions
 */

#define NAND_DATAPORT	0xb8000000
#define NAND_ADDRPORT	0xb8010000
#define NAND_COMMPORT	0xb8008000

#define ECC_BLOCK	512
#define ECC_POS		6
#define PAR_SIZE	9

#define __nand_enable()		(REG_EMC_NFCSR |= EMC_NFCSR_NFE1 | EMC_NFCSR_NFCE1)
#define __nand_disable()	(REG_EMC_NFCSR &= ~(EMC_NFCSR_NFCE1))
#define __nand_ecc_rs_encoding() \
	(REG_EMC_NFECR = EMC_NFECR_ECCE | EMC_NFECR_ERST | EMC_NFECR_RS | EMC_NFECR_RS_ENCODING)
#define __nand_ecc_rs_decoding() \
	(REG_EMC_NFECR = EMC_NFECR_ECCE | EMC_NFECR_ERST | EMC_NFECR_RS | EMC_NFECR_RS_DECODING)
#define __nand_ecc_disable()	(REG_EMC_NFECR &= ~EMC_NFECR_ECCE)
#define __nand_ecc_encode_sync() while (!(REG_EMC_NFINTS & EMC_NFINTS_ENCF))
#define __nand_ecc_decode_sync() while (!(REG_EMC_NFINTS & EMC_NFINTS_DECF))

static inline void __nand_dev_ready(void)
{
	unsigned int timeout = 10000;
	while ((REG_GPIO_PXPIN(2) & 0x40000000) && timeout--);
	while (!(REG_GPIO_PXPIN(2) & 0x40000000));
}

#define __nand_cmd(n)		(REG8(NAND_COMMPORT) = (n))
#define __nand_addr(n)		(REG8(NAND_ADDRPORT) = (n))
#define __nand_data8()		REG8(NAND_DATAPORT)
#define __nand_data16()		REG16(NAND_DATAPORT)

/*
 * NAND flash parameters
 */

#if (JZ4740_NANDBOOT_CFG == JZ4740_NANDBOOT_B8R3)
#define CFG_NAND_BUS_WIDTH	8
#define CFG_NAND_ROW_CYCLE	3
#endif

#if (JZ4740_NANDBOOT_CFG == JZ4740_NANDBOOT_B8R2)
#define CFG_NAND_BUS_WIDTH	8
#define CFG_NAND_ROW_CYCLE	2
#endif

#if (JZ4740_NANDBOOT_CFG == JZ4740_NANDBOOT_B16R3)
#define CFG_NAND_BUS_WIDTH	16
#define CFG_NAND_ROW_CYCLE	3
#endif

#if (JZ4740_NANDBOOT_CFG == JZ4740_NANDBOOT_B16R2)
#define CFG_NAND_BUS_WIDTH	16
#define CFG_NAND_ROW_CYCLE	2
#endif

#if (CFG_NAND_PAGE_SIZE == 512)
#define CFG_NAND_BAD_BLOCK_POS	5
#else
#define CFG_NAND_BAD_BLOCK_POS	0
#endif

#define CFG_NAND_PAGE_PER_BLOCK	(CFG_NAND_BLOCK_SIZE / CFG_NAND_PAGE_SIZE)
#define CFG_NAND_OOB_SIZE	(CFG_NAND_PAGE_SIZE / 32)
#define CFG_NAND_ECC_COUNT	(CFG_NAND_PAGE_SIZE / ECC_BLOCK)

/*
 * External routines
 */

extern void flush_cache_all(void);
extern int serial_init(void);
extern void serial_puts(const char *s);
extern void sdram_init(void);
extern void pll_init(void);
extern void slcd_init(void);
extern void slcd_off(void);
extern void mdelay(unsigned int ms);

/*
 * NAND flash routines
 */

#if (CFG_NAND_BUS_WIDTH == 16)
static inline void nand_read_buf(void *buf, int count)
{
	int i;
	u16 *p = (u16 *)buf;

	for (i = 0; i < count; i += 2)
		*p++ = __nand_data16();
}
#endif

#if (CFG_NAND_BUS_WIDTH == 8)
static inline void nand_read_buf(void *buf, int count)
{
	int i;
	u8 *p = (u8 *)buf;

	for (i = 0; i < count; i++)
		*p++ = __nand_data8();
}
#endif

/* Correct 1~9-bit errors in 512-bytes data */
static void rs_correct(unsigned char *dat, int idx, int mask)
{
	int i;

	idx--;

	i = idx + (idx >> 3);
	if (i >= 512)
		return;

	mask <<= (idx & 0x7);

	dat[i] ^= mask & 0xff;
	if (i < 511)
		dat[i+1] ^= (mask >> 8) & 0xff;
}

static int nand_read_oob(int page_addr, uchar *buf, int size)
{
	int col_addr;
	if (CFG_NAND_PAGE_SIZE != 512)
		col_addr = CFG_NAND_PAGE_SIZE;
	else {
		col_addr = 0;
		__nand_dev_ready();
	}

	if (CFG_NAND_PAGE_SIZE != 512)
		/* Send READ0 command */
		__nand_cmd(NAND_CMD_READ0);
	else
		/* Send READOOB command */
		__nand_cmd(NAND_CMD_READOOB);

	/* Send column address */
	__nand_addr(col_addr & 0xff);
	if (CFG_NAND_PAGE_SIZE != 512)
		__nand_addr((col_addr >> 8) & 0xff);

	/* Send page address */
	__nand_addr(page_addr & 0xff);
	__nand_addr((page_addr >> 8) & 0xff);
	if (CFG_NAND_ROW_CYCLE == 3)
		__nand_addr((page_addr >> 16) & 0xff);

	/* Send READSTART command for 2048 or 4096 ps NAND */
	if (CFG_NAND_PAGE_SIZE != 512)
		__nand_cmd(NAND_CMD_READSTART);

	/* Wait for device ready */
	__nand_dev_ready();

	/* Read oob data */
	nand_read_buf(buf, size);
	if (CFG_NAND_PAGE_SIZE == 512)
		__nand_dev_ready();
	return 0;
}

static int nand_read_page(int page_addr, uchar *dst, uchar *oob_buf)
{
	uchar *databuf = dst, *tmpbuf;
	int i, j;

	/*
	 * Read oob data
	 */
	nand_read_oob(page_addr, oob_buf, CFG_NAND_OOB_SIZE);

	/*
	 * Read page data
	 */

	/* Send READ0 command */
	__nand_cmd(NAND_CMD_READ0);

	/* Send column address */
	__nand_addr(0);
	if (CFG_NAND_PAGE_SIZE != 512)
		__nand_addr(0);

	/* Send page address */
	__nand_addr(page_addr & 0xff);
	__nand_addr((page_addr >> 8) & 0xff);
	if (CFG_NAND_ROW_CYCLE == 3)
		__nand_addr((page_addr >> 16) & 0xff);

	/* Send READSTART command for 2048 or 4096 ps NAND */
	if (CFG_NAND_PAGE_SIZE != 512)
		__nand_cmd(NAND_CMD_READSTART);

	/* Wait for device ready */
	__nand_dev_ready();

	/* Read page data */
	tmpbuf = databuf;

	for (i = 0; i < CFG_NAND_ECC_COUNT; i++) {
		volatile unsigned char *paraddr = (volatile unsigned char *)EMC_NFPAR0;
		unsigned int stat;

		/* Enable RS decoding */
		REG_EMC_NFINTS = 0x0;
		__nand_ecc_rs_decoding();

		/* Read data */
		nand_read_buf((void *)tmpbuf, ECC_BLOCK);

		/* Set PAR values */
		for (j = 0; j < PAR_SIZE; j++) {
			if (image_weird_ecc) {
				/* Weird ECC positions for ChinaChip system loader */
				*paraddr++ = oob_buf[4 + i*12 + j];
			} else {
#if defined(CFG_NAND_ECC_POS)
				*paraddr++ = oob_buf[CFG_NAND_ECC_POS + i*PAR_SIZE + j];
#else
				*paraddr++ = oob_buf[ECC_POS + i*PAR_SIZE + j];
#endif
			}
		}

		/* Set PRDY */
		REG_EMC_NFECR |= EMC_NFECR_PRDY;

		/* Wait for completion */
		__nand_ecc_decode_sync();

		/* Disable decoding */
		__nand_ecc_disable();

		/* Check result of decoding */
		stat = REG_EMC_NFINTS;
		if (stat & EMC_NFINTS_ERR) {
			/* Error occurred */
//			serial_puts(" Error occurred\n");
			if (stat & EMC_NFINTS_UNCOR) {
				/* Uncorrectable error occurred */
//				serial_puts("Uncorrectable error occurred\n");
			}
			else {
				unsigned int errcnt, index, mask;

				errcnt = (stat & EMC_NFINTS_ERRCNT_MASK) >> EMC_NFINTS_ERRCNT_BIT;
				switch (errcnt) {
				case 4:
					index = (REG_EMC_NFERR3 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT;
					mask = (REG_EMC_NFERR3 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT;
					rs_correct(tmpbuf, index, mask);
					/* FALL-THROUGH */
				case 3:
					index = (REG_EMC_NFERR2 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT;
					mask = (REG_EMC_NFERR2 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT;
					rs_correct(tmpbuf, index, mask);
					/* FALL-THROUGH */
				case 2:
					index = (REG_EMC_NFERR1 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT;
					mask = (REG_EMC_NFERR1 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT;
					rs_correct(tmpbuf, index, mask);
					/* FALL-THROUGH */
				case 1:
					index = (REG_EMC_NFERR0 & EMC_NFERR_INDEX_MASK) >> EMC_NFERR_INDEX_BIT;
					mask = (REG_EMC_NFERR0 & EMC_NFERR_MASK_MASK) >> EMC_NFERR_MASK_BIT;
					rs_correct(tmpbuf, index, mask);
					break;
				default:
					break;
				}
			}
		}

		tmpbuf += ECC_BLOCK;
	}

	return 0;
}

#ifndef CFG_NAND_BADBLOCK_PAGE
#define CFG_NAND_BADBLOCK_PAGE 0 /* NAND bad block was marked at this page in a block, starting from 0 */
#endif

static void nand_load(void)
{
	int page, pagecopy_count;
	uchar oob_buf[CFG_NAND_OOB_SIZE];

	__nand_enable();

	page = image_offs / CFG_NAND_PAGE_SIZE;
	pagecopy_count = 0;
	while (pagecopy_count < (image_size / CFG_NAND_PAGE_SIZE)) {
		if (page % CFG_NAND_PAGE_PER_BLOCK == 0) {
			nand_read_oob(page + CFG_NAND_BADBLOCK_PAGE, oob_buf, CFG_NAND_OOB_SIZE);
			if (oob_buf[CFG_NAND_BAD_BLOCK_POS] != 0xff) {
				page += CFG_NAND_PAGE_PER_BLOCK;
				/* Skip bad block */
				continue;
			}
		}
		/* Load this page to dst, do the ECC */
		nand_read_page(page, image_dst, oob_buf);

		image_dst += CFG_NAND_PAGE_SIZE;
		page++;
		pagecopy_count++;
	}

	__nand_disable();
}

static void nand_init(void) {

 	/* Optimize the timing of nand */
	REG_EMC_SMCR1 = 0x094c4400;
}
static void gpio_init(void)
{
	/*
	 * Mute audio
	 */
	__gpio_as_output(GPIO_AUDIO_ENABLE);
	__gpio_clear_pin(GPIO_AUDIO_ENABLE);

	/*
	 * Initialize SDRAM pins
	 */
#if defined(CONFIG_JZ4720)
	__gpio_as_sdram_16bit_4720();
#elif defined(CONFIG_JZ4725)
	__gpio_as_sdram_16bit_4725();
#else
	__gpio_as_sdram_32bit();
#endif

	/*
	 * Initialize UART0 pins
	 *
	 * IMPORTANT: __gpio_as_uart0() disables the pull-ups,
	 * and we need to keep them enabled to avoid spurious
	 * detection of break condition or gargabe input when
	 * the serial RX line is not driven.
	 */
	REG_GPIO_PXFUNS(3) = 0x06000000;
	REG_GPIO_PXSELS(3) = 0x06000000;
	REG_GPIO_PXPEC(3) = 0x06000000;

	/*
	 * Initialize NAND pins
	 */
	__gpio_as_nand();
}

void nand_boot(void)
{
	int i;

	gpio_init();	/* Basic hardware initialization */
	serial_init();

	serial_puts("\nNAND SPL\n");

	pll_init();	/* More hardware initialization */
	sdram_init();
	slcd_init();
	nand_init();

	/* Defaults for system loader */
	image_weird_ecc = 1;
	image_offs = 0x00040000;
	image_size = 0x000C0000;
	image_dst = (uchar *)0x80E00000;
	image_start = (image_start_t)0x80E10008;

	/* One second wait loop */
	for (i = 0; i < 100; i++) {

		/* If boot selection key is pressed at any time, 
		 * configure for U-Boot loading and exit loop */
		if (!__gpio_get_pin(GPIO_BOOT_SELECT)) {
			image_weird_ecc = 0;
			image_offs = CFG_NAND_U_BOOT_OFFS;
			image_size = CFG_NAND_U_BOOT_SIZE;
			image_dst = (uchar *)CFG_NAND_U_BOOT_DST;
			image_start = (image_start_t)CFG_NAND_U_BOOT_START;
			break;
		}

		mdelay(10);
	}

	nand_load();				/* Load image */
	flush_cache_all();			/* Flush caches */

	if (image_weird_ecc) slcd_off();	/* Switch LCD off if system loader */
						/* (leave it on for U-Boot/linux */
	(*image_start)(0);
}

