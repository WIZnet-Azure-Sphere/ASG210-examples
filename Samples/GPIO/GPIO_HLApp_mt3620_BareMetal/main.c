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
    ExitCode_Init_Led1,
    ExitCode_Init_LedBlinkTimer1,
    ExitCode_Init_Led2,
    ExitCode_Init_LedBlinkTimer2,
    ExitCode_Init_Led3,
    ExitCode_Init_LedBlinkTimer3,
    ExitCode_Init_Led4,
    ExitCode_Init_LedBlinkTimer4,
    ExitCode_Main_EventLoopFail
} ExitCode;

// File descriptors - initialized to invalid value
static EventLoop *eventLoop = NULL;
static int ledBlinkRateButtonGpioFd = -1;
static EventLoopTimer *buttonPollTimer = NULL;
static int blinkingLedGpioFd1 = -1;
static int blinkingLedGpioFd2 = -1;
static int blinkingLedGpioFd3 = -1;
static int blinkingLedGpioFd4 = -1;
static EventLoopTimer *blinkTimer = NULL;

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
    int result = GPIO_SetValue(blinkingLedGpioFd1, ledState);
    if (result != 0)
    {
        Log_Debug("ERROR: Could not set LED1 output value: %s (%d).\n", strerror(errno), errno);
        exitCode = ExitCode_LedTimer_SetLedState;
        return;
    }

    result = GPIO_SetValue(blinkingLedGpioFd2, ledState);
    if (result != 0)
    {
        Log_Debug("ERROR: Could not set LED2 output value: %s (%d).\n", strerror(errno), errno);
        exitCode = ExitCode_LedTimer_SetLedState;
        return;
    }

    result = GPIO_SetValue(blinkingLedGpioFd3, ledState);
    if (result != 0)
    {
        Log_Debug("ERROR: Could not set LED3 output value: %s (%d).\n", strerror(errno), errno);
        exitCode = ExitCode_LedTimer_SetLedState;
        return;
    }

    result = GPIO_SetValue(blinkingLedGpioFd4, ledState);
    if (result != 0)
    {
        Log_Debug("ERROR: Could not set LED4 output value: %s (%d).\n", strerror(errno), errno);
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

    // Open WIZNET_ASG_EVB_STATUS_LD1, set as output with value GPIO_Value_High (off), and set up a timer to
    // blink it
    Log_Debug("Opening WIZNET_ASG_EVB_STATUS_LD1 as output.\n");
    blinkingLedGpioFd1 = GPIO_OpenAsOutput(WIZNET_ASG_EVB_STATUS_LD1, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (blinkingLedGpioFd1 == -1)
    {
        Log_Debug("ERROR: Could not open WIZNET_ASG_EVB_STATUS_LD1 GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_Led1;
    }

    // Open WIZNET_ASG_EVB_STATUS_LD2, set as output with value GPIO_Value_High (off), and set up a timer to
    // blink it
    Log_Debug("Opening WIZNET_ASG_EVB_STATUS_LD2 as output.\n");
    blinkingLedGpioFd2 = GPIO_OpenAsOutput(WIZNET_ASG_EVB_STATUS_LD2, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (blinkingLedGpioFd2 == -1)
    {
        Log_Debug("ERROR: Could not open WIZNET_ASG_EVB_STATUS_LD2 GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_Led2;
    }

    // Open WIZNET_ASG_EVB_STATUS_LD3, set as output with value GPIO_Value_High (off), and set up a timer to
    // blink it
    Log_Debug("Opening WIZNET_ASG_EVB_STATUS_LD3 as output.\n");
    blinkingLedGpioFd3 = GPIO_OpenAsOutput(WIZNET_ASG_EVB_STATUS_LD3, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (blinkingLedGpioFd3 == -1)
    {
        Log_Debug("ERROR: Could not open WIZNET_ASG_EVB_STATUS_LD3 GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_Led3;
    }

    // Open WIZNET_ASG_EVB_STATUS_LD4, set as output with value GPIO_Value_High (off), and set up a timer to
    // blink it
    Log_Debug("Opening WIZNET_ASG_EVB_STATUS_LD4 as output.\n");
    blinkingLedGpioFd4 = GPIO_OpenAsOutput(WIZNET_ASG_EVB_STATUS_LD4, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (blinkingLedGpioFd4 == -1)
    {
        Log_Debug("ERROR: Could not open WIZNET_ASG_EVB_STATUS_LD4 GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_Led4;
    }

    blinkTimer = CreateEventLoopPeriodicTimer(eventLoop, &BlinkingLedTimerEventHandler,
                 &blinkIntervals[blinkIntervalIndex]);
    if (blinkTimer == NULL)
    {
        return ExitCode_Init_LedBlinkTimer1;
    }

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
    if (blinkingLedGpioFd1 >= 0)
    {
        GPIO_SetValue(blinkingLedGpioFd1, GPIO_Value_High);
    }

    DisposeEventLoopTimer(buttonPollTimer);
    DisposeEventLoopTimer(blinkTimer);
    EventLoop_Close(eventLoop);

    Log_Debug("Closing file descriptors.\n");
    CloseFdAndPrintError(blinkingLedGpioFd1, "BlinkingLedGpio");
    CloseFdAndPrintError(ledBlinkRateButtonGpioFd, "LedBlinkRateButtonGpio1");
}

/// <summary>
///     Main entry point for this application.
/// </summary>
int main(int argc, char *argv[])
{
    Log_Debug("GPIO_HLApp_mt3620_BareMetal application starting.\n");
    exitCode = InitPeripheralsAndHandlers();

    // Use event loop to wait for events and trigger handlers, until an error or SIGTERM happens
    while (exitCode == ExitCode_Success)
    {
        EventLoop_Run_Result result = EventLoop_Run(eventLoop, -1, true);
        // Continue if interrupted by signal, e.g. due to breakpoint being set.
        if (result == EventLoop_Run_Failed && errno != EINTR)
        {
            exitCode = ExitCode_Main_EventLoopFail;
        }
    }

    ClosePeripheralsAndHandlers();
    Log_Debug("Application exiting.\n");
    return exitCode;
}