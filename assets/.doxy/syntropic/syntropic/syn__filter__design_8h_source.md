

# File syn\_filter\_design.h

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_filter\_design.h**](syn__filter__design_8h.md)

[Go to the documentation of this file](syn__filter__design_8h.md)


```C++

#ifndef SYN_FILTER_DESIGN_H
#define SYN_FILTER_DESIGN_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"
#include "syn_biquad.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    q16_t b0; 
    q16_t b1; 
    q16_t b2; 
    q16_t a1; 
    q16_t a2; 
} SYN_BiquadCoeffs;

SYN_Status syn_filter_design_lowpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out);

SYN_Status syn_filter_design_highpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out);

SYN_Status syn_filter_design_bandpass(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out);

SYN_Status syn_filter_design_notch(q16_t fc_hz, q16_t fs_hz, q16_t Q, SYN_BiquadCoeffs *out);

SYN_Status syn_filter_design_apply_to_biquad(const SYN_BiquadCoeffs *coeffs, SYN_FilterBiquad *biquad);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FILTER_DESIGN_H */
```


