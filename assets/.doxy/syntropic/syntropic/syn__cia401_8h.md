

# File syn\_cia401.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia401.h**](syn__cia401_8h.md)

[Go to the source code of this file](syn__cia401_8h_source.md)

_CANopen CiA 401 Device Profile for Generic I/O Modules._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include "../common/syn_defs.h"`
* `#include "syn_canopen.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CiA401\_Device**](structSYN__CiA401__Device.md) <br>_CiA 401 Generic I/O Device State Instance._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cia401\_check\_analog\_delta**](#function-syn_cia401_check_analog_delta) ([**SYN\_CiA401\_Device**](structSYN__CiA401__Device.md) \* dev, uint8\_t \* channel) <br>_Check if any analog input has exceeded its delta threshold for event-driven TPDO._  |
|  void | [**syn\_cia401\_init**](#function-syn_cia401_init) ([**SYN\_CiA401\_Device**](structSYN__CiA401__Device.md) \* dev, uint8\_t dig\_in\_bytes, uint8\_t dig\_out\_bytes, uint8\_t analog\_in\_ch, uint8\_t analog\_out\_ch) <br>_Initialize a CiA 401 device state instance._  |
|  size\_t | [**syn\_cia401\_populate\_od**](#function-syn_cia401_populate_od) ([**SYN\_CiA401\_Device**](structSYN__CiA401__Device.md) \* dev, [**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md) \* od\_table, size\_t max\_entries) <br>_Populate a standard CANopen Object Dictionary array with CiA 401 entries._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_CIA401\_MAX\_ANALOG\_16BIT**](syn__cia401_8h.md#define-syn_cia401_max_analog_16bit)  `8`<br> |
| define  | [**SYN\_CIA401\_MAX\_DIGITAL\_8BIT**](syn__cia401_8h.md#define-syn_cia401_max_digital_8bit)  `4`<br>_Maximum channel capacities for CiA 401 instance._  |
| define  | [**SYN\_CIA401\_OD\_ANALOG\_INPUT\_16BIT**](syn__cia401_8h.md#define-syn_cia401_od_analog_input_16bit)  `0x6401U`<br> |
| define  | [**SYN\_CIA401\_OD\_ANALOG\_INPUT\_DELTA**](syn__cia401_8h.md#define-syn_cia401_od_analog_input_delta)  `0x6421U`<br> |
| define  | [**SYN\_CIA401\_OD\_ANALOG\_INPUT\_LIMIT\_H**](syn__cia401_8h.md#define-syn_cia401_od_analog_input_limit_h)  `0x6425U`<br> |
| define  | [**SYN\_CIA401\_OD\_ANALOG\_INPUT\_LIMIT\_L**](syn__cia401_8h.md#define-syn_cia401_od_analog_input_limit_l)  `0x6424U`<br> |
| define  | [**SYN\_CIA401\_OD\_ANALOG\_OUTPUT\_16BIT**](syn__cia401_8h.md#define-syn_cia401_od_analog_output_16bit)  `0x6411U`<br> |
| define  | [**SYN\_CIA401\_OD\_DIGITAL\_INPUT\_8BIT**](syn__cia401_8h.md#define-syn_cia401_od_digital_input_8bit)  `0x6000U`<br>_CiA 401 Standard Object Dictionary Indices._  |
| define  | [**SYN\_CIA401\_OD\_DIGITAL\_OUTPUT\_8BIT**](syn__cia401_8h.md#define-syn_cia401_od_digital_output_8bit)  `0x6200U`<br> |

## Detailed Description


Implements CiA 401 Object Dictionary definitions and helper binding state for 8-bit digital inputs/outputs and 16-bit analog inputs/outputs. 


    
## Public Functions Documentation




### function syn\_cia401\_check\_analog\_delta 

_Check if any analog input has exceeded its delta threshold for event-driven TPDO._ 
```C++
bool syn_cia401_check_analog_delta (
    SYN_CiA401_Device * dev,
    uint8_t * channel
) 
```





**Parameters:**


* `dev` Pointer to CiA 401 device instance. 
* `channel` Output channel index (0..analog\_in\_count-1) that changed. 



**Returns:**

True if a change occurred exceeding delta. 





        

<hr>



### function syn\_cia401\_init 

_Initialize a CiA 401 device state instance._ 
```C++
void syn_cia401_init (
    SYN_CiA401_Device * dev,
    uint8_t dig_in_bytes,
    uint8_t dig_out_bytes,
    uint8_t analog_in_ch,
    uint8_t analog_out_ch
) 
```





**Parameters:**


* `dev` Pointer to CiA 401 device handle. 
* `dig_in_bytes` Number of 8-bit digital input blocks (1..4). 
* `dig_out_bytes` Number of 8-bit digital output blocks (1..4). 
* `analog_in_ch` Number of 16-bit analog input channels (1..8). 
* `analog_out_ch` Number of 16-bit analog output channels (1..8). 




        

<hr>



### function syn\_cia401\_populate\_od 

_Populate a standard CANopen Object Dictionary array with CiA 401 entries._ 
```C++
size_t syn_cia401_populate_od (
    SYN_CiA401_Device * dev,
    SYN_CANOpenODEntry * od_table,
    size_t max_entries
) 
```





**Parameters:**


* `dev` Pointer to initialized CiA 401 device instance. 
* `od_table` Output array of [**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md) elements. 
* `max_entries` Capacity of od\_table array. 



**Returns:**

Number of OD entries populated. 





        

<hr>
## Macro Definition Documentation





### define SYN\_CIA401\_MAX\_ANALOG\_16BIT 

```C++
#define SYN_CIA401_MAX_ANALOG_16BIT `8`
```



Maximum 16-bit analog I/O channels 


        

<hr>



### define SYN\_CIA401\_MAX\_DIGITAL\_8BIT 

_Maximum channel capacities for CiA 401 instance._ 
```C++
#define SYN_CIA401_MAX_DIGITAL_8BIT `4`
```



Maximum 8-bit digital I/O ports 


        

<hr>



### define SYN\_CIA401\_OD\_ANALOG\_INPUT\_16BIT 

```C++
#define SYN_CIA401_OD_ANALOG_INPUT_16BIT `0x6401U`
```



Read 16-bit Analog Inputs 


        

<hr>



### define SYN\_CIA401\_OD\_ANALOG\_INPUT\_DELTA 

```C++
#define SYN_CIA401_OD_ANALOG_INPUT_DELTA `0x6421U`
```



Analog Input Delta Interrupt Threshold 


        

<hr>



### define SYN\_CIA401\_OD\_ANALOG\_INPUT\_LIMIT\_H 

```C++
#define SYN_CIA401_OD_ANALOG_INPUT_LIMIT_H `0x6425U`
```



Analog Input High Limit Alarm 


        

<hr>



### define SYN\_CIA401\_OD\_ANALOG\_INPUT\_LIMIT\_L 

```C++
#define SYN_CIA401_OD_ANALOG_INPUT_LIMIT_L `0x6424U`
```



Analog Input Low Limit Alarm 


        

<hr>



### define SYN\_CIA401\_OD\_ANALOG\_OUTPUT\_16BIT 

```C++
#define SYN_CIA401_OD_ANALOG_OUTPUT_16BIT `0x6411U`
```



Write 16-bit Analog Outputs 


        

<hr>



### define SYN\_CIA401\_OD\_DIGITAL\_INPUT\_8BIT 

_CiA 401 Standard Object Dictionary Indices._ 
```C++
#define SYN_CIA401_OD_DIGITAL_INPUT_8BIT `0x6000U`
```



Read 8-bit Digital Inputs 


        

<hr>



### define SYN\_CIA401\_OD\_DIGITAL\_OUTPUT\_8BIT 

```C++
#define SYN_CIA401_OD_DIGITAL_OUTPUT_8BIT `0x6200U`
```



Write 8-bit Digital Outputs 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia401.h`

