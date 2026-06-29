/**
 * @file syn_soft_onewire.h
 * @brief Software bit-bang 1-Wire master driver.
 *
 * Implements the Dallas/Maxim 1-Wire protocol on any GPIO pin.
 * Timing is controlled by a caller-provided loop count (delay_loops),
 * analogous to syn_soft_i2c.h — calibrate for your CPU frequency.
 *
 * Supports standard-speed 1-Wire (15.4 kbps).  No parasitic-power or
 * overdrive mode.
 *
 * Typical use (DS18B20 temperature sensor, protothread-safe):
 * @code
 *   // In a protothread — conversion wait is a yield point, not a busy-wait.
 *   static uint32_t ow_start_ms;
 *   SYN_SoftOneWire ow;
 *
 *   // --- Phase 1: start conversion ---
 *   syn_soft_onewire_init(&ow, PIN_OW_DATA,
 *                        SYN_SOFT_ONEWIRE_LOOPS_PER_US(F_CPU));
 *   if (syn_soft_onewire_reset(&ow)) {           // ~960 µs busy
 *       syn_soft_onewire_write_byte(&ow, 0xCC);  // SKIP ROM   (~560 µs)
 *       syn_soft_onewire_write_byte(&ow, 0x44);  // CONVERT T  (~560 µs)
 *       ow_start_ms = syn_port_get_tick_ms();
 *   }
 *
 *   // --- Phase 2: yield until conversion done (~750 ms) ---
 *   PT_WAIT_UNTIL(pt, syn_port_get_tick_ms() - ow_start_ms >= 750u);
 *
 *   // --- Phase 3: read result ---
 *   if (syn_soft_onewire_reset(&ow)) {
 *       syn_soft_onewire_write_byte(&ow, 0xCC);  // SKIP ROM
 *       syn_soft_onewire_write_byte(&ow, 0xBE);  // READ SCRATCHPAD
 *       uint8_t lsb = syn_soft_onewire_read_byte(&ow);
 *       uint8_t msb = syn_soft_onewire_read_byte(&ow);
 *   }
 *   // DO NOT use syn_port_delay_ms(750) — that blocks the entire scheduler.
 * @endcode
 *
 * @note Each byte write/read blocks the CPU for ~560 µs (8 × 70 µs slots).
 *       Each reset() blocks for ~960 µs.  Hardware PWM and timer ISRs continue
 *       to fire normally during these windows.  Do not call these functions
 *       from a tight control loop that must execute faster than ~2 ms/cycle.
 *       @ingroup syn_drivers
 */

#ifndef SYN_SOFT_ONEWIRE_H
#define SYN_SOFT_ONEWIRE_H

#include <stdint.h>
#include <stdbool.h>
#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Timing calibration macro ──────────────────────────────────────────── */

/**
 * @brief Compute the delay_loops value for a given CPU frequency.
 *
 * The NOP loop body compiles to approximately 4 clock cycles on most
 * architectures (load, decrement, compare, branch-not-taken).  Dividing
 * the CPU frequency by 4 gives loops per second; dividing again by 1 000 000
 * gives loops per microsecond.
 *
 * Use this macro when initialising the bus:
 * @code
 *   syn_soft_onewire_init(&ow, PIN_OW_DATA,
 *                         SYN_SOFT_ONEWIRE_LOOPS_PER_US(F_CPU));
 * @endcode
 *
 * Or with an explicit frequency:
 * @code
 *   // 16 MHz Arduino Uno
 *   syn_soft_onewire_init(&ow, PIN_OW_DATA,
 *                         SYN_SOFT_ONEWIRE_LOOPS_PER_US(16000000UL));
 *   // 8 MHz AVR / STM32 at 8 MHz
 *   syn_soft_onewire_init(&ow, PIN_OW_DATA,
 *                         SYN_SOFT_ONEWIRE_LOOPS_PER_US(8000000UL));
 * @endcode
 *
 * @param freq_hz  CPU frequency in Hz (e.g. F_CPU, 16000000UL).
 *
 * @note  The divisor (4) is a conservative estimate for an optimised build.
 *        If timing measurements show pulses running consistently long or short,
 *        tune by replacing 4 with the actual loop-body cycle count from the
 *        generated disassembly.
 */
#define SYN_SOFT_ONEWIRE_LOOPS_PER_US(freq_hz) \
    ((uint32_t)(((uint32_t)(freq_hz)) / 4000000UL))

/** @brief 1-Wire bus handle.  Caller allocates; zero heap. */
typedef struct {
    SYN_GPIO_Pin pin;          /**< GPIO pin for the data line                   */
    uint32_t     delay_loops;  /**< NOP iterations per µs — use
                                *   SYN_SOFT_ONEWIRE_LOOPS_PER_US(F_CPU)         */
} SYN_SoftOneWire;

/**
 * @brief Initialize a 1-Wire bus instance.
 * @param ow           Bus handle to initialize.  Must not be NULL.
 * @param pin          GPIO pin for the data line.
 * @param delay_loops  NOP loop iterations per µs at your CPU frequency.
 *                     Use SYN_SOFT_ONEWIRE_LOOPS_PER_US(F_CPU) or
 *                     SYN_SOFT_ONEWIRE_LOOPS_PER_US(16000000UL) for 16 MHz.
 */
void syn_soft_onewire_init(SYN_SoftOneWire *ow,
                           SYN_GPIO_Pin     pin,
                           uint32_t         delay_loops);

/**
 * @brief Issue a 1-Wire reset pulse and detect device presence.
 * @param ow  Initialized bus handle.
 * @return true if at least one device acknowledged (presence pulse detected),
 *         false if the bus is empty.
 */
bool syn_soft_onewire_reset(const SYN_SoftOneWire *ow);

/**
 * @brief Write one byte LSB-first onto the 1-Wire bus.
 * @param ow    Initialized bus handle.
 * @param byte  Byte to transmit.
 */
void syn_soft_onewire_write_byte(const SYN_SoftOneWire *ow, uint8_t byte);

/**
 * @brief Read one byte LSB-first from the 1-Wire bus.
 * @param ow  Initialized bus handle.
 * @return Byte received.
 */
uint8_t syn_soft_onewire_read_byte(const SYN_SoftOneWire *ow);

/**
 * @brief Write an 8-byte ROM code to the bus (for MATCH ROM command).
 * @param ow   Initialized bus handle.
 * @param rom  8-byte ROM code, LSB first.
 */
void syn_soft_onewire_write_rom(const SYN_SoftOneWire *ow,
                                const uint8_t          rom[8]);

/**
 * @brief Read an 8-byte ROM code from the bus (after READ ROM command).
 * @param ow   Initialized bus handle.
 * @param rom  Output buffer, must be at least 8 bytes.
 */
void syn_soft_onewire_read_rom(const SYN_SoftOneWire *ow,
                               uint8_t                rom[8]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SOFT_ONEWIRE_H */
