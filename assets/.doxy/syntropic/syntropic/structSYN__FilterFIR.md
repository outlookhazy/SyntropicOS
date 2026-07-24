

# Struct SYN\_FilterFIR



[**ClassList**](annotated.md) **>** [**SYN\_FilterFIR**](structSYN__FilterFIR.md)



_FIR filter state (Q16 fixed-point)._ 

* `#include <syn_filter.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**head**](#variable-head)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* | [**history**](#variable-history)  <br> |
|  uint16\_t | [**num\_taps**](#variable-num_taps)  <br> |
|  const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* | [**taps**](#variable-taps)  <br> |












































## Public Attributes Documentation




### variable head 

```C++
uint16_t SYN_FilterFIR::head;
```



Circular history write index 
 


        

<hr>



### variable history 

```C++
q16_t* SYN_FilterFIR::history;
```



Sample history buffer (length num\_taps) 
 


        

<hr>



### variable num\_taps 

```C++
uint16_t SYN_FilterFIR::num_taps;
```



Number of taps / filter length 
 


        

<hr>



### variable taps 

```C++
const q16_t* SYN_FilterFIR::taps;
```



Filter coefficient array (length num\_taps) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_filter.h`

