#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required(VERSION 3.20)

project(DHCP_HighLevelApp C)

azsphere_configure_tools(TOOLS_REVISION "23.05")
azsphere_configure_api(TARGET_API_SET "16")

add_executable(${PROJECT_NAME} main.c eventloop_timer_utilities.c)
target_link_libraries(${PROJECT_NAME} applibs gcc_s c)

azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../../Hardware/wiznet_asg210" TARGET_DEFINITION "wiznet_asg_evb_v1.0.json")

azsphere_target_add_image_package(${PROJECT_NAME})
