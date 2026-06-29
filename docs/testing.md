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
    tests/gen_screenshot.c syntropic/display/syn_canvas.c \
    syntropic/ui/syn_imgui.c tests/mocks/mock_port.c \
    -o gen_screenshot
./gen_screenshot
```

The output is a raw 128×64 RGB565 framebuffer (16,384 bytes). Convert to PNG with any raw-to-image tool or a short Python script using Pillow.

### Animated Live Simulation

Renders a 100-frame interactive GUI timeline (encoder ticks, button presses, touch drags, modal dialogs) to a series of raw framebuffer files:

```bash
gcc -std=c99 -pedantic -Wall -Wextra -Werror -I. -Itests \
    tests/gen_gif.c syntropic/display/syn_canvas.c \
    syntropic/ui/syn_imgui.c tests/mocks/mock_port.c \
    -o gen_gif
./gen_gif
```

