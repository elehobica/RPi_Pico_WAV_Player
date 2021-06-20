cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/../pico-sdk)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

set(project_name "RPi_Pico_WAV_Player" C CXX)
project(${project_name})
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(lib/hardware_gpio_ex)
add_subdirectory(lib/st7735_80x160)
add_subdirectory(lib/fatfs)
add_subdirectory(lib/file_menu)
add_subdirectory(lib/picojpeg)
add_subdirectory(lib/LcdElementBox)
add_subdirectory(lib/my_pico_audio)
add_subdirectory(lib/my_pico_audio_i2s)
add_subdirectory(lib/PlayAudio)

set(bin_name "RPi_Pico_WAV_Player")
add_executable(${bin_name}
    src/main.cpp
    src/ui_control.cpp
    src/UIMode.cpp
    src/ImageFitter.cpp
    src/LcdCanvas.cpp
    src/lcd_background.cpp
    src/stack.c
    src/TagRead.cpp
    src/utf_conv.cpp
    src/UserFlash.cpp
    src/ConfigMenu.cpp
    src/ConfigParam.cpp
)

pico_enable_stdio_usb(${bin_name} 1)
pico_enable_stdio_uart(${bin_name} 0)

target_link_libraries(${bin_name} 
        pico_stdlib
        pico_multicore
        hardware_uart
        hardware_pwm
        hardware_adc
        hardware_flash
        hardware_sync
        hardware_gpio_ex
        st7735_80x160
        fatfs
        file_menu
        picojpeg
        LcdElementBox
        PlayAudio
)

target_include_directories(${bin_name} INTERFACE src)

target_compile_definitions(${bin_name} PRIVATE
#    NO_BATTERY_VOLTAGE_CHECK
)

pico_add_extra_outputs(${bin_name})
