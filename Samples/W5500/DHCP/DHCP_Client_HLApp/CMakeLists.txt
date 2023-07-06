#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required(VERSION 3.20)

project(DHCP_Client_HLApp C)

azsphere_configure_tools(TOOLS_REVISION "23.05")
azsphere_configure_api(TARGET_API_SET "16")

add_executable(${PROJECT_NAME} main.c eventloop_timer_utilities.c
    ../../../../Utils/WIZnet_Driver/ioLibrary_Driver/Ethernet/W5500/w5500.c
    ../../../../Utils/WIZnet_Driver/ioLibrary_Driver/Ethernet/wizchip_conf.c
    ../../../../Utils/WIZnet_Driver/ioLibrary_Driver/Ethernet/socket.c
    ../../../../Utils/WIZnet_Driver/ioLibrary_Driver/Application/loopback/loopback.c
    ../../../../Utils/WIZnet_Driver/ioLibrary_Driver/Internet/DHCP/dhcp.c
)
target_link_libraries(${PROJECT_NAME} applibs pthread gcc_s c)

azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../../../../Hardware/wiznet_asg210" TARGET_DEFINITION "wiznet_asg_evb_v1.0.json")

azsphere_target_add_image_package(${PROJECT_NAME})

# W5500 driver
INCLUDE_DIRECTORIES("../../../../Utils/WIZnet_Driver/ioLibrary_Driver/Ethernet/")
INCLUDE_DIRECTORIES("../../../../Utils/WIZnet_Driver/ioLibrary_Driver/Application/loopback/")
INCLUDE_DIRECTORIES("../../../../Utils/WIZnet_Driver/ioLibrary_Driver/Internet/DHCP/")

