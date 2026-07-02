

# File syn\_menu.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**ui**](dir_5167a572f0687ac609ba099b941e0ec0.md) **>** [**syn\_menu.c**](syn__menu_8c.md)

[Go to the source code of this file](syn__menu_8c_source.md)

_Menu system implementation._ 

* `#include "syn_menu.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_menu\_back**](#function-syn_menu_back) ([**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Back: leave submenu or exit edit mode._  |
|  void | [**syn\_menu\_down**](#function-syn_menu_down) ([**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Move selection down. In edit mode: decrement value._  |
|  void | [**syn\_menu\_enter**](#function-syn_menu_enter) ([**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Enter: navigate into submenu, toggle, start editing, or fire callback._  |
|  void | [**syn\_menu\_init**](#function-syn_menu_init) ([**SYN\_Menu**](structSYN__Menu.md) \* menu, const [**SYN\_MenuItem**](structSYN__MenuItem.md) \* root, [**SYN\_MenuRenderFn**](syn__menu_8h.md#typedef-syn_menurenderfn) render, void \* ctx) <br>_Initialize menu system._  |
|  void | [**syn\_menu\_render**](#function-syn_menu_render) (const [**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Force a render (e.g. after external state change)._  |
|  void | [**syn\_menu\_up**](#function-syn_menu_up) ([**SYN\_Menu**](structSYN__Menu.md) \* menu) <br>_Move selection up. In edit mode: increment value._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ui/syn_menu.c`

