

# File syn\_port\_rtc.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_rtc.h**](syn__port__rtc_8h.md)

[Go to the source code of this file](syn__port__rtc_8h_source.md)

_Platform port: Real-Time Clock (RTC)._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include "../common/syn_defs.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) <br>_Calendar date and time representation._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_rtc\_get**](#function-syn_port_rtc_get) ([**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Read the current date/time from the hardware RTC._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_rtc\_init**](#function-syn_port_rtc_init) (void) <br>_Initialize the RTC peripheral._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_rtc\_set**](#function-syn_port_rtc_set) (const [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Set the hardware RTC to the given date/time._  |




























## Detailed Description


Implementors provide these three functions for the target hardware RTC. On host (tests) they are backed by mock\_port.c. 


    
## Public Functions Documentation




### function syn\_port\_rtc\_get 

_Read the current date/time from the hardware RTC._ 
```C++
SYN_Status syn_port_rtc_get (
    SYN_RTC_DateTime * dt
) 
```





**Parameters:**


* `dt` Output parameter, filled by the port. Must not be NULL. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on read failure. 





        

<hr>



### function syn\_port\_rtc\_init 

_Initialize the RTC peripheral._ 
```C++
SYN_Status syn_port_rtc_init (
    void
) 
```





**Returns:**

SYN\_OK on success, SYN\_ERROR if the peripheral cannot be started. 





        

<hr>



### function syn\_port\_rtc\_set 

_Set the hardware RTC to the given date/time._ 
```C++
SYN_Status syn_port_rtc_set (
    const SYN_RTC_DateTime * dt
) 
```





**Parameters:**


* `dt` New time to program. Must not be NULL. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on write failure. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_rtc.h`

