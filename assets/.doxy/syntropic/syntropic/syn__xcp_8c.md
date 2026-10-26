

# File syn\_xcp.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_xcp.c**](syn__xcp_8c.md)

[Go to the source code of this file](syn__xcp_8c_source.md)

_ASAM Universal Measurement and Calibration Protocol v1.x (XCP) Slave Implementation._ 

* `#include "syn_xcp.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_init**](#function-syn_xcp_init) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint16\_t station\_id) <br>_Initialize XCP slave instance context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_process\_cto**](#function-syn_xcp_process_cto) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, const uint8\_t cto, uint8\_t dto\_out) <br>_Process incoming XCP CTO command packet and prepare response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_service\_daq**](#function-syn_xcp_service_daq) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t event\_channel, uint8\_t dto\_out, uint8\_t \* list\_idx, uint8\_t \* odt\_idx) <br>_Service periodic XCP DAQ sampling event channel tick._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_set\_mta**](#function-syn_xcp_set_mta) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t ext, uintptr\_t addr) <br>_Set XCP Memory Transfer Address (MTA)._  |




























## Public Functions Documentation




### function syn\_xcp\_init 

_Initialize XCP slave instance context._ 
```C++
bool syn_xcp_init (
    SYN_XCP_Slave * slave,
    uint16_t station_id
) 
```





**Parameters:**


* `slave` Pointer to XCP slave structure. 
* `station_id` XCP slave node/station ID. 



**Returns:**

true on success, false if slave is NULL. 





        

<hr>



### function syn\_xcp\_process\_cto 

_Process incoming XCP CTO command packet and prepare response._ 
```C++
bool syn_xcp_process_cto (
    SYN_XCP_Slave * slave,
    const uint8_t cto,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` Pointer to XCP slave structure. 
* `cto` 8-byte input CTO CAN frame buffer. 
* `dto_out` 8-byte output response DTO/CRM frame buffer. 



**Returns:**

true if response frame populated, false on invalid parameters. 





        

<hr>



### function syn\_xcp\_service\_daq 

_Service periodic XCP DAQ sampling event channel tick._ 
```C++
bool syn_xcp_service_daq (
    SYN_XCP_Slave * slave,
    uint8_t event_channel,
    uint8_t dto_out,
    uint8_t * list_idx,
    uint8_t * odt_idx
) 
```





**Parameters:**


* `slave` Pointer to XCP slave structure. 
* `event_channel` Event channel index triggered. 
* `dto_out` 8-byte output CAN telemetry buffer. 
* `list_idx` In/Out pointer to DAQ list state iterator. 
* `odt_idx` In/Out pointer to ODT state iterator. 



**Returns:**

true if a DAQ DTO packet was generated, false if no packet pending. 





        

<hr>



### function syn\_xcp\_set\_mta 

_Set XCP Memory Transfer Address (MTA)._ 
```C++
bool syn_xcp_set_mta (
    SYN_XCP_Slave * slave,
    uint8_t ext,
    uintptr_t addr
) 
```





**Parameters:**


* `slave` Pointer to XCP slave structure. 
* `ext` Address extension byte. 
* `addr` 32-bit/64-bit target memory pointer. 



**Returns:**

true on success, false if slave is NULL. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xcp.c`

