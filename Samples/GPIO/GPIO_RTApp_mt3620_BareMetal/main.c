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

/******************************************************************************/
/* Configurations */
/******************************************************************************/
/* UART */
static const uint8_t uart_port_num = OS_HAL_UART_PORT0;

/* GPT */
/* GPT0 for LED blinking */
static const uint8_t gpt_timer_led = OS_HAL_GPT0;
/* GPE3 for Button status scan */
static const uint8_t gpt_timer_button = OS_HAL_GPT3;
/* 500ms, GPT0 clock speed: 1KHz or 32KHz */
static const uint32_t gpt_timer_led_blinking_perios_ms = 500;
/* 100ms, GPT3 clock speed: 1MHz */
static const uint32_t gpt_timer_button_scan_perios_ms = 100000;

/* GPIO */
/* GPIO_58 for LED1_Control */
static const uint8_t gpio_led1 = OS_HAL_GPIO_58;
/* GPIO_46 for LED2 Control */
static const uint8_t gpio_led2 = OS_HAL_GPIO_46;
/* GPIO_60 for LED3 Control */
static const uint8_t gpio_led3 = OS_HAL_GPIO_60;
/* GPIO_45 for LED4 Control */
static const uint8_t gpio_led4 = OS_HAL_GPIO_45;

/* GPIO_56 */
static const uint8_t gpio_pin_a = OS_HAL_GPIO_56;
/* GPIO_59 */
static const uint8_t gpio_pin_b = OS_HAL_GPIO_59;
/* GPIO_57 */
static const uint8_t gpio_pin_c = OS_HAL_GPIO_57;
/* GPIO_13 for USER_SW */
static const uint8_t gpio_button = OS_HAL_GPIO_13;


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

static int gpio_input(u8 gpio_no, os_hal_gpio_data *pvalue)
{
    mtk_os_hal_gpio_set_direction(gpio_no, OS_HAL_GPIO_DIR_INPUT);
    mtk_os_hal_gpio_get_input(gpio_no, pvalue);
    return 0;
}

static void TimerHandlerGpt0(void *cb_data)
{
    static bool ledOn = true;

    /* Toggle LED Status */
    ledOn = !ledOn;
    printf("GPIO Output %d : %d\r\n", gpio_led1, ledOn);
    gpio_output(gpio_led1, ledOn);

    /* Toggle GPIO */
#if 0
    printf("GPIO Output %d : %d\r\n", gpio_led3, ledOn);
    gpio_output(gpio_led3, ledOn);

    printf("GPIO Output %d : %d\r\n", gpio_led4, ledOn);
    gpio_output(gpio_led4, ledOn);
#endif

#if 0
    printf("GPIO Output %d : %d\r\n", gpio_pin_a, ledOn);
    gpio_output(gpio_pin_a, ledOn);

    printf("GPIO Output %d : %d\r\n", gpio_pin_b, ledOn);
    gpio_output(gpio_pin_b, ledOn);

    printf("GPIO Output %d : %d\r\n", gpio_pin_c, ledOn);
    gpio_output(gpio_pin_c, ledOn);
#endif
}

static void TimerHandlerGpt3(void *cb_data)
{
    os_hal_gpio_data button_status;
    static os_hal_gpio_data button_prev;

    /* Set LED_Blue according to Button */
    gpio_input(gpio_button, &button_status);

    if (button_status != button_prev)
    {
        button_prev = button_status;

        printf("GPIO Input %d : %d -> ", gpio_button, button_status);

        if (button_status)
        {
            printf("GPIO Output %d : %d\r\n", gpio_led2, OS_HAL_GPIO_DATA_HIGH);
            gpio_output(gpio_led2, OS_HAL_GPIO_DATA_HIGH);
        }
        else
        {
            printf("GPIO Output %d : %d\r\n", gpio_led2, OS_HAL_GPIO_DATA_LOW);
            gpio_output(gpio_led2, OS_HAL_GPIO_DATA_LOW);
        }
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
    printf("GPIO_RTApp_mt3620_BareMetal\r\n");
    printf("UART Inited (port_num=%d)\r\n", uart_port_num);
    printf("=============================================\r\n");

    /* Init GPT */
    mtk_os_hal_gpt_init();

    /* Init GPT0 for LED blinking, repeat mode. */
    gpt0_int.gpt_cb_hdl = TimerHandlerGpt0;
    gpt0_int.gpt_cb_data = NULL;
    mtk_os_hal_gpt_config(gpt_timer_led, false, &gpt0_int);
    mtk_os_hal_gpt_reset_timer(gpt_timer_led,
                               gpt_timer_led_blinking_perios_ms, true);
    mtk_os_hal_gpt_start(gpt_timer_led);

    /* Init GPT3 for button status monitor, one-shot mode. */
    gpt3_int.gpt_cb_hdl = TimerHandlerGpt3;
    gpt3_int.gpt_cb_data = NULL;
    mtk_os_hal_gpt_config(gpt_timer_button, false, &gpt3_int);
    mtk_os_hal_gpt_reset_timer(gpt_timer_button,
                               gpt_timer_button_scan_perios_ms, false);
    mtk_os_hal_gpt_start(gpt_timer_button);

    for (;;)
        __asm__("wfi");
}

