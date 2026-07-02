

# File syn\_menu.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**ui**](dir_5167a572f0687ac609ba099b941e0ec0.md) **>** [**syn\_menu.h**](syn__menu_8h.md)

[Go to the documentation of this file](syn__menu_8h.md)


```C++

#ifndef SYN_MENU_H
#define SYN_MENU_H

#include "../common/syn_defs.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Menu item types ────────────────────────────────────────────────────── */

typedef enum {
    SYN_MENU_ACTION_SUBMENU  = 0,  
    SYN_MENU_ACTION_CALLBACK = 1,  
    SYN_MENU_ACTION_TOGGLE   = 2,  
    SYN_MENU_ACTION_VALUE    = 3,  
} SYN_MenuAction;

/* ── Value config (for ACTION_VALUE items) ──────────────────────────────── */

typedef struct {
    int32_t *value;     
    int32_t  min;       
    int32_t  max;       
    int32_t  step;      
} SYN_MenuValueCfg;

/* ── Menu item ──────────────────────────────────────────────────────────── */

typedef struct SYN_MenuItem {
    const char                   *label;       
    uint8_t                       action;      
    union {
        struct {
            const struct SYN_MenuItem *children; 
            uint8_t                     count;   
        } submenu;                               
        struct {
            void (*func)(void *ctx);             
            void  *ctx;                          
        } callback;                              
        bool                       *toggle;      
        SYN_MenuValueCfg           value_cfg;    
    } u;                                         
} SYN_MenuItem;

#define SYN_MENU_SUBMENU(lbl, items) \
    { .label = (lbl), .action = SYN_MENU_ACTION_SUBMENU, \
      .u = { .submenu = { .children = (items), .count = sizeof(items)/sizeof((items)[0]) } } }

#define SYN_MENU_CALLBACK(lbl, fn, c) \
    { .label = (lbl), .action = SYN_MENU_ACTION_CALLBACK, \
      .u = { .callback = { .func = (fn), .ctx = (c) } } }

#define SYN_MENU_TOGGLE(lbl, ptr) \
    { .label = (lbl), .action = SYN_MENU_ACTION_TOGGLE, \
      .u = { .toggle = (ptr) } }

#define SYN_MENU_VALUE(lbl, ptr, mn, mx, st) \
    { .label = (lbl), .action = SYN_MENU_ACTION_VALUE, \
      .u = { .value_cfg = { .value = (ptr), .min = (mn), .max = (mx), .step = (st) } } }

#define SYN_MENU_ROOT(name, items) \
    static const SYN_MenuItem name = SYN_MENU_SUBMENU(#name, items)

/* ── Render callback ────────────────────────────────────────────────────── */

struct SYN_Menu;

typedef void (*SYN_MenuRenderFn)(const struct SYN_Menu *menu, void *ctx);

/* ── Menu state ─────────────────────────────────────────────────────────── */

#ifndef SYN_MENU_MAX_DEPTH
#define SYN_MENU_MAX_DEPTH  8  
#endif

typedef struct SYN_Menu {
    const SYN_MenuItem *root;          
    /* Navigation stack */
    const SYN_MenuItem *stack[SYN_MENU_MAX_DEPTH]; 
    uint8_t              stack_sel[SYN_MENU_MAX_DEPTH]; 
    uint8_t              depth;         
    /* Current view */
    const SYN_MenuItem *current;       
    uint8_t              selected;      
    bool                 editing;       
    /* Render */
    SYN_MenuRenderFn    render;        
    void                *render_ctx;   
} SYN_Menu;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_menu_init(SYN_Menu *menu, const SYN_MenuItem *root,
                     SYN_MenuRenderFn render, void *ctx);

void syn_menu_up(SYN_Menu *menu);

void syn_menu_down(SYN_Menu *menu);

void syn_menu_enter(SYN_Menu *menu);

void syn_menu_back(SYN_Menu *menu);

static inline uint8_t syn_menu_item_count(const SYN_Menu *menu)
{
    return menu->current->u.submenu.count;
}

static inline const SYN_MenuItem *syn_menu_selected_item(const SYN_Menu *menu)
{
    return &menu->current->u.submenu.children[menu->selected];
}

void syn_menu_render(const SYN_Menu *menu);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MENU_H */
```


