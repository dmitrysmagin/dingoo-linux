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

#include "delay.h"

void delay (unsigned int ms) {

	int i;

	__tcu_disable_pwm_output(3);
	__tcu_mask_half_match_irq(3);
	__tcu_mask_full_match_irq(3);
	__tcu_select_extalclk(3);
	__tcu_select_clk_div16(3);

	REG_TCU_TDFR(3) = CFG_EXTAL / 16 / 1000;
	REG_TCU_TDHR(3) = CFG_EXTAL / 16 / 1000;

	__tcu_set_count(3, 0);
	__tcu_start_counter(3);

	for (i = 0; i < ms; i++) {
		__tcu_clear_full_match_flag(3);
		while (!__tcu_full_match_flag(3));
	}

	__tcu_stop_counter(3);
}

