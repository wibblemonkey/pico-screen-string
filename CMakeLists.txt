# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(PicoScreenString C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(common)
add_subdirectory(drivers)
add_subdirectory(screens)

# Add executable. Default name is the project name, version 0.1

add_executable(
    PicoScreenString 
        main.cpp
        ZxSpectrumDisplayContext.cpp
        Display.cpp
        PicoDisplay28.cpp
        PicoScreenString.cpp
        ScreenStringLoader.cpp
        ScreenStringProgrammer.cpp
)

pico_set_program_name(PicoScreenString "PicoScreenString")
pico_set_program_version(PicoScreenString "0.1")

# Generate PIO header
#pico_generate_pio_header(PicoScreenString ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(PicoScreenString 0)
pico_enable_stdio_usb(PicoScreenString 1)

# Add the standard library to the build
target_link_libraries(
    PicoScreenString
        pico_stdlib
)

# Add the standard include files to the build
target_include_directories(
    PicoScreenString PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(
    PicoScreenString 
        pico_multicore
        pico_rand
        hardware_spi
        hardware_dma
        hardware_pio
        hardware_pwm
        rgbled
        button
        screens
)

pico_add_extra_outputs(PicoScreenString)
