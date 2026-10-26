

# File syn\_dipswitch.h

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_dipswitch.h**](syn__dipswitch_8h.md)

[Go to the documentation of this file](syn__dipswitch_8h.md)


```C++

#ifndef SYN_DIPSWITCH_H
#define SYN_DIPSWITCH_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_DIPSWITCH_MAX_PINS 16 
typedef struct {
    SYN_GPIO_Pin pins[SYN_DIPSWITCH_MAX_PINS]; 
    uint8_t count;                             
    bool active_low;                           
    uint32_t current_value;                    
    uint32_t previous_value;                   
    bool changed;                              
} SYN_DipSwitch;

SYN_Status syn_dipswitch_init(SYN_DipSwitch *ds, const SYN_GPIO_Pin *pins, uint8_t count,
                              bool active_low);

void syn_dipswitch_read(SYN_DipSwitch *ds);

uint32_t syn_dipswitch_get_value(const SYN_DipSwitch *ds);

bool syn_dipswitch_has_changed(SYN_DipSwitch *ds);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DIPSWITCH_H */
```


