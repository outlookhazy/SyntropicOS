

# File syn\_dac.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_dac.h**](syn__dac_8h.md)

[Go to the documentation of this file](syn__dac_8h.md)


```C++

#ifndef SYN_DAC_H
#define SYN_DAC_H

#include <stdint.h>
#include "../common/syn_defs.h"
#include "../port/syn_port_dac.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t channel;  
} SYN_DAC;

SYN_Status syn_dac_init(SYN_DAC *dac, uint8_t channel);

SYN_Status syn_dac_write_raw(const SYN_DAC *dac, uint16_t raw);

SYN_Status syn_dac_write_mv(const SYN_DAC *dac, uint16_t mv);

SYN_Status syn_dac_write_percent(const SYN_DAC *dac, uint8_t percent);

uint16_t syn_dac_max_raw(const SYN_DAC *dac);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DAC_H */
```


