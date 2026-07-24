

# File syn\_hysteresis.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_hysteresis.h**](syn__hysteresis_8h.md)

[Go to the source code of this file](syn__hysteresis_8h_source.md)

_Threshold comparator with hysteresis (deadband)._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Hysteresis**](structSYN__Hysteresis.md) <br>_Hysteresis comparator with configurable dead-band._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_hyst\_init**](#function-syn_hyst_init) ([**SYN\_Hysteresis**](structSYN__Hysteresis.md) \* h, int32\_t threshold, int32\_t band, [**bool**](syn__defs_8h.md#enum-bool) initial) <br>_Initialize a hysteresis comparator._  |
|  void | [**syn\_hyst\_set**](#function-syn_hyst_set) ([**SYN\_Hysteresis**](structSYN__Hysteresis.md) \* h, int32\_t threshold, int32\_t band) <br>_Update the threshold and band at runtime._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hyst\_state**](#function-syn_hyst_state) (const [**SYN\_Hysteresis**](structSYN__Hysteresis.md) \* h) <br>_Get the current output state without updating._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hyst\_update**](#function-syn_hyst_update) ([**SYN\_Hysteresis**](structSYN__Hysteresis.md) \* h, int32\_t value) <br>_Feed a new value and get the output state._  |


























## Detailed Description


Prevents rapid on/off cycling when a signal hovers near a threshold. Classic use cases: thermostat, battery low warning, tank level control.


Header-only — zero code size if unused.




**
**


```C++
SYN_Hysteresis hyst;
syn_hyst_init(&hyst, 1000, 50, false);  // threshold=1000, band=±50

// In sensor loop:
if (syn_hyst_update(&hyst, temperature)) {
    // crossed high threshold (1050) going up
    heater_off();
} else if (!syn_hyst_state(&hyst)) {
    // crossed low threshold (950) going down
    heater_on();
}
```
 





    
## Public Static Functions Documentation




### function syn\_hyst\_init 

_Initialize a hysteresis comparator._ 
```C++
static inline void syn_hyst_init (
    SYN_Hysteresis * h,
    int32_t threshold,
    int32_t band,
    bool initial
) 
```





**Parameters:**


* `h` Instance. 
* `threshold` Center threshold value. 
* `band` Half-width of hysteresis band. The high trip point is (threshold + band) and the low trip point is (threshold - band). 
* `initial` Initial output state. 




        

<hr>



### function syn\_hyst\_set 

_Update the threshold and band at runtime._ 
```C++
static inline void syn_hyst_set (
    SYN_Hysteresis * h,
    int32_t threshold,
    int32_t band
) 
```





**Parameters:**


* `h` Hysteresis instance. 
* `threshold` New center threshold. 
* `band` New half-width of dead-band. 




        

<hr>



### function syn\_hyst\_state 

_Get the current output state without updating._ 
```C++
static inline bool syn_hyst_state (
    const SYN_Hysteresis * h
) 
```





**Parameters:**


* `h` Hysteresis instance. 



**Returns:**

Current state. 





        

<hr>



### function syn\_hyst\_update 

_Feed a new value and get the output state._ 
```C++
static inline bool syn_hyst_update (
    SYN_Hysteresis * h,
    int32_t value
) 
```





**Parameters:**


* `h` Hysteresis instance. 
* `value` New input value. 



**Returns:**

true if the value crossed the high trip point (threshold + band) while state was low, or remains above the low trip point. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_hysteresis.h`

