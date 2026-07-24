

# File syn\_canopen\_mgr.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_canopen\_mgr.c**](syn__canopen__mgr_8c.md)

[Go to the source code of this file](syn__canopen__mgr_8c_source.md)

_Implementation of CANopen CiA 302 Manager, SDO Client & NMT Master._ 

* `#include "syn_canopen_mgr.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_canopen\_mgr\_build\_nmt**](#function-syn_canopen_mgr_build_nmt) ([**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, uint8\_t target\_node, uint8\_t nmt\_cmd) <br>_Build an NMT Master command frame._  |
|  void | [**syn\_canopen\_mgr\_init**](#function-syn_canopen_mgr_init) ([**SYN\_CANOpenManager**](structSYN__CANOpenManager.md) \* mgr) <br>_Initialize CANopen Manager master instance._  |
|  void | [**syn\_canopen\_mgr\_process\_frame**](#function-syn_canopen_mgr_process_frame) ([**SYN\_CANOpenManager**](structSYN__CANOpenManager.md) \* mgr, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process an incoming CAN frame for Manager processing (Heartbeats, SDO responses)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_canopen\_mgr\_sdo\_read\_init**](#function-syn_canopen_mgr_sdo_read_init) ([**SYN\_CANOpenManager**](structSYN__CANOpenManager.md) \* mgr, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, uint8\_t node\_id, uint16\_t index, uint8\_t subindex) <br>_Initiate an SDO Read request to a remote slave node._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_canopen\_mgr\_sdo\_write\_init**](#function-syn_canopen_mgr_sdo_write_init) ([**SYN\_CANOpenManager**](structSYN__CANOpenManager.md) \* mgr, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, uint8\_t node\_id, uint16\_t index, uint8\_t subindex, const void \* data, size\_t len) <br>_Initiate an SDO Write request to a remote slave node._  |
|  void | [**syn\_canopen\_mgr\_step**](#function-syn_canopen_mgr_step) ([**SYN\_CANOpenManager**](structSYN__CANOpenManager.md) \* mgr, uint16\_t delta\_ms) <br>_Advance CANopen Manager periodic timer ticks._  |




























## Public Functions Documentation




### function syn\_canopen\_mgr\_build\_nmt 

_Build an NMT Master command frame._ 
```C++
void syn_canopen_mgr_build_nmt (
    SYN_CAN_Frame * frame,
    uint8_t target_node,
    uint8_t nmt_cmd
) 
```





**Parameters:**


* `frame` Output CAN frame buffer. 
* `target_node` Target node ID (1..127 or 0 for broadcast). 
* `nmt_cmd` Command (SYN\_CANOPEN\_NMT\_CMD\_\*). 




        

<hr>



### function syn\_canopen\_mgr\_init 

_Initialize CANopen Manager master instance._ 
```C++
void syn_canopen_mgr_init (
    SYN_CANOpenManager * mgr
) 
```





**Parameters:**


* `mgr` Pointer to manager handle. 




        

<hr>



### function syn\_canopen\_mgr\_process\_frame 

_Process an incoming CAN frame for Manager processing (Heartbeats, SDO responses)._ 
```C++
void syn_canopen_mgr_process_frame (
    SYN_CANOpenManager * mgr,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `mgr` Pointer to manager handle. 
* `frame` Incoming CAN frame. 




        

<hr>



### function syn\_canopen\_mgr\_sdo\_read\_init 

_Initiate an SDO Read request to a remote slave node._ 
```C++
SYN_Status syn_canopen_mgr_sdo_read_init (
    SYN_CANOpenManager * mgr,
    SYN_CAN_Frame * frame,
    uint8_t node_id,
    uint16_t index,
    uint8_t subindex
) 
```





**Parameters:**


* `mgr` Pointer to manager handle. 
* `frame` Output CAN request frame buffer. 
* `node_id` Target slave Node-ID. 
* `index` Target Object Dictionary Index. 
* `subindex` Target Object Dictionary Subindex. 



**Returns:**

SYN\_OK if initiated, SYN\_BUSY if SDO client is busy. 





        

<hr>



### function syn\_canopen\_mgr\_sdo\_write\_init 

_Initiate an SDO Write request to a remote slave node._ 
```C++
SYN_Status syn_canopen_mgr_sdo_write_init (
    SYN_CANOpenManager * mgr,
    SYN_CAN_Frame * frame,
    uint8_t node_id,
    uint16_t index,
    uint8_t subindex,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `mgr` Pointer to manager handle. 
* `frame` Output CAN request frame buffer. 
* `node_id` Target slave Node-ID. 
* `index` Target Object Dictionary Index. 
* `subindex` Target Object Dictionary Subindex. 
* `data` Pointer to payload bytes to write. 
* `len` Data length (1..4 bytes for expedited write). 



**Returns:**

SYN\_OK if initiated, SYN\_BUSY if busy. 





        

<hr>



### function syn\_canopen\_mgr\_step 

_Advance CANopen Manager periodic timer ticks._ 
```C++
void syn_canopen_mgr_step (
    SYN_CANOpenManager * mgr,
    uint16_t delta_ms
) 
```





**Parameters:**


* `mgr` Pointer to manager handle. 
* `delta_ms` Milliseconds elapsed since last call. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen_mgr.c`

