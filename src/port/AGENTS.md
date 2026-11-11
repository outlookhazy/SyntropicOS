# MCU & Platform Port Invariants (`src/port/`)

Applies to all target platform ports (`stm32f4`, `stm32_hal`, `ch32v307`, `esp32`, `rp2040`, `arduino`, `posix`).

## Port Wiring & Host Mock Layout

- **HAL Contracts**: Portable headers live in `src/syntropic/port/syn_port_*.h`, plus Ethernet/USB contracts in `src/port/syn_port_eth.h`, `src/port/syn_port_usb.h`, and `src/port/syn_port_usb_host.h`.
- **POSIX & Unit Test Boundary**: `src/port/posix/` provides socket networking only — there is **no** POSIX system port in `src/port/posix/`. Host unit tests link against `tests/unit/mocks/mock_port.c`.
- **Focused File Reads**: Port files (`port_stm32f4.c`, `port_ch32v307.c`, `port_arduino.cpp`) bundle system ticks, critical sections, GPIO, UART, SPI, I2C, ADC, DMA, and Flash in single large files (`>1,400` lines). Run `grep -n` for target `syn_port_*` symbols and read targeted `StartLine..EndLine` slices rather than paging peripheral HAL sections unrelated to the task.

## Hardware & Concurrency Invariants

- **Nested Critical Section Save/Restore**: `syn_port_enter_critical()` / `syn_port_exit_critical()` may nest or be called while interrupts are already masked by caller/ISR context. Save and restore caller interrupt mask state (`PRIMASK` on Cortex-M, `mstatus.MIE` on RISC-V/QingKe, `SREG`/irq state on Arduino, spinlock/irq level on SMP) or maintain nesting depth — **never** unconditionally enable interrupts on exit.
- **Sub-Millisecond Tick Rollover Races (`syn_port_get_tick_us()`)**: Reading a hardware down/up-counter alongside a software millisecond tick races against timer wrap (`SYSTICK_VAL == 0` / `COUNTFLAG` / `CNTIF`). Re-read tick + counter consistently across rollover boundaries and avoid integer-MHz truncation drift when derived from divided clocks (e.g., `HCLK/8`).
- **Tickless Sleep & Wake Ordering (`syn_port_sleep_until()`)**: Invoked inside a critical section with interrupts masked (`syn_port_system.h`). Implementations must wake on pending-but-masked interrupts (`WFI` on Cortex-M, `SEV`/`WFE` pairing on multicore RP2040) and return with interrupt state consistent with the caller's critical section.
- **Flash & Cache Coherency**: On targets with instruction/data prefetch or ART caches (e.g., STM32F4), flush/invalidate affected caches on **both** successful writes and partial program/erase error paths so stale cached words cannot mask flash state.
- **UART Timeout Semantics (`syn_port_uart.h`)**: `syn_port_uart_transmit(..., timeout_ms == 0)` blocks until transmission completes (no timeout), whereas `syn_port_uart_receive(..., timeout_ms == 0)` and `syn_port_uart_receive_byte(..., timeout_ms == 0)` perform a non-blocking poll and return `SYN_TIMEOUT` immediately if no byte is ready.
- **Volatile MMIO & Multicore Barriers**: Access peripheral registers strictly via `volatile` pointers matching hardware bus width; implement `syn_port_memory_barrier()` (`DMB` / `fence`) when `SYN_USE_MULTICORE` is enabled.
