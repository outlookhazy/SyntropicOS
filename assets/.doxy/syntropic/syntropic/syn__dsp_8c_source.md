

# File syn\_dsp.c

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_dsp.c**](syn__dsp_8c.md)

[Go to the documentation of this file](syn__dsp_8c.md)


```C++

#include "syn_dsp.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
SYN_Status syn_dsp_dct2_q7(const q7_t *time_series, size_t num_samples, q7_t *dct_coeffs,
                           size_t num_coeffs)
{
    if (!time_series || !dct_coeffs || num_samples == 0 || num_coeffs == 0 ||
        num_coeffs > num_samples) {
        return SYN_INVALID_PARAM;
    }

    double inv_n = 1.0 / (double)num_samples;
    double scale_dc = sqrt(inv_n);
    double scale_ac = sqrt(2.0 * inv_n);

    for (size_t k = 0; k < num_coeffs; k++) {
        q16_t acc = 0;
        double scale = (k == 0) ? scale_dc : scale_ac;

        for (size_t n = 0; n < num_samples; n++) {
            double angle = (M_PI * (double)k * ((double)n + 0.5)) * inv_n;
            double cos_val = cos(angle);
            q7_t cos_q7 = Q7_FROM_FLOAT((float)cos_val);

            acc = q7_mac(acc, time_series[n], cos_q7);
        }

        /* Scale and convert accumulated Q16 back to Q7 */
        double acc_float = (double)acc / 65536.0;
        double scaled_val = acc_float * scale;
        dct_coeffs[k] = Q7_FROM_FLOAT((float)scaled_val);
    }

    return SYN_OK;
}
```


