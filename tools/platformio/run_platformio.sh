#!/usr/bin/env bash
set -euo pipefail

# Script to validate SyntropicOS PlatformIO library manifest and build inside a real PlatformIO project
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

echo "=== SyntropicOS PlatformIO Package Validation & Build Audit ==="

PIO_VENV="${ROOT_DIR}/build/pio-venv"
export PATH="${PIO_VENV}/bin:${PATH}"

if [ ! -x "${PIO_VENV}/bin/pio" ]; then
    echo "Installing PlatformIO CLI into cached venv..."
    python3 -m venv "${PIO_VENV}"
    "${PIO_VENV}/bin/pip" install --quiet platformio
fi

echo "=== 1. Validating library.json Manifest & Packaging ==="
mkdir -p build
pio pkg pack --output build/SyntropicOS-pio.tar.gz

echo "=== 2. Inspecting PlatformIO Package Contents ==="
tar -tzf build/SyntropicOS-pio.tar.gz > build/tar_contents.txt
head -n 25 build/tar_contents.txt

echo "=== 3. Building Real Sample PlatformIO Project ==="
cd tests/platformio/sample_project
pio run -e native

echo "=== 4. Executing PlatformIO Compiled Binary ==="
./.pio/build/native/program

echo "=== PlatformIO Real Project Integration Audit PASS ==="
