# Arduino Guide

This guide walks you through using SyntropicOS with the Arduino IDE.
No prior RTOS experience required.

## What is SyntropicOS?

SyntropicOS is a lightweight operating system for microcontrollers.
It lets you run multiple tasks at the same time — blinking an LED, reading
sensors, handling serial commands — without blocking each other.

Think of it like running multiple `loop()` functions simultaneously.

## Installation

### Option A: Git Clone (recommended)

Open a terminal and run:

```bash
cd ~/Arduino/libraries
git clone https://github.com/outlookhazy/SyntropicOS.git
```

To update later: `cd ~/Arduino/libraries/SyntropicOS && git pull`

### Option B: Download ZIP

1. Go to [github.com/outlookhazy/SyntropicOS](https://github.com/outlookhazy/SyntropicOS)
2. Click the green **Code** button → **Download ZIP**
3. In Arduino IDE: **Sketch → Include Library → Add .ZIP Library**
4. Select the downloaded ZIP file

### Verify Installation

After installing, open **File → Examples → SyntropicOS**. You should see:

- **Blink** — LED blink using the scheduler
- **SerialCLI** — Command-line interface over serial
- **SensorLogger** — ADC reading with digital filtering
- **MotorFSM** — State machine controlling a motor
- **PicoBlink** — Raspberry Pi Pico bring-up blink and diagnostics (RP2040 only)
- **PicoDualCore** — Raspberry Pi Pico AMP dual-core mailbox & CLI demo (RP2040 only)

If you see these, you're ready to go.

## Your First Sketch

Open **File → Examples → SyntropicOS → Blink** and upload it. The on-board
LED will start blinking.

Here's what the code does, line by line:

```cpp
#include <SyntropicOS.h>                 // SyntropicOS library
#include <syntropic/sched/syn_sched.h>  // Scheduler (runs your tasks)
#include <syntropic/output/syn_led.h>   // LED driver (blink patterns)

static SYN_Sched sched;    // The scheduler — manages all your tasks
static SYN_Task  tasks[1];  // Array of tasks (we have just one)
static SYN_LED   led;       // LED state — tracks the blink pattern
```

**The task function** — this is like a mini `loop()` that the scheduler calls
repeatedly:

```cpp
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);            // Required at the start of every task
    for (;;) {               // Loop forever (the scheduler handles timing)
        syn_led_update(&led); // Update the LED pattern
        PT_TASK_DELAY_MS(pt, task, 50);  // Wait 50ms, let other tasks run
    }
    PT_END(pt);              // Required at the end of every task
}
```

**Setup and loop** — familiar Arduino territory:

```cpp
void setup()
{
    // Initialize the LED on the built-in pin, blinking 500ms on / 500ms off
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    // Create the task and start the scheduler
    syn_task_create(&tasks[0], "blink", blink_task, 0, NULL);
    syn_sched_init(&sched, tasks, 1);
}

void loop()
{
    syn_sched_run(&sched);   // Run one ready task
}
```

## Key Concepts

### The Problem with `delay()`

In a normal Arduino sketch, `delay(500)` freezes **everything** for 500ms.
Nothing else can run — no serial reading, no sensor polling, nothing.

```cpp
// ❌ BAD: delay() blocks everything
void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);                        // <-- Nothing else can happen for 500ms!
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    int sensor = analogRead(A0);       // Only runs once per second
}
```

SyntropicOS solves this. Each **task** can pause itself without blocking
the others.

### Tasks — Multiple `loop()` Functions

Think of a task as its own mini `loop()`. The scheduler calls each task in
turn, and each task can **pause** at any point to let the others run.

Here's the basic pattern:

```cpp
static SYN_PT_Status my_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);             // Always start with this

    for (;;) {                // Your task's main loop
        // Do something...
        PT_TASK_DELAY_MS(pt, task, 500);   // Pause 500ms (others keep running!)
        // Do something else...
        PT_YIELD(pt);                      // Let other tasks run, resume next cycle
    }

    PT_END(pt);               // Always end with this
}
```

**That's it.** Three rules:

1. Start with `PT_BEGIN(pt);`
2. End with `PT_END(pt);`
3. Use `PT_TASK_DELAY_MS()` or `PT_YIELD()` to pause

### How It Works (Without the Jargon)

When your task hits `PT_TASK_DELAY_MS(pt, task, 500)`, it **bookmarks**
where it stopped and returns to the scheduler. The scheduler runs other
tasks. After 500ms, the scheduler calls your task again, and it
**resumes from the bookmark** — right where it left off.

You don't need to understand how the bookmarking works. Just follow the
pattern.

### ⚠️ Two Rules to Remember

1. **Use `static` for variables that need to survive across delays.**
   Regular local variables are lost when a task pauses. Mark them `static`.

2. **Don't use `switch` statements inside a task body.**
   Move them into a helper function, or use `if`/`else if` instead.

```cpp
// ✅ Use 'static' for persistent variables
static int count = 0;
count++;
PT_TASK_DELAY_MS(pt, task, 1000);  // 'count' survives

// ✅ Move switch logic into a helper
static void handle_mode(int m) { switch(m) { /* ... */ } }
handle_mode(mode);  // Call from task body
```

For detailed explanations, examples, and advanced patterns, see the
full [Cooperative Multitasking](modules/multitasking.md) documentation.

### Multiple Tasks

You can run as many tasks as you want:

```cpp
static SYN_Task tasks[3];  // Room for 3 tasks

void setup() {
    syn_task_create(&tasks[0], "blink",  blink_task,  1, NULL);
    syn_task_create(&tasks[1], "serial", serial_task, 0, NULL);  // Higher priority (0 = highest)
    syn_task_create(&tasks[2], "sensor", sensor_task, 1, NULL);
    syn_sched_init(&sched, tasks, 3);
}
```

All three run concurrently. No blocking. No threads. No heap allocation.

### Priority

The second-to-last argument in `syn_task_create()` is the priority.
Lower numbers = higher priority (0 is the highest priority). Tasks at the same priority level take turns equally.

### CLI (Command Line Interface)

SyntropicOS includes a serial command parser. See the **SerialCLI** example.
Open the Serial Monitor at 115200 baud and type `help` to see available
commands.

### LED Patterns

The LED driver supports more than just on/off:

```cpp
syn_led_on(&led);                           // Solid on
syn_led_off(&led);                          // Off
syn_led_blink(&led, 200, 200);             // Fast blink
syn_led_flash(&led, 100, 100, 5);          // Flash 5 times, then stop
syn_led_pattern(&led, "... --- ... |", 150); // SOS in Morse code
```

### Filters and Sensors

See the **SensorLogger** example for how to read analog pins with
noise filtering and running statistics (min, max, mean, variance).

### State Machines (FSM)

See the **MotorFSM** example for a table-driven state machine that
transitions between states on commands and events.

## Disabling Unused Modules

By default, all SyntropicOS modules are available. On small boards like the
Arduino Uno (32 KB flash), the linker automatically strips any code you don't
use — so you generally don't need to do anything.

If you want explicit control, create a file called `syn_config.h` in your
sketch folder. Copy the template from the library:

```
SyntropicOS/src/syntropic/syn_config_template.h
```

Rename it to `syn_config.h`, put it next to your `.ino` file, and set any
module you don't need to `0`:

```c
#define SYN_USE_MQTT    0   // Don't need MQTT
#define SYN_USE_FFT     0   // Don't need FFT
#define SYN_USE_CAN     0   // Don't need CAN bus
```

## Supported Boards

SyntropicOS works on any board supported by the Arduino IDE:

| Board | Status | Notes |
|-------|--------|-------|
| Arduino Uno / Nano / Mega | ✅ | 32 KB flash — use `syn_config.h` to disable heavy modules |
| ESP32 / ESP32-S3 | ✅ | Plenty of resources, everything enabled |
| STM32 (STM32duino) | ✅ | Blue Pill, Nucleo, etc. |
| RP2040 (Arduino-Pico) | ✅ | Raspberry Pi Pico |
| SAMD21 / SAMD51 | ✅ | Adafruit Feather M0/M4, Arduino Zero |
| Teensy 3.x / 4.x | ✅ | Full support |

## Troubleshooting

**"syn_assert_failed" undefined** — Add this to your sketch:
```cpp
extern "C" void syn_assert_failed(const char *file, int line) {
    Serial.print("ASSERT: "); Serial.print(file);
    Serial.print(":"); Serial.println(line);
    for (;;);  // Halt
}
```

**Sketch too large for Uno** — Create a `syn_config.h` (see above) and
disable modules you don't use. The biggest are `SYN_USE_FFT`, `SYN_USE_CANVAS`,
`SYN_USE_IMGUI`, and the network modules.

**Nothing happens after upload** — Make sure your `loop()` calls
`syn_sched_run(&sched)`. Without it, no tasks will execute.
