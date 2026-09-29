

# File syn\_seg7.h

[**File List**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_seg7.h**](syn__seg7_8h.md)

[Go to the documentation of this file](syn__seg7_8h.md)


```C++

#ifndef SYN_SEG7_H
#define SYN_SEG7_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_SEG7_MAX_DIGITS 8 
typedef enum {
    SYN_SEG7_COMMON_CATHODE = 0, 
    SYN_SEG7_COMMON_ANODE = 1    
} SYN_Seg7Type;

typedef struct {
    SYN_GPIO_Pin segment_pins[8];                 
    SYN_GPIO_Pin digit_pins[SYN_SEG7_MAX_DIGITS]; 
    uint8_t num_digits;                           
    SYN_Seg7Type type;                            
    uint8_t digit_buffers[SYN_SEG7_MAX_DIGITS];   
    uint8_t active_digit;                         
    bool leading_zeros;                           
} SYN_Seg7;

SYN_Status syn_seg7_init(SYN_Seg7 *seg, const SYN_GPIO_Pin segments[8], const SYN_GPIO_Pin *digits,
                         uint8_t num_digits, SYN_Seg7Type type);

void syn_seg7_scan(SYN_Seg7 *seg);

void syn_seg7_clear(SYN_Seg7 *seg);

void syn_seg7_print_int(SYN_Seg7 *seg, int32_t val);

void syn_seg7_print_float(SYN_Seg7 *seg, float val, uint8_t decimals);

void syn_seg7_print_hex(SYN_Seg7 *seg, uint32_t val);

void syn_seg7_print_str(SYN_Seg7 *seg, const char *str);

void syn_seg7_set_digit_raw(SYN_Seg7 *seg, uint8_t digit_idx, uint8_t seg_mask);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SEG7_H */
```


