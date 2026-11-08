target_sources(syntropic INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_adpcm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_audio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_audio_mixer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_sbc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_vad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_wav.c
)
