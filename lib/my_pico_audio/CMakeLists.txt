if (NOT TARGET my_pico_audio_headers)
    add_library(my_pico_audio_headers INTERFACE)
    target_include_directories(my_pico_audio_headers INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
    target_link_libraries(my_pico_audio_headers INTERFACE pico_util_buffer)
endif()

if (NOT TARGET my_pico_audio)
    add_library(my_pico_audio INTERFACE)

    target_sources(my_pico_audio INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/audio.cpp
            $<$<NOT:$<BOOL:${PICO_NO_HARDWARE}>>:${CMAKE_CURRENT_LIST_DIR}/audio_utils.S>
    )

    target_link_libraries(my_pico_audio INTERFACE my_pico_audio_headers pico_sync pico_util_buffer)
endif()
