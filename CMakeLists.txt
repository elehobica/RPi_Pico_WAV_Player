cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/../pico-sdk)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

set(project_name "android_hp_button" C CXX)
project(${project_name})
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory(hardware_gpio_ex)
add_subdirectory(st7735_80x160)
add_subdirectory(fatfs)
add_subdirectory(file_menu)
add_subdirectory(picojpeg)
add_subdirectory(LcdElementBox)
add_subdirectory(my_pico_audio)
add_subdirectory(my_pico_audio_i2s)
add_subdirectory(PlayAudio)

set(bin_name "android_hp_button")
add_executable(${bin_name}
    main.cpp
    ui_control.cpp
    UIMode.cpp
    ImageFitter.cpp
    LcdCanvas.cpp
    stack.c
    TagRead.cpp
    utf_conv.cpp
)

pico_enable_stdio_usb(${bin_name} 0)
pico_enable_stdio_uart(${bin_name} 1)

target_link_libraries(${bin_name} 
        pico_stdlib
        hardware_uart
        hardware_pwm
        hardware_adc
        hardware_gpio_ex
        st7735_80x160
        fatfs
        file_menu
        picojpeg
        LcdElementBox
        PlayAudio
)
pico_add_extra_outputs(${bin_name})