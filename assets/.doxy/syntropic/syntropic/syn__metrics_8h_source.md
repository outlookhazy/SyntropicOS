

# File syn\_metrics.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_metrics.h**](syn__metrics_8h.md)

[Go to the documentation of this file](syn__metrics_8h.md)


```C++

#ifndef SYN_METRICS_H
#define SYN_METRICS_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_METRIC_TYPE_COUNTER, 
    SYN_METRIC_TYPE_GAUGE    
} SYN_MetricType;

typedef union {
    uint64_t counter; 
    int32_t  gauge;   
} SYN_MetricVal;

typedef struct SYN_Metric {
    const char *name;      
    const char *help;      
    SYN_MetricType  type;  
    SYN_MetricVal   val;   
    bool            registered; 
    struct SYN_Metric *next; 
} SYN_Metric;

#if SYN_USE_METRICS

#define SYN_METRIC_DECLARE(var, _name, _help, _type) \
    static SYN_Metric var = { \
        .name = _name, \
        .help = _help, \
        .type = _type, \
        .val = {0}, \
        .registered = false, \
        .next = NULL \
    }

#define SYN_METRIC_REGISTER(m) syn_metric_register(&(m))

#define SYN_METRIC_INC(m) do { \
    if ((m).type == SYN_METRIC_TYPE_GAUGE) (m).val.gauge++; \
    else (m).val.counter++; \
} while(0)

#define SYN_METRIC_ADD(m, v) do { \
    if ((m).type == SYN_METRIC_TYPE_GAUGE) (m).val.gauge += (int32_t)(v); \
    else (m).val.counter += (v); \
} while(0)

#define SYN_METRIC_DEC(m) do { if ((m).type == SYN_METRIC_TYPE_GAUGE) (m).val.gauge--; } while(0)

#define SYN_METRIC_SUB(m, v) do { if ((m).type == SYN_METRIC_TYPE_GAUGE) (m).val.gauge -= (v); } while(0)

#define SYN_METRIC_SET(m, v) do { if ((m).type == SYN_METRIC_TYPE_GAUGE) (m).val.gauge = (v); } while(0)

void syn_metrics_init(void);

struct SYN_Router;
void syn_metrics_set_router(struct SYN_Router *r);

void syn_metrics_record(const char *name, float value);

void syn_metrics_count(const char *name, int32_t delta);

void syn_metric_register(SYN_Metric *m);

void syn_metrics_foreach(void (*cb)(const SYN_Metric *m, void *ctx), void *ctx);

#else /* !SYN_USE_METRICS */

#define SYN_METRIC_DECLARE(var, name, help, type)
#define SYN_METRIC_REGISTER(m) ((void)0)
#define SYN_METRIC_INC(m)      ((void)0)
#define SYN_METRIC_ADD(m, v)   ((void)0)
#define SYN_METRIC_DEC(m)      ((void)0)
#define SYN_METRIC_SUB(m, v)   ((void)0)
#define SYN_METRIC_SET(m, v)   ((void)0)

static inline void syn_metrics_init(void) {}
static inline void syn_metrics_record(const char *name, float value) { (void)name; (void)value; }
static inline void syn_metrics_count(const char *name, int32_t delta) { (void)name; (void)delta; }

static inline void syn_metrics_foreach(void (*cb)(const struct SYN_Metric *m, void *ctx), void *ctx) { (void)cb; (void)ctx; }

#endif /* SYN_USE_METRICS */

#ifdef __cplusplus
}
#endif

#endif /* SYN_METRICS_H */
```


