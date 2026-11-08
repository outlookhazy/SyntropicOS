# SyntropicOS Bare-Metal STM32 Integrated Multi-Task Protothreads (`SYN_PT`) Example

Demonstrates four concurrent stackless coroutine protothreads (`SYN_PT`) coordinating hardware peripherals and shared global resources in C99 on bare-metal STM32 hardware with zero per-task RAM stack overhead.

## Architecture & Protothread Layout

```
                  +----------------------------------------------+
                  |           SyntropicOS Super-Loop             |
                  +-------+----------+-----------+---------------+
                          |          |           |
          +---------------+          |           +---------------+
          |                          |                           |
          v                          v                           v
  +---------------+          +---------------+           +---------------+
  |  task_button  |          |  task_usart   |           |   task_led    |
  |  (Button PT)  |          |   (USART PT)  |           |    (LED PT)   |
  +-------+-------+          +-------+-------+           +-------+-------+
          |                          |                           ^
          | Post Gesture Event       | Set Mode Command          | Drive Pattern
          v                          v                           |
  +--------------------------------------------------------------+
  |              App_GlobalResources (IPC State)                 |
  |  - current_mode  - button_press_count  - active_errors       |
  +--------------------------------------------------------------+
                                 ^
                                 | Process Event
                         +-------+-------+
                         |   task_ipc    |
                         |   (IPC PT)    |
                         +---------------+
```

## Concurrent Protothread Tasks

1. **`task_button` (Button Task)**: Non-blocking sampling of user push button (`PC13`). Evaluates hold duration to post `EVENT_BUTTON_SHORT_PRESS` and `EVENT_BUTTON_LONG_PRESS` gesture events.
2. **`task_usart` (USART CLI Task)**: Ingests console serial input, line buffers characters, and dispatches zero-malloc CLI commands (`status`, `mode <idle|active|alert|config>`).
3. **`task_ipc` (Global Resource Coordinator)**: Inter-task coordinator monitoring the shared thread-safe `App_GlobalResources` struct. Translates button gestures and CLI commands into system mode state transitions.
4. **`task_led` (Status LED Task)**: Dynamic LED pattern driver (`PA5`). Adjusts flash frequency (`1Hz` slow blink, `4Hz` active blink, `10Hz` rapid alert, or solid ON config) based on `current_mode`.

## Under the Hood: Protothread Macro Expansion (Duff's Device)

Protothreads save execution position using a 2-byte line continuation variable (`lc`) and C `switch`/`case` jump tables:

```c
/* High-Level C Code */
SYN_PT_Status task_led_func(SYN_PT *pt) {
    PT_BEGIN(pt);
    while (1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        PT_WAIT_UNTIL(pt, (syn_port_get_tick_ms() - last_tick) >= 500);
        last_tick = syn_port_get_tick_ms();
    }
    PT_END(pt);
}

/* Preprocessor Macro Expansion */
SYN_PT_Status task_led_func(SYN_PT *pt) {
    char _pt_yield_flag = 1;
    switch (pt->lc) {
        case 0:
            while (1) {
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

                /* PT_WAIT_UNTIL expansion: saves __LINE__ (236) into pt->lc before returning.
                 * On next call, switch(pt->lc) jumps directly to 'case 236:' inside the while loop! */
                pt->lc = 236; case 236:
                if (!((syn_port_get_tick_ms() - last_tick) >= 500)) {
                    return PT_IDLE;
                }

                last_tick = syn_port_get_tick_ms();
            }
    }
    _pt_yield_flag = 0;
    pt->lc = 0;
    return PT_ENDED;
}
```

## Hardware Wiring

```
+-------------------+                    +-------------------+
|  STM32 NUCLEO-F4  |                    |  USB-to-UART /    |
|   Microcontroller |                    |  Serial Console   |
|                   |                    |                   |
|   USART2_TX (PA2) --------------------> RXD               |
|   USART2_RX (PA3) <-------------------- TXD               |
|   USER_LED  (PA5) --------------------> Green LED         |
|   USER_BUTTON(PC13)<------------------- Push Button (Active LOW)
|               GND --------------------> GND               |
+-------------------+                    +-------------------+
```

## Protocol & Serial Port Configuration

- **Baud Rate**: 115,200 baud
- **UART Frame**: 8 Data Bits, No Parity, 1 Stop Bit (8N1)
- **Line Termination**: `\r\n`

## Interactive CLI Commands

- `status` — Dump MCU uptime, system mode, button count, CLI commands, and active error counters.
- `mode <idle|active|alert|config>` — Explicitly switch system mode.
- `help` — Print command help catalog.
