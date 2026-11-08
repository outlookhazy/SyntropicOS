#!/usr/bin/env bash
set -euo pipefail

# SyntropicOS Binary Memory Footprint & Section Budget Analyzer
# Analyzes Flash (.text + .rodata) and RAM (.data + .bss) allocation per subsystem.

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

BUILD_DIR="build/size_analysis"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

SYN_FLAGS=(
    -DSYN_LOG_COLOR=0
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

if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    CC_TOOL="arm-none-eabi-gcc"
    SIZE_TOOL="arm-none-eabi-size"
    ARCH_FLAGS=(-std=c99 -mcpu=cortex-m4 -mthumb -Os -I. -Isrc)
    TARGET_DESC="ARM Cortex-M4 (-mcpu=cortex-m4 -mthumb -Os)"
elif command -v gcc >/dev/null 2>&1; then
    CC_TOOL="gcc"
    SIZE_TOOL="size"
    ARCH_FLAGS=(-std=c99 -Os -I. -Isrc)
    TARGET_DESC="Host GCC (-Os)"
else
    echo "ERROR: No C compiler found for size analysis." >&2
    exit 1
fi

echo "=== SyntropicOS Microcontroller Binary Footprint & Subsystem Memory Audit ==="
echo "Target Architecture: ${TARGET_DESC}"
echo ""

FILES=$(find src/syntropic -name "*.c" ! -path "*/port_stubs/*" | sort)

# Compile size-optimized objects (fail on any compile error)
for f in ${FILES}; do
    subsys=$(echo "${f}" | awk -F'/' '{print $(NF-1)}')
    mkdir -p "${BUILD_DIR}/${subsys}"
    obj="${BUILD_DIR}/${subsys}/$(basename "${f}" .c).o"
    "${CC_TOOL}" "${ARCH_FLAGS[@]}" "${SYN_FLAGS[@]}" -c "${f}" -o "${obj}"
done

printf "| %-18s | %-12s | %-12s | %-12s | %-14s |\n" "Subsystem" "Flash (Text)" "RAM (Data)" "RAM (BSS)" "Total (Bytes)"
printf "|--------------------|--------------|--------------|--------------|----------------|\n"

TOTAL_TEXT=0
TOTAL_DATA=0
TOTAL_BSS=0

for subsys_dir in "${BUILD_DIR}"/*; do
    if [ -d "${subsys_dir}" ]; then
        subsys=$(basename "${subsys_dir}")
        subsys_text=0
        subsys_data=0
        subsys_bss=0

        for obj in "${subsys_dir}"/*.o; do
            if [ -f "${obj}" ]; then
                read -r text data bss _ < <("${SIZE_TOOL}" "${obj}" | tail -n 1)
                subsys_text=$((subsys_text + text))
                subsys_data=$((subsys_data + data))
                subsys_bss=$((subsys_bss + bss))
            fi
        done

        subsys_total=$((subsys_text + subsys_data + subsys_bss))
        TOTAL_TEXT=$((TOTAL_TEXT + subsys_text))
        TOTAL_DATA=$((TOTAL_DATA + subsys_data))
        TOTAL_BSS=$((TOTAL_BSS + subsys_bss))

        printf "| %-18s | %-12d | %-12d | %-12d | %-14d |\n" "${subsys}" "${subsys_text}" "${subsys_data}" "${subsys_bss}" "${subsys_total}"
    fi
done

GRAND_TOTAL=$((TOTAL_TEXT + TOTAL_DATA + TOTAL_BSS))
printf "|--------------------|--------------|--------------|--------------|----------------|\n"
printf "| %-18s | %-12d | %-12d | %-12d | %-14d |\n" "TOTAL" "${TOTAL_TEXT}" "${TOTAL_DATA}" "${TOTAL_BSS}" "${GRAND_TOTAL}"

rm -rf "${BUILD_DIR}"
