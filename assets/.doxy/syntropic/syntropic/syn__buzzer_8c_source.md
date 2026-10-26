

# File syn\_buzzer.c

[**File List**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_buzzer.c**](syn__buzzer_8c.md)

[Go to the documentation of this file](syn__buzzer_8c.md)


```C++

#include "syn_buzzer.h"

#include "../util/syn_assert.h"

#include <string.h>

static void set_buzzer_state(SYN_Buzzer *buz, bool state)
{
    if (buz == NULL)
        return; /* LCOV_EXCL_LINE: Unreachable static helper guard; buz is verified non-NULL by
                   caller */
    bool pin_val = buz->active_high ? state : !state;
    syn_port_gpio_write(buz->pin, pin_val ? SYN_GPIO_HIGH : SYN_GPIO_LOW);
}

SYN_Status syn_buzzer_init(SYN_Buzzer *buz, SYN_GPIO_Pin pin, bool active_high)
{
    SYN_ASSERT(buz != NULL);

    if (buz ==
        NULL) { /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in release mode */
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in
                                     release mode */
    }

    memset(buz, 0, sizeof(*buz));
    buz->pin = pin;
    buz->active_high = active_high;

    syn_port_gpio_init(pin, SYN_GPIO_OUTPUT);
    set_buzzer_state(buz, false);

    return SYN_OK;
}

SYN_Status syn_buzzer_beep(SYN_Buzzer *buz, uint32_t freq_hz, uint32_t duration_ms)
{
    SYN_ASSERT(buz != NULL);
    if (buz ==
        NULL) { /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in release mode */
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in
                                     release mode */
    }

    buz->pattern_freqs = NULL;
    buz->pattern_durs = NULL;
    buz->pattern_count = 0;
    buz->pattern_index = 0;

    buz->freq_hz = freq_hz;
    buz->duration_ms = duration_ms;
    buz->elapsed_ms = 0;
    buz->is_playing = (duration_ms > 0);

    if (buz->is_playing && freq_hz > 0) {
        set_buzzer_state(buz, true);
    } else {
        set_buzzer_state(buz, false);
    }

    return SYN_OK;
}

SYN_Status syn_buzzer_play_pattern(SYN_Buzzer *buz, const uint16_t *freqs, const uint16_t *durs,
                                   size_t count)
{
    SYN_ASSERT(buz != NULL);
    SYN_ASSERT(freqs != NULL);
    SYN_ASSERT(durs != NULL);
    SYN_ASSERT(count > 0);

    if (buz == NULL || freqs == NULL || durs == NULL ||
        count ==
            0) { /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in release mode */
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in
                                     release mode */
    }

    buz->pattern_freqs = freqs;
    buz->pattern_durs = durs;
    buz->pattern_count = count;
    buz->pattern_index = 0;

    buz->freq_hz = freqs[0];
    buz->duration_ms = durs[0];
    buz->elapsed_ms = 0;
    buz->is_playing = (buz->duration_ms > 0);

    if (buz->is_playing && buz->freq_hz > 0) {
        set_buzzer_state(buz, true);
    } else {
        set_buzzer_state(buz, false);
    }

    return SYN_OK;
}

void syn_buzzer_stop(SYN_Buzzer *buz)
{
    if (buz == NULL)
        return;

    buz->is_playing = false;
    buz->pattern_count = 0;
    buz->pattern_index = 0;
    set_buzzer_state(buz, false);
}

void syn_buzzer_step(SYN_Buzzer *buz, uint32_t dt_ms)
{
    if (buz == NULL || !buz->is_playing)
        return;

    buz->elapsed_ms += dt_ms;

    if (buz->elapsed_ms >= buz->duration_ms) {
        /* Check if playing a multi-note pattern */
        if (buz->pattern_count > 0) {
            buz->pattern_index++;
            if (buz->pattern_index < buz->pattern_count) {
                buz->freq_hz = buz->pattern_freqs[buz->pattern_index];
                buz->duration_ms = buz->pattern_durs[buz->pattern_index];
                buz->elapsed_ms = 0;

                if (buz->freq_hz > 0) {
                    set_buzzer_state(buz, true);
                } else {
                    set_buzzer_state(buz, false);
                }
                return;
            }
        }

        /* Pattern complete or single beep finished */
        syn_buzzer_stop(buz);
    }
}

bool syn_buzzer_is_playing(const SYN_Buzzer *buz)
{
    return buz != NULL && buz->is_playing;
}
```


