

# Struct SYN\_Signal



[**ClassList**](annotated.md) **>** [**SYN\_Signal**](structSYN__Signal.md)



_Signal statistics — sliding window with incremental min/max/mean._ 

* `#include <syn_signal.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t \* | [**buf**](#variable-buf)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**cache\_valid**](#variable-cache_valid)  <br> |
|  int32\_t | [**cached\_max**](#variable-cached_max)  <br> |
|  int32\_t | [**cached\_min**](#variable-cached_min)  <br> |
|  size\_t | [**capacity**](#variable-capacity)  <br> |
|  size\_t | [**count**](#variable-count)  <br> |
|  size\_t | [**head**](#variable-head)  <br> |
|  int64\_t | [**sum**](#variable-sum)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
int32_t* SYN_Signal::buf;
```



Sample buffer (circular) 
 


        

<hr>



### variable cache\_valid 

```C++
bool SYN_Signal::cache_valid;
```



True if min/max caches are current 
 


        

<hr>



### variable cached\_max 

```C++
int32_t SYN_Signal::cached_max;
```



Cached max (invalidated on push) 
 


        

<hr>



### variable cached\_min 

```C++
int32_t SYN_Signal::cached_min;
```



Cached min (invalidated on push) 
 


        

<hr>



### variable capacity 

```C++
size_t SYN_Signal::capacity;
```



Buffer size 
 


        

<hr>



### variable count 

```C++
size_t SYN_Signal::count;
```



Number of valid samples (≤ capacity) 
 


        

<hr>



### variable head 

```C++
size_t SYN_Signal::head;
```



Next write position 
 


        

<hr>



### variable sum 

```C++
int64_t SYN_Signal::sum;
```



Sum of all samples in window 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_signal.h`

