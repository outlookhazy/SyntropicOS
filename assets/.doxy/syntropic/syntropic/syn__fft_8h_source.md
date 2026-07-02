

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

#ifdef __cplusplus
}
#endif

#endif /* SYN_FFT_H */
```


