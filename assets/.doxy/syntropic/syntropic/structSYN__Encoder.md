

# Struct SYN\_Encoder



[**ClassList**](annotated.md) **>** [**SYN\_Encoder**](structSYN__Encoder.md)



_Quadrature encoder instance — pins, position, delta._ 

* `#include <syn_encoder.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**delta**](#variable-delta)  <br> |
|  [**int8\_t**](structSYN__Encoder.md#variable-delta) | [**last\_dir**](#variable-last_dir)  <br> |
|  [**uint8\_t**](structSYN__Encoder.md#variable-delta) | [**last\_state**](#variable-last_state)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin\_a**](#variable-pin_a)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin\_b**](#variable-pin_b)  <br> |
|  [**int32\_t**](structSYN__Encoder.md#variable-delta) | [**position**](#variable-position)  <br> |
|  [**SYN\_Signal**](structSYN__Signal.md) \* | [**stats**](#variable-stats)  <br> |
|  [**uint8\_t**](structSYN__Encoder.md#variable-delta) | [**steps\_per\_detent**](#variable-steps_per_detent)  <br> |
|  [**int8\_t**](structSYN__Encoder.md#variable-delta) | [**sub\_count**](#variable-sub_count)  <br> |












































## Public Attributes Documentation




### variable delta 

```C++
int32_t SYN_Encoder::delta;
```



Steps since last [**get\_delta()**](structSYN__Encoder.md#variable-delta) 
 


        

<hr>



### variable last\_dir 

```C++
int8_t SYN_Encoder::last_dir;
```



Last non-zero direction 
 


        

<hr>



### variable last\_state 

```C++
uint8_t SYN_Encoder::last_state;
```



Previous A/B state (2-bit) 
 


        

<hr>



### variable pin\_a 

```C++
SYN_GPIO_Pin SYN_Encoder::pin_a;
```



Channel A GPIO pin 
 


        

<hr>



### variable pin\_b 

```C++
SYN_GPIO_Pin SYN_Encoder::pin_b;
```



Channel B GPIO pin 
 


        

<hr>



### variable position 

```C++
int32_t SYN_Encoder::position;
```



Accumulated position (signed) 
 


        

<hr>



### variable stats 

```C++
SYN_Signal* SYN_Encoder::stats;
```



If set, delta pushed on get\_delta 


        

<hr>



### variable steps\_per\_detent 

```C++
uint8_t SYN_Encoder::steps_per_detent;
```



Typically 1, 2, or 4 
 


        

<hr>



### variable sub\_count 

```C++
int8_t SYN_Encoder::sub_count;
```



Sub-step accumulator 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_encoder.h`

