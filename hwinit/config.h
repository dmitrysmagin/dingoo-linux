#ifndef _CONFIG_H
#define _CONFIG_H

/*
 * Failsafe SDRAM configuration values
 *
 * If you want to live on the edge, the Dingoo Hynix HY57V281620FTP-6
 * chips should work with these accoring to the datasheet:
 *
 *   TRAS 42
 *   RCD  18
 *   TPC  18
 *   
 */

#define CONFIG_NR_DRAM_BANKS	1  /* SDRAM BANK Number: 1, 2*/
#define SDRAM_CASL		2	/* CAS latency: 2 or 3 */
#define SDRAM_TRAS		45	/* RAS# Active Time (ns) */
#define SDRAM_RCD		20	/* RAS# to CAS# Delay (ns) */
#define SDRAM_TPC		20	/* RAS# Precharge Time (ns) */
#define SDRAM_TRWL		7	/* Write Latency Time (ns) */
#define SDRAM_TREF	        15625	/* Refresh period (ns): 4096 refresh cycles/64ms */

#define CPU_ID			0x4740
#define CFG_EXTAL		12000000
#define CFG_CPU_SPEED		336000000
#define PHM_DIV			4
#define UART_BASE		UART0_BASE
#define CONFIG_BAUDRATE		57600
#define SDRAM_BW16		0
#define SDRAM_BANK4		1
#define SDRAM_ROW		12
#define SDRAM_COL		9
#define CONFIG_MOBILE_SDRAM	0
#define IS_SHARE		1

#endif

