#!/usr/bin/env bash
set -euo pipefail

# Script to run Clang libFuzzer smoke tests on SyntropicOS protocol decoders
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

cd "${ROOT_DIR}"

if ! command -v clang >/dev/null 2>&1; then
    echo "ERROR: clang not found. Run via container: make -C tools/containers container-fuzz" >&2
    exit 1
fi

FUZZ_TIME="${FUZZ_TIME:-10}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
BUILD_DIR="${ROOT_DIR}/build/fuzz"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

COMMON_FLAGS=(
    -std=c99
    -I"${ROOT_DIR}"
    -I"${ROOT_DIR}/src"
    -I"${ROOT_DIR}/tests/unit/mocks"
    -DSYN_USE_MULTICORE=1
    -fsanitize=fuzzer,address,undefined
    -fno-sanitize-recover=all
)

compile_fuzzer() {
    local name="$1"
    shift
    clang "${COMMON_FLAGS[@]}" "$@" "${ROOT_DIR}/tests/unit/mocks/mock_port.c" -o "${BUILD_DIR}/fuzzer_${name}"
}

COMPILE_PIDS=()
spawn_compile() {
    compile_fuzzer "$@" &
    COMPILE_PIDS+=("$!")
}

echo "=== Compiling 11 libFuzzer Protocol Targets (ASan + UBSan) ==="
spawn_compile cbor \
    "${ROOT_DIR}/src/syntropic/util/syn_cbor_read.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_cbor.c"
spawn_compile nmea \
    "${ROOT_DIR}/src/syntropic/proto/syn_nmea.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_nmea.c"
spawn_compile coap \
    "${ROOT_DIR}/src/syntropic/net/syn_coap.c" \
    "${ROOT_DIR}/src/syntropic/net/syn_dtls.c" \
    "${ROOT_DIR}/src/syntropic/sched/syn_sched.c" \
    "${ROOT_DIR}/src/syntropic/sched/syn_mutex.c" \
    "${ROOT_DIR}/src/syntropic/sched/syn_event_flags.c" \
    "${ROOT_DIR}"/src/syntropic/crypto/*.c \
    "${ROOT_DIR}/src/syntropic/util/syn_backoff.c" \
    "${ROOT_DIR}/src/syntropic/util/syn_random.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_coap.c"
spawn_compile usb \
    "${ROOT_DIR}/src/syntropic/drivers/syn_usb.c" \
    "${ROOT_DIR}/src/syntropic/drivers/syn_usb_cdc.c" \
    "${ROOT_DIR}/src/syntropic/drivers/syn_usb_hid.c" \
    "${ROOT_DIR}/src/syntropic/drivers/syn_usb_host.c" \
    "${ROOT_DIR}/src/syntropic/drivers/syn_usb_host_cdc.c" \
    "${ROOT_DIR}/src/syntropic/drivers/syn_transport_usb_host_cdc.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_usb.c"
spawn_compile uds \
    "${ROOT_DIR}/src/syntropic/proto/syn_uds.c" \
    "${ROOT_DIR}/src/syntropic/crypto/syn_aes.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_uds.c"
spawn_compile isotp \
    "${ROOT_DIR}/src/syntropic/proto/syn_isotp.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_isotp.c"
spawn_compile cobs \
    "${ROOT_DIR}/src/syntropic/proto/syn_cobs.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_cobs.c"
spawn_compile dmx512 \
    "${ROOT_DIR}/src/syntropic/proto/syn_dmx512.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_dmx512.c"
spawn_compile json \
    "${ROOT_DIR}/src/syntropic/util/syn_json_read.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_json.c"
spawn_compile modbus \
    "${ROOT_DIR}/src/syntropic/proto/syn_modbus.c" \
    "${ROOT_DIR}/src/syntropic/proto/syn_modbus_tcp.c" \
    "${ROOT_DIR}/src/syntropic/util/syn_crc.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_modbus.c"
spawn_compile smbus \
    "${ROOT_DIR}/src/syntropic/proto/syn_smbus.c" \
    "${ROOT_DIR}/src/syntropic/util/syn_crc.c" \
    "${ROOT_DIR}/tests/fuzz/fuzz_smbus.c"

for cpid in "${COMPILE_PIDS[@]}"; do
    wait "${cpid}"
done

TARGETS=(cbor nmea coap usb uds isotp cobs dmx512 json modbus smbus)
PIDS=()

echo "=== Running 11 libFuzzer Targets (${FUZZ_TIME}s each across up to ${JOBS} workers) ==="
ACTIVE=0
for name in "${TARGETS[@]}"; do
    mkdir -p "${BUILD_DIR}/art_${name}"
    "${BUILD_DIR}/fuzzer_${name}" \
        -artifact_prefix="${BUILD_DIR}/art_${name}/" \
        -max_total_time="${FUZZ_TIME}" \
        >"${BUILD_DIR}/${name}.log" 2>&1 &
    PIDS+=("$!")
    ACTIVE=$((ACTIVE + 1))
    if [ "${ACTIVE}" -ge "${JOBS}" ]; then
        wait -n || true
        ACTIVE=$((ACTIVE - 1))
    fi
done

FAIL=0
for idx in "${!TARGETS[@]}"; do
    name="${TARGETS[$idx]}"
    pid="${PIDS[$idx]}"
    if wait "${pid}"; then
        summary=$(grep -E "Done [0-9]+ runs" "${BUILD_DIR}/${name}.log" | tail -n 1 || true)
        echo "  [PASS] fuzzer_${name}: ${summary:-completed (${FUZZ_TIME}s)}"
    else
        echo "  [FAIL] fuzzer_${name} crashed or failed:" >&2
        cat "${BUILD_DIR}/${name}.log" >&2
        FAIL=1
    fi
done

rm -rf "${BUILD_DIR}"

if [ "${FAIL}" -ne 0 ]; then
    echo "=== Protocol Fuzzing FAILED ===" >&2
    exit 1
fi

echo "=== Protocol Fuzzing Smoke Tests Complete (11/11 PASS) ==="
