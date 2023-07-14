/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

// This sample C application for Azure Sphere uses the Azure Sphere I2C APIs to display
// data from an accelerometer connected via I2C.
//
// It uses the APIs for the following Azure Sphere application libraries:
// - log (messages shown in Visual Studio's Device Output window during debugging)
// - i2c (communicates with LSM6DS3 accelerometer)
// - eventloop (system invokes handlers for timer events)

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include <applibs/log.h>
#include <applibs/i2c.h>
#include <applibs/gpio.h>
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

#include "eventloop_timer_utilities.h"
#include "sensors.h"
#include "lis2dh12.h"

/// <summary>
/// Exit codes for this application. These are used for the
/// application exit code. They must all be between zero and 255,
/// where zero is reserved for successful termination.
/// </summary>
typedef enum
{
    ExitCode_Success = 0,

    ExitCode_TermHandler_SigTerm = 1,

    ExitCode_Init_EventLoop = 2,
    ExitCode_Init_OpenMaster = 3,
    ExitCode_Init_SetBusSpeed = 4,
    ExitCode_Init_SetTimeout = 5,
    ExitCode_Init_SetDefaultTarget = 6,

    ExitCode_Init_OpenButton = 7,
    ExitCode_Init_ButtonPollTimer = 8,
    ExitCode_Init_I2C_NSEL = 9,
    ExitCode_Init_INT_NSEL = 10,
    ExitCode_ButtonTimer_Consume = 12,
    ExitCode_ButtonTimer_GetValue = 12,

    ExitCode_Main_EventLoopFail = 13
} ExitCode;

// Support functions.
static void TerminationHandler(int signalNumber);
static bool CheckTransferSize(const char *desc, size_t expectedBytes, ssize_t actualBytes);
static ExitCode InitPeripheralsAndHandlers(void);
static void CloseFdAndPrintError(int fd, const char *fdName);
static void ClosePeripheralsAndHandlers(void);
static void ButtonTimerEventHandler(EventLoopTimer *timer);

// File descriptors - initialized to invalid value
static int i2cFd = -1;
static int gpioButtonFd = -1;
static int gpioI2CnSel = -1;
static int gpioINTnSel = -1;

static EventLoop *eventLoop = NULL;
static EventLoopTimer *buttonPollTimer = NULL;

// State variables
static GPIO_Value_Type buttonState = GPIO_Value_High;

static const uint8_t slaveAddress = LIS2DH12_I2C_ADDRESS;
bool i2cWrite = false;

// Termination state
static volatile sig_atomic_t exitCode = ExitCode_Success;

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async-signal-safe.
    exitCode = ExitCode_TermHandler_SigTerm;
}

/// <summary>
///     Handle button timer event: if the button is pressed, send data over the UART.
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
    int result = GPIO_GetValue(gpioButtonFd, &newButtonState);
    if (result != 0)
    {
        Log_Debug("ERROR: Could not read button GPIO: %s (%d).\n", strerror(errno), errno);
        exitCode = ExitCode_ButtonTimer_GetValue;
        return;
    }

    // If the button has just been pressed, send data over the UART
    // The button has GPIO_Value_Low when pressed and GPIO_Value_High when released
    if (newButtonState != buttonState)
    {
        if (newButtonState == GPIO_Value_Low)
        {
            Log_Debug("Read button GPIO.\n");
            i2cWrite = true;
        }
        buttonState = newButtonState;
    }
}

/// <summary>
///    Checks the number of transferred bytes for I2C functions and prints an error
///    message if the functions failed or if the number of bytes is different than
///    expected number of bytes to be transferred.
/// </summary>
/// <returns>true on success, or false on failure</returns>
static bool CheckTransferSize(const char *desc, size_t expectedBytes, ssize_t actualBytes)
{
    if (actualBytes < 0)
    {
        Log_Debug("ERROR: %s: errno=%d (%s)\n", desc, errno, strerror(errno));
        return false;
    }

    if (actualBytes != (ssize_t)expectedBytes)
    {
        Log_Debug("ERROR: %s: transferred %zd bytes; expected %zd\n", desc, actualBytes,
                  expectedBytes);
        return false;
    }

    return true;
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

    // Open WIZNET_ASG_EVB_MB1_I2C_NSEL GPIO, set as output with value GPIO_Value_Low for using I2C
    Log_Debug("Opening WIZNET_ASG_EVB_MB1_I2C_NSEL as output.\n");
    gpioI2CnSel = GPIO_OpenAsOutput(WIZNET_ASG_EVB_MB1_I2C_NSEL, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    if (gpioI2CnSel == -1)
    {
        Log_Debug("ERROR: Could not open WIZNET_ASG_EVB_MB1_I2C_NSEL GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_I2C_NSEL;
    }

    // Open WIZNET_ASG_EVB_ISU3_INT_NSEL GPIO, set as output with value GPIO_Value_Low for mB1/mB2(high for external)
    Log_Debug("Opening WIZNET_ASG_EVB_ISU3_INT_NSEL as output.\n");
    gpioINTnSel = GPIO_OpenAsOutput(WIZNET_ASG_EVB_ISU3_INT_NSEL, GPIO_OutputMode_PushPull, GPIO_Value_Low);
    if (gpioINTnSel == -1)
    {
        Log_Debug("ERROR: Could not open WIZNET_ASG_EVB_ISU3_INT_NSEL GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_INT_NSEL;
    }

    i2cFd = I2CMaster_Open(WIZNET_ASG_EVB_ISU3_I2C);
    if (i2cFd == -1)
    {
        Log_Debug("ERROR: I2CMaster_Open: errno=%d (%s)\n", errno, strerror(errno));
        return ExitCode_Init_OpenMaster;
    }

    int result = I2CMaster_SetBusSpeed(i2cFd, I2C_BUS_SPEED_STANDARD);
    if (result != 0)
    {
        Log_Debug("ERROR: I2CMaster_SetBusSpeed: errno=%d (%s)\n", errno, strerror(errno));
        return ExitCode_Init_SetBusSpeed;
    }

    result = I2CMaster_SetTimeout(i2cFd, 3000);
    if (result != 0)
    {
        Log_Debug("ERROR: I2CMaster_SetTimeout: errno=%d (%s)\n", errno, strerror(errno));
        return ExitCode_Init_SetTimeout;
    }

    // This default address is used for POSIX read and write calls.  The AppLibs APIs take a target
    // address argument for each read or write.
    result = I2CMaster_SetDefaultTargetAddress(i2cFd, slaveAddress);
    if (result != 0)
    {
        Log_Debug("ERROR: I2CMaster_SetDefaultTargetAddress: errno=%d (%s)\n", errno,
                  strerror(errno));
        return ExitCode_Init_SetDefaultTarget;
    }

    // Open WIZNET_ASG_EVB_USER_SW GPIO as input, and set up a timer to poll it.
    Log_Debug("Opening WIZNET_ASG_EVB_USER_SW as input.\n");
    gpioButtonFd = GPIO_OpenAsInput(WIZNET_ASG_EVB_USER_SW);
    if (gpioButtonFd == -1)
    {
        Log_Debug("ERROR: Could not open button GPIO: %s (%d).\n", strerror(errno), errno);
        return ExitCode_Init_OpenButton;
    }
    struct timespec buttonPressCheckPeriod1Ms = {.tv_sec = 0, .tv_nsec = 1000 * 1000};
    buttonPollTimer = CreateEventLoopPeriodicTimer(eventLoop, ButtonTimerEventHandler,
                      &buttonPressCheckPeriod1Ms);
    if (buttonPollTimer == NULL)
    {
        return ExitCode_Init_ButtonPollTimer;
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
    DisposeEventLoopTimer(buttonPollTimer);
    EventLoop_Close(eventLoop);

    Log_Debug("Closing file descriptors.\n");
    CloseFdAndPrintError(gpioButtonFd, "gpioButtonFd");
    CloseFdAndPrintError(i2cFd, "i2c");
    CloseFdAndPrintError(gpioI2CnSel, "gpioI2CnSel");
}

/// <summary>
///     Main entry point for this application.
/// </summary>
int main(int argc, char *argv[])
{
    int read_cnt = 0;
    ssize_t transferredBytes;
    float acce_x;
    float acce_y;
    float acce_z;

    Log_Debug("I2C Write/Read application starting.\n");
    exitCode = InitPeripheralsAndHandlers();

    lis2dh12_init(i2cFd);

    // Use event loop to wait for events and trigger handlers, until an error or SIGTERM happens
    while (exitCode == ExitCode_Success)
    {
        EventLoop_Run_Result result = EventLoop_Run(eventLoop, -1, true);
        // Continue if interrupted by signal, e.g. due to breakpoint being set.
        if (result == EventLoop_Run_Failed && errno != EINTR)
        {
            exitCode = ExitCode_Main_EventLoopFail;
        }

        if (i2cWrite == true)
        {
            int ret = imu_lis2dh12_acquire_acce(&acce_x, &acce_y, &acce_z);
            if (ret != 0)
            {
                Log_Debug("imu_lis2dh12_acquire_acce() failed");
            }
            else
            {
                Log_Debug("accel.xyz %d\n", read_cnt);
                Log_Debug("%f %f %f\n", acce_x, acce_y, acce_z);
                Log_Debug("\n");
            }

            read_cnt++;
            if (read_cnt == 100)
            {
                i2cWrite = false;
                read_cnt = 0;
            }
        }
    }

    ClosePeripheralsAndHandlers();
    Log_Debug("Application exiting.\n");
    return exitCode;
}
