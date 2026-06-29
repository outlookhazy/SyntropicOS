#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_MENU) || SYN_USE_MENU

/**
 * @file syn_menu.c
 * @brief Menu system implementation.
 */

#include "syn_menu.h"
#include "../util/syn_assert.h"

#include <string.h>

void syn_menu_init(SYN_Menu *menu, const SYN_MenuItem *root,
                     SYN_MenuRenderFn render, void *ctx)
{
    SYN_ASSERT(menu != NULL);
    SYN_ASSERT(root != NULL);
    SYN_ASSERT(root->action == SYN_MENU_ACTION_SUBMENU);

    memset(menu, 0, sizeof(*menu));
    menu->root       = root;
    menu->current    = root;
    menu->selected   = 0;
    menu->depth      = 0;
    menu->render     = render;
    menu->render_ctx = ctx;
}

void syn_menu_up(SYN_Menu *menu)
{
    SYN_ASSERT(menu != NULL);

    if (menu->editing) {
        /* Increment value */
        const SYN_MenuItem *item = syn_menu_selected_item(menu);
        if (item->action == SYN_MENU_ACTION_VALUE) {
            int32_t *val = item->u.value_cfg.value;
            *val += item->u.value_cfg.step;
            if (*val > item->u.value_cfg.max) *val = item->u.value_cfg.max;
        }
    } else {
        /* Move selection up (with wrap) */
        if (menu->selected == 0) {
            menu->selected = menu->current->u.submenu.count - 1;
        } else {
            menu->selected--;
        }
    }

    syn_menu_render(menu);
}

void syn_menu_down(SYN_Menu *menu)
{
    SYN_ASSERT(menu != NULL);

    if (menu->editing) {
        /* Decrement value */
        const SYN_MenuItem *item = syn_menu_selected_item(menu);
        if (item->action == SYN_MENU_ACTION_VALUE) {
            int32_t *val = item->u.value_cfg.value;
            *val -= item->u.value_cfg.step;
            if (*val < item->u.value_cfg.min) *val = item->u.value_cfg.min;
        }
    } else {
        /* Move selection down (with wrap) */
        menu->selected++;
        if (menu->selected >= menu->current->u.submenu.count) {
            menu->selected = 0;
        }
    }

    syn_menu_render(menu);
}

void syn_menu_enter(SYN_Menu *menu)
{
    SYN_ASSERT(menu != NULL);

    if (menu->editing) {
        /* Exit edit mode */
        menu->editing = false;
        syn_menu_render(menu);
        return;
    }

    const SYN_MenuItem *item = syn_menu_selected_item(menu);

    switch ((SYN_MenuAction)item->action) {
    case SYN_MENU_ACTION_SUBMENU:
        if (menu->depth < SYN_MENU_MAX_DEPTH - 1) {
            /* Push current onto stack */
            menu->stack[menu->depth]     = menu->current;
            menu->stack_sel[menu->depth] = menu->selected;
            menu->depth++;
            menu->current  = item;
            menu->selected = 0;
        }
        break;

    case SYN_MENU_ACTION_CALLBACK:
        if (item->u.callback.func != NULL) {
            item->u.callback.func(item->u.callback.ctx);
        }
        break;

    case SYN_MENU_ACTION_TOGGLE:
        if (item->u.toggle != NULL) {
            *item->u.toggle = !(*item->u.toggle);
        }
        break;

    case SYN_MENU_ACTION_VALUE:
        menu->editing = true;
        break;
    }

    syn_menu_render(menu);
}

void syn_menu_back(SYN_Menu *menu)
{
    SYN_ASSERT(menu != NULL);

    if (menu->editing) {
        menu->editing = false;
        syn_menu_render(menu);
        return;
    }

    if (menu->depth > 0) {
        menu->depth--;
        menu->current  = menu->stack[menu->depth];
        menu->selected = menu->stack_sel[menu->depth];
    }

    syn_menu_render(menu);
}

void syn_menu_render(const SYN_Menu *menu)
{
    if (menu->render != NULL) {
        menu->render(menu, menu->render_ctx);
    }
}

#endif /* SYN_USE_MENU */
