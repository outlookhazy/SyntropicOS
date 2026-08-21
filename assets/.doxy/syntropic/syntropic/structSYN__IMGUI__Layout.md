

# Struct SYN\_IMGUI\_Layout



[**ClassList**](annotated.md) **>** [**SYN\_IMGUI\_Layout**](structSYN__IMGUI__Layout.md)



_Auto-layout state. Managed by syn\_imgui\_layout\_begin/end._ [More...](#detailed-description)

* `#include <syn_imgui.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int16\_t | [**cx**](#variable-cx)  <br> |
|  int16\_t | [**cy**](#variable-cy)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**in\_layout**](#variable-in_layout)  <br> |
|  int16\_t | [**origin\_x**](#variable-origin_x)  <br> |
|  int16\_t | [**prev\_row\_h**](#variable-prev_row_h)  <br> |
|  int16\_t | [**row\_h**](#variable-row_h)  <br> |
|  int16\_t | [**row\_height**](#variable-row_height)  <br> |
|  int | [**row\_item\_idx**](#variable-row_item_idx)  <br> |
|  int | [**row\_items**](#variable-row_items)  <br> |
|  int16\_t | [**row\_widths**](#variable-row_widths)  <br> |
|  int16\_t | [**row\_x**](#variable-row_x)  <br> |
|  int16\_t | [**row\_y**](#variable-row_y)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**same\_line**](#variable-same_line)  <br> |
|  int16\_t | [**width**](#variable-width)  <br> |












































## Detailed Description


When active (in\_layout == true) and a widget receives x=0 y=0 w=0 h=0, it uses the cursor position and default sizes from this struct instead. Widgets that use the cursor advance it automatically. 


    
## Public Attributes Documentation




### variable cx 

```C++
int16_t SYN_IMGUI_Layout::cx;
```



Cursor X (left edge of next widget) 
 


        

<hr>



### variable cy 

```C++
int16_t SYN_IMGUI_Layout::cy;
```



Cursor Y (top edge of next widget) 
 


        

<hr>



### variable in\_layout 

```C++
bool SYN_IMGUI_Layout::in_layout;
```



True while between layout\_begin / layout\_end 
 


        

<hr>



### variable origin\_x 

```C++
int16_t SYN_IMGUI_Layout::origin_x;
```



Left margin for new rows 
 


        

<hr>



### variable prev\_row\_h 

```C++
int16_t SYN_IMGUI_Layout::prev_row_h;
```



Height of current row before layout advancement 
 


        

<hr>



### variable row\_h 

```C++
int16_t SYN_IMGUI_Layout::row_h;
```



Height of tallest widget in the current row 
 


        

<hr>



### variable row\_height 

```C++
int16_t SYN_IMGUI_Layout::row_height;
```



Forced row height (0 = auto) 
 


        

<hr>



### variable row\_item\_idx 

```C++
int SYN_IMGUI_Layout::row_item_idx;
```



Index of next column to consume 
 


        

<hr>



### variable row\_items 

```C++
int SYN_IMGUI_Layout::row_items;
```



Number of columns in current row (0 = default) 


        

<hr>



### variable row\_widths 

```C++
int16_t SYN_IMGUI_Layout::row_widths[SYN_IMGUI_MAX_ROW_COLS];
```



Column widths 
 


        

<hr>



### variable row\_x 

```C++
int16_t SYN_IMGUI_Layout::row_x;
```



X position of end of last resolved widget + spacing 


        

<hr>



### variable row\_y 

```C++
int16_t SYN_IMGUI_Layout::row_y;
```



Y position of current row start 
 


        

<hr>



### variable same\_line 

```C++
bool SYN_IMGUI_Layout::same_line;
```



True: next widget appends right, not new row 
 


        

<hr>



### variable width 

```C++
int16_t SYN_IMGUI_Layout::width;
```



Default widget width (set in layout\_begin) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ui/syn_imgui.h`

