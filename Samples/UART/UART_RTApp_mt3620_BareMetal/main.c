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

#include "printf.h"
#include "mt3620.h"
#include "os_hal_uart.h"
#include "os_hal_gpt.h"
#include "os_hal_gpio.h"

#define USE_LED_BLT

/******************************************************************************/
/* Configurations */
/******************************************************************************/
/* UART */
static const uint8_t uart_port_num_prtf = OS_HAL_UART_PORT0;
static const uint8_t uart_port_num = OS_HAL_UART_ISU3;

/* GPIO_4 for nSDA/RXD SEL */
static const uint8_t sel_nSDA = OS_HAL_GPIO_4;
/* GPIO_6 for 485/232 SEL */
static const uint8_t sel_n232 = OS_HAL_GPIO_6;
/* GPIO_7 for 485/422 SEL */
static const uint8_t sel_n422 = OS_HAL_GPIO_7;
/******************************************************************************/
/* Applicaiton Hooks */
/******************************************************************************/
/* Hook for "printf". */
void _putchar(char character)
{
    mtk_os_hal_uart_put_char(uart_port_num_prtf, character);
    if (character == '\n')
        mtk_os_hal_uart_put_char(uart_port_num_prtf, '\r');
}

/******************************************************************************/
/* Functions */
/******************************************************************************/
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

void uart_dbg_isr(void)
{
    char ch;
    ch = mtk_os_hal_uart_get_char(uart_port_num_prtf);
    printf(">%c\r\n", ch);
    mtk_os_hal_uart_put_char(uart_port_num, ch);
}

_Noreturn void RTCoreMain(void)
{
    /* Init Vector Table */
    NVIC_SetupVectorTable();

    // 232
    gpio_output(sel_n232, OS_HAL_GPIO_DATA_LOW);
    gpio_output(sel_nSDA, OS_HAL_GPIO_DATA_HIGH);

    /* Init UART */
    mtk_os_hal_uart_ctlr_init(uart_port_num_prtf);
    printf("\r\n");
    printf("=============================================\r\n");
    printf("UART_RTApp_mt3620_BareMetal\r\n");
    printf("UART Inited (port_num=%d)\r\n", uart_port_num_prtf);
    printf("=============================================\r\n");

    mtk_os_hal_uart_set_irq(uart_port_num_prtf, UART_INT_RX_BUFFER_FULL | UART_INT_LINE_STATUS);

    NVIC_Register(4, uart_dbg_isr);
    NVIC_EnableIRQ(4);

    mtk_os_hal_uart_ctlr_init(uart_port_num);

    for (;;)
        __asm__("wfi");
}

