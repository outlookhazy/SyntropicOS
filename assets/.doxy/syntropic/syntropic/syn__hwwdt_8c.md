

# File syn\_hwwdt.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_hwwdt.c**](syn__hwwdt_8c.md)

[Go to the source code of this file](syn__hwwdt_8c_source.md)

_Hardware Watchdog Timer driver implementation._ 

* `#include "syn_hwwdt.h"`
* `#include "../port/syn_port_wdt.h"`
* `#include "../util/syn_assert.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_hwwdt\_feed**](#function-syn_hwwdt_feed) (void) <br>_Feed (reload) the hardware watchdog counter._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hwwdt\_init**](#function-syn_hwwdt_init) (uint32\_t timeout\_ms) <br>_Start the hardware watchdog with the given timeout._  |




























## Public Functions Documentation




### function syn\_hwwdt\_feed 

_Feed (reload) the hardware watchdog counter._ 
```C++
void syn_hwwdt_feed (
    void
) 
```



Call this function from your main loop or scheduler idle hook to prevent the hardware from resetting the system. 


        

<hr>



### function syn\_hwwdt\_init 

_Start the hardware watchdog with the given timeout._ 
```C++
SYN_Status syn_hwwdt_init (
    uint32_t timeout_ms
) 
```



On most MCUs the watchdog cannot be disabled once started. [**syn\_hwwdt\_feed()**](syn__hwwdt_8h.md#function-syn_hwwdt_feed) must be called more frequently than `timeout_ms`.




**Parameters:**


* `timeout_ms` Desired timeout in milliseconds. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if the hardware WDT cannot be started. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_hwwdt.c`

