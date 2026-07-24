

# File syn\_fft.h

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_fft.h**](syn__fft_8h.md)

[Go to the documentation of this file](syn__fft_8h.md)


```C++

#ifndef SYN_FFT_H
#define SYN_FFT_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_dsp_fft(q16_t *real, q16_t *imag, uint16_t n);

SYN_Status syn_fft_window_hanning(q16_t *out, uint16_t n);

SYN_Status syn_fft_window_hamming(q16_t *out, uint16_t n);

SYN_Status syn_fft_window_blackman(q16_t *out, uint16_t n);

typedef struct {
    uint16_t bin;          
    q16_t    magnitude;    
    q16_t    freq_hz;      
} SYN_FFTPeak;

SYN_Status syn_fft_magnitude_spectrum(const q16_t *real, const q16_t *imag, q16_t *mag, uint16_t n);

SYN_Status syn_fft_find_peaks(const q16_t *mag, uint16_t n_bins, q16_t sample_rate_hz,
                              SYN_FFTPeak *peaks, uint8_t max_peaks, uint8_t *num_peaks_found);

SYN_Status syn_fft_thd(const q16_t *mag, uint16_t n_bins, uint16_t fundamental_bin,
                       uint8_t max_harmonics, q16_t *thd_pct);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FFT_H */
```


