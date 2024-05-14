cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

set(project_name "RPi_Pico_WAV_Player" C CXX)
project(${project_name})
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(lib/pico_st7735_80x160)
add_subdirectory(lib/pico_fatfs)
add_subdirectory(lib/file_menu)
add_subdirectory(lib/picojpeg)
add_subdirectory(lib/LcdElementBox)
add_subdirectory(lib/pico_audio_i2s_32b/src/pico_audio_32b)
add_subdirectory(lib/pico_audio_i2s_32b)
add_subdirectory(lib/pico_stdio_usb_revised)
add_subdirectory(lib/PlayAudio)

set(bin_name ${PROJECT_NAME})
add_executable(${bin_name}
    src/main.cpp
    src/power_manage.cpp
    src/ui_control.cpp
    src/UIMode.cpp
    src/ImageFitter.cpp
    src/LcdCanvas.cpp
    src/lcd_background.cpp
    src/TagRead.cpp
    src/utf_conv.cpp
    src/UserFlash.cpp
    src/ConfigMenu.cpp
    src/ConfigParam.cpp
)

target_link_libraries(${bin_name} 
        hardware_uart
        hardware_sleep
        hardware_flash
        hardware_adc
        hardware_exception
        hardware_watchdog
        pico_stdlib
        pico_flash
        pico_stdio_uart
        pico_stdio_usb_revised
        pico_fatfs
        pico_st7735_80x160
        file_menu
        picojpeg
        LcdElementBox
        PlayAudio
)

#pico_enable_stdio_usb(${bin_name} 1) --> pico_stdio_uart
#pico_enable_stdio_uart(${bin_name} 1) --> pico_stdio_usb_revised

target_compile_definitions(${bin_name} PRIVATE
#    NO_BATTERY_VOLTAGE_CHECK
)

pico_add_extra_outputs(${bin_name})
