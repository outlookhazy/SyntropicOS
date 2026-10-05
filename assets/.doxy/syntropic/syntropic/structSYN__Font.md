

# Struct SYN\_Font



[**ClassList**](annotated.md) **>** [**SYN\_Font**](structSYN__Font.md)



_Font descriptor for text rendering._ 

* `#include <syn_canvas.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**char\_count**](#variable-char_count)  <br> |
|  const uint8\_t \* | [**data**](#variable-data)  <br> |
|  uint8\_t | [**first\_char**](#variable-first_char)  <br> |
|  uint8\_t | [**height**](#variable-height)  <br> |
|  uint8\_t | [**width**](#variable-width)  <br> |












































## Public Attributes Documentation




### variable char\_count 

```C++
uint8_t SYN_Font::char_count;
```



Number of glyphs 
 


        

<hr>



### variable data 

```C++
const uint8_t* SYN_Font::data;
```



Bitmap data (column-major per glyph) 
 


        

<hr>



### variable first\_char 

```C++
uint8_t SYN_Font::first_char;
```



First ASCII code (usually 32) 
 


        

<hr>



### variable height 

```C++
uint8_t SYN_Font::height;
```



Glyph height in pixels 
 


        

<hr>



### variable width 

```C++
uint8_t SYN_Font::width;
```



Glyph width in pixels 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/display/syn_canvas.h`

