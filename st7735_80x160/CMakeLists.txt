if (NOT TARGET st7735_80x160)
    add_library(st7735_80x160 INTERFACE)

    target_sources(st7735_80x160 INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/lcd.c
            ${CMAKE_CURRENT_LIST_DIR}/my_lcd.c
    )

    target_link_libraries(st7735_80x160 INTERFACE pico_stdlib hardware_clocks hardware_spi)
endif()
