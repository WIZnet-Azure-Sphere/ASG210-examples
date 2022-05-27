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
#include "queue.h"
#include "hdl_uart.h"

 /*
 1
 2 1ms
 3 10ms
 4 100ms
 5 1000ms
 */
#define TEST 1

#if TEST == 1
#if 1
#define PRINT_1MS
#endif
#if 0
#define PRINT_10MS
#endif
#if 0
#define PRINT_100MS
#endif
#if 0
#define PRINT_1000MS
#endif
#endif

/******************************************************************************/
/* Configurations */
/******************************************************************************/
/* UART */
static const uint8_t terminal_port_num = OS_HAL_UART_PORT0;

static const uint8_t gpt_timer_tick = OS_HAL_GPT0;
static const uint32_t gpt_timer_tick_perios = 1;
volatile uint32_t gpt_timer_tick_cnt = 0;
volatile uint32_t gpt_timer_tick_1ms = 0;
volatile uint32_t gpt_timer_tick_10ms = 0;
volatile uint32_t gpt_timer_tick_100ms = 0;
volatile uint32_t gpt_timer_tick_1000ms = 0;

/******************************************************************************/
/* Applicaiton Hooks */
/******************************************************************************/
/* Hook for "printf". */
void _putchar(char character);
/******************************************************************************/
/* Functions */
/******************************************************************************/
int gpio_output(u8 gpio_no, u8 level);
int gpio_input(u8 gpio_no, os_hal_gpio_data* pvalue);
bool time_over_check(uint32_t time_stamp, uint32_t* tick, uint32_t cnt);
void gpt_timer_tick_handler(void);

_Noreturn void RTCoreMain(void)
{
    /* Init Vector Table */
    NVIC_SetupVectorTable();

    /* Init UART */
    mtk_os_hal_uart_ctlr_init(terminal_port_num);
    printf("\r\n");
    printf("=============================================\r\n");
    printf("Timer_RTApp_mt3620_BareMetal\r\n");
    printf("UART Inited (port_num=%d)\r\n", terminal_port_num);
    printf("=============================================\r\n");

    struct os_gpt_int gpt_timer_tick_init;

    gpt_timer_tick_init.gpt_cb_hdl = gpt_timer_tick_handler;
    gpt_timer_tick_init.gpt_cb_data = NULL;

    mtk_os_hal_gpt_init();
    mtk_os_hal_gpt_config(gpt_timer_tick, false, &gpt_timer_tick_init);
    mtk_os_hal_gpt_reset_timer(gpt_timer_tick, gpt_timer_tick_perios, true);
    mtk_os_hal_gpt_start(gpt_timer_tick);

    u32 qstack;

#if 0
    // test overflow
    gpt_timer_tick_1ms = 0xffffffff - 1000;
    gpt_timer_tick_10ms = 0xffffffff - 100;
    gpt_timer_tick_100ms = 0xffffffff - 10;
    gpt_timer_tick_1000ms = 0xffffffff - 1;
#endif

    uint32_t time_stamp_1ms = gpt_timer_tick_1ms;
    uint32_t time_stamp_10ms = gpt_timer_tick_10ms;
    uint32_t time_stamp_100ms = gpt_timer_tick_100ms;
    uint32_t time_stamp_1000ms = gpt_timer_tick_1000ms;
    
    for (;;)
    {
#if TEST == 1
        if (true == time_over_check(time_stamp_1ms, &gpt_timer_tick_1ms, 1))
        {
#ifdef PRINT_1MS
            printf("1ms\r\n");
            printf("%lu %lu %lu %lu\r\n", gpt_timer_tick_1ms, gpt_timer_tick_10ms, gpt_timer_tick_100ms, gpt_timer_tick_1000ms);
#endif
            time_stamp_1ms = gpt_timer_tick_1ms;
        }
        if (true == time_over_check(time_stamp_10ms, &gpt_timer_tick_10ms, 1))
        {
#ifdef PRINT_10MS
            printf("10ms\r\n");
            printf("%lu %lu %lu %lu\r\n", gpt_timer_tick_1ms, gpt_timer_tick_10ms, gpt_timer_tick_100ms, gpt_timer_tick_1000ms);
#endif
            time_stamp_10ms = gpt_timer_tick_10ms;
        }
        if (true == time_over_check(time_stamp_100ms, &gpt_timer_tick_100ms, 1))
        {
#ifdef PRINT_100MS
            printf("100ms\r\n");
            printf("%lu %lu %lu %lu\r\n", gpt_timer_tick_1ms, gpt_timer_tick_10ms, gpt_timer_tick_100ms, gpt_timer_tick_1000ms);
#endif
            time_stamp_100ms = gpt_timer_tick_100ms;
        }
        if (true == time_over_check(time_stamp_1000ms, &gpt_timer_tick_1000ms, 1))
        {
#ifdef PRINT_1000MS
            printf("1000ms\r\n");
            printf("%lu %lu %lu %lu\r\n", gpt_timer_tick_1ms, gpt_timer_tick_10ms, gpt_timer_tick_100ms, gpt_timer_tick_1000ms);
#endif
            time_stamp_1000ms = gpt_timer_tick_1000ms;
        }
#endif

#if TEST == 2
        static u32 i = 1;

        if (true == time_over_check(time_stamp_1ms, &gpt_timer_tick_1ms, i))
        {
            printf("1ms\r\n");
            printf("1ms tick = %lu - %lu = %lu over %lu\r\n", time_stamp_1ms, gpt_timer_tick_1ms, gpt_timer_tick_1ms - time_stamp_1ms, i++);
            time_stamp_1ms = gpt_timer_tick_1ms;
        }
#endif

#if TEST == 3
        static u32 i = 1;

        if (true == time_over_check(time_stamp_10ms, &gpt_timer_tick_10ms, i))
        {
            printf("10ms\r\n");
            printf("10ms tick = %lu - %lu = %lu over %lu\r\n", time_stamp_10ms, gpt_timer_tick_10ms, gpt_timer_tick_10ms - time_stamp_10ms, i++);
            time_stamp_10ms = gpt_timer_tick_10ms;
        }
#endif

#if TEST == 4
        static u32 i = 1;

        if (true == time_over_check(time_stamp_100ms, &gpt_timer_tick_100ms, i))
        {
            printf("100ms\r\n");
            printf("100ms tick = %lu - %lu = %lu over %lu\r\n", time_stamp_100ms, gpt_timer_tick_100ms, gpt_timer_tick_100ms - time_stamp_100ms, i++);
            time_stamp_100ms = gpt_timer_tick_100ms;
        }
#endif

#if TEST == 5
        static u32 i = 1;

        if (true == time_over_check(time_stamp_1000ms, &gpt_timer_tick_1000ms, i))
        {
            printf("1000ms\r\n");
            printf("1000ms tick = %lu - %lu = %lu over %lu\r\n", time_stamp_1000ms, gpt_timer_tick_1000ms, gpt_timer_tick_1000ms - time_stamp_1000ms, i++);
            time_stamp_1000ms = gpt_timer_tick_1000ms;
        }
#endif
    }
}

void _putchar(char character)
{
    mtk_os_hal_uart_put_char(terminal_port_num, character);
    if (character == '\n')
        mtk_os_hal_uart_put_char(terminal_port_num, '\r');
}

int gpio_output(u8 gpio_no, u8 level)
{
    mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_OUTPUT);
    mtk_os_hal_gpio_set_output(gpio_no, level);
    return 0;
}

int gpio_input(u8 gpio_no, os_hal_gpio_data* pvalue)
{
    mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_INPUT);
    mtk_os_hal_gpio_get_input(gpio_no, pvalue);
    return 0;
}

bool time_over_check(uint32_t time_stamp, uint32_t* tick, uint32_t cnt)
{
    uint32_t cur_time;

    cur_time = *tick;

    if (cur_time < time_stamp)
    {
        cur_time += (0xffffffff - time_stamp) + 1;
        if (cur_time >= cnt)
        {
#if 0
            printf("time_stamp = %lu cur_time = %lu cnt = %lu\r\n", time_stamp, cur_time-1, cnt);
#endif
            return true;
        }
    }
    else
    {
        if ((cur_time - time_stamp) >= cnt)
        {
#if 0
            printf("time_stamp = %lu cur_time = %lu cnt = %lu\r\n", time_stamp, cur_time, cnt);
#endif
            return true;
        }
    }

    return false;
}

void gpt_timer_tick_handler(void)
{
    if (gpt_timer_tick_cnt == 1000)
    {
        gpt_timer_tick_cnt = 0;
    }
    gpt_timer_tick_cnt++;

    if (gpt_timer_tick_cnt % 1 == 0)
    {
        gpt_timer_tick_1ms++;
    }

    if (gpt_timer_tick_cnt % 10 == 0)
    {
        gpt_timer_tick_10ms++;
    }

    if (gpt_timer_tick_cnt % 100 == 0)
    {
        gpt_timer_tick_100ms++;
    }

    if (gpt_timer_tick_cnt % 1000 == 0)
    {
        gpt_timer_tick_1000ms++;
    }
}

