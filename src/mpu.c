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

#include "mpu.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/syscfg.h>

#include "utils.h"

static void read_xyz(uint32_t i2c, uint8_t i2caddr, uint8_t addr,
					 int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t data[6];
	read_i2c(i2c, i2caddr, addr, sizeof(data), data);
	*x = be_to_i16(&data[0]);
	*y = be_to_i16(&data[2]);
	*z = be_to_i16(&data[4]);
}

static inline uint8_t read_i2c_byte(uint32_t i2c, uint8_t i2caddr, uint8_t addr)
{
	uint8_t ret = 0;
	read_i2c(i2c, i2caddr, addr, 1, &ret);
	return ret;
}

static inline void write_i2c_byte(uint32_t i2c, uint8_t i2caddr, uint8_t addr, uint8_t val)
{
	write_i2c(i2c, i2caddr, addr, 1, &val);
}

uint8_t mpu_read(struct mpu9250 *mpu, uint8_t addr)
{
	return read_i2c_byte(mpu->i2c, mpu->addr, addr);
}

void mpu_write(struct mpu9250 *mpu, uint8_t addr, uint8_t val)
{
	write_i2c_byte(mpu->i2c, mpu->addr, addr, val);
}

void mpu_accel(struct mpu9250 *mpu, int16_t *x, int16_t *y, int16_t *z)
{
	read_xyz(mpu->i2c, mpu->addr, MPU9250_REG_ACCEL_XOUT_H, x, y, z);
}

void mpu_gyro(struct mpu9250 *mpu, int16_t *x, int16_t *y, int16_t *z)
{
	read_xyz(mpu->i2c, mpu->addr, MPU9250_REG_GYRO_XOUT_H, x, y, z);
}

void mpu_init(struct mpu9250 *mpu)
{
	/* TODO: Maybe verify the WHO_AM_I value? */

	/* Set to normal run mode */
	mpu_write(mpu, MPU9250_REG_PWR_MGMT_1, 0x00);
	delay_ms(100);

	mpu_write(mpu, MPU9250_REG_CONFIG, 0x03);
	/* Set sample rate to 200Hz */
	mpu_write(mpu, MPU9250_REG_SMPLRT_DIV, 0x04);
	mpu_write(mpu, MPU9250_REG_INT_PIN_CFG, MPU9250_INT_PIN_CFG_INT_ANYRD_2CLEAR |
											MPU9250_INT_PIN_CFG_BYPASS_EN);
	mpu_write(mpu, MPU9250_REG_INT_ENABLE, MPU9250_INT_ENABLE_RAW_RDY_EN);
}


uint8_t ak_read(struct ak8963 *ak, uint8_t addr)
{
	return read_i2c_byte(ak->i2c, AK8963_ADDR, addr);
}

void ak_write(struct ak8963 *ak, uint8_t addr, uint8_t val)
{
	write_i2c_byte(ak->i2c, AK8963_ADDR, addr, val);
}

bool ak_ready(struct ak8963 *ak)
{
	return !!(ak_read(ak, AK8963_REG_ST1) & AK8963_ST1_DRDY);
}

void ak_values(struct ak8963 *ak, int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t data[7];
	/* Notice that we are reading the ST2 register too, this is required as per datasheet 6.4.3.2 */
	/* TODO: What to do on overflow? */
	read_i2c(ak->i2c, AK8963_ADDR, AK8963_REG_HXL, sizeof(data), data);
	*x = le_to_i16(&data[0]);
	*y = le_to_i16(&data[2]);
	*z = le_to_i16(&data[4]);
}

void ak_init(struct ak8963 *ak)
{
	/* Restart the magnetometer */
	ak_write(ak, AK8963_REG_CNTL1, AK8963_CNTL1_MODE_POWER_DOWN);
	delay_ms(10);
	/* Read the ROM calibration values */
	ak_write(ak, AK8963_REG_CNTL1, AK8963_CNTL1_MODE_SELF_TEST);
	delay_ms(10);

	ak->cal_x = ak_read(ak, AK8963_REG_ASAX);
	ak->cal_y = ak_read(ak, AK8963_REG_ASAY);
	ak->cal_z = ak_read(ak, AK8963_REG_ASAZ);

	ak_write(ak, AK8963_REG_CNTL1, AK8963_CNTL1_MODE_POWER_DOWN);
	delay_ms(10);

	/* Enable continuous acquision mode with sample rate of 100Hz at 16 bits of resulution */
	ak_write(ak, AK8963_REG_CNTL1, AK8963_CNTL1_BIT_16 | AK8963_CNTL1_MODE_CONT2);
}
