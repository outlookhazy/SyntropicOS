

# File syn\_oled.h

[**File List**](files.md) **>** [**display**](dir_572b98f20fe1ab62e971e24ddc7f0bdb.md) **>** [**syn\_oled.h**](syn__oled_8h.md)

[Go to the documentation of this file](syn__oled_8h.md)


```C++

#ifndef SYN_OLED_H
#define SYN_OLED_H

#include "../common/syn_defs.h"
#include "../display/syn_canvas.h"
#include "../drivers/syn_soft_i2c.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_OLED_SSD1306 = 0, 
    SYN_OLED_SH1106 = 1,  
    SYN_OLED_SSD1309 = 2  
} SYN_OLEDType;

typedef struct {
    SYN_OLEDType type;  
    SYN_SoftI2C i2c;    
    uint8_t i2c_addr;   
    uint16_t width;     
    uint16_t height;    
    uint8_t col_offset; 
    bool inverted;      
    bool display_on;    
    uint8_t contrast;   
} SYN_OLED;

SYN_Status syn_oled_init(SYN_OLED *oled, SYN_GPIO_Pin scl, SYN_GPIO_Pin sda, uint8_t i2c_addr,
                         uint16_t w, uint16_t h, SYN_OLEDType type);

void syn_oled_set_contrast(SYN_OLED *oled, uint8_t contrast);

void syn_oled_set_invert(SYN_OLED *oled, bool invert);

void syn_oled_set_display_on(SYN_OLED *oled, bool on);

void syn_oled_flush_canvas(SYN_OLED *oled, const SYN_Canvas *c);

void syn_oled_canvas_flush_cb(const uint8_t *buf, size_t len, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_OLED_H */
```


