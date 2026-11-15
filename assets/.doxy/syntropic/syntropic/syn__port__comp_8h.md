

# File syn\_port\_comp.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_comp.h**](syn__port__comp_8h.md)

[Go to the source code of this file](syn__port__comp_8h_source.md)

_Platform Port Interface for Analog Comparators._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_COMP\_InvertingInput**](#enum-syn_comp_invertinginput)  <br>_Inverting input selection for comparator._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_comp\_enable**](#function-syn_port_comp_enable) (uint8\_t comp\_id, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable hardware analog comparator._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_comp\_init**](#function-syn_port_comp_init) (uint8\_t comp\_id, [**SYN\_COMP\_InvertingInput**](syn__port__comp_8h.md#enum-syn_comp_invertinginput) inv\_in) <br>_Initialize hardware analog comparator instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_comp\_read**](#function-syn_port_comp_read) (uint8\_t comp\_id) <br>_Read real-time logical output state of the hardware comparator._  |




























## Detailed Description


Defines low-level target hardware bindings for internal high-speed analog comparators, threshold selection (internal VREFINT / DAC / external pin), and output state reading. 


    
## Public Types Documentation




### enum SYN\_COMP\_InvertingInput 

_Inverting input selection for comparator._ 
```C++
enum SYN_COMP_InvertingInput {
    SYN_COMP_INV_VREFINT_1_4 = 0,
    SYN_COMP_INV_VREFINT_1_2 = 1,
    SYN_COMP_INV_VREFINT_3_4 = 2,
    SYN_COMP_INV_VREFINT = 3,
    SYN_COMP_INV_DAC1_CH1 = 4,
    SYN_COMP_INV_EXTERNAL = 5
};
```




<hr>
## Public Functions Documentation




### function syn\_port\_comp\_enable 

_Enable or disable hardware analog comparator._ 
```C++
SYN_Status syn_port_comp_enable (
    uint8_t comp_id,
    bool enable
) 
```





**Parameters:**


* `comp_id` Comparator hardware index. 
* `enable` true to enable comparator, false to disable. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_comp\_init 

_Initialize hardware analog comparator instance._ 
```C++
SYN_Status syn_port_comp_init (
    uint8_t comp_id,
    SYN_COMP_InvertingInput inv_in
) 
```





**Parameters:**


* `comp_id` Comparator hardware index (0 = COMP1, 1 = COMP2). 
* `inv_in` Inverting reference input source. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on hardware error. 





        

<hr>



### function syn\_port\_comp\_read 

_Read real-time logical output state of the hardware comparator._ 
```C++
bool syn_port_comp_read (
    uint8_t comp_id
) 
```





**Parameters:**


* `comp_id` Comparator hardware index. 



**Returns:**

true if non-inverting input &gt; inverting reference, false otherwise. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_comp.h`

