if (NOT TARGET pico_flash_param)
    add_library(pico_flash_param INTERFACE)

    target_sources(pico_flash_param INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/FlashParam.cpp
        ${CMAKE_CURRENT_LIST_DIR}/UserFlash.cpp
    )

    target_link_libraries(pico_flash_param INTERFACE
        hardware_exception
        pico_flash
        pico_stdlib
    )
    target_include_directories(pico_flash_param INTERFACE ${CMAKE_CURRENT_LIST_DIR})
endif()