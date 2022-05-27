/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

// This header contains the peripheral pinout definitions for the
// WIZNET Azure Sphere Guardian 210 (ASG210) V1.2

// This file is autogenerated from ../../wiznet_asg210_v1.2.json.  Do not edit it directly.

#pragma once
#include "../../../usi_mt3620_bt/inc/hw/usi_mt3620_bt.h"

// AZURE Connection status LED uses GPIO4.
#define WIZNET_ASG210_STATUS_LED1_AZURE USI_MT3620_BT_COMBO_PIN28_GPIO41

// WiFi Connection status LED uses GPIO4.
#define WIZNET_ASG210_STATUS_LED2_WIFI USI_MT3620_BT_COMBO_PIN27_GPIO42

// WLAN Connection status LED uses GPIO4.
#define WIZNET_ASG210_STATUS_LED3_ETHERNET USI_MT3620_BT_COMBO_PIN26_GPIO43

// Interface Connection status LED uses GPIO4.
#define WIZNET_ASG210_STATUS_LED4_INTERFACE USI_MT3620_BT_COMBO_PIN25_GPIO44

// BLE Connection status LED uses GPIO4.
#define WIZNET_ASG210_STATUS_LED6_BLUETOOTH USI_MT3620_BT_COMBO_PIN24_GPIO45

//  GPIO0.
#define WIZNET_ASG210_GPIO0_J5_2 USI_MT3620_BT_COMBO_PIN69_GPIO0

//  GPIO1.
#define WIZNET_ASG210_GPIO1_J5_3 USI_MT3620_BT_COMBO_PIN68_GPIO1

//  GPIO2.
#define WIZNET_ASG210_GPIO2_J5_4 USI_MT3620_BT_COMBO_PIN67_GPIO2

//  GPIO3.
#define WIZNET_ASG210_GPIO3_J5_5 USI_MT3620_BT_COMBO_PIN66_GPIO3

// User Button uses GPIO13.
#define WIZNET_ASG210_USER_BUTTON_SW2 USI_MT3620_BT_COMBO_PIN55_GPIO13

// ISU 3 configured as I2C,  pin 89 (SDA) and pin 90 (SCL).
#define WIZNET_ASG210_ISU3_I2C USI_MT3620_BT_COMBO_ISU3_I2C

// ISU 3 configured as UART, pin 89 (RX), pin 91 (TX), pin 87 (CTS), pin 90 (RTS).
#define WIZNET_ASG210_ISU3_UART USI_MT3620_BT_COMBO_ISU3_UART

// S0 Select ISU3 nSDA/RXD.
#define WIZNET_ASG210_ISU3_NSDA_RXD_SEL USI_MT3620_BT_COMBO_PIN61_GPIO8

// Select n422/485.
#define WIZNET_ASG210_ISU3_N422_485_SEL USI_MT3620_BT_COMBO_PIN60_GPIO9

// Select n232/485.
#define WIZNET_ASG210_ISU3_N232_485_SEL USI_MT3620_BT_COMBO_PIN59_GPIO10

// DE 485/422 Tx Enable.
#define WIZNET_ASG210_ISU3_485_DE USI_MT3620_BT_COMBO_PIN88_GPIO70

// ENC ISU 0 configured as SPI, pin 45 (MISO), pin 47 (SCLK), pin 43 (CSB), pin 44 (CSA), and pin 46 (MOSI).
#define WIZNET_ASG210_ENC_SPI USI_MT3620_BT_COMBO_ISU0_SPI

// ENC SPI Chip Select (CS) value "A". This is not a peripheral identifier, and so has no meaning in an app manifest.
#define WIZNET_ASG210_ENC_SPI_CS USI_MT3620_BT_COMBO_PIN44_GPIO29

// Reset ENC.
#define WIZNET_ASG210_ENC_RESET USI_MT3620_BT_COMBO_PIN54_GPIO14

// W5500 ISU 1 configured as SPI, pin 40 (MISO), pin 42 (SCLK), pin 39 (CSA), pin 41 (MOSI) and pin 38 (CSB).
#define WIZNET_ASG210_W5500_SPI USI_MT3620_BT_COMBO_ISU1_SPI

// SD SPI Chip Select (CS) value "B". This is not a peripheral identifier, and so has no meaning in an app manifest.
#define WIZNET_ASG210_W5500_SPI_CS USI_MT3620_BT_COMBO_PIN38_GPIO35

// Reset W5500.
#define WIZNET_ASG210_W5500_RESET USI_MT3620_BT_COMBO_PIN94_GPIO58

// W5500_INTn.
#define WIZNET_ASG210_W5500_INTN USI_MT3620_BT_COMBO_PIN3_GPIO57

// nRDY.
#define WIZNET_ASG210_W5500_NRDY USI_MT3620_BT_COMBO_PIN1_GPIO56

// SD ISU 2 configured as SPI, pin 35 (MISO), pin 37 (SCLK), pin 34 (CSA), pin 36 (MOSI) and pin 33 (CSB).
#define WIZNET_ASG210_SD_SPI USI_MT3620_BT_COMBO_ISU2_SPI

// SD SPI Chip Select (CS) value "A". This is not a peripheral identifier, and so has no meaning in an app manifest.
#define WIZNET_ASG210_SD_SPI_CS USI_MT3620_BT_COMBO_PIN34_GPIO39

// ISU 2 configured as UART, pin 37 (RX), pin 35 (TX), pin 34 (CTS), pin 36 (RTS).
#define WIZNET_ASG210_SD_UART USI_MT3620_BT_COMBO_ISU2_UART

// SD UART RST value "A". This is not a peripheral identifier, and so has no meaning in an app manifest.
#define WIZNET_ASG210_SD_UART_RST USI_MT3620_BT_COMBO_PIN93_GPIO60

