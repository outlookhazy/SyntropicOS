

# File syn\_keypad.h

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_keypad.h**](syn__keypad_8h.md)

[Go to the documentation of this file](syn__keypad_8h.md)


```C++

#ifndef SYN_KEYPAD_H
#define SYN_KEYPAD_H

#include "../common/syn_defs.h"
#include "../port/syn_port_gpio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_KEYPAD_MAX_ROWS 8 
#define SYN_KEYPAD_MAX_COLS 8 
typedef struct SYN_Keypad SYN_Keypad;

typedef void (*SYN_KeypadCallback)(SYN_Keypad *kp, char key, bool pressed, void *user_ctx);

struct SYN_Keypad {
    SYN_GPIO_Pin rows[SYN_KEYPAD_MAX_ROWS]; 
    uint8_t num_rows;                       
    SYN_GPIO_Pin cols[SYN_KEYPAD_MAX_COLS]; 
    uint8_t num_cols;                       
    char keymap[SYN_KEYPAD_MAX_ROWS * SYN_KEYPAD_MAX_COLS]; 
    char active_key;      
    bool is_pressed;      
    char last_raw_key;    
    uint32_t press_count; 
    SYN_KeypadCallback on_event; 
    void *user_ctx;              
};

SYN_Status syn_keypad_init(SYN_Keypad *kp, const SYN_GPIO_Pin *rows, uint8_t num_rows,
                           const SYN_GPIO_Pin *cols, uint8_t num_cols, const char *keymap);

void syn_keypad_scan(SYN_Keypad *kp);

bool syn_keypad_get_key(const SYN_Keypad *kp, char *out_key);

void syn_keypad_set_callback(SYN_Keypad *kp, SYN_KeypadCallback callback, void *user_ctx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_KEYPAD_H */
```


