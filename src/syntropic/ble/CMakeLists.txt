# BLE Host Stack CMakeLists.txt

target_sources(syntropic INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_ble_hci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_ble_l2cap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_ble_att.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_ble_gap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syn_ble_gatt.c
)

target_include_directories(syntropic INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
