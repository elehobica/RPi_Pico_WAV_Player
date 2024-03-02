if (NOT TARGET pico_audio_32b_headers)
    add_library(pico_audio_32b_headers INTERFACE)
    target_include_directories(pico_audio_32b_headers INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
    target_link_libraries(pico_audio_32b_headers INTERFACE pico_util_buffer)
endif()

if (NOT TARGET pico_audio_32b)
    add_library(pico_audio_32b INTERFACE)

    target_sources(pico_audio_32b INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/audio.cpp
            $<$<NOT:$<BOOL:${PICO_NO_HARDWARE}>>:${CMAKE_CURRENT_LIST_DIR}/audio_utils.S>
    )

    target_link_libraries(pico_audio_32b INTERFACE pico_audio_32b_headers pico_sync pico_util_buffer)
endif()
