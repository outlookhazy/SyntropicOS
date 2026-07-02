

# File syn\_menu.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**ui**](dir_5167a572f0687ac609ba099b941e0ec0.md) **>** [**syn\_menu.h**](syn__menu_8h.md)

[Go to the source code of this file](syn__menu_8h_source.md)

_Tree-structured menu system._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Menu**](structSYN__Menu.md) <br>_Menu runtime state — navigation stack and rendering._  |
| struct | [**SYN\_MenuItem**](structSYN__MenuItem.md) <br>_Menu item — label + action type + union payload._  |
| struct | [**SYN\_MenuValueCfg**](structSYN__MenuValueCfg.md) <br>_Value adjustment config for SYN\_MENU\_ACTION\_VALUE items._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_MenuAction**](#enum-syn_menuaction)  <br>_Menu item action types._  |
| typedef void(\* | [**SYN\_MenuRenderFn**](#typedef-syn_menurenderfn)  <br>_Menu render callback — called after each navigation change._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_menu\_back**](#function-syn_menu_back) ([**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Back: leave submenu or exit edit mode._  |
|  void | [**syn\_menu\_down**](#function-syn_menu_down) ([**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Move selection down. In edit mode: decrement value._  |
|  void | [**syn\_menu\_enter**](#function-syn_menu_enter) ([**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Enter: navigate into submenu, toggle, start editing, or fire callback._  |
|  void | [**syn\_menu\_init**](#function-syn_menu_init) ([**SYN\_Menu**](structSYN__Menu.md) \* menu, const [**SYN\_MenuItem**](structSYN__MenuItem.md) \* root, [**SYN\_MenuRenderFn**](syn__menu_8h.md#typedef-syn_menurenderfn) render, void \* ctx) <br>_Initialize menu system._  |
|  void | [**syn\_menu\_render**](#function-syn_menu_render) (const [**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Force a render (e.g. after external state change)._  |
|  void | [**syn\_menu\_up**](#function-syn_menu_up) ([**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Move selection up. In edit mode: increment value._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_menu\_item\_count**](#function-syn_menu_item_count) (const [**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Get the number of items in the current menu level._  |
|  const [**SYN\_MenuItem**](structSYN__MenuItem.md) \* | [**syn\_menu\_selected\_item**](#function-syn_menu_selected_item) (const [**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Get the currently selected item._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MENU\_CALLBACK**](syn__menu_8h.md#define-syn_menu_callback) (lbl, fn, c) `/* multi line expression */`<br>_Define a callback action item._  |
| define  | [**SYN\_MENU\_MAX\_DEPTH**](syn__menu_8h.md#define-syn_menu_max_depth)  `8`<br> |
| define  | [**SYN\_MENU\_ROOT**](syn__menu_8h.md#define-syn_menu_root) (name, items) `static const [**SYN\_MenuItem**](structSYN__MenuItem.md) name = [**SYN\_MENU\_SUBMENU**](syn__menu_8h.md#define-syn_menu_submenu)(#name, items)`<br>_Define the root menu._  |
| define  | [**SYN\_MENU\_SUBMENU**](syn__menu_8h.md#define-syn_menu_submenu) (lbl, items) `/* multi line expression */`<br>_Define a submenu item._  |
| define  | [**SYN\_MENU\_TOGGLE**](syn__menu_8h.md#define-syn_menu_toggle) (lbl, ptr) `/* multi line expression */`<br>_Define a boolean toggle item._  |
| define  | [**SYN\_MENU\_VALUE**](syn__menu_8h.md#define-syn_menu_value) (lbl, ptr, mn, mx, st) `/* multi line expression */`<br>_Define a numeric value editor item._  |

## Detailed Description


Static, data-driven menus for CLI or display. No dynamic allocation. Navigation via up/down/enter/back. Supports submenus, callbacks, toggles, and integer values with min/max/step.


Usage: 
```C++
static bool led_on = false;
static int32_t brightness = 50;

static const SYN_MenuItem settings_items[] = {
    SYN_MENU_TOGGLE("LED", &led_on),
    SYN_MENU_VALUE("Brightness", &brightness, 0, 100, 5),
    SYN_MENU_CALLBACK("Save", save_settings, NULL),
};

static const SYN_MenuItem root_items[] = {
    SYN_MENU_SUBMENU("Settings", settings_items),
    SYN_MENU_CALLBACK("Reboot", do_reboot, NULL),
};

SYN_MENU_ROOT(root, root_items);

static SYN_Menu menu;
syn_menu_init(&menu, &root, my_render, NULL);
```
 


    
## Public Types Documentation




### enum SYN\_MenuAction 

_Menu item action types._ 
```C++
enum SYN_MenuAction {
    SYN_MENU_ACTION_SUBMENU = 0,
    SYN_MENU_ACTION_CALLBACK = 1,
    SYN_MENU_ACTION_TOGGLE = 2,
    SYN_MENU_ACTION_VALUE = 3
};
```




<hr>



### typedef SYN\_MenuRenderFn 

_Menu render callback — called after each navigation change._ 
```C++
typedef void(* SYN_MenuRenderFn) (const struct SYN_Menu *menu, void *ctx);
```





**Parameters:**


* `menu` Menu state. 
* `ctx` User context. 




        

<hr>
## Public Functions Documentation




### function syn\_menu\_back 

_Back: leave submenu or exit edit mode._ 
```C++
void syn_menu_back (
    SYN_Menu * menu
) 
```





**Parameters:**


* `menu` Menu state. 




        

<hr>



### function syn\_menu\_down 

_Move selection down. In edit mode: decrement value._ 
```C++
void syn_menu_down (
    SYN_Menu * menu
) 
```





**Parameters:**


* `menu` Menu state. 




        

<hr>



### function syn\_menu\_enter 

_Enter: navigate into submenu, toggle, start editing, or fire callback._ 
```C++
void syn_menu_enter (
    SYN_Menu * menu
) 
```





**Parameters:**


* `menu` Menu state. 




        

<hr>



### function syn\_menu\_init 

_Initialize menu system._ 
```C++
void syn_menu_init (
    SYN_Menu * menu,
    const SYN_MenuItem * root,
    SYN_MenuRenderFn render,
    void * ctx
) 
```





**Parameters:**


* `menu` Menu state. 
* `root` Root menu item (should be a SUBMENU). 
* `render` Render callback (called after each navigation action). 
* `ctx` Context for render callback. 




        

<hr>



### function syn\_menu\_render 

_Force a render (e.g. after external state change)._ 
```C++
void syn_menu_render (
    const SYN_Menu * menu
) 
```





**Parameters:**


* `menu` Menu state. 




        

<hr>



### function syn\_menu\_up 

_Move selection up. In edit mode: increment value._ 
```C++
void syn_menu_up (
    SYN_Menu * menu
) 
```





**Parameters:**


* `menu` Menu state. 




        

<hr>
## Public Static Functions Documentation




### function syn\_menu\_item\_count 

_Get the number of items in the current menu level._ 
```C++
static inline uint8_t syn_menu_item_count (
    const SYN_Menu * menu
) 
```





**Parameters:**


* `menu` Menu state. 



**Returns:**

Item count. 





        

<hr>



### function syn\_menu\_selected\_item 

_Get the currently selected item._ 
```C++
static inline const SYN_MenuItem * syn_menu_selected_item (
    const SYN_Menu * menu
) 
```





**Parameters:**


* `menu` Menu state. 



**Returns:**

Pointer to the selected menu item. 





        

<hr>
## Macro Definition Documentation





### define SYN\_MENU\_CALLBACK 

_Define a callback action item._ 
```C++
#define SYN_MENU_CALLBACK (
    lbl,
    fn,
    c
) `/* multi line expression */`
```




<hr>



### define SYN\_MENU\_MAX\_DEPTH 

```C++
#define SYN_MENU_MAX_DEPTH `8`
```



Max nesting depth (stack size) 
 


        

<hr>



### define SYN\_MENU\_ROOT 

_Define the root menu._ 
```C++
#define SYN_MENU_ROOT (
    name,
    items
) `static const SYN_MenuItem name = SYN_MENU_SUBMENU (#name, items)`
```




<hr>



### define SYN\_MENU\_SUBMENU 

_Define a submenu item._ 
```C++
#define SYN_MENU_SUBMENU (
    lbl,
    items
) `/* multi line expression */`
```




<hr>



### define SYN\_MENU\_TOGGLE 

_Define a boolean toggle item._ 
```C++
#define SYN_MENU_TOGGLE (
    lbl,
    ptr
) `/* multi line expression */`
```




<hr>



### define SYN\_MENU\_VALUE 

_Define a numeric value editor item._ 
```C++
#define SYN_MENU_VALUE (
    lbl,
    ptr,
    mn,
    mx,
    st
) `/* multi line expression */`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ui/syn_menu.h`

