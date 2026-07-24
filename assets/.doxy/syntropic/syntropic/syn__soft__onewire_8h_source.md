

# File syn\_soft\_onewire.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_soft\_onewire.h**](syn__soft__onewire_8h.md)

[Go to the documentation of this file](syn__soft__onewire_8h.md)


```C++

#ifndef SYN_SOFT_ONEWIRE_H
#define SYN_SOFT_ONEWIRE_H

#include <stdint.h>
#include <stdbool.h>
#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Timing calibration macro ──────────────────────────────────────────── */

#define SYN_SOFT_ONEWIRE_LOOPS_PER_US(freq_hz) \
    ((uint32_t)(((uint32_t)(freq_hz)) / 4000000UL))

typedef struct {
    SYN_GPIO_Pin pin;          
    uint32_t     delay_loops;  
} SYN_SoftOneWire;

void syn_soft_onewire_init(SYN_SoftOneWire *ow,
                           SYN_GPIO_Pin     pin,
                           uint32_t         delay_loops);

bool syn_soft_onewire_reset(const SYN_SoftOneWire *ow);

void syn_soft_onewire_write_byte(const SYN_SoftOneWire *ow, uint8_t byte);

uint8_t syn_soft_onewire_read_byte(const SYN_SoftOneWire *ow);

void syn_soft_onewire_write_rom(const SYN_SoftOneWire *ow,
                                const uint8_t          rom[8]);

void syn_soft_onewire_read_rom(const SYN_SoftOneWire *ow,
                               uint8_t                rom[8]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SOFT_ONEWIRE_H */
```


