/**
 * @file syn_coredump.h
 * @brief Persistent core dump — saves fault context to flash.
 *
 * Captures CPU register state and a partial stack snapshot during a
 * hard fault and persists them to a reserved flash sector. On the
 * next boot, the dump can be read back for post-mortem diagnostics.
 *
 * Requires: SYN_USE_FAULT, CRC module, flash port.
 *
 * @par Configuration
 * Define these in syn_config.h:
 * - `SYN_COREDUMP_FLASH_ADDR` — Start address of the reserved flash sector.
 * - `SYN_COREDUMP_STACK_SIZE` — Bytes of stack to capture (default 128).
 *
 * @par Usage
 * @code
 *   // In your HardFault handler:
 *   syn_coredump_save(&fault_context);
 *
 *   // At boot:
 *   SYN_CoreDump dump;
 *   if (syn_coredump_read(&dump)) {
 *       printf("Previous crash at PC=0x%08x, uptime=%lu ms\n",
 *              dump.regs.pc, dump.uptime_ms);
 *       syn_coredump_clear();
 *   }
 * @endcode
 * @ingroup syn_system
 */

#ifndef SYN_COREDUMP_H
#define SYN_COREDUMP_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if defined(SYN_USE_COREDUMP) && SYN_USE_COREDUMP

#include "../common/syn_defs.h"
#include "syn_fault.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration defaults ─────────────────────────────────────────────── */

#ifndef SYN_COREDUMP_STACK_SIZE
  #define SYN_COREDUMP_STACK_SIZE  128  /**< Bytes of stack to capture  */
#endif

#ifndef SYN_COREDUMP_FLASH_ADDR
  #error "SYN_COREDUMP_FLASH_ADDR must be defined to the reserved flash sector address"
#endif

/* ── Magic ──────────────────────────────────────────────────────────────── */

/** @brief Magic number identifying a valid core dump ("CDMP"). */
#define SYN_COREDUMP_MAGIC  0x43444D50u

/* ── Core dump structure ────────────────────────────────────────────────── */

/**
 * @brief Persistent core dump — written to flash on fault.
 *
 * Total size: ~180 bytes with 128-byte stack capture.
 */
typedef struct {
    uint32_t         magic;                              /**< SYN_COREDUMP_MAGIC when valid */
    SYN_FaultContext regs;                               /**< CPU register snapshot         */
    uint32_t         stack_bytes;                        /**< Actual stack bytes captured    */
    uint8_t          stack[SYN_COREDUMP_STACK_SIZE];     /**< Partial stack snapshot         */
    uint32_t         uptime_ms;                          /**< System uptime at fault         */
    uint32_t         crc;                                /**< CRC-32 over preceding fields   */
} SYN_CoreDump;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Save a core dump to flash.
 *
 * Safe to call from a hard fault handler: no heap, no locks.
 * Erases the reserved sector and writes the dump.
 *
 * @param ctx  Register context captured at fault time.
 */
void syn_coredump_save(const SYN_FaultContext *ctx);

/**
 * @brief Read a core dump from flash.
 *
 * Validates the magic number and CRC. If valid, copies the dump
 * into the output buffer.
 *
 * @param out  Output buffer for the core dump.
 * @return true if a valid core dump was found.
 */
bool syn_coredump_read(SYN_CoreDump *out);

/**
 * @brief Clear the core dump from flash.
 *
 * Erases the reserved sector so subsequent reads return false.
 */
void syn_coredump_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_COREDUMP */

#endif /* SYN_COREDUMP_H */
