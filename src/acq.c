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

#include "acq.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

#include "comm.h"
#include "prios.h"
#include "mpu.h"

#define I2C_MPU I2C1
#define EXTI_MPU EXTI5

struct mpu9250 mpu = {
	.i2c = I2C_MPU,
	.addr = MPU9250_ADDR_AD0_LOW,
};

struct ak8963 ak = {
	.i2c = I2C_MPU
};

void exti4_15_isr()
{
	int16_t ax, ay, az, gx, gy, gz, mx, my, mz;

	mpu_accel(&mpu, &ax, &ay, &az);
	mpu_gyro(&mpu, &gx, &gy, &gz);
	comm_send_imu(ax, ay, az, gx, gy, gz);

	if (ak_ready(&ak)) {
		ak_values(&ak, &mx, &my, &mz);
		comm_send_magnetometer(mx, my, mz);
	}

	exti_reset_request(EXTI_MPU);
}

void acq_init()
{
	rcc_periph_clock_enable(RCC_I2C1);

	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO6 | GPIO7);
	gpio_set_af(GPIOB, GPIO_AF1, GPIO6 | GPIO7);

	/* This should be 8 MHz */
	rcc_set_i2c_clock_hsi(I2C_MPU);

	i2c_reset(I2C_MPU);
	i2c_peripheral_disable(I2C_MPU);

	i2c_enable_analog_filter(I2C_MPU);
	i2c_set_digital_filter(I2C_MPU, I2C_CR1_DNF_DISABLED);

	/* Set SCL to 400kHz. See the reference manual Table 83 */
	i2c_set_prescaler(I2C_MPU, 1);
	i2c_set_scl_low_period(I2C_MPU, 0x09);
	i2c_set_scl_high_period(I2C_MPU, 0x03);
	i2c_set_data_hold_time(I2C_MPU, 0x01);
	i2c_set_data_setup_time(I2C_MPU, 0x3);

	i2c_set_7bit_addr_mode(I2C_MPU);
	i2c_peripheral_enable(I2C_MPU);

	mpu_init(&mpu);
	ak_init(&ak);

	/* Configure the interrupt line */
	exti_select_source(EXTI_MPU, GPIOB);
	exti_set_trigger(EXTI_MPU, EXTI_TRIGGER_FALLING);
	exti_enable_request(EXTI_MPU);
	nvic_set_priority(NVIC_EXTI4_15_IRQ, PRIO_ACQ);
}

void acq_start()
{
	/* TODO: Actually properly start/stop the MPU9250 */
	exti_reset_request(EXTI_MPU);
	exti_enable_request(EXTI_MPU);
	nvic_enable_irq(NVIC_EXTI4_15_IRQ);
}

void acq_stop()
{
	nvic_disable_irq(NVIC_EXTI4_15_IRQ);
	exti_disable_request(EXTI_MPU);
}
