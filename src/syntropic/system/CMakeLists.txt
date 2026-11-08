target_sources(syntropic INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_boot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_coredump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_errlog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_fault.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_fwboot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_fwupdate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_hwwdt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_ota.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_power.c
)
