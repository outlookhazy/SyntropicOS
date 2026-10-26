

# File syn\_shiftreg.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_shiftreg.h**](syn__shiftreg_8h.md)

[Go to the documentation of this file](syn__shiftreg_8h.md)


```C++

#ifndef SYN_SHIFTREG_H
#define SYN_SHIFTREG_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_SHIFTREG_MAX_CHIPS 8 
/* ── Output Shift Register (e.g. 74HC595, CD4094, TPIC6C596) ───────────── */

typedef struct {
    SYN_GPIO_Pin data_pin;                  
    SYN_GPIO_Pin clock_pin;                 
    SYN_GPIO_Pin latch_pin;                 
    SYN_GPIO_Pin oe_pin;                    
    uint8_t num_chips;                      
    bool msb_first;                         
    uint8_t buffer[SYN_SHIFTREG_MAX_CHIPS]; 
} SYN_ShiftRegOut;

SYN_Status syn_shiftreg_out_init(SYN_ShiftRegOut *sr, SYN_GPIO_Pin data_pin, SYN_GPIO_Pin clock_pin,
                                 SYN_GPIO_Pin latch_pin, uint8_t num_chips);

void syn_shiftreg_out_set_oe_pin(SYN_ShiftRegOut *sr, SYN_GPIO_Pin oe_pin);

void syn_shiftreg_out_set_bit_order(SYN_ShiftRegOut *sr, bool msb_first);

void syn_shiftreg_out_set_bit(SYN_ShiftRegOut *sr, uint16_t bit_index, bool state);

void syn_shiftreg_out_write_byte(SYN_ShiftRegOut *sr, uint8_t chip_index, uint8_t val);

void syn_shiftreg_out_flush(SYN_ShiftRegOut *sr);

void syn_shiftreg_out_set_enable(SYN_ShiftRegOut *sr, bool enable);

/* ── Input Shift Register (e.g. 74HC165, CD4021) ────────────────────────── */

typedef struct {
    SYN_GPIO_Pin data_pin;                  
    SYN_GPIO_Pin clock_pin;                 
    SYN_GPIO_Pin load_pin;                  
    uint8_t num_chips;                      
    bool msb_first;                         
    uint8_t buffer[SYN_SHIFTREG_MAX_CHIPS]; 
} SYN_ShiftRegIn;

SYN_Status syn_shiftreg_in_init(SYN_ShiftRegIn *sr, SYN_GPIO_Pin data_pin, SYN_GPIO_Pin clock_pin,
                                SYN_GPIO_Pin load_pin, uint8_t num_chips);

void syn_shiftreg_in_set_bit_order(SYN_ShiftRegIn *sr, bool msb_first);

void syn_shiftreg_in_read(SYN_ShiftRegIn *sr);

bool syn_shiftreg_in_get_bit(const SYN_ShiftRegIn *sr, uint16_t bit_index);

uint8_t syn_shiftreg_in_get_byte(const SYN_ShiftRegIn *sr, uint8_t chip_index);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SHIFTREG_H */
```


