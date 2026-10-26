

# Struct SYN\_MenuItem



[**ClassList**](annotated.md) **>** [**SYN\_MenuItem**](structSYN__MenuItem.md)



_Menu item — label + action type + union payload._ 

* `#include <syn_menu.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**action**](#variable-action)  <br> |
|  struct [**SYN\_MenuItem**](structSYN__MenuItem.md) | [**callback**](#variable-callback)  <br> |
|  const struct [**SYN\_MenuItem**](structSYN__MenuItem.md) \* | [**children**](#variable-children)  <br> |
|  uint8\_t | [**count**](#variable-count)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  void(\* | [**func**](#variable-func)  <br> |
|  const char \* | [**label**](#variable-label)  <br> |
|  struct [**SYN\_MenuItem**](structSYN__MenuItem.md) | [**submenu**](#variable-submenu)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) \* | [**toggle**](#variable-toggle)  <br> |
|  union [**SYN\_MenuItem**](structSYN__MenuItem.md) | [**u**](#variable-u)  <br> |
|  [**SYN\_MenuValueCfg**](structSYN__MenuValueCfg.md) | [**value\_cfg**](#variable-value_cfg)  <br> |












































## Public Attributes Documentation




### variable action 

```C++
uint8_t SYN_MenuItem::action;
```



SYN\_MenuAction 
 


        

<hr>



### variable callback 

```C++
struct SYN_MenuItem SYN_MenuItem::callback;
```



SYN\_MENU\_ACTION\_CALLBACK 


        

<hr>



### variable children 

```C++
const struct SYN_MenuItem* SYN_MenuItem::children;
```



Child items array 
 


        

<hr>



### variable count 

```C++
uint8_t SYN_MenuItem::count;
```



Number of children 
 


        

<hr>



### variable ctx 

```C++
void* SYN_MenuItem::ctx;
```



Callback context 
 


        

<hr>



### variable func 

```C++
void(* SYN_MenuItem::func) (void *ctx);
```



Callback function 
 


        

<hr>



### variable label 

```C++
const char* SYN_MenuItem::label;
```



Display text 
 


        

<hr>



### variable submenu 

```C++
struct SYN_MenuItem SYN_MenuItem::submenu;
```



SYN\_MENU\_ACTION\_SUBMENU 


        

<hr>



### variable toggle 

```C++
bool* SYN_MenuItem::toggle;
```



SYN\_MENU\_ACTION\_TOGGLE 


        

<hr>



### variable u 

```C++
union SYN_MenuItem SYN_MenuItem::u;
```



Action-specific data 


        

<hr>



### variable value\_cfg 

```C++
SYN_MenuValueCfg SYN_MenuItem::value_cfg;
```



SYN\_MENU\_ACTION\_VALUE 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ui/syn_menu.h`

