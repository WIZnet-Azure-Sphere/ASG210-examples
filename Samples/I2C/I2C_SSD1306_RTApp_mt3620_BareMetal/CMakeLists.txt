# This code is based on a sample from Microsoft (see license below),
# with modifications made by MediaTek.
# Modified version of CMakeLists.txt from Microsoft Azure Sphere sample code:
# https://github.com/Azure/azure-sphere-samples/blob/master/Samples/HelloWorld/HelloWorld_RTApp_MT3620_BareMetal/CMakeLists.txt

#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required(VERSION 3.20)

# Configurations
project(I2C_SSD1306_RTApp_mt3620_BareMetal C)

azsphere_configure_tools(TOOLS_REVISION "23.05")

add_compile_definitions(OSAI_BARE_METAL)
add_compile_definitions(OSAI_ENABLE_DMA)
# When place CODE_REGION in FLASH instead of TCM, please enable this definition:
# add_compile_definitions(M4_ENABLE_XIP_FLASH)
add_link_options(-specs=nano.specs -specs=nosys.specs)

# Executable
add_executable(${PROJECT_NAME}
               main.c
               ../../../Utils/OS_HAL/src/os_hal_uart.c
               ../../../Utils/OS_HAL/src/os_hal_gpio.c
               ../../../Utils/OS_HAL/src/os_hal_dma.c
               ../../../Utils/OS_HAL/src/os_hal_gpt.c
               ../../../Utils/OS_HAL/src/os_hal_i2c.c
               SSD1306.c
               )

# Include Folders
target_include_directories(${PROJECT_NAME} PUBLIC
                           ../../../Utils/inc
                           ./)

# Libraries
add_subdirectory(../../../Utils/MT3620_M4_Driver ./lib/MT3620_M4_Driver)
target_link_libraries(${PROJECT_NAME} MT3620_M4_Driver)

# Linker, Image
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld)
azsphere_target_add_image_package(${PROJECT_NAME})
