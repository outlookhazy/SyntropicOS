#!/usr/bin/env bash
# Build and run the official W3C WebAssembly Spec Test Suite against syn_wasm
# under AddressSanitizer + UndefinedBehaviorSanitizer.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/wasm_spec"
SPEC10_DIR="${WASM_SPEC10_DIR:-${BUILD_DIR}/spec-wg-1.0}"
SPEC20_DIR="${WASM_SPEC20_DIR:-${BUILD_DIR}/spec-wg-2.0}"
WORK_DIR="${BUILD_DIR}/generated"
RUNNER_BIN="${BUILD_DIR}/spec_runner"

mkdir -p "${BUILD_DIR}"

if [ ! -d "${SPEC10_DIR}/test/core" ]; then
    echo "[wasm-spec] Cloning official W3C WebAssembly/spec @ wg-1.0..."
    rm -rf "${SPEC10_DIR}"
    git clone --depth 1 --branch wg-1.0 https://github.com/WebAssembly/spec.git "${SPEC10_DIR}"
fi

if [ ! -d "${SPEC20_DIR}/test/core" ]; then
    echo "[wasm-spec] Cloning official W3C WebAssembly/spec @ wg-2.0..."
    rm -rf "${SPEC20_DIR}"
    git clone --depth 1 --branch wg-2.0 https://github.com/WebAssembly/spec.git "${SPEC20_DIR}"
fi

echo "[wasm-spec] Compiling spec_runner + syn_wasm.c (ASan + UBSan)..."
CC_BIN="${CC:-clang}"
"${CC_BIN}" -std=c11 -O2 -g \
    -Wall -Wextra -Werror \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I"${ROOT_DIR}" \
    -I"${ROOT_DIR}/src" \
    -DSYN_WASM_MAX_FUNCTIONS=1024 \
    -DSYN_WASM_MAX_TYPES=1024 \
    -DSYN_WASM_MAX_EXPORTS=1024 \
    -DSYN_WASM_MAX_STACK=2048 \
    -DSYN_WASM_MAX_LOCALS=4096 \
    -DSYN_WASM_MAX_CALL_DEPTH=512 \
    -DSYN_WASM_MAX_LABELS=1024 \
    -DSYN_WASM_MAX_GLOBALS=256 \
    -DSYN_WASM_MAX_HOST_FUNCS=128 \
    -DSYN_WASM_MAX_HOST_ARGS=255 \
    -DSYN_WASM_MAX_TABLE_ELEMS=2048 \
    -DSYN_WASM_MAX_DATA_SEGS=128 \
    "${ROOT_DIR}/tests/wasm_spec/spec_runner.c" \
    "${ROOT_DIR}/src/syntropic/vm/syn_wasm.c" \
    -lm -o "${RUNNER_BIN}"

rm -rf "${WORK_DIR}"
mkdir -p "${WORK_DIR}"

export ASAN_OPTIONS="detect_leaks=1:halt_on_error=1"
export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1"

python3 "${ROOT_DIR}/tests/wasm_spec/run_wasm_spec.py" \
    --runner "${RUNNER_BIN}" \
    --spec10 "${SPEC10_DIR}" \
    --spec20 "${SPEC20_DIR}" \
    --workdir "${WORK_DIR}" \
    --filter "${FILTER:-}"
