#!/usr/bin/env bash
set -euo pipefail

# Script to run Doxygen documentation coverage analysis
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

cd "${ROOT_DIR}"

echo "=== Running Doxygen Documentation Coverage Analysis ==="
if ! command -v doxygen >/dev/null 2>&1; then
    echo "ERROR: doxygen not found. Run via container: make -C tools/containers container-dox" >&2
    exit 1
fi

mkdir -p build/doxygen
rm -f build/doxygen/doxygen_warnings.txt doxygen_warnings.txt
doxygen Doxyfile

if [ -f build/doxygen/doxygen_warnings.txt ]; then
    WARN_COUNT=$(wc -l < build/doxygen/doxygen_warnings.txt | tr -d ' ')
    echo "=== Doxygen Analysis Finished ==="
    echo "Documentation Warning Count: ${WARN_COUNT}"
    if [ "${WARN_COUNT}" -gt 0 ]; then
        echo "--- Doxygen Warnings (up to 25) ---"
        head -n 25 build/doxygen/doxygen_warnings.txt
        echo "-----------------------------------"
        echo "ERROR: Doxygen reported ${WARN_COUNT} warning(s)." >&2
        exit 1
    else
        echo "=== 100% Doxygen Documentation Coverage Clean ==="
    fi
else
    echo "=== 100% Doxygen Documentation Coverage Clean ==="
fi
