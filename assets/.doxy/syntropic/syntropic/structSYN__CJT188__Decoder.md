

# Struct SYN\_CJT188\_Decoder



[**ClassList**](annotated.md) **>** [**SYN\_CJT188\_Decoder**](structSYN__CJT188__Decoder.md)



_Streaming Decoder State Machine for CJ/T 188._ [More...](#detailed-description)

* `#include <syn_cjt188.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**uint8\_t**](structSYN__CJT188__Decoder.md#variable-index) | [**buf**](#variable-buf)  <br> |
|  [**size\_t**](structSYN__CJT188__Decoder.md#variable-index) | [**expected\_len**](#variable-expected_len)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**in\_frame**](#variable-in_frame)  <br> |
|  size\_t | [**index**](#variable-index)  <br> |












































## Detailed Description


Streaming Decoder State Machine for CJ/T 188. 


    
## Public Attributes Documentation




### variable buf 

```C++
uint8_t SYN_CJT188_Decoder::buf[SYN_CJT188_MAX_FRAME_SIZE];
```



Frame assembly buffer 


        

<hr>



### variable expected\_len 

```C++
size_t SYN_CJT188_Decoder::expected_len;
```



Total expected frame byte length 


        

<hr>



### variable in\_frame 

```C++
bool SYN_CJT188_Decoder::in_frame;
```



True if currently receiving active frame 


        

<hr>



### variable index 

```C++
size_t SYN_CJT188_Decoder::index;
```



Current byte index in buffer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cjt188.h`

