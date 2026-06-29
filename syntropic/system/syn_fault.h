/**
 * @file syn_fault.h
 * @brief CPU Hard Fault / Post-Mortem Diagnostic log collector.
 * @ingroup syn_system
 */

#ifndef SYN_FAULT_H
#define SYN_FAULT_H

#include "../common/syn_defs.h"
#include "syn_errlog.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_FAULT_SIGNATURE  0xFA17C0DE  /**< Magic value indicating a valid fault dump */

/**
 * @brief Register context dumped on CPU hard faults (ARM Cortex-M style).
 */
typedef struct {
    uint32_t pc;    /**< Program counter at fault                     */
    uint32_t lr;    /**< Link register (return address)               */
    uint32_t sp;    /**< Stack pointer at fault                       */
    uint32_t r0;    /**< General-purpose register R0                  */
    uint32_t r1;    /**< General-purpose register R1                  */
    uint32_t r2;    /**< General-purpose register R2                  */
    uint32_t r3;    /**< General-purpose register R3                  */
    uint32_t r12;   /**< General-purpose register R12                 */
    uint32_t xpsr;  /**< Program status register                     */
} SYN_FaultContext;

/**
 * @brief Fault dump stored in noinit RAM across resets.
 */
typedef struct {
    uint32_t         signature;  /**< Magic value (SYN_FAULT_SIGNATURE when valid) */
    SYN_FaultContext context;    /**< Saved register context                       */
} SYN_FaultDump;

/**
 * @brief Capture a CPU fault register state.
 *
 * Saves register dump to uninitialized RAM and sets the signature.
 * Safe to call from a hard fault handler (no heap, no locks).
 *
 * @param ctx  Pointer to the register context to save (NULL is ignored).
 */
void syn_fault_capture(const SYN_FaultContext *ctx);

/**
 * @brief Checks if a crash occurred in a previous boot, logs it, and clears the signature.
 *
 * @param errlog Pointer to the active error log.
 * @return true if a fault was checked, logged, and cleared.
 */
bool syn_fault_check_and_log(SYN_ErrLog *errlog);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FAULT_H */
