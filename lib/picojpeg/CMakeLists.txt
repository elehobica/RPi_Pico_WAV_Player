if (NOT TARGET picojpeg)
    add_library(picojpeg INTERFACE)

    target_sources(picojpeg INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/picojpeg.c
            ${CMAKE_CURRENT_LIST_DIR}/JPEGDecoder.cpp
    )

    target_link_libraries(picojpeg INTERFACE pico_stdlib fatfs)
    target_include_directories(picojpeg INTERFACE ${CMAKE_CURRENT_LIST_DIR})
endif()
