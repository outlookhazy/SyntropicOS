

# File syn\_button.h

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_button.h**](syn__button_8h.md)

[Go to the documentation of this file](syn__button_8h.md)


```C++

#ifndef SYN_BUTTON_H
#define SYN_BUTTON_H

#include "../common/syn_defs.h"
#include "../drivers/syn_gpio.h"
#include "../port/syn_port_system.h"
#include "../util/syn_fsm.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Button polarity ────────────────────────────────────────────────────── */

typedef enum {
    SYN_BUTTON_ACTIVE_HIGH = 0,  
    SYN_BUTTON_ACTIVE_LOW  = 1,  
} SYN_ButtonPolarity;

/* ── Button state ───────────────────────────────────────────────────────── */

typedef enum {
    SYN_BUTTON_STATE_IDLE       = 0,  
    SYN_BUTTON_STATE_DEBOUNCING = 1,  
    SYN_BUTTON_STATE_PRESSED    = 2,  
    SYN_BUTTON_STATE_HELD       = 3,  
} SYN_ButtonState;

#define SYN_BUTTON_EVT_PRESS       ((uint8_t)(1u << 0))  
#define SYN_BUTTON_EVT_RELEASE     ((uint8_t)(1u << 1))  
#define SYN_BUTTON_EVT_LONG_PRESS  ((uint8_t)(1u << 2))  
#define SYN_BUTTON_EVT_REPEAT      ((uint8_t)(1u << 3))  
/* ── Callback type ──────────────────────────────────────────────────────── */

struct SYN_Button;

typedef void (*SYN_ButtonCallback)(struct SYN_Button *btn, void *user_data);

/* ── Button descriptor ──────────────────────────────────────────────────── */

typedef struct SYN_Button {
    /* Configuration */
    SYN_GPIO_Pin         pin;            
    uint8_t               polarity;       
    uint16_t              debounce_ms;    
    uint16_t              long_press_ms;  
    uint16_t              repeat_ms;      
    /* State */
    SYN_FSM               fsm;            
    uint8_t               events;         
    bool                  raw_pressed;    
    bool                  pressed;        
    uint32_t              state_tick;     
    uint32_t              repeat_tick;    
    /* Callbacks */
    SYN_ButtonCallback   on_press;        
    void                 *on_press_ctx;   
    SYN_ButtonCallback   on_release;      
    void                 *on_release_ctx; 
    SYN_ButtonCallback   on_long_press;   
    void                 *on_long_press_ctx; 
    SYN_ButtonCallback   on_repeat;       
    void                 *on_repeat_ctx;  
} SYN_Button;

/* ── Initialization ─────────────────────────────────────────────────────── */

void syn_button_init(SYN_Button *btn,
                      SYN_GPIO_Pin pin,
                      SYN_ButtonPolarity polarity,
                      uint16_t debounce_ms);

/* ── Callback registration ──────────────────────────────────────────────── */

void syn_button_on_press(SYN_Button *btn,
                          SYN_ButtonCallback cb, void *ctx);

void syn_button_on_release(SYN_Button *btn,
                            SYN_ButtonCallback cb, void *ctx);

void syn_button_on_long_press(SYN_Button *btn,
                               SYN_ButtonCallback cb,
                               uint16_t hold_ms,
                               void *ctx);

void syn_button_on_repeat(SYN_Button *btn,
                           SYN_ButtonCallback cb,
                           uint16_t interval_ms,
                           void *ctx);

/* ── Update / service ───────────────────────────────────────────────────── */

void syn_button_update(SYN_Button *btn);

void syn_button_service(SYN_Button *buttons, size_t count);

/* ── Query ──────────────────────────────────────────────────────────────── */

static inline bool syn_button_is_pressed(const SYN_Button *btn)
{
    return btn->pressed;
}

static inline uint32_t syn_button_held_ms(const SYN_Button *btn)
{
    return syn_port_get_tick_ms() - btn->state_tick;
}

static inline uint8_t syn_button_poll_events(SYN_Button *btn)
{
    uint8_t e = btn->events;
    btn->events = 0;
    return e;
}

/* ── Protothread integration ────────────────────────────────────────────── */

#define PT_WAIT_BUTTON_PRESS(pt, btn) \
    PT_WAIT_UNTIL(pt, (btn)->events & SYN_BUTTON_EVT_PRESS); \
    (btn)->events &= (uint8_t)~SYN_BUTTON_EVT_PRESS

#define PT_WAIT_BUTTON_RELEASE(pt, btn) \
    PT_WAIT_UNTIL(pt, (btn)->events & SYN_BUTTON_EVT_RELEASE); \
    (btn)->events &= (uint8_t)~SYN_BUTTON_EVT_RELEASE

#ifdef __cplusplus
}
#endif

#endif /* SYN_BUTTON_H */
```


