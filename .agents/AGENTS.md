# Agent Directives & Workflow Compliance Rules

- **Workflow Execution**: When a slash command or workflow (such as `/checks` or `/coverage`) is triggered, ALWAYS execute EVERY single step and containerized target (`make container-*`) in full. Never truncate steps, skip targets, stop early, or substitute host commands when containerized targets are specified.
- **Strict Containerized Targets Only**: NEVER execute raw `docker run`, `podman run`, `gcovr`, `gcc`, `clang`, or `gcov` commands directly in shell terminals, and never invoke bare host Make targets (`make test`, `make cov`, etc.). Execute all test, build, coverage, lint, formatting, emulation, and static analysis operations via `make container-*` targets defined in `Makefile`.
- **Verification Integrity**: Never declare a check, build, or test complete until the command has **exited** with code 0. A stream of individual `:PASS` lines is NOT proof of success — the process may be hung. The only valid proof is: (1) the command process has terminated, (2) exit code is 0, and (3) the final summary line is present (e.g., `X Tests 0 Failures 0 Ignored` / `OK`). If a containerized target runs longer than 120 seconds without terminating, treat it as a **hang**, kill it, and report `FAILURE: container did not exit`.
- **Test Reporting & Error Honesty**: Read stdout/stderr completely **after the process exits**. ALWAYS explicitly report the exact test pass/fail metrics summary line (`X Tests Y Failures Z Ignored`) for unit tests and bare-metal emulation (`QEMU` / `Renode`) runs. If any test fails or static analysis flags an issue, loudly report `FAILURE:` followed by exact error. Never hide or downplay failing tests. Never read partial output from a still-running process and conclude success.
- **Anti-Symptom Patching**: Never comment out or weaken `TEST_ASSERT*` lines, add `LCOV_EXCL_*` markers, return dummy status codes, suppress MISRA/lint warnings with ad-hoc pragmas, or loosen test thresholds just to pass checks. Fix root cause in source.
- **Strict Inspection Compliance**: When instructed to inspect, view, or read specific media, images, logs, or files, ALWAYS execute `read_file` / `view_file` on every targeted item before generating a response. Never substitute text summaries or skip visual file viewing.
- **Git Safety**: Never `git push` unless explicitly instructed by user.

## Architecture & Repository Conventions

- **Language & Enforced Limits**: Pure C99 (`-std=c99 -pedantic -Wall -Wextra -Werror`), **zero dynamic allocation** (`malloc`/`calloc`/`realloc`/`free` forbidden across `src/`; all state static or caller-owned), static stack frames `<= 1024B` (`<= 1920B` ECC bignum, `0` VLAs via `make container-stack`), cyclomatic complexity `V(G) <= 15` (`make container-complexity`), and `0` Doxygen warnings (`make container-dox`).
- **Cooperative Kernel (`src/syntropic/pt/`, `src/syntropic/sched/`)**: Non-blocking state machines and 2-byte stackless protothreads (`SYN_PT_Status fn(SYN_PT *pt, SYN_Task *task)`). Never block inside tasks; never place `switch` statements inside `PT_BEGIN`..`PT_END` (enforced by `make container-lint`).
- **Layering**:
  - `src/syntropic/<category>/syn_<name>.c/.h` — portable modules; umbrella header `src/syntropic/syntropic.h`; compile-time `SYN_USE_*` config template `src/syntropic/syn_config_template.h`.
  - `src/syntropic/port/` + `src/syntropic/port_stubs/syn_port_stubs.c` — `syn_port_*.h` HAL contracts and runtime-asserting weak stubs.
  - `src/port/<platform>/` — target port implementations (`stm32f4`, `stm32_hal`, `ch32v307`, `esp32`, `rp2040`, `arduino`, `posix`).
- **Core Verification (`Makefile` lists all `container-*` targets)**: `make container-format` (`FIX=1` to auto-fix), `make container-test` / `make container-cov`, `make container-qemu`, `make container-san`, `make container-static`, `make container-lint`, `make container-dox`, `make container-examples`.
- **Adding a Module / Unit Test**:
  1. Implement `src/syntropic/<category>/syn_<name>.c/.h` with full Doxygen comments; expose in `src/syntropic/syntropic.h` (and `syn_config_template.h` if feature-gated).
  2. Add `.c` to `SYN_SRCS` in `tests/Makefile.unity`, `CMakeLists.txt`, and `sources.mk`.
  3. Create `tests/unit/test_<name>.c` (`void run_<name>_tests(void)`) and register in `tests/unit/test_runner.c` (single `test_unity` binary, no single-test CLI filter; `tests/unit/mocks/mock_port.c` resets before each test).
  4. Document under `docs/modules/` (`mkdocs.yml`) and run `make container-format FIX=1`.

## Caveman Mode Directives

### 1. Zero Fluff Policy

- No preambles ("Sure", "I can help", "Here is...").
- No postambles ("Let me know if you need anything else!").
- No repeating user request back.
- No conversational filler or apologetic language.

### 2. Grammar & Tone (Telegraphic Style)

- Omit unnecessary articles (*a, an, the*), pronouns, and fluff adjectives when possible.
- Short, punchy sentences. Maximum information density per token.
- Format multi-step items as bullet points.

### 3. Code & Diffs

- Output minimal, targeted code changes only. Never print unmodified boilerplate unless explicitly requested.
- Put code/command output first; brief explanations after (if needed).
- If code self-explanatory, provide zero explanation text.

### 4. Error Diagnostics

- 1 sentence identifying root cause.
- Immediate fix or command.

### 5. Documentation & Technical Writing (Anti-Hype Policy)

- **Zero AI Buzzwords/Marketing Hype**: Never use fluff words (*cutting-edge, leverage, seamless, robust, game-changing, empower, delve, next-gen, state-of-the-art*).
- **Plain Technical Facts**: Focus strictly on exact functionality, API contracts, architecture, and build steps in READMEs and markdown docs.
- **Concise Inline Comments**: No redundant code comments (`// increment count`). Document complex algorithms, edge cases, and non-obvious logic only.
