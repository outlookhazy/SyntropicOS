# Testing & Verification Guide

SyntropicOS features a multi-tiered test and quality assurance architecture ranging from ultra-fast host-side unit testing to containerized 3rd-party daemon integration, QEMU bare-metal CPU emulation, protocol fuzzing, sanitizers, and static analysis.

## Containerized Testing Workflow

All verification targets run inside an isolated **Podman** / **Docker** image (`syntropicos-test`) across developer hosts and CI/CD pipelines. Omit the `container-` prefix only when already inside the container or using an equivalent native toolchain.

| Containerized Target | In-Container / Native Target | Description |
| :--- | :--- | :--- |
| `make container-test` | `make test` | Runs Unity unit test suite & LCOV coverage |
| `make container-format` | `make format` | Checks C99 formatting via `clang-format` (`FIX=1` to apply) |
| `make container-lint` | `make lint` | Runs `clang-tidy` AST linter & protothread `switch` check |
| `make container-san` | `make san` | Executes unit tests under AddressSanitizer (ASan) & UBSan |
| `make container-qemu` | `make qemu` | Bare-metal QEMU ARM Cortex-M4 startup & test verification |
| `make container-renode` | `make renode` | Renode STM32F4 board-level peripheral emulation |
| `make container-examples` | `make examples` | Cross-compiles all examples & lean `syn_config.h` profiles |
| `make container-fuzz` | `make fuzz` | Runs LLVM `libFuzzer` protocol targets |
| `make container-cov` | `make cov` | Generates LCOV line and branch coverage summaries (`build/cov/coverage_src.info`) |
| `make container-static` | `make static` | Performs Cppcheck and Clang `scan-build` static analysis |
| `make container-dox` | `make dox` | Builds Doxygen API documentation and enforces zero warnings |
| `make container-integration` | `make integration` | Executes E2E tests against containerized 3rd-party service daemons |

---

## Tier 1: Unity Unit Tests (`tests/unit/`)

Fast, host-compiled unit test suite powered by the **Unity** framework. Tests schedulers, data structures, protothreads, DSP, fixed-point math, crypto, and protocol codecs against weak port mocks (`mock_port.c`).

```bash
make container-test
```

---

## Tier 2: Bare-Metal Hardware Emulation (`tests/emulation/`)

Verifies ARM Cortex-M4 bare-metal startup, vector tables, link-time memory sections, and peripheral drivers inside **QEMU (`lm3s6965evb`)** and **Renode (`STM32F4`)** system emulators.

```bash
make container-qemu
make container-renode
```

---

## Tier 3: 3rd-Party Daemon Integration (`tests/integration/`)

Validates SyntropicOS network and fieldbus stacks by connecting directly to containerized service daemons orchestrated via `docker-compose` / `podman-compose`:

| Protocol Component | Service Daemon | Image / Context | Test Driver |
| :--- | :--- | :--- | :--- |
| **MQTT Client** (`syn_mqtt`) | Eclipse Mosquitto | `docker.io/library/eclipse-mosquitto:latest` | `test_mqtt_integration.c` |
| **SNTP Time Sync** (`syn_sntp`) | Chrony NTP Daemon | `Debian (chrony)` | `test_sntp_integration.c` |
| **HTTP REST** (`syn_http`) | Nginx Web Server | `docker.io/library/nginx:alpine` | `test_http_integration.c` |
| **WebSockets** (`syn_websocket`) | Node.js `ws` Echo Server | `Node 18 Alpine (ws)` | `test_ws_integration.c` |
| **DNS Resolver** (`syn_dns`) | CoreDNS Name Server | `docker.io/coredns/coredns:latest` | `test_dns_integration.c` |
| **CAN / ISO-TP / UDS / CANopen** | SocketCAN Responder Daemons | `Debian (can-utils/vcan0)` | `test_can_integration.c` |
| **WireGuard VPN** (`syn_wg`) | Linux WireGuard Server | `Debian (wireguard-tools)` | `test_wg_integration.c` |
| **Modbus TCP** (`syn_modbus`) | Python Modbus Daemon | `Python 3.11 Slim` | `test_modbus_integration.c` |
| **EtherCAT Slave** (`syn_ethercat`) | SOEM Master & SOES Reference | `Debian (SOEM / SOES)` | `test_ecat_integration.c`, `test_soes_integration.c` |
| **OCPP 1.6J** (`syn_ocpp`) | WebSocket CSMS Server | `Node Alpine (ws)` | `test_ocpp_integration.c` |

```bash
make container-integration
```

---

## Tier 4: Sanitizers, Coverage, Fuzzing & Static Analysis

### 1. Runtime Sanitizers (ASan, UBSan, TSan, Valgrind)
Instrument binaries to catch out-of-bounds memory accesses, undefined behavior, data races, and uninitialized reads:

```bash
make container-san
make container-tsan
make container-valgrind
```

### 2. Code & Branch Coverage
Generates detailed line and branch coverage summaries using `lcov` (`build/cov/coverage_src.info` inside the container build cache):

```bash
make container-cov
```

### 3. Protocol Fuzzing
Uses LLVM `libFuzzer` to mutate protocol packet buffers at high speed:

```bash
make container-fuzz
```

### 4. Static Analysis, MISRA, Stack & Complexity Bounds
Runs Cppcheck, Clang `scan-build`, GCC `-fanalyzer`, MISRA C:2023 checks, static stack-frame verification, and cyclomatic complexity gates over `src/syntropic/`:

```bash
make container-static
make container-analyzer
make container-misra
make container-stack
make container-complexity
```

### 5. Doxygen Documentation Verification
Builds API documentation and enforces zero warning tolerance:

```bash
make container-dox
```

---

## Tier 5: Motor & Plant Simulation Harness (`tests/sim/`)

High-fidelity physics simulation harness modeling mass, static/coulomb/viscous friction, back-EMF, and encoder noise to verify PID and auto-tuning (`syn_autotune`) algorithms:

```bash
gcc -O2 -Isrc -o sim_harness tests/sim/sim_harness.c tests/sim/sim_plant.c \
    src/syntropic/motor/syn_motor_ctrl.c \
    src/syntropic/control/syn_autotune.c \
    src/syntropic/control/syn_pid.c \
    -lm
./sim_harness
```
