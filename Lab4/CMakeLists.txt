# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Lab4 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Lab4 
        Lab4.c
        src/adc_audio.c
        src/nmea_parser.c
        src/led_status.c
        src/eeprom.c)

pico_set_program_name(Lab4 "Lab4")
pico_set_program_version(Lab4 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Lab4 0)
pico_enable_stdio_usb(Lab4 1)

# Add the standard library to the build
target_link_libraries(Lab4
        pico_stdlib
        hardware_gpio
        hardware_irq
        hardware_sync
        hardware_adc
        hardware_pwm
        hardware_i2c)

# Add the standard include files to the build
target_include_directories(Lab4 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/src
)

pico_add_extra_outputs(Lab4)

