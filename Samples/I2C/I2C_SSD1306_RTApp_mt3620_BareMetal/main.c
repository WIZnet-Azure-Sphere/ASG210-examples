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
#include "SSD1306.h"

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

static const uint8_t i2c_master_speed = I2C_SCL_1000kHz;
static const uint8_t i2c_master_port_num = OS_HAL_I2C_ISU3;

#define I2C_MIN_LEN 1
#define I2C_MAX_LEN 64 /* For AVNET development board, please change to 8. */
#define I2C_SLAVE_TIMEOUT 10000 /* 10000ms */

static const uint8_t imageData1[] =
{
#include "image_1.h"
};

static const uint8_t imageData2[] =
{
#include "image_2.h"
};

#define IMAGE_COUNT 2
const void *image[IMAGE_COUNT] = {0};
uintptr_t imageSize = 0;
unsigned imageIndex = 0;
bool imageset = 0;

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

static void TimerHandlerGpt3(void *cb_data)
{
    os_hal_gpio_data button_status;

    /* Set LED_Blue according to Button_A */
    gpio_input(gpio_button_a, &button_status);

    if (!button_status)
    {
        imageset = true;
    }

    /* Restart GPT3 since it's one-shot mode. */
    mtk_os_hal_gpt_restart(gpt_timer_button);
}

static void imageRemap(uint8_t *dst, const uint8_t *src)
{
    uint8_t *dp;
    unsigned col;
    for (dp = dst, col = 0; col < SSD1306_WIDTH; col++)
    {
        unsigned page;
        for (page = 0; page < (SSD1306_HEIGHT / 8); page++, dp++)
        {
            uint8_t byte = 0;
            unsigned i;
            for (i = 0; i < 8; i++)
            {
                unsigned y = (page * 8) + i;
                // X inverted to perform bit reversal as PBM is big-endian for bitmaps.
                unsigned xinv = (SSD1306_WIDTH - (col + 1));
                unsigned pixel = (src[((y * SSD1306_WIDTH) + col) / 8] >> (xinv % 8)) & 1;
                // Invert source pixel as PGM stores images inverted.
                byte |= !pixel << i;
            }
            *dp = byte;
        }
    }
}

_Noreturn void RTCoreMain(void)
{
    struct os_gpt_int gpt3_int;

    /* Init Vector Table */
    NVIC_SetupVectorTable();

    /* Init UART */
    mtk_os_hal_uart_ctlr_init(uart_port_num);
    printf("\r\n");
    printf("=============================================\r\n");
    printf("I2C_SSD1306_RTApp_mt3620_BareMetal\r\n");
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

    /* Init I2C Master */
    mtk_os_hal_i2c_ctrl_init(i2c_master_port_num);
    mtk_os_hal_i2c_speed_init(i2c_master_port_num, i2c_master_speed);

    // Initialise the SSD1306.
    bool oledStatus = false;
    oledStatus = Ssd1306_Init(i2c_master_port_num);

    if (!oledStatus)
    {
        printf("Error: OLED initialization failed!\r\n");
    }

    // Remap the image data to match the correct format for the screen
    uint8_t remapData[2][sizeof(imageData1)];
    imageRemap(remapData[0], imageData1);
    imageRemap(remapData[1], imageData2);

    image[0] = remapData[0];
    image[1] = remapData[1];
    imageSize = sizeof(remapData[0]);
    imageIndex = 0;

    SSD1306_WriteFullBuffer(i2c_master_port_num, remapData[0], imageSize);
    SSD1306_SetDisplayAllOn(i2c_master_port_num, false);

    printf("Press A to toggle image.\r\n");

    for (;;)
    {
        if (imageset)
        {
            imageIndex = (imageIndex + 1) % IMAGE_COUNT;
            SSD1306_WriteFullBuffer(i2c_master_port_num, image[imageIndex], imageSize);
            imageset = false;
        }

        __asm__("wfi");
    }
}

