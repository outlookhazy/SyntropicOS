

# Struct SYN\_Sleep



[**ClassList**](annotated.md) **>** [**SYN\_Sleep**](structSYN__Sleep.md)



_Sleep coordinator — tracks wake locks and sleep statistics._ 

* `#include <syn_sleep.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enabled**](#variable-enabled)  <br> |
|  uint32\_t | [**enter\_count**](#variable-enter_count)  <br> |
|  volatile uint32\_t | [**lock\_mask**](#variable-lock_mask)  <br> |
|  [**SYN\_SleepMode**](syn__sleep_8h.md#enum-syn_sleepmode) | [**max\_depth**](#variable-max_depth)  <br> |
|  uint32\_t | [**veto\_count**](#variable-veto_count)  <br> |












































## Public Attributes Documentation




### variable enabled 

```C++
bool SYN_Sleep::enabled;
```



Global enable 
 


        

<hr>



### variable enter\_count 

```C++
uint32_t SYN_Sleep::enter_count;
```



Times we actually slept 
 


        

<hr>



### variable lock\_mask 

```C++
volatile uint32_t SYN_Sleep::lock_mask;
```



Active wake locks (bitmask) 
 


        

<hr>



### variable max\_depth 

```C++
SYN_SleepMode SYN_Sleep::max_depth;
```



Deepest allowed sleep mode 
 


        

<hr>



### variable veto\_count 

```C++
uint32_t SYN_Sleep::veto_count;
```



Times sleep was vetoed 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_sleep.h`

