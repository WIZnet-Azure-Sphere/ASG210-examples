/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include "sensors.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* SENSORS */

extern int i2c_fd;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* SENSORS */

int sensor_init(void)
{
  int ret;

  return 0;
}

int sensor_write(uint8_t addr, const uint8_t *src, size_t len, bool nostop)
{
  int ret;
  ssize_t transferredBytes;

#ifdef SENSORS_RT
  ret = mtk_os_hal_i2c_write(i2c_fd, addr, src, len);
#else
  transferredBytes = I2CMaster_Write(i2c_fd, addr, src, len);
#endif

#ifdef SENSORS_RT
  if (I2C_OK == ret)
  {
  }
  else
  {
      ret = -1;
  }
#else
  if (transferredBytes == len)
  {
      ret = 0;
  }
  else
  {
      ret = -1;
  }
#endif
  
  return ret;
}

int sensor_read(uint8_t addr, uint8_t *dst, size_t len, bool nostop)
{
  int ret;
  ssize_t transferredBytes;

#ifdef SENSORS_RT
  ret = mtk_os_hal_i2c_read(i2c_fd, addr, dst, len);
#else
  transferredBytes = I2CMaster_Read(i2c_fd, addr, dst, len);
#endif

#ifdef SENSORS_RT
  if (I2C_OK == ret)
  {
  }
  else
  {
      ret = -1;
  }
#else
  if (transferredBytes == len)
  {
      ret = 0;
  }
  else
  {
      ret = -1;
  }
#endif

  return ret;
}
