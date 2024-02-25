if (NOT TARGET my_pico_stdio_usb)
    add_library(my_pico_stdio_usb INTERFACE)

    target_sources(my_pico_stdio_usb INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/reset_interface.c
        ${CMAKE_CURRENT_LIST_DIR}/stdio_usb.c
        ${CMAKE_CURRENT_LIST_DIR}/stdio_usb_descriptors.c
    )

    target_link_libraries(my_pico_stdio_usb INTERFACE
        tinyusb_device_unmarked
        pico_stdio
        pico_time
        pico_unique_id
    )
    target_include_directories(my_pico_stdio_usb INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
endif()
