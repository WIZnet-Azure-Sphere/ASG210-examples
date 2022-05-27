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

// 1 : 485
// 2 : 422
// 3 : 232
#define ISU3 3

#if ISU3 == 1
#define ISU3_485
#elif ISU3 == 2
#define ISU3_422
#else
#define ISU3_232
#endif

#if 1
#define PRINT_INFO
#endif

#if 1
#define TX_TO_ISU3
#endif

#if ISU3 == 3
#if 0
#define TX_MIRROR
#endif
#endif

/******************************************************************************/
/* Configurations */
/******************************************************************************/
/* UART */
static const uint8_t terminal_port_num = OS_HAL_UART_PORT0;
static const uint8_t isu3_port_num = OS_HAL_UART_ISU3;

/* GPIO_8 for nSDA/RXD SEL */
static const uint8_t sel_nSDA = OS_HAL_GPIO_8;
/* GPIO_10 for 485/232 SEL */
static const uint8_t sel_n232 = OS_HAL_GPIO_10;
/* GPIO_9 for 485/422 SEL */
static const uint8_t sel_n422 = OS_HAL_GPIO_9;
/* GPIO_70 for DE */
static const uint8_t de = OS_HAL_GPIO_70;

str_qData terminal;
u8* terminal_buf_p;

str_qData isu3;
u8* isu3_buf_p;

bool terminal_sent = false;

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
void set_485_direction_tx(bool set);
void uart_rx_clean(uint8_t port, str_qData *queue);
void uart_rx_service(uint8_t port, str_qData *queue);
bool time_over_check(uint32_t time_stamp, uint32_t *tick, uint32_t cnt);
void uart_terminal_isr(void);
void uart_isu3_isr(void);
void gpt_timer_tick_handler(void);

_Noreturn void RTCoreMain(void)
{
    /* Init Vector Table */
    NVIC_SetupVectorTable();

    /* Init UART */
    mtk_os_hal_uart_ctlr_init(terminal_port_num);
    printf("\r\n");
    printf("=============================================\r\n");
    printf("UART_ISU3_RTApp_mt3620_BareMetal\r\n");
    printf("UART Inited (port_num=%d)\r\n", terminal_port_num);
    #if defined ISU3_232
    printf("ISU3 Inited as 232 (port_num=%d)\r\n", isu3_port_num);
    #elif defined ISU3_485
    printf("ISU3 Inited as 485 (port_num=%d)\r\n", isu3_port_num);
    #elif defined ISU3_422
    printf("ISU3 Inited as 422 (port_num=%d)\r\n", isu3_port_num);
    #endif
    printf("=============================================\r\n");
    
#if defined ISU3_232 || defined ISU3_485 || defined ISU3_422
    gpio_output(sel_nSDA, OS_HAL_GPIO_DATA_HIGH);
#else
    gpio_output(sel_nSDA, OS_HAL_GPIO_DATA_LOW);
#endif

#if defined ISU3_232
    gpio_output(sel_n232, OS_HAL_GPIO_DATA_LOW);
#else
    gpio_output(sel_n232, OS_HAL_GPIO_DATA_HIGH);
#endif

#if defined ISU3_422
    gpio_output(sel_n422, OS_HAL_GPIO_DATA_LOW);
#else
    gpio_output(sel_n422, OS_HAL_GPIO_DATA_HIGH);
#endif

#if defined ISU3_485 || defined ISU3_422
    gpio_output(de, OS_HAL_GPIO_DATA_HIGH);
#endif

    terminal_buf_p = (u8*)malloc(1024);
    data_queueinit(&terminal, terminal_buf_p, 1024);

    mtk_os_hal_uart_set_irq(terminal_port_num, UART_INT_RX_BUFFER_FULL | UART_INT_LINE_STATUS);
    NVIC_Register(4, uart_terminal_isr);
    NVIC_EnableIRQ(4);

    isu3_buf_p = (u8*)malloc(1024);
    data_queueinit(&isu3, isu3_buf_p, 1024);

    mtk_os_hal_uart_ctlr_init(isu3_port_num);
    mtk_os_hal_uart_set_baudrate(isu3_port_num, 115200);
    mtk_os_hal_uart_set_irq(isu3_port_num, UART_INT_RX_BUFFER_FULL | UART_INT_LINE_STATUS);
    NVIC_Register(59, uart_isu3_isr);
    NVIC_EnableIRQ(59);

    #if defined ISU3_485 || defined ISU3_422
    set_485_direction_tx(false);
    #endif

    struct os_gpt_int gpt_timer_tick_init;

    gpt_timer_tick_init.gpt_cb_hdl = gpt_timer_tick_handler;
    gpt_timer_tick_init.gpt_cb_data = NULL;

    mtk_os_hal_gpt_init();
    mtk_os_hal_gpt_config(gpt_timer_tick, false, &gpt_timer_tick_init);
    mtk_os_hal_gpt_reset_timer(gpt_timer_tick, gpt_timer_tick_perios, true);
    mtk_os_hal_gpt_start(gpt_timer_tick);

    u32 qstack;
    
    for (;;)
    {
        NVIC_DisableIRQ(59);
        
        qstack = data_queue_getlen(&isu3);

        if (qstack != 0)
        {
            u8 temp[1024];

#ifdef PRINT_INFO
            printf("\r\n>>Received %d bytes from ISU3\r\n", qstack);

#endif

            data_dequeue(&isu3, temp, qstack);
            for (int i = 0; i < qstack; i++)
            {
#ifdef TX_MIRROR
                mtk_os_hal_uart_put_char(isu3_port_num, temp[i]);
#endif
                printf("%c", temp[i]);
            }
#ifdef PRINT_INFO
            printf("\r\n>>end\r\n");
#endif
            
            NVIC_EnableIRQ(59);
        }
        else
        {
            NVIC_EnableIRQ(59);
            NVIC_DisableIRQ(4);
            
            if (terminal_sent == true)
            {
                u32 len;
                u32 i;
                u8 temp[1024];
                u8* data_p;

                len = data_queue_getlen(&terminal);
                data_p = (u8*)malloc(len);
                len = data_dequeue(&terminal, data_p, len);

                NVIC_EnableIRQ(4);

#ifdef PRINT_INFO
                printf(">>Received %d bytes from Terminal\r\n", len);
#endif

#if defined TX_MIRROR || defined PRINT_INFO
                for (i = 0; i < len; i++)
                {
#ifdef TX_MIRROR
                    mtk_os_hal_uart_put_char(terminal_port_num, *(data_p + i));
#endif
#ifdef PRINT_INFO
                    //printf("0x%x %c\r\n", *(data_p+i), *(data_p + i));
                    printf("%c", *(data_p + i));
#endif
                }
#endif
#ifdef PRINT_INFO
                printf("\r\n>>end\r\n");
#endif

#ifdef TX_TO_ISU3
#ifdef PRINT_INFO
                printf(">>Send to ISU3\r\n");
#endif
#if defined ISU3_485 || defined ISU3_422
                set_485_direction_tx(true);
#endif
                for (i = 0; i < len; i++)
                {
                    mtk_os_hal_uart_put_char(isu3_port_num, *(data_p + i));
#ifdef PRINT_INFO
                    printf("%c", *(data_p + i));
#endif
                }

#if defined ISU3_485
#ifdef PRINT_INFO
                printf(">>Clear, It will receive %d bytes back\r\n", len);
#endif
                uint32_t time_stamp_1ms;
                uint32_t retx_cnt = 1;

                do
                {
                    time_stamp_1ms = gpt_timer_tick_1ms;

                    do
                    {
                        qstack = data_queue_getlen(&isu3);
                    } while (qstack != len && !time_over_check(time_stamp_1ms, &gpt_timer_tick_1ms, len));

                    data_dequeue(&isu3, temp, qstack);

                    if(qstack >= len)
                    {
                        int ret;
                        ret = memcmp(temp, data_p, len);
                        if(ret == 0)
                        {
#ifdef PRINT_INFO
                            printf("echo matched\r\n");
#endif
                            break;
                        }
                        else
                        {
                            printf("echo not matched\r\n");
#ifdef PRINT_INFO
                            for (i = 0; i < len; i++)
                            {
                                printf("temp[%d] = 0x%x != *(data_p+%d) = 0x%x\r\n",i, temp[i], i, *(data_p + i));
                            }
#endif
                        }
                    }
                    else
                    {
                        u32 qstack_2nd;
                        
                        printf("received %d less than %d\r\n", qstack, len);
                        
                        do
                        {
                            qstack_2nd = data_queue_getlen(&isu3);
                        } while (qstack_2nd+qstack != len && !time_over_check(time_stamp_1ms, &gpt_timer_tick_1ms, len-qstack));

                        data_dequeue(&isu3, temp+qstack_2nd, qstack_2nd);

                        if(qstack+qstack_2nd >= len)
                        {
                            int ret;
                            ret = memcmp(temp, data_p, len);
                            if(ret == 0)
                            {
                                printf("echo matched\r\n");
                                break;
                            }
                            else
                            {
                                printf("echo not matched\r\n");
#ifdef PRINT_INFO
                                for (i = 0; i < len; i++)
                                {
                                    printf("temp[%d] = 0x%x != *(data_p+%d) = 0x%x\r\n",i, temp[i], i, *(data_p + i));
                                }
#endif
                            }
                        }

                        // tx failed
                        while(!time_over_check(time_stamp_1ms, &gpt_timer_tick_1ms, len-qstack));
                    }

                    if(retx_cnt > 10)
                    {
                        printf(">>Failed, Send to ISU3\r\n");
                        break;
                    }

                    printf("Retransmit data cnt %d/10\r\n", retx_cnt, 10);
                    retx_cnt++;
                    for (i = 0; i < len; i++)
                    {
                        mtk_os_hal_uart_put_char(isu3_port_num, *(data_p + i));
#ifdef PRINT_INFO
                        printf("%c", *(data_p + i));
#endif
                    }
                }while(1);
                
#ifdef PRINT_INFO
                for (i = 0; i < len; i++)
                {
                    printf("%c", temp[i]);
                }
#endif
#endif

#if defined ISU3_485 || defined ISU3_422
                set_485_direction_tx(false);
#endif
#ifdef PRINT_INFO
                printf("\r\n>>end\r\n");
#endif
#endif
                free(data_p);
                terminal_sent = false;
            }
            
            NVIC_EnableIRQ(4);
        }
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

void set_485_direction_tx(bool set)
{
    os_hal_gpio_data rts_state;

    if(set == true)
    {
        gpio_output(de, OS_HAL_GPIO_DATA_HIGH);
        do
        {
            mtk_os_hal_gpio_get_output(de, &rts_state);
        } while (rts_state != OS_HAL_GPIO_DATA_HIGH);
    }
    else
    {
        gpio_output(de, OS_HAL_GPIO_DATA_LOW);
        do
        {
            mtk_os_hal_gpio_get_output(de, &rts_state);
        } while (rts_state != OS_HAL_GPIO_DATA_LOW);
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

bool time_over_check(uint32_t time_stamp, uint32_t *tick, uint32_t cnt)
{
    uint32_t cur_time;
    
    cur_time = *tick;
    
    if(cur_time < time_stamp)
    {
        cur_time += (0xffffffff - time_stamp) + 1;
        if(cur_time >= cnt)
        {
            return true;
        }
    }
    else
    {
        if((cur_time - time_stamp) >= cnt)
        {
            return true;
        }
    }

    return false;
}

void uart_terminal_isr(void)
{
    uart_rx_service(terminal_port_num, &terminal);
}

void uart_isu3_isr(void)
{
    uart_rx_service(isu3_port_num, &isu3);
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

