

# File syn\_change\_filter.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_change\_filter.h**](syn__change__filter_8h.md)

[Go to the source code of this file](syn__change__filter_8h_source.md)

_Dead-band + rate-limited change detector (header-only)._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_ChangeFilter**](structSYN__ChangeFilter.md) <br>_Dead-band + rate-limited change filter._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_change\_filter\_init**](#function-syn_change_filter_init) ([**SYN\_ChangeFilter**](structSYN__ChangeFilter.md) \* cf, float dead\_band, uint32\_t min\_interval\_ms) <br>_Initialize a change filter._  |
|  float | [**syn\_change\_filter\_last**](#function-syn_change_filter_last) (const [**SYN\_ChangeFilter**](structSYN__ChangeFilter.md) \* cf) <br>_Get the last reported value._  |
|  void | [**syn\_change\_filter\_reset**](#function-syn_change_filter_reset) ([**SYN\_ChangeFilter**](structSYN__ChangeFilter.md) \* cf) <br>_Force the filter to report on next update._  |
|  void | [**syn\_change\_filter\_set**](#function-syn_change_filter_set) ([**SYN\_ChangeFilter**](structSYN__ChangeFilter.md) \* cf, float dead\_band, uint32\_t min\_interval\_ms) <br>_Update dead-band and interval at runtime._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_change\_filter\_update**](#function-syn_change_filter_update) ([**SYN\_ChangeFilter**](structSYN__ChangeFilter.md) \* cf, float value, uint32\_t now\_ms) <br>_Feed a new value and check if it should be reported._  |


























## Detailed Description


Reports a value as "changed" only when the difference from the last reported value exceeds a dead-band AND a minimum time interval has elapsed. Useful for suppressing noise in telemetry, sensor logging, display updates, or any push-on-change pattern.


Header-only — zero code size if unused.




**
**


```C++
SYN_ChangeFilter cf;
syn_change_filter_init(&cf, 0.5f, 1000);  // dead_band=0.5, min_ms=1000

// In sensor loop:
float temp = read_temperature();
if (syn_change_filter_update(&cf, temp, now_ms)) {
    // Value changed significantly — transmit / log / display
    send_update(temp);
}
```
 





    
## Public Static Functions Documentation




### function syn\_change\_filter\_init 

_Initialize a change filter._ 
```C++
static inline void syn_change_filter_init (
    SYN_ChangeFilter * cf,
    float dead_band,
    uint32_t min_interval_ms
) 
```





**Parameters:**


* `cf` Instance. 
* `dead_band` Minimum absolute change to report. Use 0.0f to report any change. 
* `min_interval_ms` Minimum milliseconds between reports. Use 0 to disable rate limiting. 




        

<hr>



### function syn\_change\_filter\_last 

_Get the last reported value._ 
```C++
static inline float syn_change_filter_last (
    const SYN_ChangeFilter * cf
) 
```





**Parameters:**


* `cf` Filter instance. 



**Returns:**

Last value that passed the filter. 





        

<hr>



### function syn\_change\_filter\_reset 

_Force the filter to report on next update._ 
```C++
static inline void syn_change_filter_reset (
    SYN_ChangeFilter * cf
) 
```



Resets the initialized flag so the next call to update() returns true regardless of dead-band or interval.




**Parameters:**


* `cf` Filter instance. 




        

<hr>



### function syn\_change\_filter\_set 

_Update dead-band and interval at runtime._ 
```C++
static inline void syn_change_filter_set (
    SYN_ChangeFilter * cf,
    float dead_band,
    uint32_t min_interval_ms
) 
```





**Parameters:**


* `cf` Filter instance. 
* `dead_band` New dead-band threshold. 
* `min_interval_ms` New minimum reporting interval (ms). 




        

<hr>



### function syn\_change\_filter\_update 

_Feed a new value and check if it should be reported._ 
```C++
static inline bool syn_change_filter_update (
    SYN_ChangeFilter * cf,
    float value,
    uint32_t now_ms
) 
```



The first call always returns true (initial value).




**Parameters:**


* `cf` Instance. 
* `value` New value to test. 
* `now_ms` Current tick in milliseconds. 



**Returns:**

true if the value should be reported. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_change_filter.h`

