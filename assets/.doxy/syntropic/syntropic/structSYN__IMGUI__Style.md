

# Struct SYN\_IMGUI\_Style



[**ClassList**](annotated.md) **>** [**SYN\_IMGUI\_Style**](structSYN__IMGUI__Style.md)



_Visual style parameters for IMGUI widgets._ [More...](#detailed-description)

* `#include <syn_imgui.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**accent**](#variable-accent)  <br> |
|  uint16\_t | [**bg**](#variable-bg)  <br> |
|  uint16\_t | [**disabled**](#variable-disabled)  <br> |
|  uint16\_t | [**fg**](#variable-fg)  <br> |
|  uint16\_t | [**fg\_inv**](#variable-fg_inv)  <br> |
|  uint16\_t | [**highlight**](#variable-highlight)  <br> |
|  int16\_t | [**padding**](#variable-padding)  <br> |
|  int16\_t | [**spacing**](#variable-spacing)  <br> |












































## Detailed Description


Set via [**syn\_imgui\_set\_style()**](syn__imgui_8c.md#function-syn_imgui_set_style). [**syn\_imgui\_init()**](syn__imgui_8c.md#function-syn_imgui_init) loads the defaults (monochrome-friendly: white-on-black, matching the existing hardcoded colors) so existing code is unaffected. 


    
## Public Attributes Documentation




### variable accent 

```C++
uint16_t SYN_IMGUI_Style::accent;
```



Slider fill, progress bar, gauge needle 


        

<hr>



### variable bg 

```C++
uint16_t SYN_IMGUI_Style::bg;
```



Background fill for unfocused widgets 
 


        

<hr>



### variable disabled 

```C++
uint16_t SYN_IMGUI_Style::disabled;
```



Disabled widget color 
 


        

<hr>



### variable fg 

```C++
uint16_t SYN_IMGUI_Style::fg;
```



Foreground: text, outlines 
 


        

<hr>



### variable fg\_inv 

```C++
uint16_t SYN_IMGUI_Style::fg_inv;
```



Text drawn on a highlighted background 
 


        

<hr>



### variable highlight 

```C++
uint16_t SYN_IMGUI_Style::highlight;
```



Focused widget fill color 
 


        

<hr>



### variable padding 

```C++
int16_t SYN_IMGUI_Style::padding;
```



Inner padding in pixels (default 2) 
 


        

<hr>



### variable spacing 

```C++
int16_t SYN_IMGUI_Style::spacing;
```



Inter-widget gap for auto-layout (default 3) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ui/syn_imgui.h`

