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

 /******************************************************************************/
 /* Configurations */
 /******************************************************************************/
 /* UART */
static const uint8_t uart_port_num_prtf = OS_HAL_UART_PORT0;
static const uint8_t uart_port_num = OS_HAL_UART_ISU2;

/* GPIO_40 for SD nReset*/
static const uint8_t sd_nreset = OS_HAL_GPIO_40;

str_qData terminal;
u8* terminal_buf_p;

str_qData openlog;
u8* openlog_buf_p;

bool openlog_ready = false;
bool openlog_standy = false;
bool terminal_sent = false;

/******************************************************************************/
/* Applicaiton Hooks */
/******************************************************************************/
/* Hook for "printf". */
void _putchar(char character);
/******************************************************************************/
/* Functions */
/******************************************************************************/
static int gpio_output(u8 gpio_no, u8 level);
static int gpio_input(u8 gpio_no, os_hal_gpio_data* pvalue);
void uart_dbg_isr(void);
void uart_openlog_isr(void);

_Noreturn void RTCoreMain(void)
{
    /* Init Vector Table */
    NVIC_SetupVectorTable();

    /* Init UART */
    mtk_os_hal_uart_ctlr_init(uart_port_num_prtf);
    printf("\r\n");
    printf("=============================================\r\n");
    printf("UART_Openlog_RTApp_mt3620_BareMetal_BareMetal\r\n");
    printf("UART Inited (port_num=%d)\r\n", uart_port_num_prtf);
    printf("=============================================\r\n");

    //int uart_vector = 55;   // ISU2 UART
    //int uart_vector = 59;   // ISU3 UART
    int uart_vector = 4;   // OS_HAL_UART_PORT0

    mtk_os_hal_uart_set_irq(uart_port_num_prtf, UART_INT_RX_BUFFER_FULL | UART_INT_LINE_STATUS);
    //mtk_os_hal_uart_set_hw_fc(uart_port_num_prtf, UART_EFR_HW_FC_RTS);

    NVIC_Register(uart_vector, uart_dbg_isr);
    NVIC_EnableIRQ(uart_vector);

    gpio_output(sd_nreset, OS_HAL_GPIO_DATA_LOW);
    osai_delay_ms(100);

    // UART2
    mtk_os_hal_uart_ctlr_init(uart_port_num);
    mtk_os_hal_uart_set_baudrate(uart_port_num, 9600);
    mtk_os_hal_uart_set_irq(uart_port_num, UART_INT_RX_BUFFER_FULL | UART_INT_LINE_STATUS);
    //mtk_os_hal_uart_set_hw_fc(uart_port_num, UART_EFR_HW_FC_RTS);
    NVIC_Register(55, uart_openlog_isr);
    NVIC_EnableIRQ(55);
    
    u32 qstack;
    openlog_buf_p = (u8*)malloc(1024);
    data_queueinit(&openlog, openlog_buf_p, 1024);

    terminal_buf_p = (u8*)malloc(1024);
    data_queueinit(&terminal, terminal_buf_p, 1024);

    gpio_output(sd_nreset, OS_HAL_GPIO_DATA_HIGH);

    while (1)
    {
        qstack = data_queue_getlen(&openlog);

        if (qstack != 0)
        {
            u8 temp;

            //printf("qstack = %d\r\n", qstack);
            data_dequeue(&openlog, &temp, 1);
            //printf(">>0x%x %c\r\n", temp, temp);
            printf("%c", temp);

            if (temp == '<' && openlog_ready == false)
            {
                mtk_os_hal_uart_put_char(uart_port_num, 26);
                mtk_os_hal_uart_put_char(uart_port_num, 26);
                mtk_os_hal_uart_put_char(uart_port_num, 26);

                openlog_ready = true;
            }

            if (temp == 0xa || temp == '>')
            {
                // line feed 0xa
                //printf("\r\n>>Openlog Standby\r\n");
                openlog_standy = true;
            }
            else
            {
                if (temp == '<')
                {
                    openlog_standy = true;
                }
                else
                {
                    openlog_standy = false;
                }
            }
        }
        else
        {
            if (terminal_sent == true && openlog_standy == true)
            {
                u32 len = data_queue_getlen(&terminal);
                u32 i;
                u8* data_p = (u8*)malloc(len);
                data_dequeue(&terminal, data_p, len);

                printf(">>Received\r\n");
                for (i = 0; i < len; i++)
                {
                    //printf("0x%x %c\r\n", *(data_p+i), *(data_p + i));
                    printf("%c", *(data_p + i), *(data_p + i));
                }
                printf(">>end\r\n");

                for (i = 0; i < len; i++)
                {
                    mtk_os_hal_uart_put_char(uart_port_num, *(data_p + i));
                }
                free(data_p);

                terminal_sent = false;
            }
        }
    }
}

void _putchar(char character)
{
    mtk_os_hal_uart_put_char(uart_port_num_prtf, character);
    if (character == '\n')
        mtk_os_hal_uart_put_char(uart_port_num_prtf, '\r');
}

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

void uart_dbg_isr(void)
{
    char ch;
    ch = mtk_os_hal_uart_get_char(uart_port_num_prtf);
    data_enqueue(&terminal, &ch);

    if (ch == 0x0a)
    {
        terminal_sent = true;
    }
    else
    {
        terminal_sent = false;
    }
}

void uart_openlog_isr(void)
{
    char ch;
    ch = mtk_os_hal_uart_get_char(uart_port_num);
    data_enqueue(&openlog, &ch);
}
