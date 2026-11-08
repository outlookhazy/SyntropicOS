#!/usr/bin/env bash
set -euo pipefail

# Script to run test coverage analysis and generate HTML reports
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

cd "${ROOT_DIR}"

echo "=== Preparing Coverage Environment ==="
rm -rf build/cov
mkdir -p build/cov
rm -f *.gcov

echo "=== Running Test Suite with Coverage Instrumentation ==="
JOBS=${JOBS:-$(nproc 2>/dev/null || echo 4)}
make -j"${JOBS}" -f tests/Makefile.unity test-cov BUILD_DIR=build/cov

echo "=== Coverage Summary ==="
if command -v lcov >/dev/null 2>&1; then
    lcov --capture --rc branch_coverage=1 --ignore-errors inconsistent,path --directory build/cov --output-file build/cov/coverage.info --quiet
    lcov --extract build/cov/coverage.info "*/src/*" --rc branch_coverage=1 --ignore-errors inconsistent,path --output-file build/cov/coverage_src.info --quiet
    lcov --summary build/cov/coverage_src.info --rc branch_coverage=1 --ignore-errors inconsistent
    python3 tools/coverage/analyze_coverage.py build/cov/coverage_src.info
elif command -v gcovr >/dev/null 2>&1; then
    gcovr -r . --object-directory build/cov --filter "src/.*" --txt
else
    echo "Notice: Neither lcov nor gcovr found. Run via container: make -C tools/containers container-cov"
fi
rm -f *.gcov
