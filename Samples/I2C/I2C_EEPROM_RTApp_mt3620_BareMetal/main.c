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
/* GPIO_13 for Button_A USER_SW */
static const uint8_t gpio_button_a = OS_HAL_GPIO_13;

static const uint8_t i2c_master_speed = I2C_SCL_50kHz;

#ifdef I2C_MASTER
static const uint8_t i2c_master_port_num = OS_HAL_I2C_ISU2;
static const uint8_t i2c_slave_addr = (0xa0>>1);
#else
static const uint8_t i2c_slave_port_num = OS_HAL_I2C_ISU3;
static const uint8_t i2c_slave_addr = 9;
#endif

#ifdef I2C_MASTER
static uint8_t* i2c_master_write_buf;
static uint8_t* i2c_master_read_buf;
#else
static uint8_t* i2c_slave_read_buf;
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
#ifdef I2C_MASTER
bool i2c_Write(uint8_t slavaddr)
{
    static __attribute__((section(".sysram"))) uint8_t addrbuf[2];
    static __attribute__((section(".sysram"))) uint8_t sendbuf[12];
    static __attribute__((section(".sysram"))) uint8_t wbuf[10];
    static __attribute__((section(".sysram"))) uint8_t rbuf[10];

    uint32_t i, j;
    for (i = 0, j = '0'; i < sizeof(wbuf); i++)
    {
        wbuf[i] = j++;
        rbuf[i] = 0;
        printf("wbuf[%d] = %c(0x%x)\r\n", i, wbuf[i], wbuf[i]);
    }

    addrbuf[0] = 0x0;
    addrbuf[1] = 0x0;

    memcpy(sendbuf, addrbuf, 2);
    memcpy(sendbuf+2, wbuf, sizeof(sendbuf)-2);

    for(i=0; i<3; i++)
    {
        printf("[%d] = 0x%x\r\n", i, sendbuf[i]);
    }

    int ret = mtk_os_hal_i2c_write(i2c_master_port_num, slavaddr, sendbuf, sizeof(sendbuf));
    if (I2C_OK == ret)
    {
        printf("I2C Send slavaddr 0x%x Completed\r\n", slavaddr);
    }
    else
    {
        printf("I2C Send slavaddr 0x%x Failed %d \r\n", slavaddr, ret);
        return false;
    }

    osai_delay_ms(100);

    ret = mtk_os_hal_i2c_write_read(i2c_master_port_num, slavaddr, addrbuf, rbuf, sizeof(addrbuf), 10);
    if (I2C_OK == ret)
    {
        printf("I2C Send slavaddr 0x%x Completed\r\n", slavaddr);
    }
    else
    {
        printf("I2C Send slavaddr 0x%x Failed %d \r\n", slavaddr, ret);
        return false;
    }
    
    #if 0
    ret = mtk_os_hal_i2c_read(i2c_master_port_num, slavaddr, rbuf, 1);
    if (I2C_OK == ret)
    {
        printf("I2C Receive slavaddr 0x%x Completed\r\n", slavaddr);
    }
    else
    {
        printf("I2C Receive slavaddr 0x%x Failed\r\n", slavaddr);
        return false;
    }
    #endif
    
    for (i = 0; i < 10; i++)
    {
        printf("rbuf[%d] = %c(0x%x)\r\n", i, rbuf[i], rbuf[i]);
    }

    return true;
}
#else
bool i2c_read()
{
    static __attribute__((section(".sysram"))) uint8_t wbuf[10];
    static __attribute__((section(".sysram"))) uint8_t rbuf[10];

    uint32_t i, j;
    for (i = 0; i < sizeof(wbuf); i++)
    {
        rbuf[i] = 0;
    }

    int ret = mtk_os_hal_i2c_slave_rx(i2c_slave_port_num, rbuf, sizeof(rbuf), I2C_SLAVE_TIMEOUT);
    if (I2C_OK == ret)
    {
        printf("I2C Receive Completed\r\n");
    }
    else
    {
        return false;
    }

    for (i = 0, j = sizeof(rbuf) - 1; i < sizeof(rbuf); i++)
    {
        printf("rbuf[%d] = %c(0x%x)\r\n", i, rbuf[i], rbuf[i]);
        wbuf[i] = rbuf[j--];
    }

    ret = mtk_os_hal_i2c_slave_tx(i2c_slave_port_num, wbuf, sizeof(wbuf), I2C_SLAVE_TIMEOUT);
    if (I2C_OK == ret)
    {
        printf("I2C Send Completed\r\n");
    }
    else
    {
        printf("I2C Send Failed\r\n");
        return false;
    }

    for (i = 0, j = sizeof(wbuf); i < sizeof(wbuf); i++)
    {
        printf("wbuf[%d] = %c(0x%x)\r\n", i, wbuf[i], wbuf[i]);
    }

    return true;
}
#endif

static int gpio_output(u8 gpio_no, u8 level)
{
#if 1
    // 20201030 taylor
    mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_OUTPUT);
    mtk_os_hal_gpio_set_output(gpio_no, level);
#else
    int ret;

    ret = mtk_os_hal_gpio_request(gpio_no);
    if (ret != 0)
    {
        printf("request gpio[%d] fail\n", gpio_no);
        return ret;
    }

    mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_OUTPUT);
    mtk_os_hal_gpio_set_output(gpio_no, level);
    ret = mtk_os_hal_gpio_free(gpio_no);
    if (ret != 0)
    {
        printf("free gpio[%d] fail\n", gpio_no);
        return 0;
    }
#endif
    return 0;
}

static int gpio_input(u8 gpio_no, os_hal_gpio_data* pvalue)
{
#if 1
    // 20201030 taylor
    mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_INPUT);
    mtk_os_hal_gpio_get_input(gpio_no, pvalue);

#else
    u8 ret;

    ret = mtk_os_hal_gpio_request(gpio_no);
    if (ret != 0)
    {
        printf("request gpio[%d] fail\n", gpio_no);
        return ret;
    }
    mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_INPUT);
    mtk_os_hal_gpio_get_input(gpio_no, pvalue);
    ret = mtk_os_hal_gpio_free(gpio_no);
    if (ret != 0)
    {
        printf("free gpio[%d] fail\n", gpio_no);
        return ret;
    }
#endif
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

    /* Init Vector Table */
    NVIC_SetupVectorTable();

    /* Init UART */
    mtk_os_hal_uart_ctlr_init(uart_port_num);
    printf("\r\n");
    printf("=============================================\r\n");
    printf("I2C_EEPROM_RTApp_mt3620_BareMetal\r\n");
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

    /* Init I2C Master/Slave */
#ifdef I2C_MASTER
    mtk_os_hal_i2c_ctrl_init(i2c_master_port_num);
    mtk_os_hal_i2c_speed_init(i2c_master_port_num, i2c_master_speed);
#else
    mtk_os_hal_i2c_ctrl_init(i2c_slave_port_num);
    mtk_os_hal_i2c_set_slave_addr(i2c_slave_port_num, i2c_slave_addr);
#endif

    for (;;)
    {
#ifdef I2C_MASTER
        if (i2ctest_set)
        {
            i2c_Write(i2c_slave_addr);
            i2ctest_set = false;
        }
#else
        i2c_read(i2c_slave_addr);
#endif
        __asm__("wfi");
    }
}

