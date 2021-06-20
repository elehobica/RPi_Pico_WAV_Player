if (NOT TARGET my_pico_audio_i2s)
    add_library(my_pico_audio_i2s INTERFACE)

    pico_generate_pio_header(my_pico_audio_i2s ${CMAKE_CURRENT_LIST_DIR}/audio_i2s.pio)

    target_sources(my_pico_audio_i2s INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/audio_i2s.c
    )

    target_include_directories(my_pico_audio_i2s INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
    target_link_libraries(my_pico_audio_i2s INTERFACE
        pico_stdlib
        pico_multicore
        hardware_dma
        hardware_pio
        hardware_irq
        my_pico_audio
    )
endif()