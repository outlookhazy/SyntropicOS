#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

echo "=== SyntropicOS AST Code Quality Linter (clang-tidy) ==="

if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "ERROR: clang-tidy not found. Run via container: make container-lint" >&2
    exit 1
fi

JOBS=$(nproc 2>/dev/null || echo 4)
CFLAGS="-std=c99 -D_DEFAULT_SOURCE -I. -Isrc -Itests/unit -Itests/unit/mocks \
-DSYN_LOG_COLOR=1 -DSYN_CLI_HISTORY_DEPTH=4 -DSYN_USE_COREDUMP=1 -DSYN_COREDUMP_FLASH_ADDR=0 -DSYN_USE_TICKLESS=1 \
-DSYN_USE_DMA=1 -DSYN_USE_I2C_ASYNC=1 -DSYN_USE_SPI_ASYNC=1 -DSYN_FW_USE_HMAC=1 \
-DSYN_FW_USE_ED25519=1 -DSYN_FW_USE_AES_GCM=1 -DSYN_USE_PORT_AES=1 -DSYN_USE_MULTICORE=1 \
-DSYN_USE_METRICS=1 -DSYN_USE_ROUTER=1 -DSYN_USE_LIN=1 -DSYN_USE_LINTP=1 \
-DSYN_USE_GBT27930=1 -DSYN_USE_CANNM=1 -DSYN_USE_IR=1 -DSYN_USE_SMBUS=1 -DSYN_USE_PMBUS=1 \
-DSYN_USE_MBUS=1 -DSYN_USE_DLT645=1 -DSYN_USE_AT_PARSER=1 -DSYN_USE_BACNET=1 \
-DSYN_USE_DEVICENET=1 -DSYN_USE_YMODEM=1 -DSYN_USE_AUDIO=1 -DSYN_USE_ADPCM=1 -DSYN_USE_SBC=1 \
-DSYN_USE_BLE=1 -DSYN_USE_OTA=1 -DSYN_USE_LWM2M_TASK=1 -DSYN_USE_CAN_FD=1 -DSYN_USE_RTC=1 \
-DSYN_USE_WG=1"

FILES=$(find src/syntropic -name "*.c" ! -path "*/port_stubs/*")

echo "Running clang-tidy with ${JOBS} parallel workers..."
printf "%s\n" ${FILES} | xargs -n 1 -P "${JOBS}" -I {} clang-tidy {} -- ${CFLAGS}

echo "=== Running Protothread Switch Linter ==="
python3 tools/lint/check_pt_switch.py

echo "=== Clang-Tidy Complete ==="
