

# File syn\_comp.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_comp.h**](syn__comp_8h.md)

[Go to the source code of this file](syn__comp_8h_source.md)

_High-Speed Rail-to-Rail Analog Comparator Driver._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_comp.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_COMP**](structSYN__COMP.md) <br>_Analog Comparator handle. Caller allocates; zero heap._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_comp\_enable**](#function-syn_comp_enable) ([**SYN\_COMP**](structSYN__COMP.md) \* comp, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable the analog comparator._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_comp\_init**](#function-syn_comp_init) ([**SYN\_COMP**](structSYN__COMP.md) \* comp, uint8\_t comp\_id, [**SYN\_COMP\_InvertingInput**](syn__port__comp_8h.md#enum-syn_comp_invertinginput) inv\_in) <br>_Initialize an Analog Comparator instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_comp\_read**](#function-syn_comp_read) (const [**SYN\_COMP**](structSYN__COMP.md) \* comp) <br>_Read real-time logical output state of the comparator._  |




























## Detailed Description


Provides a vendor-agnostic, zero-allocation OS interface for internal analog comparators with configurable voltage reference thresholds (VREFINT 1.23V, DAC, or external).


Usage: 
```C++
SYN_COMP comp;
syn_comp_init(&comp, 0, SYN_COMP_INV_VREFINT); // COMP1 vs 1.23V VREFINT
syn_comp_enable(&comp, true);
bool state = syn_comp_read(&comp);             // True if PA1 > 1.23V
```
 


    
## Public Functions Documentation




### function syn\_comp\_enable 

_Enable or disable the analog comparator._ 
```C++
SYN_Status syn_comp_enable (
    SYN_COMP * comp,
    bool enable
) 
```





**Parameters:**


* `comp` Initialized comparator handle. 
* `enable` true to enable, false to disable. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_comp\_init 

_Initialize an Analog Comparator instance._ 
```C++
SYN_Status syn_comp_init (
    SYN_COMP * comp,
    uint8_t comp_id,
    SYN_COMP_InvertingInput inv_in
) 
```





**Parameters:**


* `comp` Handle to initialize. Must not be NULL. 
* `comp_id` Comparator hardware index (0 = COMP1, 1 = COMP2). 
* `inv_in` Reference threshold input source. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid argument. 





        

<hr>



### function syn\_comp\_read 

_Read real-time logical output state of the comparator._ 
```C++
bool syn_comp_read (
    const SYN_COMP * comp
) 
```





**Parameters:**


* `comp` Initialized comparator handle. 



**Returns:**

true if non-inverting input voltage &gt; inverting reference voltage. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_comp.h`

