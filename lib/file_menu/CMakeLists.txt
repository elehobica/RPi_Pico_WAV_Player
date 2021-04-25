if (NOT TARGET file_menu)
    add_library(file_menu INTERFACE)

    target_sources(file_menu INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/file_menu_FatFs.c
    )

    target_link_libraries(file_menu INTERFACE pico_stdlib fatfs)
endif()
