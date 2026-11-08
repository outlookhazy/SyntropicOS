target_sources(syntropic INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_aes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_aes_cmac.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_asn1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_blake2s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_chacha20poly1305.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_ed25519.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_hkdf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_hmac_drbg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_p256.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_sha256.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_sha512.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_tpm2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_x25519.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_x509.c
)
