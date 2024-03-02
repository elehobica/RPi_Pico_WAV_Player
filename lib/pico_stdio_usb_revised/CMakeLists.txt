if (NOT TARGET pico_stdio_usb_revised)
    add_library(pico_stdio_usb_revised INTERFACE)

    target_sources(pico_stdio_usb_revised INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/reset_interface.c
        ${CMAKE_CURRENT_LIST_DIR}/stdio_usb.c
        ${CMAKE_CURRENT_LIST_DIR}/stdio_usb_descriptors.c
    )

    target_link_libraries(pico_stdio_usb_revised INTERFACE
        tinyusb_device_unmarked
        pico_stdio
        pico_time
        pico_unique_id
    )
    target_include_directories(pico_stdio_usb_revised INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
endif()
