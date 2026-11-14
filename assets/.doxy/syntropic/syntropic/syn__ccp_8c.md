

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
|  void | [**syn\_ccp\_daq\_event**](#function-syn_ccp_daq_event) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t event\_channel) <br>_Advance DAQ prescalers once for an event channel tick prior to ODT drain._  |
|  void | [**syn\_ccp\_init**](#function-syn_ccp_init) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint16\_t station\_addr) <br>_Initialize CCP slave instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ccp\_process\_cro**](#function-syn_ccp_process_cro) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, const uint8\_t cro, uint8\_t dto\_out) <br>_Process incoming Command Receive Object (CRO) frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ccp\_register\_mem\_region**](#function-syn_ccp_register_mem_region) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint32\_t base\_addr, void \* host\_ptr, uint32\_t size, [**bool**](syn__defs_8h.md#enum-bool) writable) <br>_Register a bounds-checked memory region for CCP MTA and DAQ accesses._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ccp\_service\_daq**](#function-syn_ccp_service_daq) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t event\_channel, uint8\_t dto\_out, uint8\_t \* list\_idx, uint8\_t \* odt\_idx) <br>_Service periodic DAQ sampling tick._  |
|  void | [**syn\_ccp\_set\_mta**](#function-syn_ccp_set_mta) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t mta\_num, uint8\_t ext, uintptr\_t addr) <br>_Set Memory Transfer Address (MTA0 or MTA1)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**ccp\_dispatch\_connected\_cro**](#function-ccp_dispatch_connected_cro) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cro, uint8\_t dto\_out) <br>_Dispatch any CRO command received while the slave is in connected state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ccp\_dispatch\_mem\_cro**](#function-ccp_dispatch_mem_cro) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cro, uint8\_t dto\_out) <br>_Dispatch memory transfer, calibration page, and programming CRO commands._  |
|  void | [**ccp\_handle\_cal\_pgm**](#function-ccp_handle_cal_pgm) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cro, uint8\_t dto\_out) <br>_Handle calibration page switching and flash erase/program commands._  |
|  void | [**ccp\_handle\_daq\_cmd**](#function-ccp_handle_daq_cmd) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cro, uint8\_t dto\_out) <br>_Handle DAQ list sizing, ODT entry assignment, and start/stop commands._  |
|  void | [**ccp\_handle\_daq\_run**](#function-ccp_handle_daq_run) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cro, uint8\_t dto\_out) <br>_Handle START\_STOP and START\_STOP\_ALL DAQ execution control commands._  |
|  void | [**ccp\_handle\_disconnect**](#function-ccp_handle_disconnect) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, const uint8\_t cro) <br>_Handle CCP DISCONNECT command and stop all running DAQ lists._  |
|  void | [**ccp\_handle\_dnload**](#function-ccp_handle_dnload) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cro, uint8\_t dto\_out) <br>_Handle DNLOAD and DNLOAD\_6 calibration write commands._  |
|  void | [**ccp\_handle\_session\_sec**](#function-ccp_handle_session_sec) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cro, uint8\_t dto\_out) <br>_Handle session management, version, ID exchange, and seed/key unlock CROs._  |
|  void | [**ccp\_handle\_set\_mta**](#function-ccp_handle_set_mta) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, const uint8\_t cro) <br>_Handle SET\_MTA command to update MTA0 or MTA1 pointer and extension._  |
|  void | [**ccp\_handle\_upload**](#function-ccp_handle_upload) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cro, uint8\_t dto\_out) <br>_Handle UPLOAD and SHORT\_UP memory read commands._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ccp\_has\_pending\_event**](#function-ccp_has_pending_event) (const [**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t event\_channel) <br>_Check whether any running DAQ list on the event channel has pending ODTs._  |
|  int | [**ccp\_match\_region**](#function-ccp_match_region) (const [**SYN\_CCP\_MemRegion**](structSYN__CCP__MemRegion.md) \* reg, uintptr\_t addr, uint32\_t size, [**bool**](syn__defs_8h.md#enum-bool) is\_write, uint8\_t \*\* out\_ptr) <br>_Check whether a memory access falls within a registered CCP region._  |
|  void | [**ccp\_pack\_odt**](#function-ccp_pack_odt) (const [**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, const [**SYN\_CCP\_ODT**](structSYN__CCP__ODT.md) \* odt, uint8\_t pid, uint8\_t dto\_out) <br>_Sample mapped memory entries and pack a single DAQ ODT DTO frame._  |
|  void | [**ccp\_prepare\_daq\_event**](#function-ccp_prepare_daq_event) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t event\_channel) <br>_Step DAQ event counters once per tick if not already stepped or draining._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ccp\_require\_resource**](#function-ccp_require_resource) (const [**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t mask, uint8\_t dto\_out) <br>_Verify that required CCP resource protection mask is unlocked._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ccp\_resolve\_mem**](#function-ccp_resolve_mem) (const [**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uintptr\_t addr, uint32\_t size, [**bool**](syn__defs_8h.md#enum-bool) is\_write, [**bool**](syn__defs_8h.md#enum-bool) host\_valid, uint8\_t \*\* out\_ptr) <br>_Resolve a protocol or host MTA address to a validated host memory pointer._  |
|  void | [**ccp\_step\_event\_counters**](#function-ccp_step_event_counters) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t event\_channel) <br>_Advance prescaler counters and arm transmission for matching DAQ lists._  |


























## Public Functions Documentation




### function syn\_ccp\_daq\_event 

_Advance DAQ prescalers once for an event channel tick prior to ODT drain._ 
```C++
void syn_ccp_daq_event (
    SYN_CCP_Slave * slave,
    uint8_t event_channel
) 
```





**Parameters:**


* `slave` Pointer to CCP slave handle. 
* `event_channel` Trigger event channel index. 




        

<hr>



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



### function syn\_ccp\_register\_mem\_region 

_Register a bounds-checked memory region for CCP MTA and DAQ accesses._ 
```C++
bool syn_ccp_register_mem_region (
    SYN_CCP_Slave * slave,
    uint32_t base_addr,
    void * host_ptr,
    uint32_t size,
    bool writable
) 
```





**Parameters:**


* `slave` Pointer to CCP slave handle. 
* `base_addr` 32-bit CCP protocol base address. 
* `host_ptr` Backing host memory buffer. 
* `size` Size of region in bytes. 
* `writable` True if calibration/programming writes are allowed. 



**Returns:**

True on success, false on invalid arguments or full table. 





        

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
## Public Static Functions Documentation




### function ccp\_dispatch\_connected\_cro 

_Dispatch any CRO command received while the slave is in connected state._ 
```C++
static void ccp_dispatch_connected_cro (
    SYN_CCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cro,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `cmd` CCP command code. 
* `cro` Incoming 8-byte CRO payload. 
* `dto_out` Outgoing 8-byte CRM DTO payload. 




        

<hr>



### function ccp\_dispatch\_mem\_cro 

_Dispatch memory transfer, calibration page, and programming CRO commands._ 
```C++
static bool ccp_dispatch_mem_cro (
    SYN_CCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cro,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `cmd` CCP command code. 
* `cro` Incoming 8-byte CRO payload. 
* `dto_out` Outgoing 8-byte CRM DTO payload. 



**Returns:**

true if cmd was handled by this dispatcher, false otherwise. 





        

<hr>



### function ccp\_handle\_cal\_pgm 

_Handle calibration page switching and flash erase/program commands._ 
```C++
static void ccp_handle_cal_pgm (
    SYN_CCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cro,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `cmd` CCP command code. 
* `cro` Incoming 8-byte CRO payload. 
* `dto_out` Outgoing 8-byte CRM DTO payload. 




        

<hr>



### function ccp\_handle\_daq\_cmd 

_Handle DAQ list sizing, ODT entry assignment, and start/stop commands._ 
```C++
static void ccp_handle_daq_cmd (
    SYN_CCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cro,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `cmd` CCP command code. 
* `cro` Incoming 8-byte CRO payload. 
* `dto_out` Outgoing 8-byte CRM DTO payload. 




        

<hr>



### function ccp\_handle\_daq\_run 

_Handle START\_STOP and START\_STOP\_ALL DAQ execution control commands._ 
```C++
static void ccp_handle_daq_run (
    SYN_CCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cro,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `cmd` CCP command code. 
* `cro` Incoming 8-byte CRO payload. 
* `dto_out` Outgoing 8-byte CRM DTO payload. 




        

<hr>



### function ccp\_handle\_disconnect 

_Handle CCP DISCONNECT command and stop all running DAQ lists._ 
```C++
static void ccp_handle_disconnect (
    SYN_CCP_Slave * slave,
    const uint8_t cro
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `cro` Incoming 8-byte CRO payload. 




        

<hr>



### function ccp\_handle\_dnload 

_Handle DNLOAD and DNLOAD\_6 calibration write commands._ 
```C++
static void ccp_handle_dnload (
    SYN_CCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cro,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `cmd` CCP command code. 
* `cro` Incoming 8-byte CRO payload. 
* `dto_out` Outgoing 8-byte CRM DTO payload. 




        

<hr>



### function ccp\_handle\_session\_sec 

_Handle session management, version, ID exchange, and seed/key unlock CROs._ 
```C++
static void ccp_handle_session_sec (
    SYN_CCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cro,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `cmd` CCP command code. 
* `cro` Incoming 8-byte CRO payload. 
* `dto_out` Outgoing 8-byte CRM DTO payload. 




        

<hr>



### function ccp\_handle\_set\_mta 

_Handle SET\_MTA command to update MTA0 or MTA1 pointer and extension._ 
```C++
static void ccp_handle_set_mta (
    SYN_CCP_Slave * slave,
    const uint8_t cro
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `cro` Incoming 8-byte CRO payload. 




        

<hr>



### function ccp\_handle\_upload 

_Handle UPLOAD and SHORT\_UP memory read commands._ 
```C++
static void ccp_handle_upload (
    SYN_CCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cro,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `cmd` CCP command code. 
* `cro` Incoming 8-byte CRO payload. 
* `dto_out` Outgoing 8-byte CRM DTO payload. 




        

<hr>



### function ccp\_has\_pending\_event 

_Check whether any running DAQ list on the event channel has pending ODTs._ 
```C++
static bool ccp_has_pending_event (
    const SYN_CCP_Slave * slave,
    uint8_t event_channel
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `event_channel` Event channel index. 



**Returns:**

true if at least one DAQ list already has pending\_tx set. 





        

<hr>



### function ccp\_match\_region 

_Check whether a memory access falls within a registered CCP region._ 
```C++
static int ccp_match_region (
    const SYN_CCP_MemRegion * reg,
    uintptr_t addr,
    uint32_t size,
    bool is_write,
    uint8_t ** out_ptr
) 
```





**Parameters:**


* `reg` Memory region descriptor. 
* `addr` Requested protocol or host address. 
* `size` Access byte length. 
* `is_write` true if write access is required. 
* `out_ptr` Output host byte pointer when matched and permitted. 



**Returns:**

1 if valid match, -1 if out-of-bounds or write-protected, 0 if no match. 





        

<hr>



### function ccp\_pack\_odt 

_Sample mapped memory entries and pack a single DAQ ODT DTO frame._ 
```C++
static void ccp_pack_odt (
    const SYN_CCP_Slave * slave,
    const SYN_CCP_ODT * odt,
    uint8_t pid,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `odt` Descriptor of ODT to sample. 
* `pid` Packet identifier byte for DTO[0]. 
* `dto_out` Outgoing 8-byte DAQ DTO frame. 




        

<hr>



### function ccp\_prepare\_daq\_event 

_Step DAQ event counters once per tick if not already stepped or draining._ 
```C++
static void ccp_prepare_daq_event (
    SYN_CCP_Slave * slave,
    uint8_t event_channel
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `event_channel` Event channel index. 




        

<hr>



### function ccp\_require\_resource 

_Verify that required CCP resource protection mask is unlocked._ 
```C++
static bool ccp_require_resource (
    const SYN_CCP_Slave * slave,
    uint8_t mask,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `mask` Required resource bitmask. 
* `dto_out` Outgoing 8-byte CRM DTO frame (error code populated on failure). 



**Returns:**

true if resource is unlocked, false otherwise. 





        

<hr>



### function ccp\_resolve\_mem 

_Resolve a protocol or host MTA address to a validated host memory pointer._ 
```C++
static bool ccp_resolve_mem (
    const SYN_CCP_Slave * slave,
    uintptr_t addr,
    uint32_t size,
    bool is_write,
    bool host_valid,
    uint8_t ** out_ptr
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `addr` Requested address. 
* `size` Access byte length. 
* `is_write` true if write access is required. 
* `host_valid` true if addr was set directly as a valid host pointer. 
* `out_ptr` Output resolved host byte pointer. 



**Returns:**

true if access is permitted, false otherwise. 





        

<hr>



### function ccp\_step\_event\_counters 

_Advance prescaler counters and arm transmission for matching DAQ lists._ 
```C++
static void ccp_step_event_counters (
    SYN_CCP_Slave * slave,
    uint8_t event_channel
) 
```





**Parameters:**


* `slave` CCP slave instance. 
* `event_channel` Triggered event channel index. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ccp.c`

