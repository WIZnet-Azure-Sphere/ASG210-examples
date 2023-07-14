// lis2dh12.c

// WIZnet
// 2022.05.17 created
// Referenced from Espressif Systems
//
// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lis2dh12.h"
#include "sensors.h"
#include <stdio.h>

int i2c_fd;

int lis2dh12_init(int fd)
{
  uint8_t lis2dh12_deviceid;
  lis2dh12_config_t  lis2dh12_config;
  int ret;

  i2c_fd = fd;

  ret  = lis2dh12_get_deviceid(&lis2dh12_deviceid);
  if(ret !=0)
  {
    printf("lis2dh12_get_deviceid() failed");
    return -1;
  }
  printf("lis2dh12 device address is: 0x%02x\n", lis2dh12_deviceid);
  lis2dh12_config.temp_enable = LIS2DH12_TEMP_DISABLE;
  lis2dh12_config.odr = LIS2DH12_ODR_100HZ;
  lis2dh12_config.opt_mode = LIS2DH12_OPT_NORMAL;
  lis2dh12_config.z_enable = LIS2DH12_ENABLE;
  lis2dh12_config.y_enable = LIS2DH12_ENABLE;
  lis2dh12_config.x_enable = LIS2DH12_ENABLE;
  lis2dh12_config.bdu_status = LIS2DH12_DISABLE;
  lis2dh12_config.fs = LIS2DH12_FS_4G;
  ret = lis2dh12_set_config(&lis2dh12_config);

  return ret;
}

int lis2dh12_write(uint8_t* cmd, uint32_t cmdlen)
{
    int ret;
    
    ret = sensor_write(LIS2DH12_I2C_ADDRESS, cmd, cmdlen, true);

    return ret;
}

int lis2dh12_read(uint8_t* cmd, uint32_t cmdlen, uint8_t* data, uint32_t datalen)
{
    int ret;
    
    ret = sensor_write(LIS2DH12_I2C_ADDRESS, cmd, cmdlen, true);
    if(ret != 0)
    {
      return ret;
    }
    
    ret = sensor_read(LIS2DH12_I2C_ADDRESS, data, datalen, false);

    return ret;
}


int lis2dh12_get_deviceid(uint8_t *deviceid)
{
    uint8_t cmd;
    uint8_t data;
    int ret;
    uint8_t tmp;

    cmd = LIS2DH12_WHO_AM_I_REG;
    ret = lis2dh12_read(&cmd,1,&data,1);
    if(ret !=0)
    {
      printf("lis2dh12_read() failed\n");
    }
    //printf("deviceid : %x\n",data);
    *deviceid = data;
    return ret;

}

int lis2dh12_set_config(lis2dh12_config_t *lis2dh12_config)
{
    uint8_t cmd[7];
    int ret;
    uint8_t data_rd[6];
    
    cmd[0] = LIS2DH12_TEMP_CFG_REG;
    ret = lis2dh12_read(cmd, 1, data_rd,1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    data_rd[0] &= ~LIS2DH12_TEMP_EN_MASK;
    data_rd[0] |= ((uint8_t)lis2dh12_config->temp_enable) << LIS2DH12_TEMP_EN_BIT;
    cmd[1] = data_rd[0];
    ret = lis2dh12_write(cmd,2);
    if(ret !=0)
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }
    cmd[0] =LIS2DH12_CTRL_REG1 | LIS2DH12_I2C_MULTI_REG_ONCE;
    ret = lis2dh12_read(cmd, 1, data_rd, 6);
    if(ret !=0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }


    data_rd[0] &= (uint8_t) ~(LIS2DH12_ODR_MASK | LIS2DH12_LP_EN_MASK | LIS2DH12_Z_EN_MASK | LIS2DH12_Y_EN_MASK | LIS2DH12_X_EN_MASK);
    data_rd[0] |= ((uint8_t)lis2dh12_config->odr) << LIS2DH12_ODR_BIT;
    data_rd[0] |= ((uint8_t)((lis2dh12_config->opt_mode >> 1) << LIS2DH12_LP_EN_BIT)&LIS2DH12_LP_EN_MASK);
    data_rd[0] |= ((uint8_t)lis2dh12_config->z_enable) << LIS2DH12_Z_EN_BIT;
    data_rd[0] |= ((uint8_t)lis2dh12_config->y_enable) << LIS2DH12_Y_EN_BIT;
    data_rd[0] |= ((uint8_t)lis2dh12_config->x_enable) << LIS2DH12_X_EN_BIT;

    data_rd[3] &= ~(LIS2DH12_BDU_MASK | LIS2DH12_FS_MASK | LIS2DH12_HR_MASK);
    data_rd[3] |= ((uint8_t)lis2dh12_config->bdu_status) << LIS2DH12_BDU_BIT;
    data_rd[3] |= ((uint8_t)lis2dh12_config->fs) << LIS2DH12_FS_BIT;
    data_rd[3] |= ((uint8_t)((lis2dh12_config->opt_mode) << LIS2DH12_HR_BIT)&LIS2DH12_HR_MASK);
    cmd[1] = data_rd[0];
    cmd[4] = data_rd[3];
    ret = lis2dh12_write(cmd,7);
    if(ret !=0)
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }
    return 0;
}

int lis2dh12_get_config(lis2dh12_config_t *lis2dh12_config)
{
    uint8_t cmd[2];
    int ret;
    uint8_t data_rd[6] = {0};
    
    cmd[0] = LIS2DH12_TEMP_CFG_REG;
    ret= lis2dh12_read(cmd ,1,data_rd, 1);
    if(ret!=0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    
    lis2dh12_config->temp_enable = (lis2dh12_temp_en_t)((data_rd[0] & LIS2DH12_TEMP_EN_MASK) >> LIS2DH12_TEMP_EN_BIT);

    cmd[0] = LIS2DH12_CTRL_REG1 | LIS2DH12_I2C_MULTI_REG_ONCE;
    ret= lis2dh12_read(cmd ,1,data_rd, 6);
    if(ret!=0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    
    lis2dh12_config->odr = (lis2dh12_odr_t)((data_rd[0] & LIS2DH12_ODR_MASK) >> LIS2DH12_ODR_BIT);
    lis2dh12_config->z_enable = (lis2dh12_state_t)((data_rd[0] & LIS2DH12_Z_EN_MASK) >> LIS2DH12_Z_EN_BIT);
    lis2dh12_config->y_enable = (lis2dh12_state_t)((data_rd[0] & LIS2DH12_Y_EN_MASK) >> LIS2DH12_Y_EN_BIT);
    lis2dh12_config->x_enable = (lis2dh12_state_t)((data_rd[0] & LIS2DH12_X_EN_MASK) >> LIS2DH12_X_EN_BIT);
    lis2dh12_config->bdu_status = (lis2dh12_state_t)((data_rd[3] & LIS2DH12_BDU_MASK) >> LIS2DH12_BDU_BIT);
    lis2dh12_config->fs = (lis2dh12_fs_t)((data_rd[3] & LIS2DH12_FS_MASK) >> LIS2DH12_FS_BIT);
    lis2dh12_config->opt_mode = (lis2dh12_opt_mode_t)((((data_rd[0] & LIS2DH12_LP_EN_MASK) << 1) >> LIS2DH12_LP_EN_BIT) | ((data_rd[3] & LIS2DH12_HR_MASK) >> LIS2DH12_HR_BIT));
    return 0;
}

int lis2dh12_set_temp_enable(lis2dh12_temp_en_t temp_en)
{ 
    uint8_t cmd[2];
    int ret;
    uint8_t data;
    uint8_t tmp;


    cmd[0] = LIS2DH12_TEMP_CFG_REG;
    ret = lis2dh12_read(cmd,1, &data,1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    data &= ~LIS2DH12_TEMP_EN_MASK;
    data|= ((uint8_t)temp_en) << LIS2DH12_TEMP_EN_BIT;
    cmd[1] = data;
    ret = lis2dh12_write(cmd,2);
    if(ret != 0 )
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }
    
    return 0;
}

int lis2dh12_set_odr(lis2dh12_odr_t odr)
{
    uint8_t cmd[2];
    int ret;
    uint8_t data;
    uint8_t tmp;

    cmd[0] = LIS2DH12_CTRL_REG1;
    ret = lis2dh12_read(cmd,1,&data,1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    data &= ~LIS2DH12_ODR_MASK;
    data |= ((uint8_t)odr) << LIS2DH12_ODR_BIT;
    cmd[1] = data;
    ret = lis2dh12_write(cmd,2);
    if(ret != 0)
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }
    return 0;
}

int lis2dh12_set_z_enable(lis2dh12_state_t status)
{
    uint8_t cmd[2];
    uint8_t data;
    uint8_t tmp;
    int ret;
    
    cmd[0] = LIS2DH12_CTRL_REG1;
    ret = lis2dh12_read(cmd, 1, &data,1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }

    tmp &= ~LIS2DH12_Z_EN_MASK;
    tmp |= ((uint8_t)status) << LIS2DH12_Z_EN_BIT;
    cmd[1] = tmp;
    ret = lis2dh12_write(cmd,2);
    if(ret != 0)
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }
    return 0;
}

int lis2dh12_set_y_enable(lis2dh12_state_t status)
{
    uint8_t cmd[2];
    uint8_t data;
    int ret;
    
    cmd[0] = LIS2DH12_CTRL_REG1;
    ret = lis2dh12_read(cmd,1,&data,1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    data &= ~LIS2DH12_Y_EN_MASK;
    data |= ((uint8_t)status) << LIS2DH12_Y_EN_BIT;
    cmd[1] = data;
    ret = lis2dh12_write(cmd,2);
    if(ret != 0)
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }
    return 0;
}

int lis2dh12_set_x_enable(lis2dh12_state_t status)
{
    uint8_t cmd[2];
    uint8_t data;
    int ret;

    cmd[0] = LIS2DH12_CTRL_REG1;
    ret = lis2dh12_read(cmd,1, &data, 1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    data &= ~LIS2DH12_X_EN_MASK;
    data |= ((uint8_t)status) << LIS2DH12_X_EN_BIT;
    cmd[1] = data;
    ret = lis2dh12_write(cmd,2);
    if(ret != 0)
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }
    return 0;
}

int lis2dh12_set_bdumode(lis2dh12_state_t status)
{
    uint8_t cmd[2];
    uint8_t data;
  
    int ret;

    cmd[0] = LIS2DH12_CTRL_REG4;
    ret = lis2dh12_read(cmd,1, &data, 1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    data &= ~LIS2DH12_BDU_MASK;
    data |= ((uint8_t)status) << LIS2DH12_BDU_BIT;
    cmd[1] = data;
    ret = lis2dh12_write(cmd,2);
    if(ret != 0)
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }
    return 0;
}

int lis2dh12_set_fs(lis2dh12_fs_t fs)
{
    uint8_t cmd[2];
    uint8_t data;
  
    int ret;

    cmd[0] = LIS2DH12_CTRL_REG4;
    ret = lis2dh12_read(cmd,1,&data,1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    data &= ~LIS2DH12_FS_MASK;
    data |= ((uint8_t)fs) << LIS2DH12_FS_BIT;
    cmd[1] = data;
    ret = lis2dh12_write(cmd ,2);
    if(ret != 0)
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }
    return 0;
}

int lis2dh12_get_fs(lis2dh12_fs_t *fs)
{
    uint8_t cmd[2];
    uint8_t data;
    int ret;
    
    cmd[0] =  LIS2DH12_CTRL_REG4;
    ret = lis2dh12_read(cmd,1,&data,1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    *fs = (lis2dh12_fs_t)((data & LIS2DH12_FS_MASK) >> LIS2DH12_FS_BIT);
    return 0;
}

int lis2dh12_set_opt_mode(lis2dh12_opt_mode_t opt_mode)
{
    
    uint8_t cmd[2];
    uint8_t data;
    uint8_t tmp;
    int ret;

    cmd[0] = LIS2DH12_CTRL_REG1;
    ret = lis2dh12_read(cmd,1, &data,1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    data &= ~LIS2DH12_LP_EN_MASK;
    data |= ((uint8_t)((opt_mode >> 1) << LIS2DH12_LP_EN_BIT)&LIS2DH12_LP_EN_MASK);
    cmd[1] = data;
    ret = lis2dh12_write(cmd,2);
    if(ret != 0)
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }

    cmd[0] = LIS2DH12_CTRL_REG4;
    ret = lis2dh12_read(cmd,1, &data,1);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    data &= ~LIS2DH12_HR_MASK;
    data|= ((uint8_t)opt_mode & LIS2DH12_HR_MASK) << LIS2DH12_HR_BIT;
    cmd[1] = data;
    ret = lis2dh12_write(cmd,2);
    if(ret != 0)
    {
      printf("lis2dh12_write() failed\n");
      return -1;
    }
  
    return 0;
}

int lis2dh12_get_x_acc(uint16_t *x_acc)
{
    uint8_t cmd;
    uint8_t data_rd[2];
    int ret;

    cmd = LIS2DH12_OUT_X_L_REG | LIS2DH12_I2C_MULTI_REG_ONCE;
    ret = lis2dh12_read(&cmd,1, data_rd,2);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    *x_acc = (int16_t)((((uint16_t)data_rd[1]) << 8) | (uint16_t)data_rd[0]);
    return 0;
}

int lis2dh12_get_y_acc(uint16_t *y_acc)
{
    uint8_t cmd;
    uint8_t data_rd[2];
    int ret;

    cmd = LIS2DH12_OUT_Y_L_REG | LIS2DH12_I2C_MULTI_REG_ONCE;
    ret = lis2dh12_read(&cmd,1, data_rd,2);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    *y_acc = (int16_t)((((uint16_t)data_rd[1]) << 8) | (uint16_t)data_rd[0]);
    return 0;
}

int lis2dh12_get_z_acc(uint16_t *z_acc)
{
    
    uint8_t cmd;
    uint8_t data_rd[2];
    int ret;
    cmd = LIS2DH12_OUT_Z_L_REG | LIS2DH12_I2C_MULTI_REG_ONCE;
    ret = lis2dh12_read(&cmd,1, data_rd,2);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    *z_acc = (int16_t)((((uint16_t)data_rd[1]) << 8) | (uint16_t)data_rd[0]);
    return 0;
}

int lis2dh12_get_raw_acce(lis2dh12_raw_acce_value_t *raw_acce_value)
{
    
    uint8_t cmd;
    uint8_t data_rd[6];
    int ret;
    
    cmd = LIS2DH12_OUT_X_L_REG | LIS2DH12_I2C_MULTI_REG_ONCE;
    ret = lis2dh12_read(&cmd,1, data_rd,6);
    if(ret != 0)
    {
      printf("lis2dh12_read() failed\n");
      return -1;
    }
    
    raw_acce_value->raw_acce_x = *(int16_t *)data_rd;
    raw_acce_value->raw_acce_y = *(int16_t *)(data_rd + 2);
    raw_acce_value->raw_acce_z = *(int16_t *)(data_rd + 4);
    return 0;
}

int lis2dh12_get_acce(lis2dh12_acce_value_t *acce_value)
{
    lis2dh12_fs_t fs;
    lis2dh12_raw_acce_value_t raw_acce_value;
    int ret = lis2dh12_get_fs(&fs);

    if (ret != 0) {
        return ret;
    }

    ret = lis2dh12_get_raw_acce(&raw_acce_value);

    if (fs == LIS2DH12_FS_2G) {
        acce_value->acce_x = LIS2DH12_FROM_FS_2g_HR_TO_mg(raw_acce_value.raw_acce_x) / 1000.0;
        acce_value->acce_y = LIS2DH12_FROM_FS_2g_HR_TO_mg(raw_acce_value.raw_acce_y) / 1000.0;
        acce_value->acce_z = LIS2DH12_FROM_FS_2g_HR_TO_mg(raw_acce_value.raw_acce_z) / 1000.0;
    } else if (fs == LIS2DH12_FS_4G) {
        acce_value->acce_x = LIS2DH12_FROM_FS_4g_HR_TO_mg(raw_acce_value.raw_acce_x) / 1000.0;
        acce_value->acce_y = LIS2DH12_FROM_FS_4g_HR_TO_mg(raw_acce_value.raw_acce_y) / 1000.0;
        acce_value->acce_z = LIS2DH12_FROM_FS_4g_HR_TO_mg(raw_acce_value.raw_acce_z) / 1000.0;
    } else if (fs == LIS2DH12_FS_8G) {
        acce_value->acce_x = LIS2DH12_FROM_FS_8g_HR_TO_mg(raw_acce_value.raw_acce_x) / 1000.0;
        acce_value->acce_y = LIS2DH12_FROM_FS_8g_HR_TO_mg(raw_acce_value.raw_acce_y) / 1000.0;
        acce_value->acce_z = LIS2DH12_FROM_FS_8g_HR_TO_mg(raw_acce_value.raw_acce_z) / 1000.0;
    } else if (fs == LIS2DH12_FS_16G) {
        acce_value->acce_x = LIS2DH12_FROM_FS_16g_HR_TO_mg(raw_acce_value.raw_acce_x) / 1000.0;
        acce_value->acce_y = LIS2DH12_FROM_FS_16g_HR_TO_mg(raw_acce_value.raw_acce_y) / 1000.0;
        acce_value->acce_z = LIS2DH12_FROM_FS_16g_HR_TO_mg(raw_acce_value.raw_acce_z) / 1000.0;
    }

    return ret;
}

///***sensors hal interface****/

static bool is_init = false;

int imu_lis2dh12_init(void)
{
    
     uint8_t lis2dh12_deviceid;
    lis2dh12_config_t  lis2dh12_config;
    int ret = lis2dh12_get_deviceid(&lis2dh12_deviceid);
    if(ret !=0)
    {
      printf("lis2dh12_get_deviceid() failed\n");
      return -1;
    }
    printf("lis2dh12 device address is: 0x%02x\n", lis2dh12_deviceid);
    lis2dh12_config.temp_enable = LIS2DH12_TEMP_DISABLE;
    lis2dh12_config.odr = LIS2DH12_ODR_100HZ;
    lis2dh12_config.opt_mode = LIS2DH12_OPT_NORMAL;
    lis2dh12_config.z_enable = LIS2DH12_ENABLE;
    lis2dh12_config.y_enable = LIS2DH12_ENABLE;
    lis2dh12_config.x_enable = LIS2DH12_ENABLE;
    lis2dh12_config.bdu_status = LIS2DH12_DISABLE;
    lis2dh12_config.fs = LIS2DH12_FS_4G;
    ret = lis2dh12_set_config( &lis2dh12_config);

    if (ret == 0) {
        is_init = true;
    }

    return ret;
}


int imu_lis2dh12_acquire_acce(float *acce_x, float *acce_y, float *acce_z)
{
    
    lis2dh12_acce_value_t acce = {0, 0, 0};
    if (acce_x != NULL && acce_y != NULL && acce_z != NULL) {
      if (0 == lis2dh12_get_acce( &acce)) {
          *acce_x = acce.acce_x;
          *acce_y = acce.acce_y;
          *acce_z = acce.acce_z;
          return 0;
      }
    }
     printf("imu_lis2dh12_acquire_acce() failed\n");
    *acce_x = 0;
    *acce_y = 0;
    *acce_z = 0;
    return -1;
}
