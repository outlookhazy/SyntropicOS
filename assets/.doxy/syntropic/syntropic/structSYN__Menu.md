

# Struct SYN\_Menu



[**ClassList**](annotated.md) **>** [**SYN\_Menu**](structSYN__Menu.md)



_Menu runtime state — navigation stack and rendering._ 

* `#include <syn_menu.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const [**SYN\_MenuItem**](structSYN__MenuItem.md) \* | [**current**](#variable-current)  <br> |
|  uint8\_t | [**depth**](#variable-depth)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**editing**](#variable-editing)  <br> |
|  [**SYN\_MenuRenderFn**](syn__menu_8h.md#typedef-syn_menurenderfn) | [**render**](#variable-render)  <br> |
|  void \* | [**render\_ctx**](#variable-render_ctx)  <br> |
|  const [**SYN\_MenuItem**](structSYN__MenuItem.md) \* | [**root**](#variable-root)  <br> |
|  uint8\_t | [**selected**](#variable-selected)  <br> |
|  const [**SYN\_MenuItem**](structSYN__MenuItem.md) \* | [**stack**](#variable-stack)  <br> |
|  uint8\_t | [**stack\_sel**](#variable-stack_sel)  <br> |












































## Public Attributes Documentation




### variable current 

```C++
const SYN_MenuItem* SYN_Menu::current;
```



Current menu level (submenu) 
 


        

<hr>



### variable depth 

```C++
uint8_t SYN_Menu::depth;
```



Current depth (0 = root) 
 


        

<hr>



### variable editing 

```C++
bool SYN_Menu::editing;
```



In value-edit mode? 
 


        

<hr>



### variable render 

```C++
SYN_MenuRenderFn SYN_Menu::render;
```



Called after state changes 
 


        

<hr>



### variable render\_ctx 

```C++
void* SYN_Menu::render_ctx;
```



Context for render callback 
 


        

<hr>



### variable root 

```C++
const SYN_MenuItem* SYN_Menu::root;
```



Root menu item (submenu) 
 


        

<hr>



### variable selected 

```C++
uint8_t SYN_Menu::selected;
```



Currently highlighted index 
 


        

<hr>



### variable stack 

```C++
const SYN_MenuItem* SYN_Menu::stack[SYN_MENU_MAX_DEPTH];
```



Parent chain 
 


        

<hr>



### variable stack\_sel 

```C++
uint8_t SYN_Menu::stack_sel[SYN_MENU_MAX_DEPTH];
```



Selection idx 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ui/syn_menu.h`

