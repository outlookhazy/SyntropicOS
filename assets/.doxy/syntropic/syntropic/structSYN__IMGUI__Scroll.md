

# Struct SYN\_IMGUI\_Scroll



[**ClassList**](annotated.md) **>** [**SYN\_IMGUI\_Scroll**](structSYN__IMGUI__Scroll.md)



_Scroll region state for scrollable IMGUI panels._ 

* `#include <syn_imgui.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int16\_t | [**content\_start**](#variable-content_start)  <br> |
|  uint16\_t | [**focus\_h**](#variable-focus_h)  <br> |
|  uint16\_t | [**focus\_y**](#variable-focus_y)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**in\_scroll**](#variable-in_scroll)  <br> |
|  int16\_t \* | [**scroll\_p**](#variable-scroll_p)  <br> |
|  int16\_t | [**vp\_h**](#variable-vp_h)  <br> |
|  int16\_t | [**vp\_w**](#variable-vp_w)  <br> |
|  int16\_t | [**vp\_x**](#variable-vp_x)  <br> |
|  int16\_t | [**vp\_y**](#variable-vp_y)  <br> |












































## Public Attributes Documentation




### variable content\_start 

```C++
int16_t SYN_IMGUI_Scroll::content_start;
```



Layout cursor Y at scroll\_begin 
 


        

<hr>



### variable focus\_h 

```C++
uint16_t SYN_IMGUI_Scroll::focus_h;
```



Height of focused widget 
 


        

<hr>



### variable focus\_y 

```C++
uint16_t SYN_IMGUI_Scroll::focus_y;
```



Y position of focused widget (for auto-scroll) 


        

<hr>



### variable in\_scroll 

```C++
bool SYN_IMGUI_Scroll::in_scroll;
```



True while between scroll\_begin / scroll\_end 


        

<hr>



### variable scroll\_p 

```C++
int16_t* SYN_IMGUI_Scroll::scroll_p;
```



Pointer to user-owned scroll offset 
 


        

<hr>



### variable vp\_h 

```C++
int16_t SYN_IMGUI_Scroll::vp_h;
```



Viewport height 
 


        

<hr>



### variable vp\_w 

```C++
int16_t SYN_IMGUI_Scroll::vp_w;
```



Viewport width 
 


        

<hr>



### variable vp\_x 

```C++
int16_t SYN_IMGUI_Scroll::vp_x;
```



Viewport left edge 
 


        

<hr>



### variable vp\_y 

```C++
int16_t SYN_IMGUI_Scroll::vp_y;
```



Viewport top edge 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ui/syn_imgui.h`

