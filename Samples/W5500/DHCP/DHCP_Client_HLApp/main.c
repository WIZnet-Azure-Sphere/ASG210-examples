/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

// This sample C application for Azure Sphere demonstrates General-Purpose Input/Output (GPIO)
// peripherals using a blinking LED and a button.
// The blink rate can be changed through a button press.
//
// It uses the API for the following Azure Sphere application libraries:
// - gpio (digital input for button, digital output for LED)
// - log (messages shown in Visual Studio's Device Output window during debugging)
// - eventloop (system invokes handlers for IO events)

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include <applibs/gpio.h>
#include <applibs/log.h>
#include <applibs/eventloop.h>

#include <applibs/spi.h>

#include "W5500/w5500.h"
#include "wizchip_conf.h"
#include "socket.h"

#include "loopback.h"
#include "dhcp.h"

// The following #include imports a "sample appliance" definition. This app comes with multiple
// implementations of the sample appliance, each in a separate directory, which allow the code to
// run on different hardware.
//
// By default, this app targets hardware that follows the MT3620 Reference Development Board (RDB)
// specification, such as the MT3620 Dev Kit from Seeed Studio.
//
// To target different hardware, you'll need to update CMakeLists.txt. For example, to target the
// Avnet MT3620 Starter Kit, change the TARGET_DIRECTORY argument in the call to
// azsphere_target_hardware_definition to "HardwareDefinitions/avnet_mt3620_sk".
//
// See https://aka.ms/AzureSphereHardwareDefinitions for more details.
#include <hw/wiznet_asg_evb_v1.0.h>

// This sample uses a single-thread event loop pattern.
#include "eventloop_timer_utilities.h"

/// <summary>
/// Termination codes for this application. These are used for the
/// application exit code. They must all be between zero and 255,
/// where zero is reserved for successful termination.
/// </summary>
typedef enum
{
    ExitCode_Success = 0,

    ExitCode_TermHandler_SigTerm = 1,

    ExitCode_LedTimer_Consume = 2,
    ExitCode_LedTimer_SetLedState = 3,

    ExitCode_ButtonTimer_Consume = 4,
    ExitCode_ButtonTimer_GetButtonState = 5,
    ExitCode_ButtonTimer_SetBlinkPeriod = 6,

    ExitCode_Init_EventLoop = 7,
    ExitCode_Init_Button = 8,
    ExitCode_Init_ButtonPollTimer = 9,
    ExitCode_Init_Led = 10,
    ExitCode_Init_LedBlinkTimer = 11,
    ExitCode_Main_EventLoopFail = 12
                                  ,
    ExitCode_Init_DhcpTimer = 13,
    ExitCode_DhcpTimer_Consume = 14
} ExitCode;

// File descriptors - initialized to invalid value
static EventLoop *eventLoop = NULL;
static int ledBlinkRateButtonGpioFd = -1;
static EventLoopTimer *buttonPollTimer = NULL;
static int blinkingLedGpioFd = -1;
static EventLoopTimer *blinkTimer = NULL;
static int w5500resetGpioFd = -1;
static EventLoopTimer* dhcpTimer = NULL;

// Button state variables
static GPIO_Value_Type buttonState = GPIO_Value_High;
static GPIO_Value_Type ledState = GPIO_Value_High;

// Blink interval variables
static const int numBlinkIntervals = 3;
static const struct timespec blinkIntervals[] = {{.tv_sec = 0, .tv_nsec = 125 * 1000 * 1000},
    {.tv_sec = 0, .tv_nsec = 250 * 1000 * 1000},
    {.tv_sec = 0, .tv_nsec = 500 * 1000 * 1000}
};
static int blinkIntervalIndex = 0;

// Termination state
static volatile sig_atomic_t exitCode = ExitCode_Success;

#define NETINFO_USE_MANUAL
#ifndef NETINFO_USE_MANUAL
#define NETINFO_USE_DHCP
#endif

#ifdef NETINFO_USE_MANUAL
// Default Static Network Configuration for TCP Server
wiz_NetInfo gWIZNETINFO =
{
    {0x00, 0x08, 0xdc, 0xff, 0xfa, 0xfb},
    {192, 168, 50, 1},
    {255, 255, 255, 0},
    {192, 168, 50, 1},
    {8, 8, 8, 8},
    NETINFO_STATIC
};

#else
wiz_NetInfo gWIZNETINFO =
{
    {0x00, 0x08, 0xdc, 0xff, 0xfa, 0xfb},
    {192, 168, 50, 1},
    {255, 255, 255, 0},
    {192, 168, 50, 1},
    {8, 8, 8, 8},
#ifdef NETINFO_USE_DHCP
    NETINFO_DHCP
#else
    NETINFO_STATIC
#endif
};

#endif

uint16_t w5500_tcps_port = 3000;

uint8_t s1_Buf[2 * 1024];
uint8_t s2_Buf[2 * 1024];
uint8_t gDATABUF[DATA_BUF_SIZE];

#define MY_MAX_DHCP_RETRY 2
uint8_t my_dhcp_retry = 0;
extern uint32_t dhcp_tick_1s;

static void TerminationHandler(int signalNumber);
static void BlinkingLedTimerEventHandler(EventLoopTimer *timer);
static void ButtonTimerEventHandler(EventLoopTimer *timer);
static ExitCode InitPeripheralsAndHandlers(void);
static void CloseFdAndPrintError(int fd, const char *fdName);
static void ClosePeripheralsAndHandlers(void);

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async-signal-safe.
    exitCode = ExitCode_TermHandler_SigTerm;
}

/// <summary>
///     Handle LED timer event: blink LED.
/// </summary>
static void BlinkingLedTimerEventHandler(EventLoopTimer *timer)
{
    if (ConsumeEventLoopTimerEvent(timer) != 0)
    {
        exitCode = ExitCode_LedTimer_Consume;
        return;
    }

    // The blink interval has elapsed, so toggle the LED state
    // The LED is active-low so GPIO_Value_Low is on and GPIO_Value_High is off
    ledState = (ledState == GPIO_Value_Low ? GPIO_Value_High : GPIO_Value_Low);
    int result = GPIO_SetValue(blinkingLedGpioFd, ledState);
    if (result != 0)
    {
        Log_Debug("ERROR: Could not set LED output value: %s (%d).\n", strerror(errno), errno);
        exitCode = ExitCode_LedTimer_SetLedState;
        return;
    }
}

/// <summary>
///     Handle button timer event: if the button is pressed, change the LED blink rate.
/// </summary>
static void ButtonTimerEventHandler(EventLoopTimer *timer)
{
    if (ConsumeEventLoopTimerEvent(timer) != 0)
    {
        exitCode = ExitCode_ButtonTimer_Consume;
        return;
    }

    // Check for a button press
    GPIO_Value_Type newButtonState;
    int result = GPIO_GetValue(ledBlinkRateButtonGpioFd, &newButtonState);
    if (result != 0)
    {
        Log_Debug("ERROR: Could not read button GPIO: %s (%d).\n", strerror(errno), errno);
        exitCode = ExitCode_ButtonTimer_GetButtonState;
        return;
    }

    // If the button has just been pressed, change the LED blink interval
    // The button has GPIO_Value_Low when pressed and GPIO_Value_High when released
    if (newButtonState != buttonState)
    {
        if (newButtonState == GPIO_Value_Low)
        {
            blinkIntervalIndex = (blinkIntervalIndex + 1) % numBlinkIntervals;
            if (SetEventLoopTimerPeriod(blinkTimer, &blinkIntervals[blinkIntervalIndex]) != 0)
            {
                exitCode = ExitCode_ButtonTimer_SetBlinkPeriod;
            }
        }
        buttonState = newButtonState;
    }
}

#ifdef NETINFO_USE_DHCP
static void DHCPTimerEventHandler(EventLoopTimer* timer)
{
    if (ConsumeEventLoopTimerEvent(timer) != 0)
    {
        exitCode = ExitCode_DhcpTimer_Consume;
        return;
    }

    dhcp_tick_1s++;
#if 0
    Log_Debug("dhcp_tick_1s = 0x%x\r\n", dhcp_tick_1s);
#endif
}
#endif

/// <summary>
///     Set up SIGTERM termination handler, initialize peripherals, and set up event handlers.
/// </summary>
/// <returns>
///     ExitCode_Success if all resources were allocated successfully; otherwise another
///     ExitCode value which indicates the specific failure.
/// </returns>
static ExitCode InitPeripheralsAndHandlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    eventLoop = EventLoop_Create();
    if (eventLoop == NULL)
    {
        Log_Debug("Could not create event loop.\n");
        return ExitCode_Init_EventLoop;
    }

    // Open WIZNET_ASG_EVB_USER_SW GPIO as input, and set up a timer to poll it
    Log_Debug("Opening WIZNET_ASG_EVB_USER_SW as input.\n");
    ledBlinkRateButtonGpioFd = GPIO_OpenAsInput(WIZNET_ASG_EVB_USER_SW);
    if (ledBlinkRateButtonGpioFd == -1)
    {
        Log_Debug("ERROR: Could not open WIZNET_ASG_EVB_USER_SW: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_Button;
    }
    struct timespec buttonPressCheckPeriod = {.tv_sec = 0, .tv_nsec = 1000000};
    buttonPollTimer =
        CreateEventLoopPeriodicTimer(eventLoop, &ButtonTimerEventHandler, &buttonPressCheckPeriod);
    if (buttonPollTimer == NULL)
    {
        return ExitCode_Init_ButtonPollTimer;
    }

    // Open WIZNET_ASG_EVB_STATUS_LD1 GPIO, set as output with value GPIO_Value_High (off), and set up a timer to
    // blink it
    Log_Debug("Opening WIZNET_ASG_EVB_STATUS_LD1 as output.\n");
    blinkingLedGpioFd = GPIO_OpenAsOutput(WIZNET_ASG_EVB_STATUS_LD1, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (blinkingLedGpioFd == -1)
    {
        Log_Debug("ERROR: Could not open WIZNET_ASG_EVB_STATUS_LD1 GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_Led;
    }
    blinkTimer = CreateEventLoopPeriodicTimer(eventLoop, &BlinkingLedTimerEventHandler,
                 &blinkIntervals[blinkIntervalIndex]);
    if (blinkTimer == NULL)
    {
        return ExitCode_Init_LedBlinkTimer;
    }

    Log_Debug("Opening WIZNET_ASG_EVB_W5500_RESET as output.\n");
    w5500resetGpioFd = GPIO_OpenAsOutput(WIZNET_ASG_EVB_W5500_RESET, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    sleep(1);
    if (w5500resetGpioFd == -1)
    {
        Log_Debug("ERROR: Could not open WIZNET_ASG_EVB_W5500_RESET GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_Led;
    }

#ifdef NETINFO_USE_DHCP
    static const struct timespec dhcpTimerPeriod = { .tv_sec = 1, .tv_nsec = 0 };
    dhcpTimer = CreateEventLoopPeriodicTimer(eventLoop, &DHCPTimerEventHandler, &dhcpTimerPeriod);
    if (dhcpTimer == NULL)
    {
        return ExitCode_Init_DhcpTimer;
    }
#endif

    return ExitCode_Success;
}

/// <summary>
///     Closes a file descriptor and prints an error on failure.
/// </summary>
/// <param name="fd">File descriptor to close</param>
/// <param name="fdName">File descriptor name to use in error message</param>
static void CloseFdAndPrintError(int fd, const char *fdName)
{
    if (fd >= 0)
    {
        int result = close(fd);
        if (result != 0)
        {
            Log_Debug("ERROR: Could not close fd %s: %s (%d).\n", fdName, strerror(errno), errno);
        }
    }
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void)
{
    // Leave the LED off
    if (blinkingLedGpioFd >= 0)
    {
        GPIO_SetValue(blinkingLedGpioFd, GPIO_Value_High);
    }

    DisposeEventLoopTimer(buttonPollTimer);
    DisposeEventLoopTimer(blinkTimer);
    EventLoop_Close(eventLoop);

    Log_Debug("Closing file descriptors.\n");
    CloseFdAndPrintError(blinkingLedGpioFd, "BlinkingLedGpio");
    CloseFdAndPrintError(ledBlinkRateButtonGpioFd, "LedBlinkRateButtonGpio");
}

// check w5500 network setting
void InitPrivateNetInfo(void)
{
    uint8_t tmpstr[6];
    int result;

    ctlwizchip(CW_GET_ID, (void*)tmpstr);

    result = GPIO_SetValue(w5500resetGpioFd, GPIO_Value_High);
    if (result != 0)
    {
        Log_Debug("ERROR: Could not set w5500resetGpioFd output value: %s (%d).\n", strerror(errno), errno);
    }
    sleep(1);

    wizchip_setnetinfo_partial(&gWIZNETINFO);

    if (ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO) < 0)
    {
        Log_Debug("ERROR: ctlnetwork SET\r\n");
        while (1);
    }

    wiz_NetInfo netinfo_temp;

    memset((void*)&netinfo_temp, 0, sizeof(netinfo_temp));
    ctlnetwork(CN_GET_NETINFO, (void*)&netinfo_temp);

    if (memcmp((void*)&netinfo_temp, (void*)&gWIZNETINFO, sizeof(netinfo_temp)))
    {
        Log_Debug("ERROR: NETINFO not matched\r\n");
        while (1);
    }

    Log_Debug("\r\n=== %s NET CONF ===\r\n", (char*)tmpstr);
    Log_Debug("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", gWIZNETINFO.mac[0], gWIZNETINFO.mac[1], gWIZNETINFO.mac[2],
              gWIZNETINFO.mac[3], gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
    Log_Debug("SIP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
    Log_Debug("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
    Log_Debug("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
    Log_Debug("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0], gWIZNETINFO.dns[1], gWIZNETINFO.dns[2], gWIZNETINFO.dns[3]);
    Log_Debug("======================\r\n");
}

#ifdef NETINFO_USE_DHCP
/*******************************************************
 * @ brief Call back for ip assing & ip update from DHCP
 *******************************************************/
void my_ip_assign(void)
{
    getIPfromDHCP(gWIZNETINFO.ip);
    getGWfromDHCP(gWIZNETINFO.gw);
    getSNfromDHCP(gWIZNETINFO.sn);
    getDNSfromDHCP(gWIZNETINFO.dns);
    gWIZNETINFO.dhcp = NETINFO_DHCP;
    /* Network initialization */
    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);

    Log_Debug("Network Configuration from DHCP Server\r\n");
    InitPrivateNetInfo();
    Log_Debug("DHCP LEASED TIME : %ld Sec.\r\n", getDHCPLeasetime());
    Log_Debug("\r\n");
}

/************************************
 * @ brief Call back for ip Conflict
 ************************************/
void my_ip_conflict(void)
{
    Log_Debug("CONFLICT IP from DHCP\r\n");
    //halt or reset or any...
    while (1); // this example is halt.
}
#endif

// Init SPI master interface for W5500.
static void InitPrivateEthernet(void)
{
    if (Init_SPIMaster() != 0)
    {
        Log_Debug("ERROR: Init_SPIMaster() error\n");
    }

    Log_Debug("SPI Master initialized...\n");
    InitPrivateNetInfo();

#ifdef NETINFO_USE_DHCP
    /* Init DHCP */
    DHCP_init(0, gDATABUF);
    // if you want different action instead default ip assign, update, conflict.
    // if cbfunc == 0, act as default.
    reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);
#endif
}

/// <summary>
///     Main entry point for this application.
/// </summary>
int main(int argc, char *argv[])
{
    // 20201015 taylor
    bool run_user_applications = false;

    Log_Debug("DHCP_Client_HLApp application starting.\n");
    exitCode = InitPeripheralsAndHandlers();

    // Init SPI interface for W5500
    InitPrivateEthernet();

#ifndef NETINFO_USE_DHCP
    run_user_applications = true;
#endif

    // Use event loop to wait for events and trigger handlers, until an error or SIGTERM happens
    while (exitCode == ExitCode_Success)
    {
        EventLoop_Run_Result result = EventLoop_Run(eventLoop, -1, true);
        // Continue if interrupted by signal, e.g. due to breakpoint being set.
        if (result == EventLoop_Run_Failed && errno != EINTR)
        {
            exitCode = ExitCode_Main_EventLoopFail;
        }

#ifdef NETINFO_USE_DHCP
        // 20201015 taylor

        /* DHCP */
        /* DHCP IP allocation and check the DHCP lease time (for IP renewal) */
        if (gWIZNETINFO.dhcp == NETINFO_DHCP)
        {
            switch (DHCP_run())
            {
            case DHCP_IP_ASSIGN:
            case DHCP_IP_CHANGED:
                /* If this block empty, act with default_ip_assign & default_ip_update */
                //
                // This example calls my_ip_assign in the two case.
                //
                // Add to ...
                //
                break;
            case DHCP_IP_LEASED:
                //
                // TODO: insert user's code here
                run_user_applications = true;
                //
                break;
            case DHCP_FAILED:
                /* ===== Example pseudo code =====  */
                // The below code can be replaced your code or omitted.
                // if omitted, retry to process DHCP
                my_dhcp_retry++;
                if (my_dhcp_retry > MY_MAX_DHCP_RETRY)
                {
                    gWIZNETINFO.dhcp = NETINFO_STATIC;
                    DHCP_stop(); // if restart, recall DHCP_init()
#ifdef _MAIN_DEBUG_
                    Log_Debug(">> DHCP %d Failed\r\n", my_dhcp_retry);
                    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
                    InitPrivateNetInfo(); // print out static netinfo to serial
#endif
                    my_dhcp_retry = 0;
                }
                break;
            default:
                break;
            }
        }
#endif

        // 20201015 taylor
        if (run_user_applications == true)
        {
            int32_t ret;
            if ((ret = loopback_tcps(1, s1_Buf, 50000)) < 0) // TCP server loopback test
            {
#ifdef _MAIN_DEBUG_
                Log_Debug("SOCKET ERROR : %ld\r\n", ret);
#endif
            }

            if ((ret = loopback_tcps(2, s2_Buf, 50001)) < 0) // TCP server loopback test
            {
#ifdef _MAIN_DEBUG_
                Log_Debug("SOCKET ERROR : %ld\r\n", ret);
#endif
            }
        }
    }

    ClosePeripheralsAndHandlers();
    Log_Debug("Application exiting.\n");
    return exitCode;
}
