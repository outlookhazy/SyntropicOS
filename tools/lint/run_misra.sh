#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

echo "=== SyntropicOS MISRA C:2023 Safety Rule Compliance Checker ==="

if ! command -v cppcheck >/dev/null 2>&1; then
    echo "ERROR: cppcheck not found. Run via container: make container-misra" >&2
    exit 1
fi

JOBS=$(nproc 2>/dev/null || echo 4)
mkdir -p build/cppcheck_err build/cppcheck_misra

SYN_CPPCHECK_DEFS=(
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

# 1. Hard error gate: fail on any Cppcheck error-severity defect
cppcheck --std=c99 -j "${JOBS}" \
    --check-level=exhaustive \
    --cppcheck-build-dir=build/cppcheck_err \
    --error-exitcode=1 \
    --inline-suppr \
    --suppress=missingIncludeSystem \
    "${SYN_CPPCHECK_DEFS[@]}" \
    -I src -I . \
    src/

# 2. MISRA C:2023 addon audit (fails if cppcheck crashes; advisory style notes do not false-fail)
cppcheck --std=c99 -j "${JOBS}" \
    --cppcheck-build-dir=build/cppcheck_misra \
    --enable=warning,style,performance,portability \
    --addon=misra \
    --inline-suppr \
    --suppress=missingIncludeSystem \
    --suppress=unusedFunction \
    "${SYN_CPPCHECK_DEFS[@]}" \
    -I src -I . \
    src/

echo "=== MISRA C:2023 Compliance Scan Complete ==="
