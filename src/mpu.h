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

#ifndef MPU_H
#define MPU_H

#include <stdint.h>
#include <stdbool.h>

#include "utils.h"

#define MPU9250_REG_SELF_TEST_X_GYRO			0x00
#define MPU9250_REG_SELF_TEST_Y_GYRO			0x01
#define MPU9250_REG_SELF_TEST_Z_GYRO			0x02
#define MPU9250_REG_SELF_TEST_X_ACCEL			0x0d
#define MPU9250_REG_SELF_TEST_Y_ACCEL			0x0e
#define MPU9250_REG_SELF_TEST_Z_ACCEL			0x0f
#define MPU9250_REG_XG_OFFSET_H					0x13
#define MPU9250_REG_XG_OFFSET_L					0x14
#define MPU9250_REG_YG_OFFSET_H					0x15
#define MPU9250_REG_YG_OFFSET_L					0x16
#define MPU9250_REG_ZG_OFFSET_H					0x17
#define MPU9250_REG_ZG_OFFSET_L					0x18
#define MPU9250_REG_SMPLRT_DIV					0x19
#define MPU9250_REG_CONFIG						0x1a
#define MPU9250_REG_GYRO_CONFIG					0x1b
#define MPU9250_REG_ACCEL_CONFIG				0x1c
#define MPU9250_REG_ACCEL_CONFIG2				0x1d
#define MPU9250_REG_LP_ACCEL_ODR				0x1e
#define MPU9250_REG_WOM_THR						0x1f
#define MPU9250_REG_FIFO_EN						0x23
#define MPU9250_REG_I2C_MST_CTRL				0x24
#define MPU9250_REG_I2C_SLV0_ADDR				0x25
#define MPU9250_REG_I2C_SLV0_REG				0x26
#define MPU9250_REG_I2C_SLV0_CTRL				0x27
#define MPU9250_REG_I2C_SLV1_ADDR				0x28
#define MPU9250_REG_I2C_SLV1_REG				0x29
#define MPU9250_REG_I2C_SLV1_CTRL				0x2a
#define MPU9250_REG_I2C_SLV2_ADDR				0x2b
#define MPU9250_REG_I2C_SLV2_REG				0x2c
#define MPU9250_REG_I2C_SLV2_CTRL				0x2d
#define MPU9250_REG_I2C_SLV3_ADDR				0x2e
#define MPU9250_REG_I2C_SLV3_REG				0x2f
#define MPU9250_REG_I2C_SLV3_CTRL				0x30
#define MPU9250_REG_I2C_SLV4_ADDR				0x31
#define MPU9250_REG_I2C_SLV4_REG				0x32
#define MPU9250_REG_I2C_SLV4_DO					0x33
#define MPU9250_REG_I2C_SLV4_CTRL				0x34
#define MPU9250_REG_I2C_SLV4_DI					0x35
#define MPU9250_REG_I2C_MSG_STATUS				0x36
#define MPU9250_REG_INT_PIN_CFG					0x37
#define MPU9250_REG_INT_ENABLE					0x38
#define MPU9250_REG_INT_STATUS					0x3a
#define MPU9250_REG_ACCEL_XOUT_H				0x3b
#define MPU9250_REG_ACCEL_XOUT_L				0x3c
#define MPU9250_REG_ACCEL_YOUT_H				0x3d
#define MPU9250_REG_ACCEL_YOUT_L				0x3e
#define MPU9250_REG_ACCEL_ZOUT_H				0x3f
#define MPU9250_REG_ACCEL_ZOUT_L				0x40
#define MPU9250_REG_TEMP_OUT_H					0x41
#define MPU9250_REG_TEMP_OUT_L					0x42
#define MPU9250_REG_GYRO_XOUT_H					0x43
#define MPU9250_REG_GYRO_XOUT_L					0x44
#define MPU9250_REG_GYRO_YOUT_H					0x45
#define MPU9250_REG_GYRO_YOUT_L					0x46
#define MPU9250_REG_GYRO_ZOUT_H					0x47
#define MPU9250_REG_GYRO_ZOUT_L					0x48
#define MPU9250_REG_EXT_SENS_DATA_00			0x49
#define MPU9250_REG_EXT_SENS_DATA_01			0x4a
#define MPU9250_REG_EXT_SENS_DATA_02			0x4b
#define MPU9250_REG_EXT_SENS_DATA_03			0x4c
#define MPU9250_REG_EXT_SENS_DATA_04			0x4d
#define MPU9250_REG_EXT_SENS_DATA_05			0x4e
#define MPU9250_REG_EXT_SENS_DATA_06			0x4f
#define MPU9250_REG_EXT_SENS_DATA_07			0x50
#define MPU9250_REG_EXT_SENS_DATA_08			0x51
#define MPU9250_REG_EXT_SENS_DATA_09			0x52
#define MPU9250_REG_EXT_SENS_DATA_10			0x53
#define MPU9250_REG_EXT_SENS_DATA_11			0x54
#define MPU9250_REG_EXT_SENS_DATA_12			0x55
#define MPU9250_REG_EXT_SENS_DATA_13			0x56
#define MPU9250_REG_EXT_SENS_DATA_14			0x57
#define MPU9250_REG_EXT_SENS_DATA_15			0x58
#define MPU9250_REG_EXT_SENS_DATA_16			0x59
#define MPU9250_REG_EXT_SENS_DATA_17			0x5a
#define MPU9250_REG_EXT_SENS_DATA_18			0x5b
#define MPU9250_REG_EXT_SENS_DATA_19			0x5c
#define MPU9250_REG_EXT_SENS_DATA_20			0x5d
#define MPU9250_REG_EXT_SENS_DATA_21			0x5e
#define MPU9250_REG_EXT_SENS_DATA_22			0x5f
#define MPU9250_REG_EXT_SENS_DATA_23			0x60
#define MPU9250_REG_I2C_SLV0_DO					0x63
#define MPU9250_REG_I2C_SLV1_DO					0x64
#define MPU9250_REG_I2C_SLV2_DO					0x65
#define MPU9250_REG_I2C_SLV3_DO					0x66
#define MPU9250_REG_I2C_MST_DELAY_CTRL			0x67
#define MPU9250_REG_SIGNAL_PATH_RESET			0x68
#define MPU9250_REG_MOT_DETECT_CTRL				0x69
#define MPU9250_REG_USER_CTRL					0x6a
#define MPU9250_REG_PWR_MGMT_1					0x6b
#define MPU9250_REG_PWR_MGMT_2					0x6c
#define MPU9250_REG_FIFO_COUNTH					0x72
#define MPU9250_REG_FIFO_COUNTL					0x73
#define MPU9250_REG_FIFO_R_W					0x74
#define MPU9250_REG_WHO_AM_I					0x75
#define MPU9250_REG_XA_OFFSET_H					0x77
#define MPU9250_REG_XA_OFFSET_L					0x78
#define MPU9250_REG_YA_OFFSET_H					0x7a
#define MPU9250_REG_YA_OFFSET_L					0x7b
#define MPU9250_REG_ZA_OFFSET_H					0x7d
#define MPU9250_REG_ZA_OFFSET_L					0x7e

/* Incomplete, TODO */
#define MPU9250_PWR_MGMT_1_H_RESET				BIT(7)
#define MPU9250_PWR_MGMT_1_CLKSEL_INTERNAL		(0 << 0)
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO			(1 << 0)
#define MPU9250_PWR_MGMT_1_CLKSEL_STOP			(7 << 0)

#define MPU9250_INT_PIN_CFG_ACTL				BIT(7)
#define MPU9250_INT_PIN_CFG_OPEN				BIT(6)
#define MPU9250_INT_PIN_CFG_LATCH_INT_EN		BIT(5)
#define MPU9250_INT_PIN_CFG_INT_ANYRD_2CLEAR	BIT(4)
#define MPU9250_INT_PIN_CFG_ACTL_FSYNC			BIT(3)
#define MPU9250_INT_PIN_CFG_FSYNC_INT_MODE_EN	BIT(2)
#define MPU9250_INT_PIN_CFG_BYPASS_EN			BIT(1)

#define MPU9250_INT_ENABLE_WOM_EN				BIT(6)
#define MPU9250_INT_ENABLE_FIFO_OVERFLOW_EN		BIT(4)
#define MPU9250_INT_ENABLE_FSYNC_INT_EN			BIT(3)
#define MPU9250_INT_ENABLE_RAW_RDY_EN			BIT(0)

#define MPU9250_ADDR_AD0_LOW					0x68
#define MPU9250_ADDR_AD0_HIGH					0x69

#define MPU9250_WHO_AM_I_VAL					0x71

#define AK8963_REG_WIA							0x00
#define AK8963_REG_INFO							0x01
#define AK8963_REG_ST1							0x02
#define AK8963_REG_HXL							0x03
#define AK8963_REG_HXH							0x04
#define AK8963_REG_HYL							0x05
#define AK8963_REG_HYH							0x06
#define AK8963_REG_HZL							0x07
#define AK8963_REG_HZH							0x08
#define AK8963_REG_ST2							0x09
#define AK8963_REG_CNTL1						0x0a
#define AK8963_REG_CNTL2						0x0b
#define AK8963_REG_ASTC							0x0c
#define AK8963_REG_TS1							0x0d
#define AK8963_REG_TS2							0x0e
#define AK8963_REG_I2CDIS						0x0f
#define AK8963_REG_ASAX							0x10
#define AK8963_REG_ASAY							0x11
#define AK8963_REG_ASAZ							0x12
#define AK8963_REG_RSV							0x13

#define AK8963_ADDR								0x0c

#define AK8963_WIA_VAL							0x48

#define AK8963_ST1_DOR							BIT(1)
#define AK8963_ST1_DRDY							BIT(0)

#define AK8963_CNTL1_MODE_POWER_DOWN			0x00
#define AK8963_CNTL1_MODE_SINGLE				0x01
#define AK8963_CNTL1_MODE_CONT1					0x02
#define AK8963_CNTL1_MODE_CONT2					0x06
#define AK8963_CNTL1_MODE_EXTERNAL				0x04
#define AK8963_CNTL1_MODE_SELF_TEST				0x08
#define AK8963_CNTL1_MODE_FUSE_ROM				0x0f
#define AK8963_CNTL1_BIT_16						BIT(4)

struct mpu9250 {
	uint32_t i2c;
	uint8_t addr;
};

void mpu_init(struct mpu9250 *mpu);

uint8_t mpu_read(struct mpu9250 *mpu, uint8_t addr);
void mpu_write(struct mpu9250 *mpu, uint8_t addr, uint8_t val);

void mpu_accel(struct mpu9250 *mpu, int16_t *x, int16_t *y, int16_t *z);
void mpu_gyro(struct mpu9250 *mpu, int16_t *x, int16_t *y, int16_t *z);


struct ak8963 {
	uint32_t i2c;
	uint8_t cal_x;
	uint8_t cal_y;
	uint8_t cal_z;
};

void ak_init(struct ak8963 *ak);

uint8_t ak_read(struct ak8963 *ak, uint8_t addr);
void ak_write(struct ak8963 *ak, uint8_t addr, uint8_t val);

bool ak_ready(struct ak8963 *ak);
void ak_values(struct ak8963 *ak, int16_t *x, int16_t *y, int16_t *z);

#endif
