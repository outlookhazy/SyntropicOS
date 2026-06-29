/**
 * @file syn_trace.h
 * @brief Lightweight trace buffer — timestamped event recorder.
 *
 * Records events (id + optional 16-bit value) into a fixed-size circular
 * buffer for post-mortem debugging. No UART, no printf — just stamp
 * and store. Read it out with a debugger or dump over CLI.
 *
 * @par Usage
 * @code
 *   static SYN_TraceEntry entries[64];
 *   static SYN_Trace trace;
 *   syn_trace_init(&trace, entries, 64);
 *
 *   SYN_TRACE(&trace, EVT_ISR_ENTER, irq_num);
 *   SYN_TRACE(&trace, EVT_TASK_SWITCH, task_id);
 *
 *   // Dump from CLI or debugger:
 *   syn_trace_dump(&trace, print_func);
 * @endcode
 * @ingroup syn_debug
 */

#ifndef SYN_TRACE_H
#define SYN_TRACE_H

#include "../port/syn_port_system.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Trace entry ────────────────────────────────────────────────────────── */

/** @brief Single trace event entry. */
typedef struct {
    uint32_t  timestamp;  /**< Tick when event occurred                   */
    uint16_t  event_id;   /**< Application-defined event ID              */
    uint16_t  value;      /**< Optional 16-bit payload                   */
} SYN_TraceEntry;

/* ── Trace buffer ───────────────────────────────────────────────────────── */

/** @brief Circular trace event buffer. */
typedef struct {
    SYN_TraceEntry *entries;   /**< Circular buffer (caller-owned)      */
    uint16_t         capacity;  /**< Buffer size                         */
    uint16_t         head;      /**< Next write position                 */
    uint32_t         count;     /**< Total events recorded (may wrap)    */
    bool             enabled;   /**< Recording state active flag */
} SYN_Trace;

/** Print callback for dump. */
typedef void (*SYN_TracePrintFunc)(const char *str);

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the trace buffer.
 *
 * @param trace     Trace instance to initialize.
 * @param entries   Array of trace entries (caller-owned).
 * @param capacity  Number of entries in the array.
 */
void syn_trace_init(SYN_Trace *trace,
                     SYN_TraceEntry *entries,
                     uint16_t capacity);

/**
 * @brief Record a trace event.
 *
 * @param trace     Trace instance.
 * @param event_id  Application-defined event ID.
 * @param value     Optional 16-bit payload.
 */
void syn_trace_record(SYN_Trace *trace, uint16_t event_id, uint16_t value);

/** Convenience macro with auto-timestamping. */
#define SYN_TRACE(trace, id, val) syn_trace_record((trace), (id), (val))

/**
 * @brief Enable/disable recording.
 *
 * @param trace  Trace instance.
 * @param enable True to enable recording, false to pause.
 */
void syn_trace_enable(SYN_Trace *trace, bool enable);

/**
 * @brief Clear the trace buffer.
 *
 * @param trace Trace instance to reset.
 */
void syn_trace_clear(SYN_Trace *trace);

/**
 * @brief Get total event count (may exceed capacity — indicates wrapping).
 *
 * @param trace Trace instance.
 * @return Total number of events written since initialization.
 */
static inline uint32_t syn_trace_count(const SYN_Trace *trace)
{
    return trace->count;
}

/**
 * @brief Read an entry by index (0 = oldest available).
 *
 * @param trace  Trace instance.
 * @param index  Index from oldest available.
 * @param entry  Output entry.
 * @return true if a valid entry was read.
 */
bool syn_trace_read(const SYN_Trace *trace, uint32_t index,
                     SYN_TraceEntry *entry);

/**
 * @brief Dump all entries via a print callback.
 *
 * Output format: "[tick] event_id=0xNNNN value=0xNNNN\n"
 *
 * @param trace  Trace instance.
 * @param print  Print function (e.g., write to UART).
 */
void syn_trace_dump(const SYN_Trace *trace, SYN_TracePrintFunc print);

#ifdef __cplusplus
}
#endif

#endif /* SYN_TRACE_H */
