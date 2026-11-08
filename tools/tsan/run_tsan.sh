#!/usr/bin/env bash
set -euo pipefail

# SyntropicOS ThreadSanitizer (TSan) Data Race Analysis Runner
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

if ! command -v gcc >/dev/null 2>&1; then
    echo "ERROR: gcc not found. Run via container: make container-tsan" >&2
    exit 1
fi

echo "=== SyntropicOS ThreadSanitizer (TSan) Multithread & Data Race Analysis ==="

BUILD_DIR="build/tests_tsan"
mkdir -p "${BUILD_DIR}"
make -f tests/Makefile.unity wasm-fixtures BUILD_DIR=build/tests

SYN_FLAGS=(
    -DSYN_LOG_COLOR=1
    -DSYN_CLI_HISTORY_DEPTH=4
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
)

# Compile full unit test suite against mock_port.c (no live POSIX socket override)
gcc -std=c99 -g -O1 -fno-omit-frame-pointer -fsanitize=thread -pthread \
    -I. -Isrc -Itests/unit -Itests/unit/mocks -Ibuild/tests \
    "${SYN_FLAGS[@]}" \
    $(find src/syntropic -name "*.c" ! -path "*/port_stubs/*" ! -name "syn_wg.c") \
    tests/unit/unity/unity.c \
    tests/unit/mocks/mock_port.c \
    $(find tests/unit -name "test_*.c") \
    -o "${BUILD_DIR}/test_tsan" -lm

TSAN_OPTIONS="halt_on_error=1" "./${BUILD_DIR}/test_tsan"
rm -f "${BUILD_DIR}/test_tsan"

echo "=== ThreadSanitizer (TSan) Audit Completed Cleanly (0 Data Races) ==="
