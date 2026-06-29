/**
 * @file syn_profiler.h
 * @brief Task profiler — CPU time tracking per scheduler task.
 *
 * Hooks into the scheduler to measure how much time each task
 * consumes. Reports percentage, peak time, and call count.
 *
 * @par Usage
 * @code
 *   static SYN_ProfileEntry prof_entries[8];
 *   static SYN_Profiler profiler;
 *   syn_profiler_init(&profiler, prof_entries, 8);
 *
 *   // In scheduler loop (before/after each task run):
 *   syn_profiler_task_begin(&profiler, task_index);
 *   // ... run task ...
 *   syn_profiler_task_end(&profiler, task_index);
 *
 *   // Periodically (e.g., every 1s):
 *   syn_profiler_update(&profiler);  // calculates percentages
 *   syn_profiler_dump(&profiler, print_func);
 * @endcode
 * @ingroup syn_debug
 */

#ifndef SYN_PROFILER_H
#define SYN_PROFILER_H

#include "../port/syn_port_system.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Profile entry (per task) ───────────────────────────────────────────── */

/** @brief Per-task profile entry — timing stats and CPU usage. */
typedef struct {
    const char *name;           /**< Task name                            */
    uint32_t    total_us;       /**< Total time in current period (µs)    */
    uint32_t    peak_us;        /**< Peak single-run time (µs)            */
    uint32_t    run_count;      /**< Number of runs in current period     */
    uint16_t    cpu_percent_x10; /**< CPU% × 10 (e.g., 125 = 12.5%)      */

    /* Internal */
    uint32_t    _start_tick;    /**< Internal: tick at task_begin()       */
} SYN_ProfileEntry;

/* ── Profiler ───────────────────────────────────────────────────────────── */

/** @brief Task profiler instance. */
typedef struct {
    SYN_ProfileEntry *entries;     /**< Profile entry array               */
    uint8_t            capacity;    /**< Number of entries                  */
    uint32_t           period_start; /**< Tick at start of current period  */
    uint32_t           period_ms;    /**< Measurement period (ms)          */
    bool               enabled;      /**< Profiling active                 */
} SYN_Profiler;

/**
 * @brief Print callback for profiler dump output.
 * @param str  Null-terminated string to output.
 */
typedef void (*SYN_ProfilerPrintFunc)(const char *str);

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the profiler.
 *
 * @param prof       Profiler instance.
 * @param entries    Array of profile entries (one per task).
 * @param capacity   Number of entries.
 */
void syn_profiler_init(SYN_Profiler *prof,
                        SYN_ProfileEntry *entries,
                        uint8_t capacity);

/**
 * @brief Register a task for profiling.
 *
 * @param prof   Profiler.
 * @param index  Task index (must match scheduler task index).
 * @param name   Task name for display.
 */
void syn_profiler_register(SYN_Profiler *prof, uint8_t index,
                            const char *name);

/**
 * @brief Mark the start of a task's execution.
 * @param prof   Profiler.
 * @param index  Task index.
 */
void syn_profiler_task_begin(SYN_Profiler *prof, uint8_t index);

/**
 * @brief Mark the end of a task's execution.
 * @param prof   Profiler.
 * @param index  Task index.
 */
void syn_profiler_task_end(SYN_Profiler *prof, uint8_t index);

/**
 * @brief Update CPU percentages and reset counters for the next period.
 *
 * Call periodically (e.g., every 1 second).
 *
 * @param prof  Profiler.
 */
void syn_profiler_update(SYN_Profiler *prof);

/**
 * @brief Dump profiler results via print callback.
 * @param prof   Profiler.
 * @param print  Print function.
 */
void syn_profiler_dump(const SYN_Profiler *prof,
                        SYN_ProfilerPrintFunc print);

/**
 * @brief Enable/disable profiling.
 * @param prof    Profiler.
 * @param enable  true to enable, false to disable.
 */
void syn_profiler_enable(SYN_Profiler *prof, bool enable);

/**
 * @brief Get a profile entry (read-only).
 * @param prof   Profiler.
 * @param index  Task index.
 * @return Profile entry, or NULL if index out of range.
 */
static inline const SYN_ProfileEntry *
syn_profiler_get(const SYN_Profiler *prof, uint8_t index)
{
    return (index < prof->capacity) ? &prof->entries[index] : (const SYN_ProfileEntry *)0;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PROFILER_H */
