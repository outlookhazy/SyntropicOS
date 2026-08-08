

# File syn\_mfcc.h

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_mfcc.h**](syn__mfcc_8h.md)

[Go to the documentation of this file](syn__mfcc_8h.md)


```C++

#ifndef SYN_MFCC_H
#define SYN_MFCC_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/dsp/syn_fft.h"
#include "syntropic/util/syn_dsp.h"
#include "syntropic/util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_MFCC_MAX_FFT_SIZE 128U   
#define SYN_MFCC_NUM_MEL_FILTERS 26U 
#define SYN_MFCC_NUM_COEFFS 13U      
typedef struct {
    uint16_t sample_rate_hz;                      
    uint16_t fft_size;                            
    q16_t fft_real[SYN_MFCC_MAX_FFT_SIZE];        
    q16_t fft_imag[SYN_MFCC_MAX_FFT_SIZE];        
    q16_t mel_energies[SYN_MFCC_NUM_MEL_FILTERS]; 
    q7_t mfcc_coeffs[SYN_MFCC_NUM_COEFFS];        
} SYN_MFCC;

SYN_Status syn_mfcc_init(SYN_MFCC *mfcc, uint16_t sample_rate_hz, uint16_t fft_size);

SYN_Status syn_mfcc_process_frame(SYN_MFCC *mfcc, const int16_t *pcm_in,
                                  q7_t mfcc_out[SYN_MFCC_NUM_COEFFS]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MFCC_H */
```


