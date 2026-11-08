#!/usr/bin/env bash
set -euo pipefail

# Script to cross-compile SyntropicOS for ARM Cortex-M4 and execute in QEMU
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
QEMU_DIR="${ROOT_DIR}/tests/emulation"

cd "${ROOT_DIR}"

echo "=== Cross-compiling SyntropicOS for ARM Cortex-M4 (bare-metal) ==="

JOBS=${JOBS:-$(nproc 2>/dev/null || echo 4)}
make -j"${JOBS}" -f tests/Makefile.unity qemu-build BUILD_DIR=build

bash "${QEMU_DIR}/measure_arm_size.sh"

echo "=== Executing in QEMU ARM Cortex-M4 (mps2-an386) ==="
if command -v qemu-system-arm >/dev/null 2>&1; then
    stdbuf -o0 -e0 qemu-system-arm -machine mps2-an386 -nographic -monitor null -serial stdio -semihosting-config enable=on,target=native -kernel build/test_cortexm4.elf
    echo "=== QEMU ARM Execution Complete ==="
else
    echo "Notice: qemu-system-arm not found on host. Run via container: make -C tools/containers container-qemu"
fi
