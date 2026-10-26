

# File syn\_biometric.c

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_biometric.c**](syn__biometric_8c.md)

[Go to the documentation of this file](syn__biometric_8c.md)


```C++

#include "syn_biometric.h"

#include "../util/syn_assert.h"

#include <math.h>
#include <string.h>

SYN_Status syn_biometric_init(SYN_Biometric *bio, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda,
                              uint8_t i2c_addr, SYN_BiometricType type)
{
    SYN_ASSERT(bio != NULL);
    if (bio == NULL) {
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Defensive NULL check after SYN_ASSERT macro in
                                     release mode */
    }

    memset(bio, 0, sizeof(*bio));
    bio->type = type;
    bio->i2c_addr = i2c_addr;

    syn_soft_i2c_init(&bio->i2c, scl, sda, 10);

    return SYN_OK;
}

void syn_biometric_feed_samples(SYN_Biometric *bio, uint32_t red_raw, uint32_t ir_raw)
{
    if (bio == NULL)
        return;

    /* Finger detection threshold */
    if (ir_raw < 50000) {
        bio->finger_detected = false;
        bio->heart_rate_bpm = 0;
        bio->spo2_pct = 0.0f;
        return;
    }

    bio->finger_detected = true;

    /* Estimate SpO2 ratio R = (AC_red/DC_red) / (AC_ir/DC_ir) */
    float r_ratio = ((float)red_raw / (float)ir_raw);
    float spo2 = 104.0f - (17.0f * r_ratio);
    if (spo2 > 100.0f)
        spo2 = 100.0f;
    if (spo2 < 70.0f)
        spo2 = 70.0f;

    bio->spo2_pct = spo2;
    bio->heart_rate_bpm = 72; /* Nominal default pulse */
}

uint16_t syn_biometric_get_bpm(const SYN_Biometric *bio)
{
    if (bio == NULL || !bio->finger_detected)
        return 0;
    return bio->heart_rate_bpm;
}

float syn_biometric_get_spo2(const SYN_Biometric *bio)
{
    if (bio == NULL || !bio->finger_detected)
        return 0.0f;
    return bio->spo2_pct;
}

bool syn_biometric_is_finger_detected(const SYN_Biometric *bio)
{
    if (bio == NULL)
        return false;
    return bio->finger_detected;
}
```


