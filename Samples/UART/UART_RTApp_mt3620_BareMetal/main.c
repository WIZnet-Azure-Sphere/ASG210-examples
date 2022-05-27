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

#define PRINT_INFO

 /******************************************************************************/
 /* Configurations */
 /******************************************************************************/
 /* UART */
static const uint8_t terminal_port_num = OS_HAL_UART_PORT0;

str_qData terminal;
u8* terminal_buf_p;

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
void uart_rx_service(uint8_t port, str_qData* queue);
void uart_terminal_isr(void);

_Noreturn void RTCoreMain(void)
{
    /* Init Vector Table */
    NVIC_SetupVectorTable();

    /* Init UART */
    mtk_os_hal_uart_ctlr_init(terminal_port_num);
    printf("\r\n");
    printf("=============================================\r\n");
    printf("UART_RTApp_mt3620_BareMetal\r\n");
    printf("UART Inited (port_num=%d)\r\n", terminal_port_num);
    printf("=============================================\r\n");

    terminal_buf_p = (u8*)malloc(1024);
    data_queueinit(&terminal, terminal_buf_p, 1024);

    mtk_os_hal_uart_set_irq(terminal_port_num, UART_INT_RX_BUFFER_FULL | UART_INT_LINE_STATUS);
    NVIC_Register(4, uart_terminal_isr);
    NVIC_EnableIRQ(4);

    u32 qstack;

    for (;;)
    {
        NVIC_DisableIRQ(4);

        qstack = data_queue_getlen(&terminal);

        if (qstack != 0)
        {
            u8 temp[1024];

#ifdef PRINT_INFO
            printf("\r\n>>Received %d bytes from UART\r\n", qstack);
#endif

            data_dequeue(&terminal, temp, qstack);
            for (int i = 0; i < qstack; i++)
            {
#if 1
                mtk_os_hal_uart_put_char(terminal_port_num, temp[i]);
#else
                printf("%c", temp[i]);
#endif
            }
#ifdef PRINT_INFO
            printf("\r\n>>end\r\n");
#endif
        }
        NVIC_EnableIRQ(4);
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

void uart_rx_clean(uint8_t port, str_qData* queue)
{
    char ch;
    u32 regv;
    u32 i;

    if (mtk_os_hal_uart_readreg(port, UART_VFIFO_EN_REG))
    {
        // VFIFO_EN == 1
    }
    else
    {
        // VFIFO_EN == 0

        // read rx_offset(number of data in RX FIFO)
        regv = mtk_os_hal_uart_readreg(port, RX_OFFSET);
        //printf("Received %d\r\n", regv);

        for (i = 0; i < regv; i++)
        {
            ch = mtk_os_hal_uart_get_char(port);
        }
    }
}

void uart_rx_service(uint8_t port, str_qData* queue)
{
    char ch;
    u32 regv;
    u32 i;

    regv = mtk_os_hal_uart_readreg(port, UART_IIR);
    //printf("IIR 0x%x\r\n", regv);

    switch (regv & 0x3F)
    {
    case 0x06:
        // Line Status Interrupt
        regv = mtk_os_hal_uart_readreg(port, UART_LSR);
        printf("Port %d\r\n", port);
        printf("LSR 0x%x\r\n", regv);

        if (regv & 1 << 7)
        {
            // RX FIFO Error Indicator
            printf("PE, FE, BI\r\n");
        }
        if (regv & 1 << 6)
        {
            // TX Holding Register(or TX FIFO) and the TX Shift Register are empty
            printf("TEMT\r\n");
        }
        if (regv & 1 << 5)
        {
            // TX Holding Register Empty
            printf("THRE\r\n");
        }
        if (regv & 1 << 4)
        {
            // Break Interrupt
            printf("BI\r\n");
            uart_rx_clean(port, queue);
            return;
        }
        if (regv & 1 << 3)
        {
            // Framing Error
            printf("FE\r\n");
            uart_rx_clean(port, queue);
            return;
        }
        if (regv & 1 << 2)
        {
            // Parity Error
            printf("PE\r\n");
            uart_rx_clean(port, queue);
            return;
        }
        if (regv & 1 << 1)
        {
            // Overrun Error
            printf("OE\r\n");
            uart_rx_clean(port, queue);
            return;
        }
        if (regv & 1 << 0)
        {
            // Data Ready
            printf("DR\r\n");
        }
        break;
    case 0x0c:
        // RX Data Timeout Interrupt
    case 0x04:
        // RX Data Received Interrupt
        if (mtk_os_hal_uart_readreg(port, UART_VFIFO_EN_REG))
        {
            // VFIFO_EN == 1
        }
        else
        {
            // VFIFO_EN == 0

            // read rx_offset(number of data in RX FIFO)
            regv = mtk_os_hal_uart_readreg(port, RX_OFFSET);
            //printf("Received %d\r\n", regv);

            for (i = 0; i < regv; i++)
            {
                ch = mtk_os_hal_uart_get_char(port);
                data_enqueue(queue, &ch);
            }
        }
        break;
    case 0x02:
        // TX Holding Register Empty Interrupt
        break;
    case 0x00:
        // Modem Status Control Interrupt
        break;
    case 0x10:
        // Software Flow Control Interrupt
        break;
    case 0x20:
        // Hardware Flow Control Interrupt
        break;
    case 0x01:
        // No interrupt pending
        break;

    }
}

void uart_terminal_isr(void)
{
    uart_rx_service(terminal_port_num, &terminal);
}

