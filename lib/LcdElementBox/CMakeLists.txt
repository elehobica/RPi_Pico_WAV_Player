if (NOT TARGET LcdElementBox)
    add_library(LcdElementBox INTERFACE)

    target_sources(LcdElementBox INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/LcdElementBox.cpp
    )

    target_link_libraries(LcdElementBox INTERFACE pico_stdlib fatfs)
    target_include_directories(LcdElementBox INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../)
endif()
