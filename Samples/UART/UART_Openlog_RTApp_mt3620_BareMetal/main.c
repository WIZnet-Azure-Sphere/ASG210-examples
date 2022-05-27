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

 /******************************************************************************/
 /* Configurations */
 /******************************************************************************/
 /* UART */
static const uint8_t terminal_port_num = OS_HAL_UART_PORT0;
static const uint8_t openlog_port_num = OS_HAL_UART_ISU2;

/* GPIO_60 for SD nReset*/
static const uint8_t sd_nreset = OS_HAL_GPIO_60;

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
void openlog_reset(bool set);
void uart_rx_clean(uint8_t port, str_qData *queue);
void uart_rx_service(uint8_t port, str_qData* queue);
void uart_terminal_isr(void);
void uart_openlog_isr(void);

_Noreturn void RTCoreMain(void)
{
    /* Init Vector Table */
    NVIC_SetupVectorTable();

    /* Init UART */
    mtk_os_hal_uart_ctlr_init(terminal_port_num);
    printf("\r\n");
    printf("=============================================\r\n");
    printf("UART_Openlog_RTApp_mt3620_BareMetal\r\n");
    printf("UART Inited (port_num=%d)\r\n", terminal_port_num);
    printf("=============================================\r\n");

    terminal_buf_p = (u8*)malloc(1024);
    data_queueinit(&terminal, terminal_buf_p, 1024);
    
    mtk_os_hal_uart_set_irq(terminal_port_num, UART_INT_RX_BUFFER_FULL | UART_INT_LINE_STATUS);
    NVIC_Register(4, uart_terminal_isr);
    NVIC_EnableIRQ(4);

    openlog_reset(true);
    
    openlog_buf_p = (u8*)malloc(1024);
    data_queueinit(&openlog, openlog_buf_p, 1024);
    
    mtk_os_hal_uart_ctlr_init(openlog_port_num);
    mtk_os_hal_uart_set_baudrate(openlog_port_num, 9600);
    mtk_os_hal_uart_set_irq(openlog_port_num, UART_INT_RX_BUFFER_FULL | UART_INT_LINE_STATUS);
    NVIC_Register(55, uart_openlog_isr);
    NVIC_EnableIRQ(55);
    
    openlog_reset(false);
    
    u32 qstack;

    while (1)
    {
        NVIC_DisableIRQ(55);
        
        qstack = data_queue_getlen(&openlog);

        if (qstack != 0)
        {
            u8 temp;
            
            data_dequeue(&openlog, &temp, 1);
            printf("%c", temp);

            if (temp == '<' && openlog_ready == false)
            {
                mtk_os_hal_uart_put_char(openlog_port_num, 26);
                mtk_os_hal_uart_put_char(openlog_port_num, 26);
                mtk_os_hal_uart_put_char(openlog_port_num, 26);

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

            NVIC_EnableIRQ(55);
        }
        else
        {
            NVIC_EnableIRQ(55);
            
            if (terminal_sent == true && openlog_standy == true)
            {
                NVIC_DisableIRQ(4);
                
                u32 len = data_queue_getlen(&terminal);
                u32 i;
                u8* data_p = (u8*)malloc(len);
                data_dequeue(&terminal, data_p, len);

                NVIC_EnableIRQ(4);

                printf(">>Received\r\n");
                for (i = 0; i < len; i++)
                {
                    //printf("0x%x %c\r\n", *(data_p+i), *(data_p + i));
                    printf("%c", *(data_p + i));
                }
                printf(">>end\r\n");

                for (i = 0; i < len; i++)
                {
                    mtk_os_hal_uart_put_char(openlog_port_num, *(data_p + i));
                }
                free(data_p);

                terminal_sent = false;
            }
        }
    }
}

void _putchar(char character)
{
    mtk_os_hal_uart_put_char(terminal_port_num, character);
    if (character == '\n')
        mtk_os_hal_uart_put_char(terminal_port_num, '\r');
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

void openlog_reset(bool set)
{
    os_hal_gpio_data sd_nreset_state;

    if(set == true)
    {
        gpio_output(sd_nreset, OS_HAL_GPIO_DATA_LOW);
        do
        {
            mtk_os_hal_gpio_get_output(sd_nreset, &sd_nreset_state);
        } while (sd_nreset_state != OS_HAL_GPIO_DATA_LOW);
    }
    else
    {
        gpio_output(sd_nreset, OS_HAL_GPIO_DATA_HIGH);
        do
        {
            mtk_os_hal_gpio_get_output(sd_nreset, &sd_nreset_state);
        } while (sd_nreset_state != OS_HAL_GPIO_DATA_HIGH);
    }
}

void uart_rx_clean(uint8_t port, str_qData *queue)
{
    char ch;
    u32 regv;
    u32 i;
    
    if(mtk_os_hal_uart_readreg(port, UART_VFIFO_EN_REG))
    {
        // VFIFO_EN == 1
    }
    else
    {
        // VFIFO_EN == 0

        // read rx_offset(number of data in RX FIFO)
        regv = mtk_os_hal_uart_readreg(port, RX_OFFSET);
        //printf("Received %d\r\n", regv);

        for(i=0; i<regv; i++)
        {
            ch = mtk_os_hal_uart_get_char(port);
        }
    }
}

void uart_rx_service(uint8_t port, str_qData *queue)
{
    char ch;
    u32 regv;
    u32 i;

    regv = mtk_os_hal_uart_readreg(port, UART_IIR);
    //printf("IIR 0x%x\r\n", regv);

    switch(regv & 0x3F)
    {
        case 0x06:
            // Line Status Interrupt
            regv = mtk_os_hal_uart_readreg(port, UART_LSR);
            printf("Port %d\r\n", port);
            printf("LSR 0x%x\r\n", regv);

            if(regv & 1<<7)
            {
                // RX FIFO Error Indicator
                printf("PE, FE, BI\r\n");
            }
            if(regv & 1<<6)
            {
                // TX Holding Register(or TX FIFO) and the TX Shift Register are empty
                printf("TEMT\r\n");
            }
            if(regv & 1<<5)
            {
                // TX Holding Register Empty
                printf("THRE\r\n");
            }
            if(regv & 1<<4)
            {
                // Break Interrupt
                printf("BI\r\n");
                uart_rx_clean(port, queue);
                return ;
            }
            if(regv & 1<<3)
            {
                // Framing Error
                printf("FE\r\n");
                uart_rx_clean(port, queue);
                return ;
            }
            if(regv & 1<<2)
            {
                // Parity Error
                printf("PE\r\n");
                uart_rx_clean(port, queue);
                return ;
            }
            if(regv & 1<<1)
            {
                // Overrun Error
                printf("OE\r\n");
                uart_rx_clean(port, queue);
                return ;
            }
            if(regv & 1<<0)
            {
                // Data Ready
                printf("DR\r\n");
            }
            break;
        case 0x0c:
            // RX Data Timeout Interrupt
        case 0x04:
            // RX Data Received Interrupt
            if(mtk_os_hal_uart_readreg(port, UART_VFIFO_EN_REG))
            {
                // VFIFO_EN == 1
            }
            else
            {
                // VFIFO_EN == 0

                // read rx_offset(number of data in RX FIFO)
                regv = mtk_os_hal_uart_readreg(port, RX_OFFSET);
                //printf("Received %d\r\n", regv);

                for(i=0; i<regv; i++)
                {
                    ch = mtk_os_hal_uart_get_char(port);
                    data_enqueue(queue, &ch);
                    if (ch == 0x0a && port == terminal_port_num)
                    {
                        terminal_sent = true;
                    }
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

void uart_openlog_isr(void)
{
    uart_rx_service(openlog_port_num, &openlog);
}

