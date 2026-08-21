

# Struct SYN\_Canvas



[**ClassList**](annotated.md) **>** [**SYN\_Canvas**](structSYN__Canvas.md)



_Canvas instance — framebuffer + drawing state._ 

* `#include <syn_canvas.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**bpp**](#variable-bpp)  <br> |
|  size\_t | [**buf\_size**](#variable-buf_size)  <br> |
|  int16\_t | [**clip\_h**](#variable-clip_h)  <br> |
|  int16\_t | [**clip\_w**](#variable-clip_w)  <br> |
|  int16\_t | [**clip\_x**](#variable-clip_x)  <br> |
|  int16\_t | [**clip\_y**](#variable-clip_y)  <br> |
|  void \* | [**flush\_ctx**](#variable-flush_ctx)  <br> |
|  [**SYN\_Canvas\_FlushFn**](syn__canvas_8h.md#typedef-syn_canvas_flushfn) | [**flush\_fn**](#variable-flush_fn)  <br> |
|  const [**SYN\_Font**](structSYN__Font.md) \* | [**font**](#variable-font)  <br> |
|  uint8\_t \* | [**framebuf**](#variable-framebuf)  <br> |
|  uint16\_t | [**height**](#variable-height)  <br> |
|  uint16\_t | [**width**](#variable-width)  <br> |












































## Public Attributes Documentation




### variable bpp 

```C++
uint8_t SYN_Canvas::bpp;
```



Bits per pixel (1 or 16) 
 


        

<hr>



### variable buf\_size 

```C++
size_t SYN_Canvas::buf_size;
```



Total buffer size (bytes) 
 


        

<hr>



### variable clip\_h 

```C++
int16_t SYN_Canvas::clip_h;
```



Clip region height 
 


        

<hr>



### variable clip\_w 

```C++
int16_t SYN_Canvas::clip_w;
```



Clip region width 
 


        

<hr>



### variable clip\_x 

```C++
int16_t SYN_Canvas::clip_x;
```



Clip region left edge 
 


        

<hr>



### variable clip\_y 

```C++
int16_t SYN_Canvas::clip_y;
```



Clip region top edge 
 


        

<hr>



### variable flush\_ctx 

```C++
void* SYN_Canvas::flush_ctx;
```



Context for flush callback 
 


        

<hr>



### variable flush\_fn 

```C++
SYN_Canvas_FlushFn SYN_Canvas::flush_fn;
```



Push framebuf to display 
 


        

<hr>



### variable font 

```C++
const SYN_Font* SYN_Canvas::font;
```



Active font 
 


        

<hr>



### variable framebuf 

```C++
uint8_t* SYN_Canvas::framebuf;
```



Caller-owned pixel buffer 
 


        

<hr>



### variable height 

```C++
uint16_t SYN_Canvas::height;
```



Display height in pixels 
 


        

<hr>



### variable width 

```C++
uint16_t SYN_Canvas::width;
```



Display width in pixels 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/display/syn_canvas.h`

