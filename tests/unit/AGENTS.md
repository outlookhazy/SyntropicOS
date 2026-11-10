# Unit Test Suite Architecture & Constraints (`tests/unit/`)

Applies to all Unity test suites (`tests/unit/test_*.c`), runner (`test_runner.c`), and mocks (`mocks/`).

## Runner, Mocks & Build Targets

- **Single Binary & Automatic Reset**: All `test_*.c` suites compile into a single runner (`tests/unit/test_runner.c`). `setUp()` resets `tests/unit/mocks/mock_port.c` and global router/CLI/transport hooks before every `RUN_TEST(...)`.
- **Fast Inner Loop (`make container-unit`)**: Run `make container-unit FILTER=<substr>` (comma-separated case-insensitive match against `test_*.c` path or `test_*` function name, `<2s` warm incremental build in `build/unit`). Fails non-zero if `FILTER` matches `0` tests.
- **Full Gate (`make container-test`)**: Runs a clean rebuild with LCOV coverage across all tests — always run before declaring work complete.
- **Wasm Fixtures**: Binary `.wasm` test fixtures are compiled from `tests/wasm/*.c` into `$(BUILD_DIR)/wasm/*.wasm.h`.

## Authoring & Context Rules

- **Slice Reads on Large Suites**: `49` files in `tests/unit/` exceed `800` lines (`test_uds.c`, `test_sched.c`, `test_modbus.c`). Run `grep -n "^void test_"` (or `grep -n` for target APIs) first and read targeted `StartLine..EndLine` slices rather than paging multi-thousand-line files end-to-end.
- **Dual Host + Bare-Metal ARM Cortex-M4 Compatibility**: Every `test_*.c` file in `tests/Makefile.unity` compiles and runs on **both** 64-bit POSIX host and 32-bit bare-metal ARM Cortex-M4 (`make container-qemu` via `tests/emulation/startup_cortexm4.c`):
  - QEMU builds define `UNITY_EXCLUDE_FLOAT_PRINT`; avoid relying on formatted float print macros unsupported under `UNITY_EXCLUDE_FLOAT_PRINT`.
  - Keep large buffers `static` rather than stack-allocated so bare-metal Cortex-M4 stack limits (`qemu_arm.ld`) are respected.
  - Do not assume 64-bit `size_t`/`uintptr_t` or unaligned memory access (`-mno-unaligned-access` is enforced on ARM).
- **Anti-Symptom Rule**: Never weaken assertions, delete failing edge cases, or add coverage exclusions (`LCOV_EXCL_*`) to force green builds.
