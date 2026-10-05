

# File syn\_ccp.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ccp.h**](syn__ccp_8h.md)

[Go to the source code of this file](syn__ccp_8h_source.md)

_ASAM CAN Calibration Protocol v2.1 (CCP) Slave Implementation._ [More...](#detailed-description)

* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CCP\_DAQList**](structSYN__CCP__DAQList.md) <br>_DAQ List structure._  |
| struct | [**SYN\_CCP\_ODT**](structSYN__CCP__ODT.md) <br>_ODT (Object Descriptor Table) structure._  |
| struct | [**SYN\_CCP\_ODTEntry**](structSYN__CCP__ODTEntry.md) <br>_ODT Element mapping entry._  |
| struct | [**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) <br>_CCP Slave Instance Handle._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_CCP\_EraseFn**](#typedef-syn_ccp_erasefn)  <br>_CCP Memory Erase callback._  |
| typedef uint32\_t(\* | [**SYN\_CCP\_SeedFn**](#typedef-syn_ccp_seedfn)  <br>_CCP Security Seed Generator callback._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_CCP\_UnlockFn**](#typedef-syn_ccp_unlockfn)  <br>_CCP Security Unlock Key verification callback._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_ccp\_init**](#function-syn_ccp_init) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint16\_t station\_addr) <br>_Initialize CCP slave instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ccp\_process\_cro**](#function-syn_ccp_process_cro) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, const uint8\_t cro, uint8\_t dto\_out) <br>_Process incoming Command Receive Object (CRO) frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ccp\_service\_daq**](#function-syn_ccp_service_daq) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t event\_channel, uint8\_t dto\_out, uint8\_t \* list\_idx, uint8\_t \* odt\_idx) <br>_Service periodic DAQ sampling tick._  |
|  void | [**syn\_ccp\_set\_mta**](#function-syn_ccp_set_mta) ([**SYN\_CCP\_Slave**](structSYN__CCP__Slave.md) \* slave, uint8\_t mta\_num, uint8\_t ext, uintptr\_t addr) <br>_Set Memory Transfer Address (MTA0 or MTA1)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_CCP\_CMD\_BUILD\_PKT**](syn__ccp_8h.md#define-syn_ccp_cmd_build_pkt)  `0x10U`<br> |
| define  | [**SYN\_CCP\_CMD\_CLEAR\_MEMORY**](syn__ccp_8h.md#define-syn_ccp_cmd_clear_memory)  `0x18U`<br> |
| define  | [**SYN\_CCP\_CMD\_CONNECT**](syn__ccp_8h.md#define-syn_ccp_cmd_connect)  `0x01U`<br> |
| define  | [**SYN\_CCP\_CMD\_DISCONNECT**](syn__ccp_8h.md#define-syn_ccp_cmd_disconnect)  `0x17U`<br> |
| define  | [**SYN\_CCP\_CMD\_DNLOAD**](syn__ccp_8h.md#define-syn_ccp_cmd_dnload)  `0x03U`<br> |
| define  | [**SYN\_CCP\_CMD\_DNLOAD\_6**](syn__ccp_8h.md#define-syn_ccp_cmd_dnload_6)  `0x23U`<br> |
| define  | [**SYN\_CCP\_CMD\_EXCHANGE\_ID**](syn__ccp_8h.md#define-syn_ccp_cmd_exchange_id)  `0x02U`<br> |
| define  | [**SYN\_CCP\_CMD\_GET\_ACTIVE\_CAL\_PAGE**](syn__ccp_8h.md#define-syn_ccp_cmd_get_active_cal_page)  `0x09U`<br> |
| define  | [**SYN\_CCP\_CMD\_GET\_CCP\_VERSION**](syn__ccp_8h.md#define-syn_ccp_cmd_get_ccp_version)  `0x1BU`<br> |
| define  | [**SYN\_CCP\_CMD\_GET\_DAQ\_SIZE**](syn__ccp_8h.md#define-syn_ccp_cmd_get_daq_size)  `0x1AU`<br> |
| define  | [**SYN\_CCP\_CMD\_GET\_SEED**](syn__ccp_8h.md#define-syn_ccp_cmd_get_seed)  `0x12U`<br> |
| define  | [**SYN\_CCP\_CMD\_PROGRAM**](syn__ccp_8h.md#define-syn_ccp_cmd_program)  `0x22U`<br> |
| define  | [**SYN\_CCP\_CMD\_SET\_CAL\_PAGE**](syn__ccp_8h.md#define-syn_ccp_cmd_set_cal_page)  `0x14U`<br> |
| define  | [**SYN\_CCP\_CMD\_SET\_DAQ\_SIZE**](syn__ccp_8h.md#define-syn_ccp_cmd_set_daq_size)  `0x15U`<br> |
| define  | [**SYN\_CCP\_CMD\_SET\_MTA**](syn__ccp_8h.md#define-syn_ccp_cmd_set_mta)  `0x08U`<br> |
| define  | [**SYN\_CCP\_CMD\_SHORT\_UP**](syn__ccp_8h.md#define-syn_ccp_cmd_short_up)  `0x0FU`<br> |
| define  | [**SYN\_CCP\_CMD\_START\_STOP**](syn__ccp_8h.md#define-syn_ccp_cmd_start_stop)  `0x11U`<br> |
| define  | [**SYN\_CCP\_CMD\_START\_STOP\_ALL**](syn__ccp_8h.md#define-syn_ccp_cmd_start_stop_all)  `0x06U`<br> |
| define  | [**SYN\_CCP\_CMD\_UNLOCK**](syn__ccp_8h.md#define-syn_ccp_cmd_unlock)  `0x13U`<br> |
| define  | [**SYN\_CCP\_CMD\_UPLOAD**](syn__ccp_8h.md#define-syn_ccp_cmd_upload)  `0x04U`<br> |
| define  | [**SYN\_CCP\_ERR\_ACCESS\_DENIED**](syn__ccp_8h.md#define-syn_ccp_err_access_denied)  `0x11U`<br> |
| define  | [**SYN\_CCP\_ERR\_CMD\_SYNTAX**](syn__ccp_8h.md#define-syn_ccp_err_cmd_syntax)  `0x02U`<br> |
| define  | [**SYN\_CCP\_ERR\_CMD\_UNKNOWN**](syn__ccp_8h.md#define-syn_ccp_err_cmd_unknown)  `0x01U`<br> |
| define  | [**SYN\_CCP\_ERR\_NOT\_CONNECTED**](syn__ccp_8h.md#define-syn_ccp_err_not_connected)  `0x12U`<br> |
| define  | [**SYN\_CCP\_ERR\_PARAM\_OUT\_OF\_RANGE**](syn__ccp_8h.md#define-syn_ccp_err_param_out_of_range)  `0x10U`<br> |
| define  | [**SYN\_CCP\_ERR\_RESOURCE\_LOCKED**](syn__ccp_8h.md#define-syn_ccp_err_resource_locked)  `0x18U`<br> |
| define  | [**SYN\_CCP\_ERR\_SUCCESS**](syn__ccp_8h.md#define-syn_ccp_err_success)  `0x00U`<br> |
| define  | [**SYN\_CCP\_MAX\_DAQ\_LISTS**](syn__ccp_8h.md#define-syn_ccp_max_daq_lists)  `4U`<br>_Maximum number of DAQ lists per CCP node._  |
| define  | [**SYN\_CCP\_MAX\_ENTRIES\_PER\_ODT**](syn__ccp_8h.md#define-syn_ccp_max_entries_per_odt)  `7U`<br>_Maximum entries per ODT._  |
| define  | [**SYN\_CCP\_MAX\_ODT\_PER\_DAQ**](syn__ccp_8h.md#define-syn_ccp_max_odt_per_daq)  `8U`<br>_Maximum ODTs per DAQ list._  |
| define  | [**SYN\_CCP\_PID\_CRM**](syn__ccp_8h.md#define-syn_ccp_pid_crm)  `0xFFU`<br> |
| define  | [**SYN\_CCP\_PID\_EVENT**](syn__ccp_8h.md#define-syn_ccp_pid_event)  `0xFEU`<br> |
| define  | [**SYN\_CCP\_RESOURCE\_CAL**](syn__ccp_8h.md#define-syn_ccp_resource_cal)  `0x01U`<br> |
| define  | [**SYN\_CCP\_RESOURCE\_DAQ**](syn__ccp_8h.md#define-syn_ccp_resource_daq)  `0x02U`<br> |
| define  | [**SYN\_CCP\_RESOURCE\_PGM**](syn__ccp_8h.md#define-syn_ccp_resource_pgm)  `0x40U`<br> |

## Detailed Description


Provides a lightweight, zero-allocation CCP v2.1 slave protocol stack for ECU calibration, live parameter tuning, memory upload/download, and DAQ (Data Acquisition) list streaming over CAN 2.0B. 


    
## Public Types Documentation




### typedef SYN\_CCP\_EraseFn 

_CCP Memory Erase callback._ 
```C++
typedef bool(* SYN_CCP_EraseFn) (uint32_t addr, uint32_t size, void *ctx);
```




<hr>



### typedef SYN\_CCP\_SeedFn 

_CCP Security Seed Generator callback._ 
```C++
typedef uint32_t(* SYN_CCP_SeedFn) (uint8_t resource, void *ctx);
```




<hr>



### typedef SYN\_CCP\_UnlockFn 

_CCP Security Unlock Key verification callback._ 
```C++
typedef bool(* SYN_CCP_UnlockFn) (uint8_t resource, uint32_t key, void *ctx);
```




<hr>
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
## Macro Definition Documentation





### define SYN\_CCP\_CMD\_BUILD\_PKT 

```C++
#define SYN_CCP_CMD_BUILD_PKT `0x10U`
```



Build DAQ packet 


        

<hr>



### define SYN\_CCP\_CMD\_CLEAR\_MEMORY 

```C++
#define SYN_CCP_CMD_CLEAR_MEMORY `0x18U`
```



Clear flash/RAM memory 


        

<hr>



### define SYN\_CCP\_CMD\_CONNECT 

```C++
#define SYN_CCP_CMD_CONNECT `0x01U`
```



Establish connection 


        

<hr>



### define SYN\_CCP\_CMD\_DISCONNECT 

```C++
#define SYN_CCP_CMD_DISCONNECT `0x17U`
```



Disconnect station 


        

<hr>



### define SYN\_CCP\_CMD\_DNLOAD 

```C++
#define SYN_CCP_CMD_DNLOAD `0x03U`
```



Download data block 


        

<hr>



### define SYN\_CCP\_CMD\_DNLOAD\_6 

```C++
#define SYN_CCP_CMD_DNLOAD_6 `0x23U`
```



Fast 6-byte download 


        

<hr>



### define SYN\_CCP\_CMD\_EXCHANGE\_ID 

```C++
#define SYN_CCP_CMD_EXCHANGE_ID `0x02U`
```



Exchange station ID 


        

<hr>



### define SYN\_CCP\_CMD\_GET\_ACTIVE\_CAL\_PAGE 

```C++
#define SYN_CCP_CMD_GET_ACTIVE_CAL_PAGE `0x09U`
```



Get active calibration page 


        

<hr>



### define SYN\_CCP\_CMD\_GET\_CCP\_VERSION 

```C++
#define SYN_CCP_CMD_GET_CCP_VERSION `0x1BU`
```



Get CCP protocol version 


        

<hr>



### define SYN\_CCP\_CMD\_GET\_DAQ\_SIZE 

```C++
#define SYN_CCP_CMD_GET_DAQ_SIZE `0x1AU`
```



Get DAQ list size 


        

<hr>



### define SYN\_CCP\_CMD\_GET\_SEED 

```C++
#define SYN_CCP_CMD_GET_SEED `0x12U`
```



Get seed for security unlock 


        

<hr>



### define SYN\_CCP\_CMD\_PROGRAM 

```C++
#define SYN_CCP_CMD_PROGRAM `0x22U`
```



Program flash memory 


        

<hr>



### define SYN\_CCP\_CMD\_SET\_CAL\_PAGE 

```C++
#define SYN_CCP_CMD_SET_CAL_PAGE `0x14U`
```



Set calibration page 


        

<hr>



### define SYN\_CCP\_CMD\_SET\_DAQ\_SIZE 

```C++
#define SYN_CCP_CMD_SET_DAQ_SIZE `0x15U`
```



Set DAQ list size 


        

<hr>



### define SYN\_CCP\_CMD\_SET\_MTA 

```C++
#define SYN_CCP_CMD_SET_MTA `0x08U`
```



Set memory transfer address 


        

<hr>



### define SYN\_CCP\_CMD\_SHORT\_UP 

```C++
#define SYN_CCP_CMD_SHORT_UP `0x0FU`
```



Short upload 


        

<hr>



### define SYN\_CCP\_CMD\_START\_STOP 

```C++
#define SYN_CCP_CMD_START_STOP `0x11U`
```



Start/stop single DAQ list 


        

<hr>



### define SYN\_CCP\_CMD\_START\_STOP\_ALL 

```C++
#define SYN_CCP_CMD_START_STOP_ALL `0x06U`
```



Start/stop all DAQ lists 


        

<hr>



### define SYN\_CCP\_CMD\_UNLOCK 

```C++
#define SYN_CCP_CMD_UNLOCK `0x13U`
```



Send key to unlock resource 


        

<hr>



### define SYN\_CCP\_CMD\_UPLOAD 

```C++
#define SYN_CCP_CMD_UPLOAD `0x04U`
```



Upload data block 


        

<hr>



### define SYN\_CCP\_ERR\_ACCESS\_DENIED 

```C++
#define SYN_CCP_ERR_ACCESS_DENIED `0x11U`
```



Access denied 


        

<hr>



### define SYN\_CCP\_ERR\_CMD\_SYNTAX 

```C++
#define SYN_CCP_ERR_CMD_SYNTAX `0x02U`
```



Command syntax error 


        

<hr>



### define SYN\_CCP\_ERR\_CMD\_UNKNOWN 

```C++
#define SYN_CCP_ERR_CMD_UNKNOWN `0x01U`
```



Command unknown 


        

<hr>



### define SYN\_CCP\_ERR\_NOT\_CONNECTED 

```C++
#define SYN_CCP_ERR_NOT_CONNECTED `0x12U`
```



Station not connected 


        

<hr>



### define SYN\_CCP\_ERR\_PARAM\_OUT\_OF\_RANGE 

```C++
#define SYN_CCP_ERR_PARAM_OUT_OF_RANGE `0x10U`
```



Parameter out of range 


        

<hr>



### define SYN\_CCP\_ERR\_RESOURCE\_LOCKED 

```C++
#define SYN_CCP_ERR_RESOURCE_LOCKED `0x18U`
```



Resource locked 


        

<hr>



### define SYN\_CCP\_ERR\_SUCCESS 

```C++
#define SYN_CCP_ERR_SUCCESS `0x00U`
```



Action succeeded 


        

<hr>



### define SYN\_CCP\_MAX\_DAQ\_LISTS 

_Maximum number of DAQ lists per CCP node._ 
```C++
#define SYN_CCP_MAX_DAQ_LISTS `4U`
```




<hr>



### define SYN\_CCP\_MAX\_ENTRIES\_PER\_ODT 

_Maximum entries per ODT._ 
```C++
#define SYN_CCP_MAX_ENTRIES_PER_ODT `7U`
```




<hr>



### define SYN\_CCP\_MAX\_ODT\_PER\_DAQ 

_Maximum ODTs per DAQ list._ 
```C++
#define SYN_CCP_MAX_ODT_PER_DAQ `8U`
```




<hr>



### define SYN\_CCP\_PID\_CRM 

```C++
#define SYN_CCP_PID_CRM `0xFFU`
```



Command response message 


        

<hr>



### define SYN\_CCP\_PID\_EVENT 

```C++
#define SYN_CCP_PID_EVENT `0xFEU`
```



Event packet 


        

<hr>



### define SYN\_CCP\_RESOURCE\_CAL 

```C++
#define SYN_CCP_RESOURCE_CAL `0x01U`
```



Calibration resource mask 


        

<hr>



### define SYN\_CCP\_RESOURCE\_DAQ 

```C++
#define SYN_CCP_RESOURCE_DAQ `0x02U`
```



Data acquisition resource mask 


        

<hr>



### define SYN\_CCP\_RESOURCE\_PGM 

```C++
#define SYN_CCP_RESOURCE_PGM `0x40U`
```



Flash programming resource mask 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ccp.h`

