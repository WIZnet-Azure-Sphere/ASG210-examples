/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SENSORS_H_
#define _SENSORS_H_

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#if 1
#else
#include <applibs/log.h>
#endif

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */

#if 1
#include "printf.h"
#else
#define printf Log_Debug
#endif

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* SENSORS */

/*! \brief Initialize sensor function
 *  \ingroup sensor
 *
 *  Add a I2C initialize.
 *
 *  \param none
 */
int sensor_init(void);

/*! \brief I2C write function
 *  \ingroup sensor
 *
 *  Add a I2C write.
 *
 * \param addr 7-bit address of device to write to
 * \param src Pointer to data to send
 * \param len Length of data in bytes to send
 * \param nostop  If true, master retains control of the bus at the end of the transfer (no Stop is issued),
 *           and the next transfer will begin with a Restart rather than a Start.
 * \return Number of bytes written, or PICO_ERROR_GENERIC if address not acknowledged, no device present.
 */
int sensor_write(uint8_t addr, const uint8_t *src, size_t len, bool nostop);

/*! \brief I2C read function
 *  \ingroup sensor
 *
 *  Add a I2C read.
 *
 * \param addr 7-bit address of device to read from
 * \param dst Pointer to buffer to receive data
 * \param len Length of data in bytes to receive
 * \param nostop  If true, master retains control of the bus at the end of the transfer (no Stop is issued),
 *           and the next transfer will begin with a Restart rather than a Start.
 * \return Number of bytes read, or PICO_ERROR_GENERIC if address not acknowledged or no device present.
 */
int sensor_read(uint8_t addr, uint8_t *dst, size_t len, bool nostop);

#endif /* _SENSORS_H_ */
