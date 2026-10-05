

# File syn\_biquad.h

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_biquad.h**](syn__biquad_8h.md)

[Go to the documentation of this file](syn__biquad_8h.md)


```C++

#ifndef SYN_BIQUAD_H
#define SYN_BIQUAD_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    q16_t b0; 
    q16_t b1; 
    q16_t b2; 
    q16_t a1; 
    q16_t a2; 
    q16_t x1; 
    q16_t x2; 
    q16_t y1; 
    q16_t y2; 
} SYN_FilterBiquad;

void syn_filter_biquad_init(SYN_FilterBiquad *f, q16_t b0, q16_t b1, q16_t b2, q16_t a1, q16_t a2);

void syn_filter_biquad_reset(SYN_FilterBiquad *f);

q16_t syn_filter_biquad_update(SYN_FilterBiquad *f, q16_t sample);

void syn_filter_biquad_process_block(SYN_FilterBiquad *f, const q16_t *in, q16_t *out,
                                     uint16_t count);

void syn_filter_biquad_lowpass(SYN_FilterBiquad *f, q16_t fc, q16_t fs);

void syn_filter_biquad_highpass(SYN_FilterBiquad *f, q16_t fc, q16_t fs);

void syn_filter_biquad_bandpass(SYN_FilterBiquad *f, q16_t fc, q16_t fs, q16_t q);

void syn_filter_biquad_notch(SYN_FilterBiquad *f, q16_t fc, q16_t fs, q16_t q);

void syn_filter_biquad_allpass(SYN_FilterBiquad *f, q16_t fc, q16_t fs, q16_t q);

void syn_filter_biquad_peaking_eq(SYN_FilterBiquad *f, q16_t fc, q16_t fs, q16_t gain_db, q16_t q);

/* ── Cascaded Biquad Filter Bank ─────────────────────────────────────────── */

#define SYN_BIQUAD_CASCADE_MAX_STAGES 8 
typedef struct {
    SYN_FilterBiquad stages[SYN_BIQUAD_CASCADE_MAX_STAGES]; 
    uint8_t num_stages;                                     
} SYN_FilterBiquadCascade;

SYN_Status syn_filter_biquad_cascade_init(SYN_FilterBiquadCascade *c, uint8_t num_stages);

void syn_filter_biquad_cascade_reset(SYN_FilterBiquadCascade *c);

q16_t syn_filter_biquad_cascade_update(SYN_FilterBiquadCascade *c, q16_t sample);

void syn_filter_biquad_cascade_process_block(SYN_FilterBiquadCascade *c, const q16_t *in,
                                             q16_t *out, uint16_t count);

#ifdef __cplusplus
}
#endif

#endif /* SYN_BIQUAD_H */
```


