

# File syn\_joystick.h

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_joystick.h**](syn__joystick_8h.md)

[Go to the documentation of this file](syn__joystick_8h.md)


```C++

#ifndef SYN_JOYSTICK_H
#define SYN_JOYSTICK_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_JOYSTICK_DIR_CENTER = 0,
    SYN_JOYSTICK_DIR_UP = 1,
    SYN_JOYSTICK_DIR_UP_RIGHT = 2,
    SYN_JOYSTICK_DIR_RIGHT = 3,
    SYN_JOYSTICK_DIR_DOWN_RIGHT = 4,
    SYN_JOYSTICK_DIR_DOWN = 5,
    SYN_JOYSTICK_DIR_DOWN_LEFT = 6,
    SYN_JOYSTICK_DIR_LEFT = 7,
    SYN_JOYSTICK_DIR_UP_LEFT = 8
} SYN_JoystickDir;

typedef struct {
    uint16_t center_x;     
    uint16_t center_y;     
    uint16_t adc_max;      
    uint16_t deadband;     
    int16_t current_x_pct; 
    int16_t current_y_pct; 
    SYN_JoystickDir dir;   
    bool button_pressed;   
} SYN_Joystick;

SYN_Status syn_joystick_init(SYN_Joystick *joy, uint16_t center_x, uint16_t center_y,
                             uint16_t adc_max, uint16_t deadband);

void syn_joystick_feed_adc(SYN_Joystick *joy, uint16_t raw_x, uint16_t raw_y, bool btn);

int16_t syn_joystick_get_x_pct(const SYN_Joystick *joy);

int16_t syn_joystick_get_y_pct(const SYN_Joystick *joy);

SYN_JoystickDir syn_joystick_get_dir(const SYN_Joystick *joy);

#ifdef __cplusplus
}
#endif

#endif /* SYN_JOYSTICK_H */
```


