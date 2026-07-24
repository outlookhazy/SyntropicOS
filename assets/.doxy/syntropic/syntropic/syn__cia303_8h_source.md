

# File syn\_cia303.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia303.h**](syn__cia303_8h.md)

[Go to the documentation of this file](syn__cia303_8h.md)


```C++

#ifndef SYN_CIA303_H
#define SYN_CIA303_H

#include "../common/syn_defs.h"
#include "../output/syn_led.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_CIA303_RUN_OFF          = 0, 
    SYN_CIA303_RUN_SINGLE_FLASH = 1, 
    SYN_CIA303_RUN_BLINKING     = 2, 
    SYN_CIA303_RUN_SOLID_ON     = 3, 
} SYN_CiA303_RunState;

typedef enum {
    SYN_CIA303_ERR_OFF          = 0, 
    SYN_CIA303_ERR_SINGLE_FLASH = 1, 
    SYN_CIA303_ERR_DOUBLE_FLASH = 2, 
    SYN_CIA303_ERR_TRIPLE_FLASH = 3, 
    SYN_CIA303_ERR_SOLID_ON     = 4, 
} SYN_CiA303_ErrState;

typedef struct {
    SYN_LED             *run_led;   
    SYN_LED             *err_led;   
    SYN_CiA303_RunState  run_state; 
    SYN_CiA303_ErrState  err_state; 
} SYN_CiA303_Indicator;

void syn_cia303_init(SYN_CiA303_Indicator *ind, SYN_LED *run_led, SYN_LED *err_led);

void syn_cia303_set_nmt_state(SYN_CiA303_Indicator *ind, uint8_t nmt_state);

void syn_cia303_set_error_state(SYN_CiA303_Indicator *ind, SYN_CiA303_ErrState err_state);

void syn_cia303_step(SYN_CiA303_Indicator *ind);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CIA303_H */
```


