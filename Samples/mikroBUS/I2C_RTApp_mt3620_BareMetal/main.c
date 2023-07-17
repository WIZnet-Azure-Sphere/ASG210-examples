/*
 * (C) 2005-2020 MediaTek Inc. All rights reserved.
 *
 * Copyright Statement:
 *
 * This MT3620 driver software/firmware and related documentation
 * ("MediaTek Software") are protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. ("MediaTek"). You may only use, reproduce, modify, or
 * distribute (as applicable) MediaTek Software if you have agreed to and been
 * bound by this Statement and the applicable license agreement with MediaTek
 * ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User"). If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
 * PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS
 * ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO
 * LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED
 * HEREUNDER WILL BE ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
 * RECEIVER TO MEDIATEK DURING THE PRECEDING TWELVE (12) MONTHS FOR SUCH
 * MEDIATEK SOFTWARE AT ISSUE.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "printf.h"
#include "mt3620.h"
#include "os_hal_uart.h"
#include "os_hal_gpt.h"
#include "os_hal_gpio.h"
#include "os_hal_i2c.h"

#define I2C_MASTER

/******************************************************************************/
/* Configurations */
/******************************************************************************/
/* UART */
static const uint8_t uart_port_num = OS_HAL_UART_PORT0;

/* GPT */
/* GPE3 for Button status scan */
static const uint8_t gpt_timer_button = OS_HAL_GPT3;
/* 100ms, GPT3 clock speed: 1MHz */
static const uint32_t gpt_timer_button_scan_perios_ms = 100000;

/* GPIO */
/* GPIO_14 for ISU3_I2C nSEL */
static const uint8_t sel_nI2C = OS_HAL_GPIO_14;
/* GPIO_12 for ISU3 Intenal nSEL */
static const uint8_t sel_nINT = OS_HAL_GPIO_12;
/* GPIO_13 for Button_A USER_SW */
static const uint8_t gpio_button_a = OS_HAL_GPIO_13;

static const uint8_t i2c_master_speed = I2C_SCL_50kHz;

#ifdef I2C_MASTER
static const uint8_t i2c_master_port_num = OS_HAL_I2C_ISU3;
static const uint8_t i2c_slave_addr = 8;
#else
static const uint8_t i2c_slave_port_num = OS_HAL_I2C_ISU3;
static const uint8_t i2c_slave_addr = 9;
#endif

#define I2C_SLAVE_TIMEOUT 10000 /* 10000ms */
bool i2ctest_set = 0;

/******************************************************************************/
/* Applicaiton Hooks */
/******************************************************************************/
/* Hook for "printf". */
void _putchar(char character)
{
    mtk_os_hal_uart_put_char(uart_port_num, character);
    if (character == '\n')
        mtk_os_hal_uart_put_char(uart_port_num, '\r');
}

/******************************************************************************/
/* Functions */
/******************************************************************************/

static int gpio_output(u8 gpio_no, u8 level)
{
    mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_OUTPUT);
    mtk_os_hal_gpio_set_output(gpio_no, level);
    return 0;
}

static int gpio_input(u8 gpio_no, os_hal_gpio_data* pvalue)
{
    mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_INPUT);
    mtk_os_hal_gpio_get_input(gpio_no, pvalue);
    return 0;
}

static void TimerHandlerGpt3(void* cb_data)
{
    os_hal_gpio_data button_status;

    /* Set LED_Blue according to Button_A */
    gpio_input(gpio_button_a, &button_status);

    if (!button_status)
    {
        i2ctest_set = true;
    }

    /* Restart GPT3 since it's one-shot mode. */
    mtk_os_hal_gpt_restart(gpt_timer_button);
}

_Noreturn void RTCoreMain(void)
{
    struct os_gpt_int gpt0_int;
    struct os_gpt_int gpt3_int;
    int read_cnt = 0;
    float acce_x;
    float acce_y;
    float acce_z;

    /* Init Vector Table */
    NVIC_SetupVectorTable();

    /* Init UART */
    mtk_os_hal_uart_ctlr_init(uart_port_num);
    printf("\r\n");
    printf("=============================================\r\n");
    printf("I2C_RTApp_mt3620_BareMetal\r\n");
    printf("UART Inited (port_num=%d)\r\n", uart_port_num);
    printf("=============================================\r\n");

    /* Init GPT */
    mtk_os_hal_gpt_init();

    /* Init GPT3 for button status monitor, one-shot mode. */
    gpt3_int.gpt_cb_hdl = TimerHandlerGpt3;
    gpt3_int.gpt_cb_data = NULL;
    mtk_os_hal_gpt_config(gpt_timer_button, false, &gpt3_int);
    mtk_os_hal_gpt_reset_timer(gpt_timer_button,
                               gpt_timer_button_scan_perios_ms, false);
    mtk_os_hal_gpt_start(gpt_timer_button);

    gpio_output(sel_nI2C, OS_HAL_GPIO_DATA_LOW);
#if 1
    // Internal mB1 / mB2 / grove
    gpio_output(sel_nINT, OS_HAL_GPIO_DATA_LOW);
#else
    // External
    gpio_output(sel_nINT, OS_HAL_GPIO_DATA_HIGH);
#endif

    /* Init I2C Master/Slave */
#ifdef I2C_MASTER
    mtk_os_hal_i2c_ctrl_init(i2c_master_port_num);
    mtk_os_hal_i2c_speed_init(i2c_master_port_num, i2c_master_speed);
#else
    mtk_os_hal_i2c_ctrl_init(i2c_slave_port_num);
    mtk_os_hal_i2c_set_slave_addr(i2c_slave_port_num, i2c_slave_addr);
#endif

    lis2dh12_init(i2c_master_port_num);

    for (;;)
    {
        if (i2ctest_set == true)
        {
            int ret = imu_lis2dh12_acquire_acce(&acce_x, &acce_y, &acce_z);
            if (ret != 0)
            {
                printf("imu_lis2dh12_acquire_acce() failed");
            }
            else
            {
                printf("accel.xyz %d\n", read_cnt);
                printf("%f %f %f\n", acce_x, acce_y, acce_z);
                printf("\n");
            }

            read_cnt++;
            if (read_cnt == 100)
            {
                i2ctest_set = false;
                read_cnt = 0;
            }
        }
        __asm__("wfi");
    }
}

