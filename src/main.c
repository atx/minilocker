/* 
 * Copyright (C) 2016 Josef Gajdusek <atx@atx.name>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 */

#include <stdlib.h>
#include <string.h>
#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/crs.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f0/rcc.h>
#include <libopencm3/stm32/f0/syscfg.h>
#include <libopencm3/stm32/f0/i2c.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/usb/usbd.h>

#include "acq.h"
#include "cdc.h"
#include "utils.h"

static void init()
{
	cm_disable_interrupts();

	rcc_periph_clock_enable(RCC_SYSCFG_COMP);
	rcc_clock_setup_in_hsi48_out_48mhz();
	crs_autotrim_usb_enable();

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_I2C1);

	acq_init();
	cdc_init();

	cm_enable_interrupts();
}

int main(void)
{
	delay_ms(1000);
	init();

	while (true) {
		__asm__("wfi");
	}
}
