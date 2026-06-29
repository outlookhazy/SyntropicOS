/**
 * @file syn_boot.h
 * @brief Boot manager — reset reason, boot counter, safe-mode detection.
 *
 * Uses the parameter store to persist boot state across resets. Detects
 * crash loops (too many resets without a successful "mark healthy") and
 * can trigger safe-mode behavior.
 *
 * @par Usage
 * @code
 *   static SYN_Boot boot;
 *   static SYN_ParamStore boot_store;
 *
 *   // In early init:
 *   syn_param_init(&boot_store, BOOT_FLASH_ADDR, 2, sizeof(SYN_BootData));
 *   syn_boot_init(&boot, &boot_store, 3);  // safe mode after 3 failures
 *
 *   if (syn_boot_is_safe_mode(&boot)) {
 *       // Load minimal config, disable risky features
 *   }
 *
 *   // After successful startup:
 *   syn_boot_mark_healthy(&boot);
 * @endcode
 * @ingroup syn_system
 */

#ifndef SYN_BOOT_H
#define SYN_BOOT_H

#include "../common/syn_defs.h"
#include "../storage/syn_param.h"
#include "../system/syn_errlog.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Boot data (persisted to flash) ─────────────────────────────────────── */

/** @brief Boot data persisted to flash across resets. */
typedef struct {
    uint32_t  boot_count;       /**< Total boot count (monotonic)         */
    uint16_t  fail_count;       /**< Consecutive boots without healthy    */
    uint8_t   last_reset;       /**< Last reset reason (application-set)  */
    uint8_t   was_healthy;      /**< Did last boot complete successfully? */
} SYN_BootData;

/* ── Reset reasons (application can extend) ─────────────────────────────── */

/** @brief Reset reason codes — application can extend from SYN_RESET_USER. */
typedef enum {
    SYN_RESET_UNKNOWN     = 0,   /**< Unknown reset cause.            */
    SYN_RESET_POWER_ON    = 1,   /**< Power-on reset.                 */
    SYN_RESET_WATCHDOG    = 2,   /**< Watchdog reset.                 */
    SYN_RESET_SOFTWARE    = 3,   /**< Software reset.                 */
    SYN_RESET_EXTERNAL    = 4,   /**< External reset pin.             */
    SYN_RESET_BROWNOUT    = 5,   /**< Brown-out detected.             */
    SYN_RESET_ASSERT      = 6,   /**< Assert failure.                 */
    SYN_RESET_USER        = 0x80,  /**< Application-specific start    */
} SYN_ResetReason;

/* ── Boot manager ───────────────────────────────────────────────────────── */

/** @brief Boot manager — crash-loop detection and safe-mode state. */
typedef struct {
    SYN_ParamStore  *store;         /**< Param store for persistence     */
    SYN_BootData     data;          /**< Current boot data               */
    uint16_t          safe_threshold; /**< fail_count to trigger safe mode */
    bool              safe_mode;     /**< Are we in safe mode?            */
    bool              initialized;   /**< Init complete flag              */
    SYN_ErrLog       *errlog;      /**< If set, boot events are logged  */
} SYN_Boot;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the boot manager.
 *
 * Loads boot data from flash, increments boot counter, checks for
 * crash loops. Call this very early in boot.
 *
 * @param boot            Boot manager instance.
 * @param store           Initialized parameter store for boot data.
 * @param safe_threshold  Number of consecutive failed boots before safe mode.
 *                        Set to 0 to disable safe mode detection.
 * @return SYN_OK on normal boot, SYN_ERROR on first-ever boot (no data).
 */
SYN_Status syn_boot_init(SYN_Boot *boot, SYN_ParamStore *store,
                           uint16_t safe_threshold);

/**
 * @brief Mark this boot as healthy.
 *
 * Call after your application has started successfully (e.g., after
 * all peripherals initialized, comms established). Resets the
 * fail counter.
 *
 * @param boot  Boot manager.
 * @return SYN_OK on success.
 */
SYN_Status syn_boot_mark_healthy(SYN_Boot *boot);

/**
 * @brief Set the reset reason for the current boot.
 *
 * Call before saving (or it gets saved on next boot_init).
 *
 * @param boot    Boot manager.
 * @param reason  Reset reason code (SYN_ResetReason or custom).
 */
void syn_boot_set_reset_reason(SYN_Boot *boot, uint8_t reason);

/**
 * @brief Check if we're in safe mode.
 * @param boot  Boot manager.
 * @return true if in safe mode.
 */
static inline bool syn_boot_is_safe_mode(const SYN_Boot *boot)
{
    return boot->safe_mode;
}

/**
 * @brief Get total boot count.
 * @param boot  Boot manager.
 * @return Monotonic boot count.
 */
static inline uint32_t syn_boot_count(const SYN_Boot *boot)
{
    return boot->data.boot_count;
}

/**
 * @brief Get consecutive fail count.
 * @param boot  Boot manager.
 * @return Number of boots without a healthy mark.
 */
static inline uint16_t syn_boot_fail_count(const SYN_Boot *boot)
{
    return boot->data.fail_count;
}

/**
 * @brief Get last reset reason.
 * @param boot  Boot manager.
 * @return SYN_ResetReason or custom code.
 */
static inline uint8_t syn_boot_reset_reason(const SYN_Boot *boot)
{
    return boot->data.last_reset;
}

/**
 * @brief Force clear safe mode and reset fail counter.
 *
 * Use from CLI to recover from safe mode.
 *
 * @param boot  Boot manager.
 * @return SYN_OK on success.
 */
SYN_Status syn_boot_clear_safe_mode(SYN_Boot *boot);

/**
 * @brief Set the errlog instance for boot event recording.
 *
 * Call after both boot and errlog are initialized, then call
 * syn_boot_log_events() to retroactively log boot-related events.
 *
 * @param boot    Boot manager.
 * @param errlog  Initialized error log instance.
 */
void syn_boot_set_errlog(SYN_Boot *boot, SYN_ErrLog *errlog);

/**
 * @brief Log boot events to errlog.
 *
 * Records crash-loop (non-healthy previous boot) and safe-mode
 * entry events. Call after syn_boot_set_errlog().
 *
 * @param boot  Boot manager.
 */
void syn_boot_log_events(SYN_Boot *boot);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BOOT_H */
