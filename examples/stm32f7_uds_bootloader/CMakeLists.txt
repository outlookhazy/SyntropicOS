cmake_minimum_required(VERSION 3.16)
project(stm32f7_uds_bootloader C)

set(CMAKE_C_STANDARD 99)

include_directories(
    ../../src
    ../../src/syntropic
)

# 1. Build Bootloader Binary (Sector 0 @ 0x08000000)
add_executable(stm32f7_fbl
    bootloader/src/main.c
    ../../src/syntropic/proto/syn_uds.c
    ../../src/syntropic/proto/syn_isotp.c
    ../../src/syntropic/crypto/syn_aes.c
    ../../src/syntropic/util/syn_qmath.c
)

# 2. Build Application Binary (Bank A / Bank B)
add_executable(stm32f7_app
    app/src/main.c
    ../../src/syntropic/proto/syn_uds.c
    ../../src/syntropic/proto/syn_isotp.c
    ../../src/syntropic/crypto/syn_aes.c
    ../../src/syntropic/util/syn_qmath.c
)

# 3. Post-Build Target to Automatically Patch App Binary Header
find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_custom_command(TARGET stm32f7_app POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:stm32f7_app> $<TARGET_FILE_DIR:stm32f7_app>/app.bin
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/patch_header.py $<TARGET_FILE_DIR:stm32f7_app>/app.bin 1 1 0
    COMMENT "Post-Build: Generating raw binary and patching SYN_FBL_AppHeader at offset 0..."
)
