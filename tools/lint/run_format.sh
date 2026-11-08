#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

MODE="check"
if [[ "${FIX:-0}" == "1" || "${1:-}" == "--fix" || "${1:-}" == "FIX=1" ]]; then
    MODE="fix"
fi

echo "=== SyntropicOS Code Formatting Check (clang-format) ==="

if ! command -v clang-format >/dev/null 2>&1; then
    echo "ERROR: clang-format not found. Run via container: make container-format" >&2
    exit 1
fi

FILES=$(find src tests \( -name "*.c" -o -name "*.h" \) ! -path "*/unity/*" ! -name "stb_*.h")

if [[ "${MODE}" == "fix" ]]; then
    echo "Applying formatting fixes..."
    clang-format -i ${FILES}
    echo "=== Formatting Complete ==="
else
    echo "Checking code formatting..."
    UNFORMATTED=$(clang-format --dry-run --Werror ${FILES} 2>&1 || true)
    if [[ -n "${UNFORMATTED}" ]]; then
        echo "${UNFORMATTED}"
        echo ""
        echo "Notice: Formatting issues found. Run 'make format FIX=1' to auto-fix."
        exit 1
    else
        echo "=== Code Formatting PASS ==="
    fi
fi
