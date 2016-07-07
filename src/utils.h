/* 
 * Copyright (C) 2016 Institute of Applied and Experimental Physics (http://www.utef.cvut.cz/)
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

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define min(a, b) \
	({ __typeof__(a) _a = (a); \
	   __typeof__(b) _b = (b); \
	  _a < _b ? _a : _b; })

#define max(a, b) \
	({ __typeof__(a) _a = (a); \
	   __typeof__(b) _b = (b); \
	  _a > _b ? _a : _b; })

#define UNUSED(x) \
	(void)(x)

#define BIT(x) (1 << (x))

#define BYTE(n, val) (((val) >> (8 * n)) & 0xff)

#define HIBYTE(val) BYTE(1, val)
#define LOBYTE(val) BYTE(0, val)

#define RAMFUNC __attribute__((section(".ramfuncs")))
#define ALWAYS_INLINE	__attribute__((always_inline))

inline static int16_t be_to_i16(uint8_t *data)
{
	return (int16_t)((data[0] << 8) | (data[1] << 0));
}

inline static int16_t le_to_i16(uint8_t *data)
{
	return (int16_t)((data[1] << 8) | (data[0] << 0));
}

void delay_raw(int cycles);
/* This delay is very approximate */
#define delay_ms(x) delay_raw((int)(((x) * F_CPU) / 1000UL * 1.57f))

#endif
