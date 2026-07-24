/**
 * @file syn_metrics.c
 * @brief Implementation of system-wide metrics registry.
 */

#ifdef SYN_CONFIG_FILE
#include SYN_CONFIG_FILE
#elif __has_include("syn_config.h")
#include "syn_config.h"
#endif

#include "syn_metrics.h"
#include "../util/syn_fmt.h"
#include <string.h>
#include <stdio.h>

#if SYN_USE_METRICS

void syn_metrics_init(void) {}

#if SYN_USE_ROUTER
#include "../net/syn_router.h"

static struct SYN_Router *g_metrics_router = NULL;

void syn_metrics_set_router(struct SYN_Router *r) {
    g_metrics_router = r;
}

void syn_metrics_record(const char *name, float value) {
    if (!g_metrics_router) return;
    char buf[64];
    int slen = snprintf(buf, sizeof(buf), "%s:%.2f|g", name, (double)value);
    if (slen > 0 && slen < (int)sizeof(buf)) {
        syn_router_send(g_metrics_router, 0x01 /* GATEWAY */, 0x04, (const uint8_t *)buf, (uint8_t)slen, false);
    }
}

void syn_metrics_count(const char *name, int32_t delta) {
    if (!g_metrics_router) return;
    char buf[64];
    int slen = snprintf(buf, sizeof(buf), "%s:%d|c", name, (int)delta);
    if (slen > 0 && slen < (int)sizeof(buf)) {
        syn_router_send(g_metrics_router, 0x01 /* GATEWAY */, 0x04, (const uint8_t *)buf, (uint8_t)slen, false);
    }
}
#endif /* SYN_USE_ROUTER */

static SYN_Metric *g_metrics_head = NULL;

void syn_metric_register(SYN_Metric *m)
{
    if (!m || m->registered) return;
    m->registered = true;
    
    /* Simple thread-safe-ish insertion (assuming init happens on primary core) */
    m->next = g_metrics_head;
    g_metrics_head = m;
}

void syn_metrics_foreach(void (*cb)(const SYN_Metric *m, void *ctx), void *ctx)
{
    if (!cb) return;
    
    SYN_Metric *curr = g_metrics_head;
    while (curr) {
        cb(curr, ctx);
        curr = curr->next;
    }
}

#endif /* SYN_USE_METRICS */
