

# Struct SYN\_FilterMA



[**ClassList**](annotated.md) **>** [**SYN\_FilterMA**](structSYN__FilterMA.md)



_Moving average filter state._ 

* `#include <syn_filter.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int16\_t | [**buf**](#variable-buf)  <br> |
|  uint8\_t | [**count**](#variable-count)  <br> |
|  uint8\_t | [**idx**](#variable-idx)  <br> |
|  int32\_t | [**sum**](#variable-sum)  <br> |
|  uint8\_t | [**window**](#variable-window)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
int16_t SYN_FilterMA::buf[SYN_FILTER_MAX_WINDOW];
```



Sample buffer 
 


        

<hr>



### variable count 

```C++
uint8_t SYN_FilterMA::count;
```



Samples inserted so far 
 


        

<hr>



### variable idx 

```C++
uint8_t SYN_FilterMA::idx;
```



Circular write index 
 


        

<hr>



### variable sum 

```C++
int32_t SYN_FilterMA::sum;
```



Running sum 
 


        

<hr>



### variable window 

```C++
uint8_t SYN_FilterMA::window;
```



Configured window size 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_filter.h`

