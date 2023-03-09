/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

// This header contains the peripheral pinout definitions for the
// WIZNET Azure Sphere Guardian EVB V1.0

// This file is autogenerated from ../../wiznet_asg_evb_v1.0.json.  Do not edit it directly.

#pragma once
#include "../../../usi_mt3620_bt/inc/hw/usi_mt3620_bt.h"

// AZURE Connection status LED uses GPIO58.
#define WIZNET_ASG_EVB_STATUS_LED1_AZURE USI_MT3620_BT_COMBO_PIN94_GPIO58

// WiFi Connection status LED uses GPIO46.
#define WIZNET_ASG_EVB_STATUS_LED2_WIFI USI_MT3620_BT_COMBO_PIN23_GPIO46

// WLAN Connection status LED uses GPIO60.
#define WIZNET_ASG_EVB_STATUS_LED3_ETHERNET USI_MT3620_BT_COMBO_PIN93_GPIO60

// Interface Connection status LED uses GPIO44.
#define WIZNET_ASG_EVB_STATUS_LED4_INTERFACE USI_MT3620_BT_COMBO_PIN25_GPIO44

// BLE Connection status LED uses GPIO45.
#define WIZNET_ASG_EVB_STATUS_LED5_BLUETOOTH USI_MT3620_BT_COMBO_PIN24_GPIO45

//  GPIO56.
#define WIZNET_ASG_EVB_GPIO56 USI_MT3620_BT_COMBO_PIN1_GPIO56

//  GPIO59.
#define WIZNET_ASG_EVB_GPIO59 USI_MT3620_BT_COMBO_PIN2_GPIO59

//  GPIO57.
#define WIZNET_ASG_EVB_GPIO57 USI_MT3620_BT_COMBO_PIN3_GPIO57

// User Switch uses GPIO13.
#define WIZNET_ASG_EVB_USER_SW USI_MT3620_BT_COMBO_PIN55_GPIO13

// ISU 1 configured as UART, pin 40 (RX), pin 42 (TX), pin 39 (CTS) and pin 41 (RTS).
#define WIZNET_ASG_EVB_ISU1_UART USI_MT3620_BT_COMBO_ISU1_UART

// ISU 1 configured as SPI, pin 40 (MISO), pin 42 (SCLK), pin 39 (CSA) and pin 41 (MOSI).
#define WIZNET_ASG_EVB_ISU1_SPI USI_MT3620_BT_COMBO_ISU1_SPI

// ISU 1 configured as I2C,  pin 40 (SDA) and pin 41 (SCL).
#define WIZNET_ASG_EVB_ISU1_I2C USI_MT3620_BT_COMBO_ISU1_I2C

// Select ISU1 as UART.
#define WIZNET_ASG_EVB_ISU1_UART_NSEL USI_MT3620_BT_COMBO_PIN63_GPIO6

// Select ISU1 as SPI.
#define WIZNET_ASG_EVB_ISU1_SPI_SEL USI_MT3620_BT_COMBO_PIN63_GPIO6

// ISU 2 configured as UART, pin 35 (RX), pin 37 (TX), pin 34 (CTS) and pin 36 (RTS).
#define WIZNET_ASG_EVB_ISU2_UART USI_MT3620_BT_COMBO_ISU2_UART

// ISU 2 configured as SPI, pin 35 (MISO), pin 37 (SCLK), pin 34 (CSA) and pin 36 (MOSI).
#define WIZNET_ASG_EVB_ISU2_SPI USI_MT3620_BT_COMBO_ISU2_SPI

// ISU 2 configured as I2C,  pin 35 (SDA) and pin 36 (SCL).
#define WIZNET_ASG_EVB_ISU2_I2C USI_MT3620_BT_COMBO_ISU2_I2C

// Select ISU2 as UART.
#define WIZNET_ASG_EVB_ISU2_UART_NSEL USI_MT3620_BT_COMBO_PIN62_GPIO7

// Select ISU2 as SPI.
#define WIZNET_ASG_EVB_ISU2_SPI_SEL USI_MT3620_BT_COMBO_PIN62_GPIO7

// ISU 3 configured as UART, pin 89 (RX), pin 91 (TX), pin 87 (CTS) and pin 90 (RTS).
#define WIZNET_ASG_EVB_ISU3_UART USI_MT3620_BT_COMBO_ISU3_UART

// ISU 3 configured as SPI, pin 89 (MISO), pin 91 (SCLK), pin 87 (CSA) and pin 90 (MOSI).
#define WIZNET_ASG_EVB_ISU3_SPI USI_MT3620_BT_COMBO_ISU3_SPI

// ISU 3 configured as I2C,  pin 89 (SDA) and pin 90 (SCL).
#define WIZNET_ASG_EVB_ISU3_I2C USI_MT3620_BT_COMBO_ISU3_I2C

// Select ISU3 as UART.
#define WIZNET_ASG_EVB_ISU3_UART_SEL USI_MT3620_BT_COMBO_PIN54_GPIO14

// Select ISU3 as SPI.
#define WIZNET_ASG_EVB_ISU3_SPI_SEL USI_MT3620_BT_COMBO_PIN54_GPIO14

// Select ISU3 as I2C.
#define WIZNET_ASG_EVB_ISU3_I2C_NSEL USI_MT3620_BT_COMBO_PIN54_GPIO14

// Select ISU3 external header and also can configured as UART, SPI and I2C.
#define WIZNET_ASG_EVB_ISU3_EXT_SEL USI_MT3620_BT_COMBO_PIN56_GPIO12

// Select ISU3 mB1/mB2 internal header.
#define WIZNET_ASG_EVB_ISU3_INT_NSEL USI_MT3620_BT_COMBO_PIN56_GPIO12

// Select ADC 0-7.
#define WIZNET_ASG_EVB_ADC USI_MT3620_BT_COMBO_ADC_CONTROLLER0

// Select GPIO0 as MB1 RST.
#define WIZNET_ASG_EVB_MB1_RST USI_MT3620_BT_COMBO_PIN69_GPIO0

// Select GPIO3 as MB2 RST.
#define WIZNET_ASG_EVB_MB2_RST USI_MT3620_BT_COMBO_PIN66_GPIO3

// Select GPIO1 as MB1 INT.
#define WIZNET_ASG_EVB_MB1_INT USI_MT3620_BT_COMBO_PIN68_GPIO1

// Select GPIO4 as MB2 INT.
#define WIZNET_ASG_EVB_MB2_INT USI_MT3620_BT_COMBO_PIN65_GPIO4

// Select GPIO8-11 as PWM 8-11.
#define WIZNET_ASG_EVB_PWM0 USI_MT3620_BT_COMBO_PWM_CONTROLLER2

// W5500 ISU 0 configured as SPI, pin 45 (MISO), pin 48 (SCLK), pin 44 (CSA) and pin 46 (MOSI).
#define WIZNET_ASG_EVB_W5500_SPI USI_MT3620_BT_COMBO_ISU0_SPI

// SD SPI Chip Select (CS) value "B". This is not a peripheral identifier, and so has no meaning in an app manifest.
#define WIZNET_ASG_EVB_W5500_SPI_CS USI_MT3620_BT_COMBO_PIN44_GPIO29

// Reset W5500.
#define WIZNET_ASG_EVB_W5500_RESET USI_MT3620_BT_COMBO_PIN67_GPIO2

// W5500_INTn.
#define WIZNET_ASG_EVB_W5500_INTN USI_MT3620_BT_COMBO_PIN64_GPIO5

// NRF52 RESET.
#define WIZNET_ASG_EVB_BT_NRF52_RESET USI_MT3620_BT_COMBO_NRF52_RESET

// NRF52 DFU.
#define WIZNET_ASG_EVB_BT_NRF52_DFU USI_MT3620_BT_COMBO_NRF52_DFU

// ISU 4.
#define WIZNET_ASG_EVB_BT_NRF52_ISU4_UART USI_MT3620_BT_COMBO_NRF52_UART
