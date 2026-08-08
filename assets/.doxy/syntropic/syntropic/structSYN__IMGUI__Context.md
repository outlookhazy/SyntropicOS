

# Struct SYN\_IMGUI\_Context



[**ClassList**](annotated.md) **>** [**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md)



_Immediate-mode GUI context — inputs, navigation, style, layout._ 

* `#include <syn_imgui.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**active\_id**](#variable-active_id)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**btn\_back**](#variable-btn_back)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**btn\_select**](#variable-btn_select)  <br> |
|  uint8\_t | [**disabled\_depth**](#variable-disabled_depth)  <br> |
|  int32\_t | [**enc\_delta**](#variable-enc_delta)  <br> |
|  uint16\_t | [**focused\_id**](#variable-focused_id)  <br> |
|  [**SYN\_GfxContext**](syn__gfx_8h.md#typedef-syn_gfxcontext) | [**gfx**](#variable-gfx)  <br> |
|  uint16\_t | [**last\_max\_id**](#variable-last_max_id)  <br> |
|  [**SYN\_IMGUI\_Layout**](structSYN__IMGUI__Layout.md) | [**layout**](#variable-layout)  <br> |
|  uint16\_t | [**next\_id**](#variable-next_id)  <br> |
|  [**SYN\_IMGUI\_Scroll**](structSYN__IMGUI__Scroll.md) | [**scroll**](#variable-scroll)  <br> |
|  [**SYN\_IMGUI\_Style**](structSYN__IMGUI__Style.md) | [**style**](#variable-style)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**touch\_down**](#variable-touch_down)  <br> |
|  int16\_t | [**touch\_x**](#variable-touch_x)  <br> |
|  int16\_t | [**touch\_y**](#variable-touch_y)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**updated\_focus**](#variable-updated_focus)  <br> |












































## Public Attributes Documentation




### variable active\_id 

```C++
uint16_t SYN_IMGUI_Context::active_id;
```



ID of widget currently in "active/editing" mode 


        

<hr>



### variable btn\_back 

```C++
bool SYN_IMGUI_Context::btn_back;
```



Cancel / Back button pressed 


        

<hr>



### variable btn\_select 

```C++
bool SYN_IMGUI_Context::btn_select;
```



OK / Enter button pressed 


        

<hr>



### variable disabled\_depth 

```C++
uint8_t SYN_IMGUI_Context::disabled_depth;
```



&gt;0: widgets skip input, draw dimmed 
 


        

<hr>



### variable enc\_delta 

```C++
int32_t SYN_IMGUI_Context::enc_delta;
```



Rotary encoder delta or +/- navigation count 


        

<hr>



### variable focused\_id 

```C++
uint16_t SYN_IMGUI_Context::focused_id;
```



ID of currently highlighted/focused widget 


        

<hr>



### variable gfx 

```C++
SYN_GfxContext SYN_IMGUI_Context::gfx;
```



Graphics context used for drawing 


        

<hr>



### variable last\_max\_id 

```C++
uint16_t SYN_IMGUI_Context::last_max_id;
```



Total count of widgets from the previous frame 


        

<hr>



### variable layout 

```C++
SYN_IMGUI_Layout SYN_IMGUI_Context::layout;
```



Auto-layout cursor state 
 


        

<hr>



### variable next\_id 

```C++
uint16_t SYN_IMGUI_Context::next_id;
```



Transient counter for widgets in current frame 


        

<hr>



### variable scroll 

```C++
SYN_IMGUI_Scroll SYN_IMGUI_Context::scroll;
```



Scroll region state 
 


        

<hr>



### variable style 

```C++
SYN_IMGUI_Style SYN_IMGUI_Context::style;
```



Visual style — set via [**syn\_imgui\_set\_style()**](syn__imgui_8c.md#function-syn_imgui_set_style) 


        

<hr>



### variable touch\_down 

```C++
bool SYN_IMGUI_Context::touch_down;
```



True if screen is touched 


        

<hr>



### variable touch\_x 

```C++
int16_t SYN_IMGUI_Context::touch_x;
```



Touched X coordinate 


        

<hr>



### variable touch\_y 

```C++
int16_t SYN_IMGUI_Context::touch_y;
```



Touched Y coordinate 


        

<hr>



### variable updated\_focus 

```C++
bool SYN_IMGUI_Context::updated_focus;
```



Set when focused widget is visited 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ui/syn_imgui.h`

