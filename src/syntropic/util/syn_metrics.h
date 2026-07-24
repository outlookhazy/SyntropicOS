/**
 * @file syn_metrics.h
 * @brief Lightweight telemetry and metrics (Counter/Gauge).
 * @ingroup syn_util
 *
 * This module follows an opt-in pattern. If SYN_USE_METRICS is not defined
 * or is 0, all macros expand to nothing, resulting in zero overhead.
 */

#ifndef SYN_METRICS_H
#define SYN_METRICS_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Type of value stored in a metric. */
typedef enum {
    SYN_METRIC_TYPE_COUNTER, /**< Monotonically increasing 64-bit value */
    SYN_METRIC_TYPE_GAUGE    /**< 32-bit value that can go up and down */
} SYN_MetricType;

/** @brief Internal union for metric values. */
typedef union {
    uint64_t counter; /**< Value for SYN_METRIC_TYPE_COUNTER */
    int32_t  gauge;   /**< Value for SYN_METRIC_TYPE_GAUGE */
} SYN_MetricVal;

/** 
 * @brief Metric descriptor and storage.
 * 
 * Metrics are typically declared statically and registered in the global 
 * registry. They are designed to be zero-heap and bare-metal friendly.
 */
typedef struct SYN_Metric {
    const char *name;      /**< Unique name (e.g., "wg_tx_bytes") */
    const char *help;      /**< Human-readable description */
    SYN_MetricType  type;  /**< Counter or Gauge */
    SYN_MetricVal   val;   /**< Current value */
    bool            registered; /**< True if in global registry */
    struct SYN_Metric *next; /**< Next in global registry list */
} SYN_Metric;

#if SYN_USE_METRICS

/**
 * @brief Declare a metric statically.
 * @param var   Variable name.
 * @param _name String name for export.
 * @param _help Description.
 * @param _type SYN_METRIC_TYPE_COUNTER or SYN_METRIC_TYPE_GAUGE.
 */
#define SYN_METRIC_DECLARE(var, _name, _help, _type) \
    static SYN_Metric var = { \
        .name = _name, \
        .help = _help, \
        .type = _type, \
        .val = {0}, \
        .registered = false, \
        .next = NULL \
    }

/** @brief Register a previously declared metric. */
#define SYN_METRIC_REGISTER(m) syn_metric_register(&(m))

/** @brief Increment a counter or gauge. */
#define SYN_METRIC_INC(m) do { \
    if ((m).type == SYN_METRIC_TYPE_GAUGE) (m).val.gauge++; \
    else (m).val.counter++; \
} while(0)

/** @brief Add value to a counter or gauge. */
#define SYN_METRIC_ADD(m, v) do { \
    if ((m).type == SYN_METRIC_TYPE_GAUGE) (m).val.gauge += (int32_t)(v); \
    else (m).val.counter += (v); \
} while(0)

/** @brief Decrement a gauge. */
#define SYN_METRIC_DEC(m) do { if ((m).type == SYN_METRIC_TYPE_GAUGE) (m).val.gauge--; } while(0)

/** @brief Subtract value from a gauge. */
#define SYN_METRIC_SUB(m, v) do { if ((m).type == SYN_METRIC_TYPE_GAUGE) (m).val.gauge -= (v); } while(0)

/** @brief Set a gauge value. */
#define SYN_METRIC_SET(m, v) do { if ((m).type == SYN_METRIC_TYPE_GAUGE) (m).val.gauge = (v); } while(0)

/** @brief Initialize metrics module. */
void syn_metrics_init(void);

/** @brief Set the router instance for telemetry transmission. */
struct SYN_Router;
void syn_metrics_set_router(struct SYN_Router *r);

/** @brief Record a gauge metric via router (StatsD format). */
void syn_metrics_record(const char *name, float value);

/** @brief Record a counter metric via router (StatsD format). */
void syn_metrics_count(const char *name, int32_t delta);

/** 
 * @brief Register a metric in the global registry.
 * @param m Metric to register.
 */
void syn_metric_register(SYN_Metric *m);

/**
 * @brief Iterate over all registered metrics.
 * @param cb  Callback for each metric.
 * @param ctx User context passed to callback.
 */
void syn_metrics_foreach(void (*cb)(const SYN_Metric *m, void *ctx), void *ctx);

#else /* !SYN_USE_METRICS */

/** @brief Declare a metric statically (disabled). */
#define SYN_METRIC_DECLARE(var, name, help, type)
/** @brief Register a metric (disabled). */
#define SYN_METRIC_REGISTER(m) ((void)0)
/** @brief Increment a metric (disabled). */
#define SYN_METRIC_INC(m)      ((void)0)
/** @brief Add value to a metric (disabled). */
#define SYN_METRIC_ADD(m, v)   ((void)0)
/** @brief Decrement a metric (disabled). */
#define SYN_METRIC_DEC(m)      ((void)0)
/** @brief Subtract value from a metric (disabled). */
#define SYN_METRIC_SUB(m, v)   ((void)0)
/** @brief Set a metric value (disabled). */
#define SYN_METRIC_SET(m, v)   ((void)0)

static inline void syn_metrics_init(void) {}
static inline void syn_metrics_record(const char *name, float value) { (void)name; (void)value; }
static inline void syn_metrics_count(const char *name, int32_t delta) { (void)name; (void)delta; }

/**
 * @brief Iterate over all registered metrics (no-op when disabled).
 * @param cb  Callback.
 * @param ctx Context.
 */
static inline void syn_metrics_foreach(void (*cb)(const struct SYN_Metric *m, void *ctx), void *ctx) { (void)cb; (void)ctx; }

#endif /* SYN_USE_METRICS */

#ifdef __cplusplus
}
#endif

#endif /* SYN_METRICS_H */
