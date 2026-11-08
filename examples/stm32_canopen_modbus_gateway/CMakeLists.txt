cmake_minimum_required(VERSION 3.16)
project(stm32_canopen_modbus_gateway C)

set(CMAKE_C_STANDARD 99)
set(SYN_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

add_executable(stm32_canopen_modbus_gateway
    src/main.c
    ${SYN_ROOT}/src/syntropic/proto/syn_canopen.c
    ${SYN_ROOT}/src/syntropic/proto/syn_modbus.c
    ${SYN_ROOT}/src/syntropic/util/syn_crc.c
    ${SYN_ROOT}/src/syntropic/port_stubs/syn_port_stubs.c
)

target_compile_options(stm32_canopen_modbus_gateway PRIVATE -ffunction-sections -fdata-sections)
target_include_directories(stm32_canopen_modbus_gateway PRIVATE
    ${SYN_ROOT}/src
)
target_link_options(stm32_canopen_modbus_gateway PRIVATE -Wl,--gc-sections)
