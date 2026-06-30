cmake_minimum_required(VERSION 3.13)

# ── Pico SDK ────────────────────────────────────────────────────────────────
# PICO_SDK_PATH must be set in the environment or passed to CMake.
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico_dual_core C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# ── SyntropicOS ─────────────────────────────────────────────────────────────
set(SYN_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

# Pull in the syntropic interface library (provides core sources + includes)
add_subdirectory(${SYN_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/syntropic)

# ── Example executable ──────────────────────────────────────────────────────
add_executable(pico_dual_core
    main.c
    ${SYN_ROOT}/src/port/rp2040/port_rp2040.c
    ${SYN_ROOT}/src/port/rp2040/port_rp2040_multicore.c
)

target_include_directories(pico_dual_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(pico_dual_core PRIVATE
    SYN_USE_MULTICORE=1
    SYN_SPINLOCK_COUNT=4
)

target_link_libraries(pico_dual_core
    syntropic
    syn_stubs
    pico_stdlib
    pico_multicore
    hardware_sync
)

# USB serial output (not UART)
pico_enable_stdio_usb(pico_dual_core 1)
pico_enable_stdio_uart(pico_dual_core 0)

# Generate UF2 for drag-and-drop flash
pico_add_extra_outputs(pico_dual_core)
