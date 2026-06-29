#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_PROFILER) || SYN_USE_PROFILER

/**
 * @file syn_profiler.c
 * @brief Task profiler implementation.
 */

#include "syn_profiler.h"
#include "../util/syn_assert.h"
#include "../util/syn_fmt.h"

#include <string.h>

void syn_profiler_init(SYN_Profiler *prof,
                        SYN_ProfileEntry *entries,
                        uint8_t capacity)
{
    SYN_ASSERT(prof != NULL);
    SYN_ASSERT(entries != NULL);
    SYN_ASSERT(capacity > 0);

    memset(entries, 0, sizeof(*entries) * capacity);
    prof->entries      = entries;
    prof->capacity     = capacity;
    prof->period_start = syn_port_get_tick_ms();
    prof->period_ms    = 0;
    prof->enabled      = true;
}

void syn_profiler_register(SYN_Profiler *prof, uint8_t index,
                            const char *name)
{
    SYN_ASSERT(prof != NULL);
    SYN_ASSERT(index < prof->capacity);
    prof->entries[index].name = name;
}

void syn_profiler_task_begin(SYN_Profiler *prof, uint8_t index)
{
    SYN_ASSERT(prof != NULL);
    if (!prof->enabled) return;
    if (index >= prof->capacity) return;

    prof->entries[index]._start_tick = syn_port_get_tick_ms();
}

void syn_profiler_task_end(SYN_Profiler *prof, uint8_t index)
{
    SYN_ASSERT(prof != NULL);
    if (!prof->enabled) return;
    if (index >= prof->capacity) return;

    SYN_ProfileEntry *e = &prof->entries[index];
    uint32_t elapsed = syn_port_get_tick_ms() - e->_start_tick;

    /* We work in ms (our tick resolution) but report as µs for display.
     * On platforms with µs ticks, this is exact; with ms ticks it's ×1000. */
    uint32_t elapsed_us = elapsed * 1000;

    e->total_us += elapsed_us;
    if (elapsed_us > e->peak_us) {
        e->peak_us = elapsed_us;
    }
    e->run_count++;
}

void syn_profiler_update(SYN_Profiler *prof)
{
    SYN_ASSERT(prof != NULL);

    uint32_t now = syn_port_get_tick_ms();
    uint32_t period = now - prof->period_start;
    prof->period_ms = period;

    if (period == 0) period = 1; /* avoid divide by zero */

    /* Calculate CPU percentages (×10 for 0.1% resolution) */
    for (uint8_t i = 0; i < prof->capacity; i++) {
        SYN_ProfileEntry *e = &prof->entries[i];
        if (e->name == NULL) continue;

        /* total_us is in microseconds, period is in ms */
        /* cpu% = (total_us / 1000) / period * 100
         *      = total_us / (period * 10)
         * cpu% × 10 = total_us / period                    */
        e->cpu_percent_x10 = (uint16_t)(e->total_us / period);
    }

    /* Reset for next period (keep peak) */
    for (uint8_t i = 0; i < prof->capacity; i++) {
        prof->entries[i].total_us  = 0;
        prof->entries[i].run_count = 0;
        /* peak_us is NOT reset — it's an all-time high watermark */
    }
    prof->period_start = now;
}

void syn_profiler_dump(const SYN_Profiler *prof,
                        SYN_ProfilerPrintFunc print)
{
    SYN_ASSERT(prof != NULL);
    if (print == NULL) return;

    char buf[80];
    char num[12];
    size_t pos;

    /* Header */
    pos = 0;
    const char *h1[] = { "--- Task Profiler (period=", NULL, "ms) ---\n" };
    syn_fmt_uint(num, sizeof(num), (uint32_t)prof->period_ms);
    h1[1] = num;
    syn_fmt_concat(buf, sizeof(buf), h1, 3);
    print(buf);

    print("Task          CPU%     Peak(us)     Runs\n");

    for (uint8_t i = 0; i < prof->capacity; i++) {
        const SYN_ProfileEntry *e = &prof->entries[i];
        if (e->name == NULL) continue;

        /* Build line: "name          XX.X%   peak     runs\n" */
        pos = 0;

        /* Name (pad to 14 chars) */
        size_t nlen = strlen(e->name);
        if (nlen > 13) nlen = 13;
        memcpy(buf + pos, e->name, nlen);
        pos += nlen;
        while (pos < 14) buf[pos++] = ' ';

        /* CPU% as X.X% */
        syn_fmt_uint(num, sizeof(num), e->cpu_percent_x10 / 10);
        size_t nl = strlen(num);
        memcpy(buf + pos, num, nl); pos += nl;
        buf[pos++] = '.';
        buf[pos++] = (char)('0' + (e->cpu_percent_x10 % 10));
        buf[pos++] = '%';
        while (pos < 22) buf[pos++] = ' ';

        /* Peak (us) */
        syn_fmt_uint(num, sizeof(num), (uint32_t)e->peak_us);
        nl = strlen(num);
        size_t pad = (8 > nl) ? 8 - nl : 0;
        while (pad-- > 0) buf[pos++] = ' ';
        memcpy(buf + pos, num, nl); pos += nl;
        while (pos < 35) buf[pos++] = ' ';

        /* Runs */
        syn_fmt_uint(num, sizeof(num), (uint32_t)e->run_count);
        nl = strlen(num);
        pad = (8 > nl) ? 8 - nl : 0;
        while (pad-- > 0) buf[pos++] = ' ';
        memcpy(buf + pos, num, nl); pos += nl;

        buf[pos++] = '\n';
        buf[pos] = '\0';
        print(buf);
    }
}

void syn_profiler_enable(SYN_Profiler *prof, bool enable)
{
    SYN_ASSERT(prof != NULL);
    prof->enabled = enable;
}

#endif /* SYN_USE_PROFILER */
