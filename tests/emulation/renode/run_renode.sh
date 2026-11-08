#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
cd "${ROOT_DIR}"

echo "=== SyntropicOS Renode Board-Level Hardware Emulation Suite ==="

if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    echo "=== Cross-Compiling Bare-Metal STM32F407 Renode Firmware ELFs ==="
    make -f tests/emulation/renode/Makefile.renode clean all
    make -f tests/emulation/renode/Makefile.renode clean
fi

make -f tests/Makefile.unity wasm-fixtures BUILD_DIR=build/tests

CFLAGS=(
    -std=c99
    -D_DEFAULT_SOURCE
    -pedantic
    -Wall
    -Wextra
    -Werror
    -I.
    -Isrc
    -Itests/unit
    -Itests/unit/mocks
    -Ibuild/tests
    -DSYN_LOG_COLOR=1
    -DSYN_USE_COREDUMP=1
    -DSYN_COREDUMP_FLASH_ADDR=0
    -DSYN_USE_TICKLESS=1
    -DSYN_USE_DMA=1
    -DSYN_USE_I2C_ASYNC=1
    -DSYN_USE_SPI_ASYNC=1
    -DSYN_FW_USE_HMAC=1
    -DSYN_FW_USE_ED25519=1
    -DSYN_FW_USE_AES_GCM=1
    -DSYN_USE_PORT_AES=1
    -DSYN_USE_MULTICORE=1
    -DUNITY_INCLUDE_DOUBLE
    -DSYN_USE_METRICS=1
    -DSYN_USE_ROUTER=1
    -DSYN_USE_LIN=1
    -DSYN_USE_LINTP=1
    -DSYN_USE_GBT27930=1
    -DSYN_USE_CANNM=1
    -DSYN_USE_IR=1
    -DSYN_USE_SMBUS=1
    -DSYN_USE_PMBUS=1
    -DSYN_USE_MBUS=1
    -DSYN_USE_DLT645=1
    -DSYN_USE_AT_PARSER=1
    -DSYN_USE_BACNET=1
    -DSYN_USE_DEVICENET=1
    -DSYN_USE_YMODEM=1
    -DSYN_USE_AUDIO=1
    -DSYN_USE_ADPCM=1
    -DSYN_USE_SBC=1
    -DSYN_USE_BLE=1
    -DSYN_USE_OTA=1
    -DSYN_USE_LWM2M_TASK=1
    -DSYN_USE_CAN_FD=1
    -DSYN_USE_RTC=1
    -DSYN_USE_WG=1
)
SRC_FILES=$(find src/syntropic -name "*.c" ! -path "*/port_stubs/*" ! -name "syn_lfs.c")

echo "=== Compiling Hardware Emulation Test Drivers ==="
gcc "${CFLAGS[@]}" ${SRC_FILES} tests/unit/unity/unity.c tests/unit/mocks/mock_port.c tests/emulation/renode/test_renode_board.c -o test_renode_board -lm
gcc "${CFLAGS[@]}" ${SRC_FILES} tests/unit/unity/unity.c tests/unit/mocks/mock_port.c tests/emulation/renode/test_renode_multinode.c -o test_renode_multinode -lm
gcc "${CFLAGS[@]}" ${SRC_FILES} tests/unit/unity/unity.c tests/unit/mocks/mock_port.c tests/emulation/renode/test_renode_lfs.c -o test_renode_lfs -lm
gcc "${CFLAGS[@]}" ${SRC_FILES} tests/unit/unity/unity.c tests/unit/mocks/mock_port.c tests/emulation/renode/test_renode_display.c -o test_renode_display -lm
gcc "${CFLAGS[@]}" ${SRC_FILES} tests/unit/unity/unity.c tests/unit/mocks/mock_port.c tests/emulation/renode/test_renode_control.c -o test_renode_control -lm

echo "=== 1. W25Q64 SPI Flash & MPU6050 I2C Emulation ==="
./test_renode_board

echo "=== 2. Multi-Node Inter-MCU COBS Transport Emulation ==="
./test_renode_multinode

echo "=== 3. SPI Flash Wear-Leveled Parameter Storage Emulation ==="
./test_renode_lfs

echo "=== 4. Display Controller Canvas Framebuffer Emulation ==="
./test_renode_display

echo "=== 5. Closed-Loop ADC -> PID -> PWM Control Emulation ==="
./test_renode_control

rm -f test_renode_board test_renode_multinode test_renode_lfs test_renode_display test_renode_control
echo "=== Renode Hardware Emulation Suite Complete ==="
