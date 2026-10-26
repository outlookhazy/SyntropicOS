

# Struct SYN\_GCode\_Block



[**ClassList**](annotated.md) **>** [**SYN\_GCode\_Block**](structSYN__GCode__Block.md)



_Parsed RS-274 G-Code line block._ 

* `#include <syn_gcode.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**f**](#variable-f)  <br> |
|  uint32\_t | [**g\_code**](#variable-g_code)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_f**](#variable-has_f)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_g**](#variable-has_g)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_i**](#variable-has_i)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_j**](#variable-has_j)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_k**](#variable-has_k)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_m**](#variable-has_m)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_p**](#variable-has_p)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_s**](#variable-has_s)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_t**](#variable-has_t)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_x**](#variable-has_x)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_y**](#variable-has_y)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_z**](#variable-has_z)  <br> |
|  float | [**i**](#variable-i)  <br> |
|  float | [**j**](#variable-j)  <br> |
|  float | [**k**](#variable-k)  <br> |
|  uint32\_t | [**m\_code**](#variable-m_code)  <br> |
|  float | [**p**](#variable-p)  <br> |
|  float | [**s**](#variable-s)  <br> |
|  uint32\_t | [**t**](#variable-t)  <br> |
|  float | [**x**](#variable-x)  <br> |
|  float | [**y**](#variable-y)  <br> |
|  float | [**z**](#variable-z)  <br> |












































## Public Attributes Documentation




### variable f 

```C++
float SYN_GCode_Block::f;
```



Feedrate in units/min or units/sec 


        

<hr>



### variable g\_code 

```C++
uint32_t SYN_GCode_Block::g_code;
```



Numeric G-code (e.g. 0, 1, 2, 3, 90, 91) 


        

<hr>



### variable has\_f 

```C++
bool SYN_GCode_Block::has_f;
```



True if F word was present 


        

<hr>



### variable has\_g 

```C++
bool SYN_GCode_Block::has_g;
```



True if G-code was present 


        

<hr>



### variable has\_i 

```C++
bool SYN_GCode_Block::has_i;
```



True if I word was present 


        

<hr>



### variable has\_j 

```C++
bool SYN_GCode_Block::has_j;
```



True if J word was present 


        

<hr>



### variable has\_k 

```C++
bool SYN_GCode_Block::has_k;
```



True if K word was present 


        

<hr>



### variable has\_m 

```C++
bool SYN_GCode_Block::has_m;
```



True if M-code was present 


        

<hr>



### variable has\_p 

```C++
bool SYN_GCode_Block::has_p;
```



True if P word was present 


        

<hr>



### variable has\_s 

```C++
bool SYN_GCode_Block::has_s;
```



True if S word was present 


        

<hr>



### variable has\_t 

```C++
bool SYN_GCode_Block::has_t;
```



True if T word was present 


        

<hr>



### variable has\_x 

```C++
bool SYN_GCode_Block::has_x;
```



True if X word was present 


        

<hr>



### variable has\_y 

```C++
bool SYN_GCode_Block::has_y;
```



True if Y word was present 


        

<hr>



### variable has\_z 

```C++
bool SYN_GCode_Block::has_z;
```



True if Z word was present 


        

<hr>



### variable i 

```C++
float SYN_GCode_Block::i;
```



Arc center X offset 


        

<hr>



### variable j 

```C++
float SYN_GCode_Block::j;
```



Arc center Y offset 


        

<hr>



### variable k 

```C++
float SYN_GCode_Block::k;
```



Arc center Z offset 


        

<hr>



### variable m\_code 

```C++
uint32_t SYN_GCode_Block::m_code;
```



Numeric M-code (e.g. 3, 4, 5, 8, 9, 30) 


        

<hr>



### variable p 

```C++
float SYN_GCode_Block::p;
```



Parameter / Dwell seconds 


        

<hr>



### variable s 

```C++
float SYN_GCode_Block::s;
```



Spindle speed (RPM) 


        

<hr>



### variable t 

```C++
uint32_t SYN_GCode_Block::t;
```



Tool select index 


        

<hr>



### variable x 

```C++
float SYN_GCode_Block::x;
```



Target X coordinate 


        

<hr>



### variable y 

```C++
float SYN_GCode_Block::y;
```



Target Y coordinate 


        

<hr>



### variable z 

```C++
float SYN_GCode_Block::z;
```



Target Z coordinate 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_gcode.h`

