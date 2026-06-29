/**
 * @file syn_watchdog.h
 * @brief Task-level watchdog monitor.
 *
 * Each registered task must "check in" periodically. If any task misses
 * its deadline, the watchdog fires a callback (typically system reset).
 *
 * This is distinct from a hardware watchdog — it monitors individual
 * software tasks, not just the main loop.
 *
 * @par Usage
 * @code
 *   static SYN_Watchdog wdt;
 *   static SYN_WDT_Entry entries[3];
 *
 *   syn_watchdog_init(&wdt, entries, 3, on_timeout, NULL);
 *
 *   int id0 = syn_watchdog_register(&wdt, "sensor", 5000);  // 5s timeout
 *   int id1 = syn_watchdog_register(&wdt, "comms",  2000);
 *
 *   // In each task:
 *   syn_watchdog_checkin(&wdt, id0);
 *
 *   // In main loop:
 *   syn_watchdog_update(&wdt);
 * @endcode
 * @ingroup syn_sched
 */

#ifndef SYN_WATCHDOG_H
#define SYN_WATCHDOG_H

#include "../common/syn_defs.h"
#include "../port/syn_port_system.h"
#include "../system/syn_errlog.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Watchdog entry ─────────────────────────────────────────────────────── */

/** @brief Single watchdog entry for task monitoring. */
typedef struct {
    const char  *name;          /**< Task name (for debug)                */
    uint32_t     timeout_ms;    /**< Deadline in milliseconds             */
    uint32_t     last_checkin;  /**< Tick of last check-in                */
    bool         active;        /**< Is this entry in use?                */
} SYN_WDT_Entry;

/* ── Callback ───────────────────────────────────────────────────────────── */

struct SYN_Watchdog;

/**
 * @brief Called when a task misses its check-in deadline.
 *
 * @param wdt   Watchdog instance.
 * @param entry The entry that timed out.
 * @param ctx   User context.
 */
typedef void (*SYN_WDT_TimeoutCallback)(struct SYN_Watchdog *wdt,
                                         const SYN_WDT_Entry *entry,
                                         void *ctx);

/* ── Watchdog instance ──────────────────────────────────────────────────── */

/** @brief Software watchdog — monitors task deadlines. */
typedef struct SYN_Watchdog {
    SYN_WDT_Entry          *entries;    /**< Array of task monitoring entries */
    uint8_t                  capacity;   /**< Total slots allocated in entries array */
    uint8_t                  count;      /**< Current number of registered tasks */
    SYN_WDT_TimeoutCallback callback;   /**< Function called on timeout deadline miss */
    void                    *ctx;        /**< Context pointer for the callback */
    SYN_ErrLog             *errlog;     /**< If set, timeouts are logged */
} SYN_Watchdog;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the watchdog monitor.
 *
 * @param wdt       Watchdog instance.
 * @param entries   Array of entries (caller-owned).
 * @param capacity  Maximum number of monitored tasks.
 * @param callback  Timeout handler.
 * @param ctx       Context for callback.
 */
void syn_watchdog_init(SYN_Watchdog *wdt,
                        SYN_WDT_Entry *entries, uint8_t capacity,
                        SYN_WDT_TimeoutCallback callback, void *ctx);

/**
 * @brief Register a task for monitoring.
 *
 * @param wdt         Watchdog.
 * @param name        Task name.
 * @param timeout_ms  Maximum allowed time between check-ins.
 * @return Task ID (index) for use with syn_watchdog_checkin(), or -1 if full.
 */
int8_t syn_watchdog_register(SYN_Watchdog *wdt, const char *name,
                              uint32_t timeout_ms);

/**
 * @brief Check in a task (reset its timeout).
 *
 * @param wdt  Watchdog.
 * @param id   Task ID returned by syn_watchdog_register().
 */
void syn_watchdog_checkin(SYN_Watchdog *wdt, int8_t id);

/**
 * @brief Unregister a task.
 *
 * @param wdt  Watchdog.
 * @param id   Task ID returned by syn_watchdog_register().
 */
void syn_watchdog_unregister(SYN_Watchdog *wdt, int8_t id);

/**
 * @brief Check all tasks for timeouts.
 *
 * Call from your main loop or a periodic timer. If any task has exceeded
 * its timeout, the callback is invoked.
 *
 * @param wdt  Watchdog instance.
 */
void syn_watchdog_update(SYN_Watchdog *wdt);

#ifdef __cplusplus
}
#endif

#endif /* SYN_WATCHDOG_H */
