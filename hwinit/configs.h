#ifndef _CONFIGS_H
#define _CONFIGS_H

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

extern volatile u32 CPU_ID;
extern volatile u8 SDRAM_BW16;
extern volatile u8 SDRAM_BANK4;
extern volatile u8 SDRAM_ROW;
extern volatile u8 SDRAM_COL;
extern volatile u8 CONFIG_MOBILE_SDRAM;
extern volatile u32 CFG_CPU_SPEED;
extern volatile u8 PHM_DIV;
extern volatile u32 CFG_EXTAL;
extern volatile u32 CONFIG_BAUDRATE;
extern volatile u32 UART_BASE;
extern volatile u8 CONFIG_MOBILE_SDRAM;
extern volatile u8 IS_SHARE;

#endif 
