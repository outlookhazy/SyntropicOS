/**
 * @file syn_sleep.h
 * @brief Sleep coordinator — low-power mode management.
 *
 * Coordinates sleep entry with the scheduler. When all tasks are idle
 * (or only timer-blocked), the system can enter a low-power mode.
 * Clients can veto sleep by holding a "stay awake" lock.
 *
 * @par Architecture
 * - Each subsystem that needs the CPU awake holds a lock.
 * - When all locks are released, the scheduler calls
 *   syn_sleep_enter() which invokes the port sleep function.
 * - Interrupts wake the MCU, and the scheduler resumes.
 *
 * @par Usage
 * @code
 *   SYN_Sleep sleep;
 *   syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);
 *
 *   // UART ISR: hold lock while transmitting
 *   syn_sleep_lock(&sleep, SYN_SLEEP_LOCK_UART);
 *   // ... transmit complete ...
 *   syn_sleep_unlock(&sleep, SYN_SLEEP_LOCK_UART);
 *
 *   // In scheduler idle hook:
 *   syn_sleep_enter(&sleep);  // sleeps only if no locks held
 * @endcode
 * @ingroup syn_system
 */

#ifndef SYN_SLEEP_H
#define SYN_SLEEP_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Sleep depth ────────────────────────────────────────────────────────── */

/** @brief Sleep depth selector. */
typedef enum {
    SYN_SLEEP_NONE   = 0,  /**< No sleep (busy wait)                    */
    SYN_SLEEP_LIGHT  = 1,  /**< WFI / idle — fast wakeup                */
    SYN_SLEEP_DEEP   = 2,  /**< Stop mode — slower wakeup, lower power  */
} SYN_SleepMode;

/** @name Sleep Lock IDs (bit positions)
 * Users can define their own; these are common examples.
 * @{
 */
#define SYN_SLEEP_LOCK_UART     (1u << 0)  /**< UART peripheral active     */
#define SYN_SLEEP_LOCK_SPI      (1u << 1)  /**< SPI peripheral active      */
#define SYN_SLEEP_LOCK_I2C      (1u << 2)  /**< I2C peripheral active      */
#define SYN_SLEEP_LOCK_DMA      (1u << 3)  /**< DMA transfer in progress   */
#define SYN_SLEEP_LOCK_ADC      (1u << 4)  /**< ADC conversion in progress */
#define SYN_SLEEP_LOCK_TIMER    (1u << 5)  /**< Hardware timer active      */
#define SYN_SLEEP_LOCK_APP0     (1u << 8)  /**< Application-defined lock 0 */
#define SYN_SLEEP_LOCK_APP1     (1u << 9)  /**< Application-defined lock 1 */
#define SYN_SLEEP_LOCK_APP2     (1u << 10) /**< Application-defined lock 2 */
#define SYN_SLEEP_LOCK_APP3     (1u << 11) /**< Application-defined lock 3 */
/** @} */

/* ── Sleep coordinator ──────────────────────────────────────────────────── */

/** @brief Sleep coordinator — tracks wake locks and sleep statistics. */
typedef struct {
    SYN_SleepMode   max_depth;       /**< Deepest allowed sleep mode     */
    volatile uint32_t lock_mask;      /**< Active wake locks (bitmask)    */
    uint32_t          enter_count;    /**< Times we actually slept        */
    uint32_t          veto_count;     /**< Times sleep was vetoed         */
    bool              enabled;        /**< Global enable                  */
} SYN_Sleep;

/* ── Port function (user implements) ────────────────────────────────────── */

/**
 * @brief Enter low-power mode. Implement for your platform.
 *
 * For Cortex-M: SYN_SLEEP_LIGHT → WFI, SYN_SLEEP_DEEP → STOP mode.
 * Must return when an interrupt wakes the MCU.
 *
 * @param mode  Requested sleep depth.
 */
extern void syn_port_sleep(SYN_SleepMode mode);

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the sleep coordinator.
 * @param s          Sleep instance.
 * @param max_depth  Deepest sleep mode allowed.
 */
static inline void syn_sleep_init(SYN_Sleep *s, SYN_SleepMode max_depth)
{
    s->max_depth   = max_depth;
    s->lock_mask   = 0;
    s->enter_count = 0;
    s->veto_count  = 0;
    s->enabled     = true;
}

/**
 * @brief Acquire a wake lock (prevents sleep).
 * @param s        Sleep instance.
 * @param lock_id  Lock bitmask (e.g. SYN_SLEEP_LOCK_UART).
 */
static inline void syn_sleep_lock(SYN_Sleep *s, uint32_t lock_id)
{
    s->lock_mask |= lock_id;
}

/**
 * @brief Release a wake lock.
 * @param s        Sleep instance.
 * @param lock_id  Lock bitmask to release.
 */
static inline void syn_sleep_unlock(SYN_Sleep *s, uint32_t lock_id)
{
    s->lock_mask &= ~lock_id;
}

/**
 * @brief Check if a specific lock is held.
 * @param s        Sleep instance.
 * @param lock_id  Lock bitmask to check.
 * @return true if the lock is active.
 */
static inline bool syn_sleep_is_locked(const SYN_Sleep *s, uint32_t lock_id)
{
    return (s->lock_mask & lock_id) != 0;
}

/**
 * @brief Check if any lock is held.
 * @param s  Sleep instance.
 * @return true if any lock is active.
 */
static inline bool syn_sleep_any_locked(const SYN_Sleep *s)
{
    return s->lock_mask != 0;
}

/**
 * @brief Try to enter sleep mode.
 *
 * If no locks are held and sleep is enabled, calls syn_port_sleep().
 *
 * @param s  Sleep instance.
 * @return true if we actually slept, false if vetoed.
 */
static inline bool syn_sleep_enter(SYN_Sleep *s)
{
    if (!s->enabled || s->lock_mask != 0) {
        s->veto_count++;
        return false;
    }

    s->enter_count++;
    syn_port_sleep(s->max_depth);
    return true;
}

/**
 * @brief Enable/disable sleep globally.
 * @param s   Sleep instance.
 * @param en  true to enable, false to disable.
 */
static inline void syn_sleep_enable(SYN_Sleep *s, bool en)
{
    s->enabled = en;
}

/**
 * @brief Get active lock mask (for debug).
 * @param s  Sleep instance.
 * @return Active lock bitmask.
 */
static inline uint32_t syn_sleep_locks(const SYN_Sleep *s)
{
    return s->lock_mask;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SLEEP_H */
