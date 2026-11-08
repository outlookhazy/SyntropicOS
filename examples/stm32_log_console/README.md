# STM32 Severity-Filtered Logging Example (`syn_log`)

This example demonstrates how to integrate SyntropicOS **`syn_log`** into an STM32 hardware application for tagged, timestamped, severity-filtered logging and binary memory hex dumps over UART Serial.

---

## Hardware Configuration & Setup

Connect the STM32 UART pins to your PC via a **USB-to-TTL Serial Adapter** (FTDI / CP2102):

| STM32 Pin | Serial Adapter Pin | Description |
|---|---|---|
| **PA2 (USART2_TX)** | `RXD` | Transmit log output |
| **PA3 (USART2_RX)** | `TXD` | Receive command input |
| **GND** | `GND` | Ground reference |

Open a terminal emulator (PuTTY, Minicom, TeraTerm, or VS Code Serial Monitor) at **115200 Baud, 8 Data bits, No Parity, 1 Stop bit (8N1)**.

---

## Integration Steps

### Step 1: Create a Output Backend Function
Write a 3-line wrapper function calling `HAL_UART_Transmit()`:

```c
static void stm32_log_output(const char *str, size_t len) {
    HAL_UART_Transmit(&huart2, (const uint8_t *)str, (uint16_t)len, 100);
}
```

### Step 2: Initialize `syn_log` in `main()`
Call `syn_log_init()` before using any log macros:

```c
// Enable all log levels down to TRACE
syn_log_init(stm32_log_output, SYN_LOG_TRACE);
```

### Step 3: Use Tagged Logging Macros
Include `#define TAG "MY_MODULE"` in your source files:

```c
#define TAG "APP"

SYN_LOG_T(TAG, "Trace message: x=%d", x);   // TRACE (Level 0)
SYN_LOG_D(TAG, "Debug message: val=%u", v);  // DEBUG (Level 1)
SYN_LOG_I(TAG, "Info: System ready!");       // INFO  (Level 2)
SYN_LOG_W(TAG, "Warning: Voltage low");      // WARN  (Level 3)
SYN_LOG_E(TAG, "Error: Timeout status=%d", s); // ERROR (Level 4)

// Binary Hex Dump
syn_log_hexdump(TAG, buffer, len);
```

---

## Output Preview

```text
[   1000] I/SYSTEM: SyntropicOS syn_log console initialized on USART2 @ 115200 baud
[   2500] T/SYSTEM: Task tick iteration = 1
[   2501] D/SYSTEM: ADC raw sample channel 0 = 2049
[   2502] I/SYSTEM: Heartbeat active, uptime = 2502 ms
[   4000] T/SYSTEM: Task tick iteration = 2
[   4001] D/SYSTEM: ADC raw sample channel 0 = 2050
[   4002] I/SYSTEM: Heartbeat active, uptime = 4002 ms
[   4003] W/SYSTEM: High CPU load detected on scheduled task
[   4004] D/SYSTEM: RS-485 Buffer (8 bytes):
0000: 68 11 22 33 44 55 66 16                          h."3DUf.
```
