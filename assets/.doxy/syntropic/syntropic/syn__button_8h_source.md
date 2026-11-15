

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
    SYN_BUTTON_ACTIVE_LOW = 1,  
} SYN_ButtonPolarity;

/* ── Button state ───────────────────────────────────────────────────────── */

typedef enum {
    SYN_BUTTON_STATE_IDLE = 0,       
    SYN_BUTTON_STATE_DEBOUNCING = 1, 
    SYN_BUTTON_STATE_PRESSED = 2,    
    SYN_BUTTON_STATE_HELD = 3,       
} SYN_ButtonState;

#define SYN_BUTTON_EVT_PRESS ((uint8_t)(1u << 0))        
#define SYN_BUTTON_EVT_RELEASE ((uint8_t)(1u << 1))      
#define SYN_BUTTON_EVT_LONG_PRESS ((uint8_t)(1u << 2))   
#define SYN_BUTTON_EVT_REPEAT ((uint8_t)(1u << 3))       
#define SYN_BUTTON_EVT_SINGLE_CLICK ((uint8_t)(1u << 4)) 
#define SYN_BUTTON_EVT_DOUBLE_CLICK ((uint8_t)(1u << 5)) 
#define SYN_BUTTON_EVT_TRIPLE_CLICK ((uint8_t)(1u << 6)) 
#define SYN_BUTTON_EVT_MULTI_CLICK ((uint8_t)(1u << 7))  

/* ── Callback type ──────────────────────────────────────────────────────── */

struct SYN_Button;

typedef void (*SYN_ButtonCallback)(struct SYN_Button *btn, void *user_data);

/* ── Button descriptor ──────────────────────────────────────────────────── */

typedef struct SYN_Button {
    /* Configuration */
    SYN_GPIO_Pin pin;         
    uint8_t polarity;         
    uint16_t debounce_ms;     
    uint16_t long_press_ms;   
    uint16_t repeat_ms;       
    uint16_t double_click_ms; 

    /* State */
    SYN_FSM fsm;             
    volatile uint8_t events; 
    bool raw_pressed;        
    bool pressed;            
    bool in_combo;           
    uint8_t click_count;     
    uint32_t state_tick;     
    uint32_t repeat_tick;    
    uint32_t click_tick;     
    uint32_t now_tick;       

    /* Callbacks */
    SYN_ButtonCallback on_press;        
    void *on_press_ctx;                 
    SYN_ButtonCallback on_release;      
    void *on_release_ctx;               
    SYN_ButtonCallback on_long_press;   
    void *on_long_press_ctx;            
    SYN_ButtonCallback on_repeat;       
    void *on_repeat_ctx;                
    SYN_ButtonCallback on_single_click; 
    void *on_single_click_ctx;          
    SYN_ButtonCallback on_double_click; 
    void *on_double_click_ctx;          
    SYN_ButtonCallback on_triple_click; 
    void *on_triple_click_ctx;          
    SYN_ButtonCallback on_multi_click;  
    void *on_multi_click_ctx;           
} SYN_Button;

/* ── Combo button descriptor ────────────────────────────────────────────── */

typedef struct SYN_ButtonCombo {
    SYN_Button *const *buttons;  
    size_t count;                
    SYN_ButtonCallback on_combo; 
    void *on_combo_ctx;          
    bool active;                 
    uint8_t events;              
} SYN_ButtonCombo;

/* ── Initialization ─────────────────────────────────────────────────────── */

void syn_button_init(SYN_Button *btn, SYN_GPIO_Pin pin, SYN_ButtonPolarity polarity,
                     uint16_t debounce_ms);

/* ── Configuration ──────────────────────────────────────────────────────── */

void syn_button_set_click_window(SYN_Button *btn, uint16_t window_ms);

/* ── Callback registration ──────────────────────────────────────────────── */

void syn_button_on_press(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx);

void syn_button_on_release(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx);

void syn_button_on_long_press(SYN_Button *btn, SYN_ButtonCallback cb, uint16_t hold_ms, void *ctx);

void syn_button_on_repeat(SYN_Button *btn, SYN_ButtonCallback cb, uint16_t interval_ms, void *ctx);

void syn_button_on_single_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx);

void syn_button_on_double_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx);

void syn_button_on_triple_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx);

void syn_button_on_multi_click(SYN_Button *btn, SYN_ButtonCallback cb, void *ctx);

/* ── Combo button functions ─────────────────────────────────────────────── */

void syn_button_combo_init(SYN_ButtonCombo *combo, SYN_Button *const *buttons, size_t count,
                           SYN_ButtonCallback cb, void *ctx);

void syn_button_combo_update(SYN_ButtonCombo *combo);

/* ── Update / service ───────────────────────────────────────────────────── */

void syn_button_update(SYN_Button *btn);

void syn_button_service(SYN_Button *buttons, size_t count);

/* ── Query ──────────────────────────────────────────────────────────────── */

static inline bool syn_button_is_pressed(const SYN_Button *btn)
{
    return (btn != NULL) && btn->pressed;
}

static inline uint32_t syn_button_held_ms(const SYN_Button *btn)
{
    if (btn == NULL) {
        return 0U;
    }
    return syn_port_get_tick_ms() - btn->state_tick;
}

static inline uint8_t syn_button_clicks(const SYN_Button *btn)
{
    return (btn != NULL) ? btn->click_count : 0U;
}

static inline bool syn_button_combo_is_active(const SYN_ButtonCombo *combo)
{
    return combo != NULL && combo->active;
}

static inline uint8_t syn_button_poll_events(SYN_Button *btn)
{
    if (btn == NULL) {
        return 0U;
    }
    syn_port_enter_critical();
    uint8_t e = btn->events;
    btn->events = 0U;
    syn_port_exit_critical();
    return e;
}

static inline bool syn_button_consume_event(SYN_Button *btn, uint8_t mask)
{
    if (btn == NULL) {
        return false;
    }
    bool matched = false;
    syn_port_enter_critical();
    if ((btn->events & mask) != 0U) {
        btn->events = (uint8_t)(btn->events & (uint8_t)~mask);
        matched = true;
    }
    syn_port_exit_critical();
    return matched;
}

/* ── Protothread integration ────────────────────────────────────────────── */

#define PT_WAIT_BUTTON_PRESS(pt, btn) \
    PT_WAIT_UNTIL((pt), syn_button_consume_event((btn), SYN_BUTTON_EVT_PRESS))

#define PT_BUTTON_BLOCK_PRESS(pt, task, btn) \
    PT_BLOCK_CONDITION((pt), (task), syn_button_consume_event((btn), SYN_BUTTON_EVT_PRESS))

#define PT_WAIT_BUTTON_RELEASE(pt, btn) \
    PT_WAIT_UNTIL((pt), syn_button_consume_event((btn), SYN_BUTTON_EVT_RELEASE))

#define PT_BUTTON_BLOCK_RELEASE(pt, task, btn) \
    PT_BLOCK_CONDITION((pt), (task), syn_button_consume_event((btn), SYN_BUTTON_EVT_RELEASE))

#ifdef __cplusplus
}
#endif

#endif /* SYN_BUTTON_H */
```


