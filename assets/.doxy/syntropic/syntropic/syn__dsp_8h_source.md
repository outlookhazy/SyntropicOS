

# File syn\_dsp.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_dsp.h**](syn__dsp_8h.md)

[Go to the documentation of this file](syn__dsp_8h.md)


```C++

#ifndef SYN_DSP_H
#define SYN_DSP_H

#include "syn_qmath.h"
#include "syntropic/common/syn_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_dsp_dct2_q7(const q7_t *time_series, size_t num_samples, q7_t *dct_coeffs,
                           size_t num_coeffs);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DSP_H */
```


