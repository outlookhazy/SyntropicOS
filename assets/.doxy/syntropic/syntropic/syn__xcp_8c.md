

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
|  void | [**syn\_xcp\_daq\_event**](#function-syn_xcp_daq_event) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t event\_channel) <br>_Advance XCP DAQ prescalers once for an event channel tick prior to ODT drain._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_init**](#function-syn_xcp_init) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint16\_t station\_id) <br>_Initialize XCP slave instance context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_process\_cto**](#function-syn_xcp_process_cto) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, const uint8\_t cto, uint8\_t dto\_out) <br>_Process incoming XCP CTO command packet and prepare response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_register\_mem\_region**](#function-syn_xcp_register_mem_region) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint32\_t base\_addr, void \* host\_ptr, uint32\_t size, [**bool**](syn__defs_8h.md#enum-bool) writable) <br>_Register a bounds-checked memory region for XCP MTA and DAQ accesses._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_service\_daq**](#function-syn_xcp_service_daq) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t event\_channel, uint8\_t dto\_out, uint8\_t \* list\_idx, uint8\_t \* odt\_idx) <br>_Service periodic XCP DAQ sampling event channel tick._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_set\_mta**](#function-syn_xcp_set_mta) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t ext, uintptr\_t addr) <br>_Set XCP Memory Transfer Address (MTA)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**xcp\_dispatch\_cal\_or\_daq**](#function-xcp_dispatch_cal_or_daq) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cto, uint8\_t dto\_out) <br>_Dispatch XCP MTA, calibration upload/download, or DAQ configuration/control commands._  |
|  void | [**xcp\_handle\_daq\_ctrl**](#function-xcp_handle_daq_ctrl) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cto, uint8\_t dto\_out) <br>_Handle XCP DAQ mode and start/stop control commands._  |
|  void | [**xcp\_handle\_daq\_ptr\_write**](#function-xcp_handle_daq_ptr_write) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cto, uint8\_t dto\_out) <br>_Handle XCP SET\_DAQ\_PTR and WRITE\_DAQ ODT entry configuration commands._  |
|  void | [**xcp\_handle\_download**](#function-xcp_handle_download) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cto, uint8\_t dto\_out) <br>_Handle XCP DOWNLOAD and DOWNLOAD\_MAX calibration memory write commands._  |
|  void | [**xcp\_handle\_security**](#function-xcp_handle_security) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cto, uint8\_t dto\_out) <br>_Handle XCP GET\_SEED and UNLOCK resource protection commands._  |
|  void | [**xcp\_handle\_std\_info**](#function-xcp_handle_std_info) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t cmd, uint8\_t dto\_out) <br>_Handle standard XCP connection and status commands (CONNECT, DISCONNECT, GET\_STATUS, etc)._  |
|  void | [**xcp\_handle\_upload**](#function-xcp_handle_upload) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t cmd, const uint8\_t cto, uint8\_t dto\_out) <br>_Handle XCP UPLOAD and SHORT\_UPLOAD memory read commands._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**xcp\_has\_pending\_event**](#function-xcp_has_pending_event) (const [**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t event\_channel) <br>_Check if any running DAQ list on the given event channel already has a pending ODT TX._  |
|  int | [**xcp\_match\_region**](#function-xcp_match_region) (const [**SYN\_XCP\_MemRegion**](structSYN__XCP__MemRegion.md) \* reg, uintptr\_t addr, uint32\_t size, [**bool**](syn__defs_8h.md#enum-bool) is\_write, uint8\_t \*\* out\_ptr, uint8\_t \* err\_code) <br>_Check whether an address range matches a registered XCP memory region._  |
|  void | [**xcp\_pack\_odt**](#function-xcp_pack_odt) (const [**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, const [**SYN\_XCP\_ODT**](structSYN__XCP__ODT.md) \* odt, uint8\_t pid, uint8\_t dto\_out) <br>_Sample all configured entries of an XCP ODT and pack them into an 8-byte DTO frame._  |
|  void | [**xcp\_prepare\_daq\_event**](#function-xcp_prepare_daq_event) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t event\_channel) <br>_Step XCP DAQ event counters once per tick if not already stepped or draining._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**xcp\_require\_resource**](#function-xcp_require_resource) (const [**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t mask, uint8\_t dto\_out) <br>_Verify that the required XCP resource bitmask is unlocked, or format ERR\_ACCESS\_LOCKED._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**xcp\_resolve\_mem**](#function-xcp_resolve_mem) (const [**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uintptr\_t addr, uint32\_t size, [**bool**](syn__defs_8h.md#enum-bool) is\_write, [**bool**](syn__defs_8h.md#enum-bool) host\_valid, uint8\_t \*\* out\_ptr, uint8\_t \* err\_code) <br>_Resolve an XCP address and byte count to a validated host memory pointer._  |
|  void | [**xcp\_set\_err**](#function-xcp_set_err) (uint8\_t dto\_out, uint8\_t err\_code) <br>_Populate an XCP error response DTO packet (0xFE + error code)._  |
|  void | [**xcp\_step\_event\_counters**](#function-xcp_step_event_counters) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t event\_channel) <br>_Advance prescaler cycle counters and arm pending\_tx for running DAQ lists on an event._  |


























## Public Functions Documentation




### function syn\_xcp\_daq\_event 

_Advance XCP DAQ prescalers once for an event channel tick prior to ODT drain._ 
```C++
void syn_xcp_daq_event (
    SYN_XCP_Slave * slave,
    uint8_t event_channel
) 
```





**Parameters:**


* `slave` Pointer to XCP slave structure. 
* `event_channel` Event channel index triggered. 




        

<hr>



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



### function syn\_xcp\_register\_mem\_region 

_Register a bounds-checked memory region for XCP MTA and DAQ accesses._ 
```C++
bool syn_xcp_register_mem_region (
    SYN_XCP_Slave * slave,
    uint32_t base_addr,
    void * host_ptr,
    uint32_t size,
    bool writable
) 
```





**Parameters:**


* `slave` Pointer to XCP slave structure. 
* `base_addr` 32-bit XCP protocol base address. 
* `host_ptr` Backing host memory buffer. 
* `size` Size of region in bytes. 
* `writable` True if calibration/download writes are allowed. 



**Returns:**

true on success, false on invalid arguments or full table. 





        

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
## Public Static Functions Documentation




### function xcp\_dispatch\_cal\_or\_daq 

_Dispatch XCP MTA, calibration upload/download, or DAQ configuration/control commands._ 
```C++
static void xcp_dispatch_cal_or_daq (
    SYN_XCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cto,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `cmd` XCP CTO command code. 
* `cto` 8-byte inbound CTO command buffer. 
* `dto_out` 8-byte output DTO response buffer. 




        

<hr>



### function xcp\_handle\_daq\_ctrl 

_Handle XCP DAQ mode and start/stop control commands._ 
```C++
static void xcp_handle_daq_ctrl (
    SYN_XCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cto,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `cmd` XCP CTO command code (SET\_DAQ\_LIST\_MODE, START\_STOP\_DAQ\_LIST, START\_STOP\_SYNCH). 
* `cto` 8-byte inbound CTO command buffer. 
* `dto_out` 8-byte output DTO response buffer. 




        

<hr>



### function xcp\_handle\_daq\_ptr\_write 

_Handle XCP SET\_DAQ\_PTR and WRITE\_DAQ ODT entry configuration commands._ 
```C++
static void xcp_handle_daq_ptr_write (
    SYN_XCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cto,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `cmd` XCP CTO command code (SET\_DAQ\_PTR or WRITE\_DAQ). 
* `cto` 8-byte inbound CTO command buffer. 
* `dto_out` 8-byte output DTO response buffer. 




        

<hr>



### function xcp\_handle\_download 

_Handle XCP DOWNLOAD and DOWNLOAD\_MAX calibration memory write commands._ 
```C++
static void xcp_handle_download (
    SYN_XCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cto,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `cmd` XCP CTO command code (DOWNLOAD or DOWNLOAD\_MAX). 
* `cto` 8-byte inbound CTO command buffer. 
* `dto_out` 8-byte output DTO response buffer. 




        

<hr>



### function xcp\_handle\_security 

_Handle XCP GET\_SEED and UNLOCK resource protection commands._ 
```C++
static void xcp_handle_security (
    SYN_XCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cto,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `cmd` XCP CTO command code (GET\_SEED or UNLOCK). 
* `cto` 8-byte inbound CTO command buffer. 
* `dto_out` 8-byte output DTO response buffer. 




        

<hr>



### function xcp\_handle\_std\_info 

_Handle standard XCP connection and status commands (CONNECT, DISCONNECT, GET\_STATUS, etc)._ 
```C++
static void xcp_handle_std_info (
    SYN_XCP_Slave * slave,
    uint8_t cmd,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `cmd` XCP CTO command code. 
* `dto_out` 8-byte output DTO response buffer. 




        

<hr>



### function xcp\_handle\_upload 

_Handle XCP UPLOAD and SHORT\_UPLOAD memory read commands._ 
```C++
static void xcp_handle_upload (
    SYN_XCP_Slave * slave,
    uint8_t cmd,
    const uint8_t cto,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `cmd` XCP CTO command code (UPLOAD or SHORT\_UPLOAD). 
* `cto` 8-byte inbound CTO command buffer. 
* `dto_out` 8-byte output DTO response buffer. 




        

<hr>



### function xcp\_has\_pending\_event 

_Check if any running DAQ list on the given event channel already has a pending ODT TX._ 
```C++
static bool xcp_has_pending_event (
    const SYN_XCP_Slave * slave,
    uint8_t event_channel
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `event_channel` Event channel index. 



**Returns:**

true if at least one matching DAQ list has pending\_tx set, false otherwise. 





        

<hr>



### function xcp\_match\_region 

_Check whether an address range matches a registered XCP memory region._ 
```C++
static int xcp_match_region (
    const SYN_XCP_MemRegion * reg,
    uintptr_t addr,
    uint32_t size,
    bool is_write,
    uint8_t ** out_ptr,
    uint8_t * err_code
) 
```





**Parameters:**


* `reg` Memory region descriptor. 
* `addr` Target protocol or host address. 
* `size` Requested access length in bytes. 
* `is_write` true for write access check, false for read access check. 
* `out_ptr` Output resolved host memory pointer on match. 
* `err_code` Output XCP error code when access fails bounds or write-protection check. 



**Returns:**

1 if matched and permitted, 0 if outside region, -1 if matched but rejected. 





        

<hr>



### function xcp\_pack\_odt 

_Sample all configured entries of an XCP ODT and pack them into an 8-byte DTO frame._ 
```C++
static void xcp_pack_odt (
    const SYN_XCP_Slave * slave,
    const SYN_XCP_ODT * odt,
    uint8_t pid,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `odt` ODT descriptor to sample. 
* `pid` Absolute ODT packet identifier byte. 
* `dto_out` 8-byte output DTO buffer. 




        

<hr>



### function xcp\_prepare\_daq\_event 

_Step XCP DAQ event counters once per tick if not already stepped or draining._ 
```C++
static void xcp_prepare_daq_event (
    SYN_XCP_Slave * slave,
    uint8_t event_channel
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `event_channel` Event channel index. 




        

<hr>



### function xcp\_require\_resource 

_Verify that the required XCP resource bitmask is unlocked, or format ERR\_ACCESS\_LOCKED._ 
```C++
static bool xcp_require_resource (
    const SYN_XCP_Slave * slave,
    uint8_t mask,
    uint8_t dto_out
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `mask` Required resource protection bitmask. 
* `dto_out` 8-byte output DTO buffer populated on lock error. 



**Returns:**

true if unlocked, false if locked. 





        

<hr>



### function xcp\_resolve\_mem 

_Resolve an XCP address and byte count to a validated host memory pointer._ 
```C++
static bool xcp_resolve_mem (
    const SYN_XCP_Slave * slave,
    uintptr_t addr,
    uint32_t size,
    bool is_write,
    bool host_valid,
    uint8_t ** out_ptr,
    uint8_t * err_code
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `addr` Target protocol or host address. 
* `size` Requested byte length. 
* `is_write` true for write access, false for read access. 
* `host_valid` true if direct host pointer fallback is allowed when no regions exist. 
* `out_ptr` Output resolved host memory pointer. 
* `err_code` Output XCP error code on failure. 



**Returns:**

true if memory access is permitted, false otherwise. 





        

<hr>



### function xcp\_set\_err 

_Populate an XCP error response DTO packet (0xFE + error code)._ 
```C++
static void xcp_set_err (
    uint8_t dto_out,
    uint8_t err_code
) 
```





**Parameters:**


* `dto_out` 8-byte output DTO buffer. 
* `err_code` XCP error code byte. 




        

<hr>



### function xcp\_step\_event\_counters 

_Advance prescaler cycle counters and arm pending\_tx for running DAQ lists on an event._ 
```C++
static void xcp_step_event_counters (
    SYN_XCP_Slave * slave,
    uint8_t event_channel
) 
```





**Parameters:**


* `slave` XCP slave instance. 
* `event_channel` Event channel index. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xcp.c`

