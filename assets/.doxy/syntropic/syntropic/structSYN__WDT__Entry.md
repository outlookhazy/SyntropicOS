

# Struct SYN\_WDT\_Entry



[**ClassList**](annotated.md) **>** [**SYN\_WDT\_Entry**](structSYN__WDT__Entry.md)



_Single watchdog entry for task monitoring._ 

* `#include <syn_watchdog.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint32\_t | [**last\_checkin**](#variable-last_checkin)  <br> |
|  const char \* | [**name**](#variable-name)  <br> |
|  uint32\_t | [**timeout\_ms**](#variable-timeout_ms)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_WDT_Entry::active;
```



Is this entry in use? 
 


        

<hr>



### variable last\_checkin 

```C++
uint32_t SYN_WDT_Entry::last_checkin;
```



Tick of last check-in 
 


        

<hr>



### variable name 

```C++
const char* SYN_WDT_Entry::name;
```



Task name (for debug) 
 


        

<hr>



### variable timeout\_ms 

```C++
uint32_t SYN_WDT_Entry::timeout_ms;
```



Deadline in milliseconds 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_watchdog.h`

