

# Struct SYN\_ChangeFilter



[**ClassList**](annotated.md) **>** [**SYN\_ChangeFilter**](structSYN__ChangeFilter.md)



_Dead-band + rate-limited change filter._ 

* `#include <syn_change_filter.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**dead\_band**](#variable-dead_band)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialized**](#variable-initialized)  <br> |
|  uint32\_t | [**last\_time\_ms**](#variable-last_time_ms)  <br> |
|  float | [**last\_value**](#variable-last_value)  <br> |
|  uint32\_t | [**min\_interval\_ms**](#variable-min_interval_ms)  <br> |












































## Public Attributes Documentation




### variable dead\_band 

```C++
float SYN_ChangeFilter::dead_band;
```



Minimum \|delta\| to count as changed 
 


        

<hr>



### variable initialized 

```C++
bool SYN_ChangeFilter::initialized;
```



false until first update 
 


        

<hr>



### variable last\_time\_ms 

```C++
uint32_t SYN_ChangeFilter::last_time_ms;
```



Tick when last reported 
 


        

<hr>



### variable last\_value 

```C++
float SYN_ChangeFilter::last_value;
```



Last reported value 
 


        

<hr>



### variable min\_interval\_ms 

```C++
uint32_t SYN_ChangeFilter::min_interval_ms;
```



Minimum ms between reports (0=no limit) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_change_filter.h`

