# Sample: GPIO_RTApp_mt3620 M4 real-time application - Bare Metal GPIO
### Description
This sample demonstrates how to use GPIO, GPT and UART on an MT3620 real-time core.  
- On-board LED GPIO41, 42, 43, 44, 45 and Pin J5 GPIO0, 1, 2, 3 are used as GPIO output.
- On-board user button(GPIO13) is used as GPIO input.
- GPT0 is used for LED blinking.
- GPT3 is used for motitoring user button status.
- PORT0 UART interface is used to print the output log.
- User could press user button to turn on LED GPIO44 or 45, and release to turn off.
(Note, UART port number in main.c could be changed from **OS_HAL_UART_PORT0** to **OS_HAL_UART_ISU3** to use M4 dedicate UART port.)
Please refer to the [MT3620 M4 API Rerference Manual](https://support.mediatek.com/AzureSphere/mt3620/M4_API_Reference_Manual) for the detailed API description.

### Prerequisites

The sample requires the following hardware:

![Azure Sphere Guardian 210](https://doc.wiznet.io/img/AzureSphere/ASG210_board_description.png)

* **Software**
    * Refer to [Azure Sphere software installation guide](https://docs.microsoft.com/en-ca/azure-sphere/install/overview).
    * A terminal emulator (such as Telnet or [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) to display the output log).

### How to build and run the sample
1. Start Visual Studio.  
2. From **File** menu, select **Open > CMake...** and navigate to the folder that contains this sample.  
3. Select **CMakeList.txt** and then click **Open**.  
4. Wait few seconds until Visual Studio finish create the project files.
5. From **Build** menu, select **Build ALL (Ctrl+Shift+B)**.  
6. Click **Select Start Item** and then select **GDB Debugger (RTCore)** as following.
7. Press **F5** to start the application with debugging.
