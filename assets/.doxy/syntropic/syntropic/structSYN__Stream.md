

# Struct SYN\_Stream



[**ClassList**](annotated.md) **>** [**SYN\_Stream**](structSYN__Stream.md)



_Cooperative byte stream — ringbuf with readability awareness._ [More...](#detailed-description)

* `#include <syn_stream.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**delim\_en**](#variable-delim_en)  <br> |
|  uint8\_t | [**delimiter**](#variable-delimiter)  <br> |
|  uint32\_t | [**overflow\_count**](#variable-overflow_count)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**overflowing**](#variable-overflowing)  <br> |
|  [**SYN\_RingBuf**](structSYN__RingBuf.md) | [**rb**](#variable-rb)  <br> |
|  size\_t | [**threshold**](#variable-threshold)  <br> |












































## Detailed Description


All fields are considered private. Use the API functions below. 


    
## Public Attributes Documentation




### variable delim\_en 

```C++
bool SYN_Stream::delim_en;
```



true if delimiter mode is active 
 


        

<hr>



### variable delimiter 

```C++
uint8_t SYN_Stream::delimiter;
```



Delimiter byte for line mode 
 


        

<hr>



### variable overflow\_count 

```C++
uint32_t SYN_Stream::overflow_count;
```



Total stream overflow events 
 


        

<hr>



### variable overflowing 

```C++
bool SYN_Stream::overflowing;
```



true if discarding tail after overflow 


        

<hr>



### variable rb 

```C++
SYN_RingBuf SYN_Stream::rb;
```



Underlying ring buffer (composition) 
 


        

<hr>



### variable threshold 

```C++
size_t SYN_Stream::threshold;
```



Readable when count &gt;= this (0 = any) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_stream.h`

