# Testing & Verification Guide

SyntropicOS features a multi-tiered test and quality assurance architecture ranging from ultra-fast host-side unit testing to containerized 3rd-party daemon integration, QEMU bare-metal CPU emulation, protocol fuzzing, sanitizers, and static analysis.

## Containerized Testing Workflow

All testing targets are fully containerized using **Podman** / **Docker**, providing isolated, repeatable execution environments across all developer platforms and CI/CD pipelines.

| Make Target | Containerized Target | Description |
| :--- | :--- | :--- |
| `make test` | `make container-test` | Runs host Unity unit test suite (2000+ unit tests) |
| `make san` | `make container-san` | Executes unit tests with AddressSanitizer (ASan) & UBSan |
| `make qemu` | `make container-qemu` | Bare-metal QEMU Cortex-M4 startup & RAM verification |
| `make fuzz` | `make container-fuzz` | Runs LLVM `libFuzzer` protocol targets (COBS, Modbus, MQTT, HTTP, IR) |
| `make cov` | `make container-cov` | Generates LCOV line and branch code coverage summaries (`build/cov/coverage_src.info`) |
| `make static` | `make container-static` | Performs Cppcheck and Clang `scan-build` static analysis |
| `make dox` | `make container-dox` | Builds Doxygen API documentation and verifies zero warnings |
| `make integration` | `make container-integration` | Executes E2E tests against genuine 3rd-party container daemons |

---

## Tier 1: Unity Unit Tests (`tests/unit/`)

Fast, host-compiled unit test suite powered by the **Unity** framework. Tests schedulers, data structures, protothreads, DSP, fixed-point math, and protocol codecs against weak port mocks (`mock_port.c`).

```bash
make test
# Or containerized:
make container-test
```

---

## Tier 2: QEMU Bare-Metal Emulation (`tests/qemu/`)

Verifies ARM Cortex-M4 bare-metal startup, vector tables, link-time memory sections, and hardware initialization inside a **QEMU `lm3s6965evb`** system emulator.

```bash
make qemu
# Or containerized:
make container-qemu
```

---

## Tier 3: 3rd-Party Production Daemon Integration (`tests/integration/`)

Validates SyntropicOS network and protocol client stacks by connecting directly to **genuine production-grade container daemons** orchestrated via `docker-compose` / `podman-compose` (no synthetic stubs):

| Protocol Component | Service Daemon | Image / Context | Test Driver |
| :--- | :--- | :--- | :--- |
| **MQTT Client** (`syn_mqtt`) | Eclipse Mosquitto | `docker.io/library/eclipse-mosquitto:latest` | `test_mqtt_integration.c` |
| **SNTP Time Sync** (`syn_sntp`) | Chrony NTP Daemon | `Debian (chrony)` | `test_sntp_integration.c` |
| **HTTP REST** (`syn_http`) | Nginx Web Server | `docker.io/library/nginx:alpine` | `test_http_integration.c` |
| **WebSockets** (`syn_websocket`) | Node.js `ws` Echo Server | `Node 18 Alpine (ws)` | `test_ws_integration.c` |
| **DNS Resolver** (`syn_dns`) | CoreDNS Name Server | `docker.io/coredns/coredns:latest` | `test_dns_integration.c` |
| **CANopen / CiA 402** (`syn_canopen`) | SocketCAN Daemon | `Debian (can-utils/vcan0)` | `test_can_integration.c` |
| **WireGuard VPN** (`syn_wg`) | Linux WireGuard Server | `Debian (wireguard-tools)` | `test_wg_integration.c` |
| **Modbus TCP** (`syn_modbus`) | Python Modbus Daemon | `Python 3.11 Slim` | `test_modbus_integration.c` |

```bash
make integration
# Or containerized:
make container-integration
```

---

## Tier 4: Sanitizers, Coverage, Fuzzing & Static Analysis

### 1. Runtime Sanitizers (ASan & UBSan)
Instrument binaries with AddressSanitizer and UndefinedBehaviorSanitizer to catch memory leaks, out-of-bound pointer accesses, and uninitialized reads:

```bash
make san
```

### 2. Code & Branch Coverage
Generates detailed line and branch coverage summaries using `lcov` (`build/cov/coverage_src.info`):

```bash
make cov
```

### 3. Protocol Fuzzing
Uses LLVM `libFuzzer` to mutate protocol packet buffers at high speed:

```bash
make fuzz
```

### 4. Static Analysis
Runs Cppcheck and Clang `scan-build` over all `src/syntropic/` sources:

```bash
make static
```

### 5. Doxygen Documentation Verification
Builds API documentation and enforces zero warning tolerance:

```bash
make dox
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
