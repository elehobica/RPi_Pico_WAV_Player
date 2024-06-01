if (NOT TARGET LcdElementBox)
    add_library(LcdElementBox INTERFACE)

    target_sources(LcdElementBox INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/LcdElementBox.cpp
    )

    target_link_libraries(LcdElementBox INTERFACE
        pico_stdlib
        pico_fatfs
        pico_st7735_80x160
    )
    target_include_directories(LcdElementBox INTERFACE ${CMAKE_CURRENT_LIST_DIR})
endif()
