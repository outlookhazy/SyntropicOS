

# File syn\_control\_stats.c

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_control\_stats.c**](syn__control__stats_8c.md)

[Go to the documentation of this file](syn__control__stats_8c.md)


```C++
#include "syntropic/control/syn_control_stats.h"
#include "syntropic/util/syn_assert.h"
#include <math.h>
#include <string.h>

void syn_control_stats_reset(SYN_ControlStats *stats)
{
    SYN_ASSERT(stats != NULL);
    memset(stats, 0, sizeof(*stats));
}

void syn_control_stats_update(SYN_ControlStats *stats, int32_t error, int32_t output)
{
    SYN_ASSERT(stats != NULL);

    int32_t abs_err = (error < 0) ? -error : error;
    int32_t abs_out = (output < 0) ? -output : output;
    
    /* Accumulate error stats */
    stats->sum_sq_err += (int64_t)error * error;
    if (abs_err > stats->max_error) {
        stats->max_error = abs_err;
    }

    /* Accumulate efficiency (effort) stats */
    stats->sum_sq_out += (int64_t)output * output;

    /* Accumulate jitter (stress) stats */
    if (stats->samples > 0) {
        int32_t delta = output - stats->last_output;
        stats->sum_abs_delta += (delta < 0) ? -delta : delta;
    }
    stats->last_output = output;

    /* Accumulate precision (ITAE) stats 
     * We use sample index as a proxy for time. */
    stats->sum_itae += (int64_t)stats->samples * abs_err;

    stats->samples++;
}

void syn_control_stats_report(const SYN_ControlStats *stats, SYN_ControlReport *report)
{
    SYN_ASSERT(stats != NULL);
    SYN_ASSERT(report != NULL);

    if (stats->samples == 0) {
        memset(report, 0, sizeof(*report));
        return;
    }

    report->max_error = stats->max_error;
    
    /* RMS Error */
    report->rms_error = (int32_t)sqrt((double)stats->sum_sq_err / stats->samples);

    /* Control Effort (scaled to 0-10000 for 0.00% to 100.00%) */
    /* Assumes output is in range -100 to 100. If larger, we scale down. */
    double rms_out = sqrt((double)stats->sum_sq_out / stats->samples);
    report->control_effort = (int32_t)(rms_out * 100.0);

    /* Jitter (Average delta) */
    report->jitter = (int32_t)(stats->sum_abs_delta / stats->samples);

    /* ITAE */
    report->itae = (int32_t)(stats->sum_itae / stats->samples);
}
```


