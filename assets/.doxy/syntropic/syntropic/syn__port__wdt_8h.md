

# File syn\_port\_wdt.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_wdt.h**](syn__port__wdt_8h.md)

[Go to the source code of this file](syn__port__wdt_8h_source.md)

_Platform port: Hardware Watchdog Timer (WDT)._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include "../common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_port\_wdt\_feed**](#function-syn_port_wdt_feed) (void) <br>_Feed (pet) the hardware watchdog, resetting its counter._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_wdt\_init**](#function-syn_port_wdt_init) (uint32\_t timeout\_ms) <br>_Configure and start the hardware watchdog._  |




























## Detailed Description


The hardware watchdog resets the MCU if not fed within the timeout window — even if the CPU is hard-locked (unlike the software watchdog in [**syn\_watchdog.h**](syn__watchdog_8h.md) which requires the scheduler to keep running). 


    
## Public Functions Documentation




### function syn\_port\_wdt\_feed 

_Feed (pet) the hardware watchdog, resetting its counter._ 
```C++
void syn_port_wdt_feed (
    void
) 
```



Must be called more frequently than the timeout configured in [**syn\_port\_wdt\_init()**](syn__port__wdt_8h.md#function-syn_port_wdt_init) to prevent a hardware reset. 


        

<hr>



### function syn\_port\_wdt\_init 

_Configure and start the hardware watchdog._ 
```C++
SYN_Status syn_port_wdt_init (
    uint32_t timeout_ms
) 
```



Once started, the watchdog cannot be stopped without a reset on most MCUs. Call [**syn\_port\_wdt\_feed()**](syn__port__wdt_8h.md#function-syn_port_wdt_feed) regularly within the timeout window.




**Parameters:**


* `timeout_ms` Desired watchdog timeout in milliseconds. The port will round to the nearest supported value. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if the WDT cannot be started. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_wdt.h`

