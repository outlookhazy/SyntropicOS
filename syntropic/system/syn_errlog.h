/**
 * @file syn_errlog.h
 * @brief Persistent error registry — survives resets via param store.
 *
 * Records errors with code, timestamp, boot count, and optional context.
 * Stored in flash via syn_param so they survive power cycles. Can be
 * queried via CLI for field diagnostics.
 *
 * @par Usage
 * @code
 *   SYN_ErrEntry entries[16];
 *   SYN_ErrLog elog;
 *   syn_errlog_init(&elog, entries, 16, &my_param_store, 42);
 *
 *   syn_errlog_record(&elog, ERR_MOTOR_STALL, 0x0001);
 *   syn_errlog_record(&elog, ERR_OVERTEMP, 85);
 *
 *   // Read back:
 *   SYN_ErrEntry e;
 *   for (size_t i = 0; i < syn_errlog_count(&elog); i++) {
 *       syn_errlog_read(&elog, i, &e);
 *       // e.code, e.context, e.timestamp, e.boot_count
 *   }
 * @endcode
 * @ingroup syn_debug
 */

#ifndef SYN_ERRLOG_H
#define SYN_ERRLOG_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Error severity ─────────────────────────────────────────────────────── */

/** @brief Error severity levels. */
typedef enum {
    SYN_ERR_INFO    = 0,   /**< Informational (logged but not critical)  */
    SYN_ERR_WARNING = 1,   /**< Warning (degraded but operational)       */
    SYN_ERR_ERROR   = 2,   /**< Error (feature failed)                   */
    SYN_ERR_FATAL   = 3,   /**< Fatal (system will reset/halt)           */
} SYN_ErrSeverity;

/* ── Error entry ────────────────────────────────────────────────────────── */

/** @brief Error log entry — code + severity + context + timestamps. */
typedef struct {
    uint16_t  code;         /**< Application-defined error code           */
    uint8_t   severity;     /**< SYN_ErrSeverity                         */
    uint8_t   _pad;         /**< Padding for alignment                   */
    uint32_t  context;      /**< Application-defined context value        */
    uint32_t  timestamp;    /**< Tick at time of error                    */
    uint32_t  boot_count;   /**< Boot number when error occurred          */
} SYN_ErrEntry;

/* ── Error log instance ─────────────────────────────────────────────────── */

/** @brief Error log instance — circular buffer of error entries. */
typedef struct {
    SYN_ErrEntry  *entries;     /**< Circular buffer of entries          */
    size_t          capacity;    /**< Max entries                         */
    size_t          head;        /**< Next write position                 */
    size_t          total_count; /**< Total errors ever recorded          */

    uint32_t        boot_count;  /**< Current boot number                 */
    bool            enabled;     /**< Recording enabled                   */
} SYN_ErrLog;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the error log.
 *
 * @param log        Error log instance.
 * @param entries    Caller-provided entry buffer.
 * @param capacity   Number of entries in buffer.
 * @param boot_count Current boot count (from boot manager or 0).
 */
void syn_errlog_init(SYN_ErrLog *log, SYN_ErrEntry *entries,
                      size_t capacity, uint32_t boot_count);

/**
 * @brief Record an error.
 * @param log       Error log.
 * @param code      Application-defined error code.
 * @param severity  Severity level.
 * @param context   Application-defined context value.
 */
void syn_errlog_record(SYN_ErrLog *log, uint16_t code,
                        SYN_ErrSeverity severity, uint32_t context);

/**
 * @brief Read an error entry by index (0 = oldest available).
 * @param log    Error log.
 * @param index  Entry index.
 * @param out    Output entry.
 * @return true if entry exists.
 */
bool syn_errlog_read(const SYN_ErrLog *log, size_t index,
                      SYN_ErrEntry *out);

/**
 * @brief Total errors ever recorded (including overwritten).
 * @param log  Error log.
 * @return Total error count.
 */
static inline size_t syn_errlog_count(const SYN_ErrLog *log)
{
    return log->total_count;
}

/**
 * @brief Number of entries currently available for reading.
 * @param log  Error log.
 * @return Available entry count.
 */
static inline size_t syn_errlog_available(const SYN_ErrLog *log)
{
    return (log->total_count < log->capacity)
         ? log->total_count
         : log->capacity;
}

/**
 * @brief Enable/disable recording.
 * @param log  Error log.
 * @param en   true to enable, false to disable.
 */
static inline void syn_errlog_enable(SYN_ErrLog *log, bool en)
{
    log->enabled = en;
}

/**
 * @brief Clear all entries.
 * @param log  Error log.
 */
void syn_errlog_clear(SYN_ErrLog *log);

/**
 * @brief Get the most recent error entry.
 * @param log  Error log.
 * @return Pointer to latest entry, or NULL if empty.
 */
const SYN_ErrEntry *syn_errlog_latest(const SYN_ErrLog *log);

/**
 * @brief Count errors with a specific severity level.
 * @param log       Error log.
 * @param severity  Severity to count.
 * @return Number of matching entries.
 */
size_t syn_errlog_count_severity(const SYN_ErrLog *log,
                                  SYN_ErrSeverity severity);

#ifdef __cplusplus
}
#endif

#endif /* SYN_ERRLOG_H */
