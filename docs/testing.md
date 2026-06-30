# Testing

SyntropicOS has two test tiers to balance iteration speed with hardware-level fidelity.

## Tier 1: Host-Side Unity Tests (Fast Iteration)

A comprehensive host-compiled test suite using the **Unity** test framework. Tests logic, scheduler, data structures, DSP modules, and more using a mock port layer. Compiles and runs in under a second on your host PC.

```bash
make -f tests/Makefile.unity test
```

## Tier 2: Target-Side Renode Simulation (Hardware-Level Fidelity)

Verifies real peripheral drivers (SysTick, GPIO, UART, EXTI) and networking protocols on a simulated **STM32F4 Discovery** board using **Renode** and **Robot Framework**.

**Prerequisites:** `arm-none-eabi-gcc` and Renode installed.

### Single-Node Peripheral Verification

Validates SysTick interrupts, GPIO outputs/inputs, COBS, ring buffer, and CRC logic directly on the emulated Cortex-M4 target:

```bash
make -f port/stm32f4/Makefile.renode renode-test
```

### Multi-Node Network Stack Verification

Spawns two simulated STM32F4 nodes connected via a virtual UART hub, checking node-to-node routing, COBS transport framing, and heartbeat discovery end-to-end:

```bash
make -f port/stm32f4/Makefile.renode renode-multinode-test
```

!!! tip
    Both Renode test targets run with `--show-log` by default, streaming full UART debug messages and emulator events to the console in real time. If a test fails, you'll immediately see exactly what each simulated node output before the error. To override flags: `make -f port/stm32f4/Makefile.renode renode-test RENODE_FLAGS=""`.

## Visual Validation & Simulation (GUI)

SyntropicOS provides host-compiled tools to validate canvas drawing and IMGUI layouts without hardware.

### Static Mockup Capture

Renders a 16bpp (RGB565) dashboard mockup to a raw framebuffer file (`tests/framebuffer.bin`):

```bash
gcc -std=c99 -pedantic -Wall -Wextra -Werror -I. -Itests \
    tests/gen_screenshot.c src/syntropic/display/syn_canvas.c \
    src/syntropic/ui/syn_imgui.c tests/mocks/mock_port.c \
    -o gen_screenshot
./gen_screenshot
```

The output is a raw 128×64 RGB565 framebuffer (16,384 bytes). Convert to PNG with any raw-to-image tool or a short Python script using Pillow.

### Animated Live Simulation

Renders a 100-frame interactive GUI timeline (encoder ticks, button presses, touch drags, modal dialogs) to a series of raw framebuffer files:

```bash
gcc -std=c99 -pedantic -Wall -Wextra -Werror -I. -Itests \
    tests/gen_gif.c src/syntropic/display/syn_canvas.c \
    src/syntropic/ui/syn_imgui.c tests/mocks/mock_port.c \
    -o gen_gif
./gen_gif
```

## Advanced Diagnostics & Verification

SyntropicOS supports several advanced compiler-level verification tools to guarantee code quality, memory safety, and portability.

### 1. Code Coverage (Line & Branch Coverage)

You can clean, rebuild, run the tests, and generate both console summaries and visual HTML reports with full branch coverage enabled:

```bash
make -f tests/Makefile.unity clean coverage
```

The output report will detail line, function, and branch coverage. The HTML report will be generated under `coverage_html/index.html`.

### 2. Runtime Sanitizers (ASan & UBSan)

AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan) compile instrumentation checks into the binary to catch memory corruption, overflows, null pointer dereferences, and signed integer shifts at runtime:

```bash
make -f tests/Makefile.unity clean test CFLAGS="-std=gnu99 -g -fsanitize=address,undefined -I. -Isrc -Itests -Itests/mocks"
```

Any runtime violation will cause the test suite to abort and print a detailed stack trace of the violation.

### 3. Protocol Fuzz Testing

The communication protocols (COBS and Modbus) have LLVM `libFuzzer` targets compiled via Clang:

```bash
# Fuzz COBS decoder
make -f tests/Makefile.fuzz fuzz-cobs

# Fuzz Modbus frame parser
make -f tests/Makefile.fuzz fuzz-modbus
```

This mutates packet data randomly at high speed to find inputs that cause memory corruptions or decoder hangs.

### 4. Stack Usage Analysis

Since stack overflow is a primary cause of micro-controller crashes, GCC can statically calculate the stack usage of every function:

```bash
# Compile and generate .su files
make -f tests/Makefile.unity clean test CFLAGS="-std=gnu99 -fstack-usage -I. -Isrc -Itests -Itests/mocks"
```

This generates a `.su` text file for each compiled source file. You can find the heaviest stack frame allocations in production by sorting the outputs:

```bash
find . -name "*.su" -exec cat {} + | grep "src/syntropic/" | sort -n -k 2 -r | head -n 20
```


