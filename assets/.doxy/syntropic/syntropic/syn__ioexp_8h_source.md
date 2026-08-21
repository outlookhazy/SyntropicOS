

# File syn\_ioexp.h

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_ioexp.h**](syn__ioexp_8h.md)

[Go to the documentation of this file](syn__ioexp_8h.md)


```C++

#ifndef SYN_IOEXP_H
#define SYN_IOEXP_H

#include "../common/syn_defs.h"
#include "../drivers/syn_soft_i2c.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_IOEXP_MCP23017 = 0, 
    SYN_IOEXP_MCP23008 = 1, 
    SYN_IOEXP_PCF8574 = 2,  
    SYN_IOEXP_TCA9555 = 3   
} SYN_IOExpType;

typedef struct {
    SYN_IOExpType type;   
    SYN_SoftI2C i2c;      
    uint8_t i2c_addr;     
    uint8_t num_pins;     
    uint16_t out_mask;    
    uint16_t dir_mask;    
    uint16_t pullup_mask; 
} SYN_IOExp;

SYN_Status syn_ioexp_init(SYN_IOExp *ioexp, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint8_t i2c_addr,
                          SYN_IOExpType type);

void syn_ioexp_set_pin_mode(SYN_IOExp *ioexp, uint8_t pin, SYN_GPIO_Mode mode);

void syn_ioexp_write_pin(SYN_IOExp *ioexp, uint8_t pin, SYN_GPIO_State state);

SYN_GPIO_State syn_ioexp_read_pin(SYN_IOExp *ioexp, uint8_t pin);

void syn_ioexp_write_port(SYN_IOExp *ioexp, uint16_t mask);

uint16_t syn_ioexp_read_port(SYN_IOExp *ioexp);

#ifdef __cplusplus
}
#endif

#endif /* SYN_IOEXP_H */
```


