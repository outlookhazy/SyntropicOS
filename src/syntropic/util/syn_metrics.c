/**
 * @file syn_metrics.c
 * @brief Implementation of system-wide metrics registry.
 */

#include "syn_metrics.h"

#include "../common/syn_defs.h"
#include "../port/syn_port_system.h"
#include "../util/syn_fmt.h"

#include <stdio.h>
#include <string.h>

#if defined(SYN_USE_METRICS) && SYN_USE_METRICS

static SYN_Metric *g_metrics_head = NULL;

void syn_metrics_init(void)
{
    syn_port_enter_critical();
    SYN_Metric *curr = g_metrics_head;
    while (curr != NULL) {
        SYN_Metric *next = curr->next;
        curr->registered = false;
        curr->next = NULL;
        curr = next;
    }
    g_metrics_head = NULL;
    syn_port_exit_critical();
}

#if defined(SYN_USE_ROUTER) && SYN_USE_ROUTER
#include "../net/syn_router.h"

static struct SYN_Router *g_metrics_router = NULL;

void syn_metrics_set_router(struct SYN_Router *r)
{
    g_metrics_router = r;
}

void syn_metrics_record(const char *name, float value)
{
    if (!g_metrics_router)
        return;
    char buf[64];
    int slen = snprintf(buf, sizeof(buf), "%s:%.2f|g", name, (double)value);
    if (slen > 0 && slen < (int)sizeof(buf)) {
        syn_router_send(g_metrics_router, 0x01 /* GATEWAY */, 0x04, (const uint8_t *)buf,
                        (uint8_t)slen, false);
    }
}

void syn_metrics_count(const char *name, int32_t delta)
{
    if (!g_metrics_router)
        return;
    char buf[64];
    int slen = snprintf(buf, sizeof(buf), "%s:%d|c", name, (int)delta);
    if (slen > 0 && slen < (int)sizeof(buf)) {
        syn_router_send(g_metrics_router, 0x01 /* GATEWAY */, 0x04, (const uint8_t *)buf,
                        (uint8_t)slen, false);
    }
}
#endif /* SYN_USE_ROUTER */

void syn_metric_register(SYN_Metric *m)
{
    if (!m) {
        return;
    }

    syn_port_enter_critical();
    if (!m->registered) {
        m->registered = true;
        m->next = g_metrics_head;
        g_metrics_head = m;
    }
    syn_port_exit_critical();
}

void syn_metrics_foreach(void (*cb)(const SYN_Metric *m, void *ctx), void *ctx)
{
    if (!cb)
        return;

    SYN_Metric *curr = g_metrics_head;
    while (curr) {
        cb(curr, ctx);
        curr = curr->next;
    }
}

#endif /* SYN_USE_METRICS */
