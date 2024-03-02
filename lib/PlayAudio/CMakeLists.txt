if (NOT TARGET PlayAudio)
    add_library(PlayAudio INTERFACE)

    target_sources(PlayAudio INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/audio_init.cpp
            ${CMAKE_CURRENT_LIST_DIR}/audio_codec.cpp
            ${CMAKE_CURRENT_LIST_DIR}/ReadBuffer.cpp
            ${CMAKE_CURRENT_LIST_DIR}/PlayAudio.cpp
            ${CMAKE_CURRENT_LIST_DIR}/PlayNone.cpp
            ${CMAKE_CURRENT_LIST_DIR}/PlayWav.cpp
    )

    target_link_libraries(PlayAudio INTERFACE
        pico_stdlib
        pico_fatfs
        my_pico_audio
        my_pico_audio_i2s
    )
    target_include_directories(PlayAudio INTERFACE ${CMAKE_CURRENT_LIST_DIR})
endif()