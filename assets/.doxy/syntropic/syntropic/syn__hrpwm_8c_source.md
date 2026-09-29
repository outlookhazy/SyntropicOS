

# File syn\_hrpwm.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_hrpwm.c**](syn__hrpwm_8c.md)

[Go to the documentation of this file](syn__hrpwm_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_HRPWM) || SYN_USE_HRPWM

#include "../util/syn_assert.h"
#include "syn_hrpwm.h"

SYN_Status syn_hrpwm_init(SYN_HRPWM *hrpwm, uint8_t channel, uint32_t freq_hz)
{
    if (hrpwm == NULL || freq_hz == 0) {
        return SYN_INVALID_PARAM;
    }

    hrpwm->channel = channel;
    hrpwm->freq_hz = freq_hz;
    hrpwm->rise_ns = 0;
    hrpwm->fall_ns = 0;
    hrpwm->phase_deg = 0;
    hrpwm->enabled = false;

    return syn_port_hrpwm_init(channel, freq_hz);
}

SYN_Status syn_hrpwm_set_duty_q16(const SYN_HRPWM *hrpwm, int32_t duty_q16)
{
    if (hrpwm == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (duty_q16 < 0)
        duty_q16 = 0;
    if (duty_q16 > 65536)
        duty_q16 = 65536;

    return syn_port_hrpwm_set_duty_q16(hrpwm->channel, duty_q16);
}

SYN_Status syn_hrpwm_set_duty_float(const SYN_HRPWM *hrpwm, float duty_float)
{
    if (hrpwm == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (duty_float < 0.0f)
        duty_float = 0.0f;
    if (duty_float > 1.0f)
        duty_float = 1.0f;

    int32_t duty_q16 = (int32_t)(duty_float * 65536.0f);
    return syn_port_hrpwm_set_duty_q16(hrpwm->channel, duty_q16);
}

SYN_Status syn_hrpwm_set_deadtime_ns(SYN_HRPWM *hrpwm, uint16_t rise_ns, uint16_t fall_ns)
{
    if (hrpwm == NULL) {
        return SYN_INVALID_PARAM;
    }

    hrpwm->rise_ns = rise_ns;
    hrpwm->fall_ns = fall_ns;

    return syn_port_hrpwm_set_deadtime_ns(hrpwm->channel, rise_ns, fall_ns);
}

SYN_Status syn_hrpwm_set_phase_deg(SYN_HRPWM *hrpwm, uint16_t phase_deg)
{
    if (hrpwm == NULL || phase_deg > 360) {
        return SYN_INVALID_PARAM;
    }

    hrpwm->phase_deg = phase_deg;
    return syn_port_hrpwm_set_phase_deg(hrpwm->channel, phase_deg);
}

SYN_Status syn_hrpwm_bind_fault(const SYN_HRPWM *hrpwm, uint8_t fault_id)
{
    if (hrpwm == NULL) {
        return SYN_INVALID_PARAM;
    }

    return syn_port_hrpwm_enable_fault(hrpwm->channel, fault_id, true);
}

SYN_Status syn_hrpwm_enable(SYN_HRPWM *hrpwm, bool enable)
{
    if (hrpwm == NULL) {
        return SYN_INVALID_PARAM;
    }

    hrpwm->enabled = enable;
    return syn_port_hrpwm_enable(hrpwm->channel, enable);
}

#endif /* SYN_USE_HRPWM */
```


