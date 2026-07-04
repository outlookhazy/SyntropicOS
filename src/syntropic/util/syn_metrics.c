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

void syn_metrics_record(const char *name, float value) {
    char buf[64];
    size_t len = 0;
    
    /* Format: name:value|g */
    size_t name_len = strlen(name);
    if (name_len > 32) name_len = 32;
    memcpy(buf, name, name_len);
    len = name_len;
    buf[len++] = ':';
    
    len += syn_fmt_float(buf + len, sizeof(buf) - len - 3, value, 2);
    buf[len++] = '|';
    buf[len++] = 'g';
    buf[len] = '\0';
    
    syn_router_send(0x0001 /* GATEWAY */, 0x04, buf, len);
}

void syn_metrics_count(const char *name, int32_t delta) {
    char buf[64];
    int slen = snprintf(buf, sizeof(buf), "%s:%d|c", name, (int)delta);
    if (slen > 0) {
        syn_router_send(0x0001 /* GATEWAY */, 0x04, buf, (size_t)slen);
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
