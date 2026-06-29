/**
 * @file syn_menu.h
 * @brief Tree-structured menu system.
 *
 * Static, data-driven menus for CLI or display. No dynamic allocation.
 * Navigation via up/down/enter/back. Supports submenus, callbacks,
 * toggles, and integer values with min/max/step.
 *
 * Usage:
 * @code
 *   static bool led_on = false;
 *   static int32_t brightness = 50;
 *
 *   static const SYN_MenuItem settings_items[] = {
 *       SYN_MENU_TOGGLE("LED", &led_on),
 *       SYN_MENU_VALUE("Brightness", &brightness, 0, 100, 5),
 *       SYN_MENU_CALLBACK("Save", save_settings, NULL),
 *   };
 *
 *   static const SYN_MenuItem root_items[] = {
 *       SYN_MENU_SUBMENU("Settings", settings_items),
 *       SYN_MENU_CALLBACK("Reboot", do_reboot, NULL),
 *   };
 *
 *   SYN_MENU_ROOT(root, root_items);
 *
 *   static SYN_Menu menu;
 *   syn_menu_init(&menu, &root, my_render, NULL);
 * @endcode
 * @ingroup syn_display
 */

#ifndef SYN_MENU_H
#define SYN_MENU_H

#include "../common/syn_defs.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Menu item types ────────────────────────────────────────────────────── */

/** @brief Menu item action types. */
typedef enum {
    SYN_MENU_ACTION_SUBMENU  = 0,  /**< Navigate into child menu        */
    SYN_MENU_ACTION_CALLBACK = 1,  /**< Call a function                  */
    SYN_MENU_ACTION_TOGGLE   = 2,  /**< Toggle a bool                   */
    SYN_MENU_ACTION_VALUE    = 3,  /**< Adjust an int32_t value          */
} SYN_MenuAction;

/* ── Value config (for ACTION_VALUE items) ──────────────────────────────── */

/** @brief Value adjustment config for SYN_MENU_ACTION_VALUE items. */
typedef struct {
    int32_t *value;     /**< Pointer to the value to adjust              */
    int32_t  min;       /**< Minimum allowed value                       */
    int32_t  max;       /**< Maximum allowed value                       */
    int32_t  step;      /**< Increment/decrement step                    */
} SYN_MenuValueCfg;

/* ── Menu item ──────────────────────────────────────────────────────────── */

/** @brief Menu item — label + action type + union payload. */
typedef struct SYN_MenuItem {
    const char                   *label;       /**< Display text            */
    uint8_t                       action;      /**< SYN_MenuAction         */
    union {
        struct {
            const struct SYN_MenuItem *children; /**< Child items array    */
            uint8_t                     count;   /**< Number of children   */
        } submenu;                               /**< SYN_MENU_ACTION_SUBMENU */
        struct {
            void (*func)(void *ctx);             /**< Callback function    */
            void  *ctx;                          /**< Callback context     */
        } callback;                              /**< SYN_MENU_ACTION_CALLBACK */
        bool                       *toggle;      /**< SYN_MENU_ACTION_TOGGLE */
        SYN_MenuValueCfg           value_cfg;    /**< SYN_MENU_ACTION_VALUE */
    } u;                                         /**< Action-specific data */
} SYN_MenuItem;

/** @name Convenience macros for static menu definition
 * @{
 */

/** @brief Define a submenu item. */
#define SYN_MENU_SUBMENU(lbl, items) \
    { .label = (lbl), .action = SYN_MENU_ACTION_SUBMENU, \
      .u = { .submenu = { .children = (items), .count = sizeof(items)/sizeof((items)[0]) } } }

/** @brief Define a callback action item. */
#define SYN_MENU_CALLBACK(lbl, fn, c) \
    { .label = (lbl), .action = SYN_MENU_ACTION_CALLBACK, \
      .u = { .callback = { .func = (fn), .ctx = (c) } } }

/** @brief Define a boolean toggle item. */
#define SYN_MENU_TOGGLE(lbl, ptr) \
    { .label = (lbl), .action = SYN_MENU_ACTION_TOGGLE, \
      .u = { .toggle = (ptr) } }

/** @brief Define a numeric value editor item. */
#define SYN_MENU_VALUE(lbl, ptr, mn, mx, st) \
    { .label = (lbl), .action = SYN_MENU_ACTION_VALUE, \
      .u = { .value_cfg = { .value = (ptr), .min = (mn), .max = (mx), .step = (st) } } }

/** @brief Define the root menu. */
#define SYN_MENU_ROOT(name, items) \
    static const SYN_MenuItem name = SYN_MENU_SUBMENU(#name, items)

/** @} */

/* ── Render callback ────────────────────────────────────────────────────── */

struct SYN_Menu;

/**
 * @brief Menu render callback — called after each navigation change.
 * @param menu  Menu state.
 * @param ctx   User context.
 */
typedef void (*SYN_MenuRenderFn)(const struct SYN_Menu *menu, void *ctx);

/* ── Menu state ─────────────────────────────────────────────────────────── */

#ifndef SYN_MENU_MAX_DEPTH
#define SYN_MENU_MAX_DEPTH  8  /**< Max nesting depth (stack size)      */
#endif

/** @brief Menu runtime state — navigation stack and rendering. */
typedef struct SYN_Menu {
    const SYN_MenuItem *root;          /**< Root menu item (submenu)      */

    /* Navigation stack */
    const SYN_MenuItem *stack[SYN_MENU_MAX_DEPTH]; /**< Parent chain    */
    uint8_t              stack_sel[SYN_MENU_MAX_DEPTH]; /**< Selection idx */
    uint8_t              depth;         /**< Current depth (0 = root)      */

    /* Current view */
    const SYN_MenuItem *current;       /**< Current menu level (submenu)  */
    uint8_t              selected;      /**< Currently highlighted index   */
    bool                 editing;       /**< In value-edit mode?           */

    /* Render */
    SYN_MenuRenderFn    render;        /**< Called after state changes    */
    void                *render_ctx;   /**< Context for render callback   */
} SYN_Menu;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize menu system.
 *
 * @param menu    Menu state.
 * @param root    Root menu item (should be a SUBMENU).
 * @param render  Render callback (called after each navigation action).
 * @param ctx     Context for render callback.
 */
void syn_menu_init(SYN_Menu *menu, const SYN_MenuItem *root,
                     SYN_MenuRenderFn render, void *ctx);

/**
 * @brief Move selection up. In edit mode: increment value.
 * @param menu  Menu state.
 */
void syn_menu_up(SYN_Menu *menu);

/**
 * @brief Move selection down. In edit mode: decrement value.
 * @param menu  Menu state.
 */
void syn_menu_down(SYN_Menu *menu);

/**
 * @brief Enter: navigate into submenu, toggle, start editing, or fire callback.
 * @param menu  Menu state.
 */
void syn_menu_enter(SYN_Menu *menu);

/**
 * @brief Back: leave submenu or exit edit mode.
 * @param menu  Menu state.
 */
void syn_menu_back(SYN_Menu *menu);

/**
 * @brief Get the number of items in the current menu level.
 * @param menu  Menu state.
 * @return Item count.
 */
static inline uint8_t syn_menu_item_count(const SYN_Menu *menu)
{
    return menu->current->u.submenu.count;
}

/**
 * @brief Get the currently selected item.
 * @param menu  Menu state.
 * @return Pointer to the selected menu item.
 */
static inline const SYN_MenuItem *syn_menu_selected_item(const SYN_Menu *menu)
{
    return &menu->current->u.submenu.children[menu->selected];
}

/**
 * @brief Force a render (e.g. after external state change).
 * @param menu  Menu state.
 */
void syn_menu_render(const SYN_Menu *menu);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MENU_H */
