

# File syn\_cia401.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cia401.c**](syn__cia401_8c.md)

[Go to the source code of this file](syn__cia401_8c_source.md)

_Implementation of CANopen CiA 401 Generic I/O Device Profile._ 

* `#include "syn_cia401.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`
* `#include <stdlib.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cia401\_check\_analog\_delta**](#function-syn_cia401_check_analog_delta) ([**SYN\_CiA401\_Device**](structSYN__CiA401__Device.md) \* dev, uint8\_t \* channel) <br>_Check if any analog input has exceeded its delta threshold for event-driven TPDO._  |
|  void | [**syn\_cia401\_init**](#function-syn_cia401_init) ([**SYN\_CiA401\_Device**](structSYN__CiA401__Device.md) \* dev, uint8\_t dig\_in\_bytes, uint8\_t dig\_out\_bytes, uint8\_t analog\_in\_ch, uint8\_t analog\_out\_ch) <br>_Initialize a CiA 401 device state instance._  |
|  size\_t | [**syn\_cia401\_populate\_od**](#function-syn_cia401_populate_od) ([**SYN\_CiA401\_Device**](structSYN__CiA401__Device.md) \* dev, [**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md) \* od\_table, size\_t max\_entries) <br>_Populate a standard CANopen Object Dictionary array with CiA 401 entries._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cia401.c`

