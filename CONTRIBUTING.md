# Contributing to SyntropicOS

Thank you for contributing to SyntropicOS. Because SyntropicOS targets bare-metal microcontrollers with as little as 2 KB of RAM alongside industrial and networked 32-bit systems, every module follows strict architectural, memory, and verification invariants. Reviewing these rules before writing code avoids rework during review.

---

## 1. Core Architectural Rules

Every contribution under `src/syntropic/` must uphold five non-negotiable invariants:

1. **Cooperative & Non-Blocking Execution**: Every driver, protocol engine, and service must execute a bounded unit of work and return immediately (`_poll()`, `_update()`, `_process()`, or a `SYN_PT` coroutine). Never busy-wait on hardware flags without a bounded timeout, sleep, or call blocking third-party APIs inside portable modules.
2. **100% Zero-Heap Allocation**: `malloc()`, `calloc()`, `realloc()`, `free()`, and variable-length arrays (VLAs / `alloca`) are forbidden across `src/`. All contexts and buffers are caller-owned or statically sized via compile-time macros (`src/syntropic/syn_config_template.h`).
3. **Portable C99**: Portable code must compile cleanly under `-std=c99 -pedantic -Wall -Wextra -Werror` across GCC, Clang, ARMCC (Keil), IAR, and C++ (`extern "C"` guards required in every header). Use compiler abstraction macros in `src/syntropic/common/syn_compiler.h` (`SYN_PACKED`, `SYN_WEAK`, `SYN_NORETURN`, `SYN_ALIGNED`) instead of raw `__attribute__` syntax.
4. **Strict HAL Isolation**: Portable modules under `src/syntropic/` may only access hardware or OS primitives through `src/syntropic/port/syn_port_*.h`. Never include vendor SDK headers (`stm32*.h`, `esp_*.h`, `Arduino.h`, POSIX `<sys/socket.h>`, etc.) outside `src/port/<platform>/`.
5. **Bounded Stack & Complexity**:
   - Static stack frame size: `<= 1024 B` per function (`<= 1920 B` for constant-time ECC bignum primitives), `0` dynamic/VLA frames (`make container-stack`).
   - Cyclomatic complexity: `V(G) <= 15` per function (`make container-complexity`). Split large state machines into static per-state helpers.

---

## 2. Protocol Scope & Standards Fidelity

SyntropicOS aims to support a broad catalog of embedded, industrial, automotive, and network protocols while keeping the core framework vendor-neutral:

- **Stick to Published Specifications**: Protocol modules (`src/syntropic/proto/`, `net/`, `ble/`) must implement official specifications (IETF RFCs, ISO, IEC, IEEE, SAE, CiA, Bluetooth SIG, ODVA, OASIS, ASHRAE, etc.). Cite the governing standard and section numbers in code and PRs when implementing state transitions, timers, or wire formats.
- **No Vendor-Specific Quirks in Core**: Avoid hardcoding single-vendor workarounds, non-standard framing deviations, or proprietary register maps inside `src/syntropic/`.
- **Keep Vendor Extensions Out-of-Tree**: Manufacturer- or OEM-specific protocol extensions (for example, OEM UDS DIDs/routines, user-defined Modbus function codes, CANopen manufacturer profile objects `0x2000–0x5FFF`, J1939 PropA/PropB payloads, OCPP `DataTransfer` schemas, or custom 128-bit BLE GATT profiles) should in most cases live in user application code or separate companion libraries layered on top of SyntropicOS, rather than inside `src/syntropic/`.
- **Expose Generic Extension Hooks Instead**: Where an official specification defines extension points or custom ranges, SyntropicOS should provide clean user callbacks or raw passthrough hooks so downstream code can implement vendor behavior without coupling it into the core OS.

---

## 3. Protothread (`SYN_PT`) Guidelines

Protothreads (`src/syntropic/pt/syn_pt.h`) implement 2-byte stackless coroutines via a `switch (pt->lc)` continuation jump table:

- **No `switch` inside `PT_BEGIN` .. `PT_END`**: Placing a bare `switch` statement inside a protothread body corrupts Duff's-device continuation cases (enforced by `tools/lint/check_pt_switch.py` in `make container-lint`). Use `if` / `else if` chains or move the `switch` into a helper function called between yields.
- **No stack-local persistence across yields**: Local non-`static` variables inside a protothread function lose their values across any yield/wait macro (`PT_PASS`, `PT_CONTINUE`, `PT_WAIT_UNTIL`, `PT_SPAWN`, `PT_TASK_DELAY_MS`, `PT_BLOCK_*`), and prologue-initialized locals re-run their initializer on resume (enforced via Clang AST/CFG dataflow by `tools/lint/check_pt_locals.py` in `make container-lint`). Store persistent task state in `static` variables or the caller-owned struct passed via `task->user_data`.
- **Explicit yield intent**:
  - Use `PT_PASS(pt)` / `PT_WAIT_UNTIL(pt, cond)` / `PT_TASK_DELAY_MS(pt, task, ms)` when the task has no immediate work remaining this round (`PT_IDLE`).
  - Use `PT_CONTINUE(pt)` or `PT_CONTINUE_IF_NEEDED(pt, task)` when slicing long work across cooperative steps while more work remains queued (`PT_MORE`).

---

## 4. Coding Style & API Conventions

- **Formatting**: Enforced by `.clang-format` (LLVM base, 4-space indentation, no tabs, 100-column limit, right-aligned pointers `uint8_t *buf`, function braces on new line, control-statement braces on same line).
  - Enable the local pre-commit check once after cloning: `make install-hooks`
  - Auto-format modified files: `make container-format FIX=1`
- **Naming**:
  - **Files**: `src/syntropic/<category>/syn_<module>.h` and `syn_<module>.c`.
  - **Types & Structs**: `SYN_PascalCase` (`SYN_Sched`, `SYN_Modbus`, `SYN_Status`).
  - **Macros & Enum Constants**: `SYN_UPPER_SNAKE_CASE` (`SYN_OK`, `SYN_PRIO_NORMAL`).
  - **Public Functions**: `syn_<module>_<action>()` with the module context pointer as the first parameter.
  - **Internal Helpers**: `static` file-scoped `lower_snake_case` functions.
- **Error Handling & Defensive Bounds**:
  - Validate all public pointer and buffer-size arguments (`NULL` checks, capacity/length bounds, integer overflow checks on wire lengths) before dereferencing.
  - Use `SYN_Status` (`src/syntropic/common/syn_defs.h`: `SYN_OK`, `SYN_ERROR`, `SYN_BUSY`, `SYN_TIMEOUT`, `SYN_INVALID_PARAM`, `SYN_NOT_IMPLEMENTED`, `SYN_NOT_FOUND`) unless a domain-specific return type (`bool`, `size_t`, `SYN_PT_Status`) is clearer.
- **Literals & Static Analysis**: Use uppercase literal suffixes (`0U`, `1UL`, `0.5F`) as configured in `.clang-tidy`. Do not suppress compiler, `clang-tidy`, or MISRA warnings with inline pragmas without clear technical justification.
- **Documentation**:
  - Every public file, struct, enum, macro, and function requires Doxygen comments (`@file`, `@brief`, `@ingroup`, `@param`, `@return`). `make container-dox` enforces **0 warnings**.
  - Inline code comments should explain non-obvious invariants, hardware errata, protocol spec sections, or edge cases — never restate what the C syntax already says.

---

## 5. Containerized Build & Verification

Podman or Docker is the only required host dependency. All compilers (`gcc`, `clang`, `arm-none-eabi-gcc`, `wasm32` `clang`), emulators (`QEMU`, `Renode`), sanitizers, and static analyzers run inside `tools/containers/Containerfile`.

### Core Pre-PR Checks

Run these containerized targets before opening a pull request:

```bash
make container-format       # Check clang-format compliance (use FIX=1 to format)
make container-test         # Build & run Unity unit test suite + LCOV coverage report
make container-qemu         # Cross-compile & run full Unity suite on bare-metal Cortex-M4 QEMU
make container-san          # Run unit test suite under AddressSanitizer + UBSan
make container-static       # Run Cppcheck + Clang scan-build static analysis
make container-lint         # Run clang-tidy AST checks + Protothread Switch & Local Linters
make container-stack        # Verify ARM static stack frames (<= 1024B) and 0 VLAs
make container-complexity   # Verify cyclomatic complexity V(G) <= 15
make container-dox          # Verify Doxygen API documentation (0 warnings)
make container-examples     # Cross-compile all examples/ and lean syn_config.h matrix
```

### Subsystem-Specific Checks

Run additional targets when modifying relevant subsystems:

```bash
make container-tsan         # Concurrency, lock-free structures, multicore/ISR primitives
make container-valgrind     # Parsers, buffers, and memory-sensitive state machines
make container-fuzz         # Wire protocol decoders & binary/text parsers (tests/fuzz/)
make container-renode       # STM32F4 bare-metal port or peripheral drivers
make container-misra        # Safety-critical control, automotive, or crypto modules
make container-integration  # Network/fieldbus stacks (MQTT, HTTP, WS, DNS, SNTP, WG, Modbus, CAN, UDS)
```

---

## 6. Checklists by Contribution Type

### Adding a New Portable Module

1. **Implementation**: Create `src/syntropic/<category>/syn_<name>.h` and `syn_<name>.c` (including the `syn_config.h` hook, `#ifdef __cplusplus` guards, and full Doxygen annotations).
2. **Umbrella & Configuration**: Add the header to `src/syntropic/syntropic.h`. If the module is optional or has tunable static buffer limits, document defaults in `src/syntropic/syn_config_template.h`.
3. **Build Manifests**: Add the `.c` source path to all three build lists so Make, CMake, PlatformIO, and external projects stay in sync:
   - `tests/Makefile.unity` (`SYN_SRCS`, plus `-DSYN_USE_*` flags if feature-gated)
   - `CMakeLists.txt`
   - `sources.mk`
4. **Unit Tests**: Create `tests/unit/test_<name>.c` exposing `void run_<name>_tests(void)` using Unity (`RUN_TEST(...)`), and register it in `tests/unit/test_runner.c`. Use `tests/unit/mocks/mock_port.c` (`mock_port_reset()` runs before every test) for hardware interactions. Test nominal flows, boundary conditions, malformed inputs, and `NULL`/invalid-parameter paths.
5. **Fuzzing (Protocol / Parser Modules)**: When adding a wire protocol or untrusted data parser, add or extend a `libFuzzer` harness under `tests/fuzz/`.
6. **Documentation & Examples**: Add user-facing documentation under `docs/modules/` (wired into `mkdocs.yml`) and, for major subsystems or drivers, a minimal self-contained example under `examples/`.

### Adding or Extending a Hardware Port

- **Implementing an existing port contract for a new MCU**: Add `src/port/<platform>/port_<platform>.c` implementing the headers in `src/syntropic/port/syn_port_*.h`, and document platform setup notes in `docs/porting-guide.md`.
- **Adding a new HAL port function**:
  1. Declare the portable contract in `src/syntropic/port/syn_port_<periph>.h` with Doxygen docs.
  2. Add a `SYN_WEAK` runtime-asserting fallback stub in `src/syntropic/port_stubs/syn_port_stubs.c`.
  3. Add deterministic mock support in `tests/unit/mocks/mock_port.c` (resetting state in `mock_port_reset()`).
  4. Implement across applicable reference ports under `src/port/`.

---

## 7. Commit Messages, Changelog & Pull Requests

- **Commit Format**: Use concise [Conventional Commits](https://www.conventionalcommits.org/) (`feat(ble): ...`, `fix(net): ...`, `docs: ...`, `test(sched): ...`, `refactor(proto): ...`, `chore: ...`). Keep commits atomic and reviewable; never commit build outputs or editor caches.
- **Changelog**: SyntropicOS uses [Calendar Versioning](https://calver.org/) (`YYYY.M.MINOR`) and [Keep a Changelog](https://keepachangelog.com/en/1.0.0/). Document user-visible features, bug fixes, and breaking API changes under `## [Unreleased]` in [CHANGELOG.md](CHANGELOG.md).
- **Pull Request Description**:
  - State the problem or specification reference (RFC, ISO, CiA, Bluetooth Core Spec, MCU reference manual section).
  - Note RAM/Flash impact (`make container-size`) or `syn_config.h` additions where applicable.
  - List the `make container-*` verification targets executed and their pass summaries.

---

## 8. License

By submitting a contribution to SyntropicOS, you agree that your contribution is licensed under the [Apache License, Version 2.0](LICENSE) (per Section 5, *Submission of Contributions*). Include third-party attribution only if incorporating permissively licensed reference tables or test vectors compatible with Apache-2.0.
