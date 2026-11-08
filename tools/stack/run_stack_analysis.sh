#!/usr/bin/env bash
set -euo pipefail

# SyntropicOS Stack Usage Analyzer (-fstack-usage)
# Analyzes static stack frame allocations across all SyntropicOS source files.

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

STACK_THRESHOLD_BYTES=${STACK_THRESHOLD_BYTES:-1024}
ECC_STACK_THRESHOLD_BYTES=${ECC_STACK_THRESHOLD_BYTES:-1920}
BUILD_DIR="build/stack_analysis"
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
    ARCH_FLAGS=(-mcpu=cortex-m4 -mthumb -Os)
    TARGET_DESC="ARM Cortex-M4 (-mcpu=cortex-m4 -mthumb -Os)"
elif command -v gcc >/dev/null 2>&1; then
    CC_TOOL="gcc"
    ARCH_FLAGS=(-Os)
    TARGET_DESC="Host GCC (-Os)"
else
    echo "ERROR: No C compiler found for stack analysis." >&2
    exit 1
fi

echo "=== SyntropicOS Stack Frame Usage Analysis ==="
echo "Target Architecture: ${TARGET_DESC}"
echo "Max Stack Frame Threshold: ${STACK_THRESHOLD_BYTES} bytes (ECC bignum: ${ECC_STACK_THRESHOLD_BYTES} bytes)"

FILES=$(find src/syntropic -name "*.c" ! -path "*/port_stubs/*" | sort)

for f in ${FILES}; do
    rel_dir=$(dirname "${f}")
    mkdir -p "${BUILD_DIR}/${rel_dir}"
    obj="${BUILD_DIR}/${f%.c}.o"
    "${CC_TOOL}" -std=c99 "${ARCH_FLAGS[@]}" -I. -Isrc \
        "${SYN_FLAGS[@]}" -fstack-usage -c "${f}" -o "${obj}"
done

SU_FILES=$(find "${BUILD_DIR}" -name "*.su" | sort)
if [ -z "${SU_FILES}" ]; then
    echo "ERROR: No .su stack usage files were generated." >&2
    exit 1
fi

OUT_FILE="${BUILD_DIR}/su_results.txt"
: > "${OUT_FILE}"
DYNAMIC_COUNT=0

for su in ${SU_FILES}; do
    while IFS=$'\t' read -r location size qual; do
        if [ -n "${location}" ] && [ -n "${size}" ]; then
            func=$(echo "${location}" | awk -F: '{print $NF}')
            file=$(echo "${location}" | awk -F: '{print $1}')
            printf "%-8d %-42s %-32s %s\n" "${size}" "${file}" "${func}()" "${qual}" >> "${OUT_FILE}"
            if [[ "${qual}" != "static" ]]; then
                echo "ERROR: Non-static/VLA stack allocation in ${file} ${func}(): ${size}B (${qual})" >&2
                DYNAMIC_COUNT=$((DYNAMIC_COUNT + 1))
            fi
        fi
    done < "${su}"
done

echo ""
echo "--- Top 15 Stack Frame Allocations ---"
printf "%-8s %-42s %-32s %s\n" "Bytes" "File" "Function" "Type"
echo "--------------------------------------------------------------------------------------------------"
sort -nr -k1 "${OUT_FILE}" | sed -n '1,15p'

OVER_BUDGET=$(awk -v limit="${STACK_THRESHOLD_BYTES}" -v ecc_limit="${ECC_STACK_THRESHOLD_BYTES}" '
    {
        cap = ($2 ~ /\/(syn_ed25519|syn_p256|syn_x25519)\.c$/) ? ecc_limit : limit;
        if ($1 > cap) c++;
    }
    END { print c+0 }
' "${OUT_FILE}")
if [ "${OVER_BUDGET}" -gt 0 ]; then
    echo ""
    echo "ERROR: Functions exceeding stack frame budget (${STACK_THRESHOLD_BYTES}B general / ${ECC_STACK_THRESHOLD_BYTES}B ECC):" >&2
    awk -v limit="${STACK_THRESHOLD_BYTES}" -v ecc_limit="${ECC_STACK_THRESHOLD_BYTES}" '
        {
            cap = ($2 ~ /\/(syn_ed25519|syn_p256|syn_x25519)\.c$/) ? ecc_limit : limit;
            if ($1 > cap) print "  " $0 " (limit " cap "B)";
        }
    ' "${OUT_FILE}" >&2
fi

rm -rf "${BUILD_DIR}"

echo ""
if [ "${OVER_BUDGET}" -gt 0 ] || [ "${DYNAMIC_COUNT}" -gt 0 ]; then
    echo "=== Stack Usage Scan FAILED: ${OVER_BUDGET} over budget, ${DYNAMIC_COUNT} dynamic frames ===" >&2
    exit 1
fi

echo "=== Stack Usage Scan PASS: All functions static and within ${STACK_THRESHOLD_BYTES}B frame budget (ECC <= ${ECC_STACK_THRESHOLD_BYTES}B) ==="
