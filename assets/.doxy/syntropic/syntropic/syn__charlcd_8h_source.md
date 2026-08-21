

# File syn\_charlcd.h

[**File List**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_charlcd.h**](syn__charlcd_8h.md)

[Go to the documentation of this file](syn__charlcd_8h.md)


```C++

#ifndef SYN_CHARLCD_H
#define SYN_CHARLCD_H

#include "../common/syn_defs.h"
#include "../drivers/syn_soft_i2c.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_CHARLCD_MODE_I2C = 0,  
    SYN_CHARLCD_MODE_GPIO_4BIT 
} SYN_CharLCDMode;

typedef struct {
    SYN_CharLCDMode mode;    
    uint8_t cols;            
    uint8_t rows;            
    uint8_t display_control; 
    uint8_t display_mode;    
    bool backlight;          
    /* I2C Mode Context */
    SYN_SoftI2C i2c;        
    uint8_t i2c_addr;       
    uint8_t backlight_mask; 
    /* 4-Bit GPIO Mode Context */
    SYN_GPIO_Pin rs_pin;    
    SYN_GPIO_Pin en_pin;    
    SYN_GPIO_Pin d_pins[4]; 
} SYN_CharLCD;

SYN_Status syn_charlcd_init_i2c(SYN_CharLCD *lcd, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda,
                                uint8_t i2c_addr, uint8_t cols, uint8_t rows);

SYN_Status syn_charlcd_init_gpio(SYN_CharLCD *lcd, SYN_GPIO_Pin rs, SYN_GPIO_Pin en,
                                 SYN_GPIO_Pin d4, SYN_GPIO_Pin d5, SYN_GPIO_Pin d6, SYN_GPIO_Pin d7,
                                 uint8_t cols, uint8_t rows);

void syn_charlcd_clear(SYN_CharLCD *lcd);

void syn_charlcd_set_cursor(SYN_CharLCD *lcd, uint8_t col, uint8_t row);

void syn_charlcd_print(SYN_CharLCD *lcd, const char *str);

void syn_charlcd_set_backlight(SYN_CharLCD *lcd, bool enable);

void syn_charlcd_create_char(SYN_CharLCD *lcd, uint8_t slot, const uint8_t charmap[8]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CHARLCD_H */
```


