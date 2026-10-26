

# File syn\_ccp.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ccp.c**](syn__ccp_8c.md)

[Go to the source code of this file](syn__ccp_8c_source.md)

_ASAM CAN Calibration Protocol v2.1 (CCP) Slave Implementation._ 

* `#include "syn_ccp.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_ccp\_init**](#function-syn_ccp_init) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint16\_t station\_addr) <br>_Initialize CCP slave instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ccp\_process\_cro**](#function-syn_ccp_process_cro) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, const uint8\_t cro, uint8\_t dto\_out) <br>_Process incoming Command Receive Object (CRO) frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ccp\_service\_daq**](#function-syn_ccp_service_daq) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t event\_channel, uint8\_t dto\_out, uint8\_t \* list\_idx, uint8\_t \* odt\_idx) <br>_Service periodic DAQ sampling tick._  |
|  void | [**syn\_ccp\_set\_mta**](#function-syn_ccp_set_mta) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t mta\_num, uint8\_t ext, uintptr\_t addr) <br>_Set Memory Transfer Address (MTA0 or MTA1)._  |




























## Public Functions Documentation




### function syn\_ccp\_init 

_Initialize CCP slave instance._ 
```C++
void syn_ccp_init (
    SYN_CCP_Slave * slave,
    uint16_t station_addr
) 
```





**Parameters:**


* `slave` Pointer to CCP slave handle. 
* `station_addr` 16-bit CCP station address. 




        

<hr>



### function syn\_ccp\_process\_cro 

_Process incoming Command Receive Object (CRO) frame._ 
```C++
bool syn_ccp_process_cro (
    SYN_CCP_Slave * slave,
    const uint8_t cro,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` Pointer to CCP slave handle. 
* `cro` 8-byte CRO input CAN payload. 
* `dto_out` 8-byte buffer to receive Command Return Message (DTO/CRM). 



**Returns:**

True if DTO response frame is ready to transmit. 





        

<hr>



### function syn\_ccp\_service\_daq 

_Service periodic DAQ sampling tick._ 
```C++
bool syn_ccp_service_daq (
    SYN_CCP_Slave * slave,
    uint8_t event_channel,
    uint8_t dto_out,
    uint8_t * list_idx,
    uint8_t * odt_idx
) 
```





**Parameters:**


* `slave` Pointer to CCP slave handle. 
* `event_channel` Trigger event channel index. 
* `dto_out` Output buffer for DAQ DTO packet. 
* `list_idx` Output index of transmitting DAQ list. 
* `odt_idx` Output index of transmitting ODT. 



**Returns:**

True if a DAQ packet is ready to send. 





        

<hr>



### function syn\_ccp\_set\_mta 

_Set Memory Transfer Address (MTA0 or MTA1)._ 
```C++
void syn_ccp_set_mta (
    SYN_CCP_Slave * slave,
    uint8_t mta_num,
    uint8_t ext,
    uintptr_t addr
) 
```





**Parameters:**


* `slave` Pointer to CCP slave handle. 
* `mta_num` 0 for MTA0, 1 for MTA1. 
* `ext` Address extension. 
* `addr` Target memory address. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ccp.c`

