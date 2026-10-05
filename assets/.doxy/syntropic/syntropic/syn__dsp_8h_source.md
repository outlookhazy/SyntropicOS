

# File syn\_dsp.h

[**File List**](files.md) **>** [**dsp**](dir_4d73dd81b532b98734987ed09a51400b.md) **>** [**syn\_dsp.h**](syn__dsp_8h.md)

[Go to the documentation of this file](syn__dsp_8h.md)


```C++

#ifndef SYN_DSP_H
#define SYN_DSP_H

#include "../util/syn_qmath.h"
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


