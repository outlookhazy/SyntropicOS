

# File syn\_xcp.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_xcp.h**](syn__xcp_8h.md)

[Go to the source code of this file](syn__xcp_8h_source.md)

_ASAM Universal Measurement and Calibration Protocol v1.x (XCP) Slave._ [More...](#detailed-description)

* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_XCP\_DAQList**](structSYN__XCP__DAQList.md) <br>_DAQ List structure._  |
| struct | [**SYN\_XCP\_ODT**](structSYN__XCP__ODT.md) <br>_ODT (Object Descriptor Table) structure._  |
| struct | [**SYN\_XCP\_ODTEntry**](structSYN__XCP__ODTEntry.md) <br>_ODT Element mapping entry._  |
| struct | [**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) <br>_XCP Slave instance context._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_init**](#function-syn_xcp_init) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint16\_t station\_id) <br>_Initialize XCP slave instance context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_process\_cto**](#function-syn_xcp_process_cto) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, const uint8\_t cto, uint8\_t dto\_out) <br>_Process incoming XCP CTO command packet and prepare response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_service\_daq**](#function-syn_xcp_service_daq) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t event\_channel, uint8\_t dto\_out, uint8\_t \* list\_idx, uint8\_t \* odt\_idx) <br>_Service periodic XCP DAQ sampling event channel tick._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_xcp\_set\_mta**](#function-syn_xcp_set_mta) ([**SYN\_XCP\_Slave**](structSYN__XCP__Slave.md) \* slave, uint8\_t ext, uintptr\_t addr) <br>_Set XCP Memory Transfer Address (MTA)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_XCP\_CMD\_CONNECT**](syn__xcp_8h.md#define-syn_xcp_cmd_connect)  `0xFFU`<br> |
| define  | [**SYN\_XCP\_CMD\_DISCONNECT**](syn__xcp_8h.md#define-syn_xcp_cmd_disconnect)  `0xFEU`<br> |
| define  | [**SYN\_XCP\_CMD\_DOWNLOAD**](syn__xcp_8h.md#define-syn_xcp_cmd_download)  `0xF0U`<br> |
| define  | [**SYN\_XCP\_CMD\_DOWNLOAD\_MAX**](syn__xcp_8h.md#define-syn_xcp_cmd_download_max)  `0xEEU`<br> |
| define  | [**SYN\_XCP\_CMD\_GET\_COMM\_MODE\_INFO**](syn__xcp_8h.md#define-syn_xcp_cmd_get_comm_mode_info)  `0xFBU`<br> |
| define  | [**SYN\_XCP\_CMD\_GET\_ID**](syn__xcp_8h.md#define-syn_xcp_cmd_get_id)  `0xFAU`<br> |
| define  | [**SYN\_XCP\_CMD\_GET\_SEED**](syn__xcp_8h.md#define-syn_xcp_cmd_get_seed)  `0xF8U`<br> |
| define  | [**SYN\_XCP\_CMD\_GET\_STATUS**](syn__xcp_8h.md#define-syn_xcp_cmd_get_status)  `0xFDU`<br> |
| define  | [**SYN\_XCP\_CMD\_SET\_DAQ\_LIST\_MODE**](syn__xcp_8h.md#define-syn_xcp_cmd_set_daq_list_mode)  `0xE0U`<br> |
| define  | [**SYN\_XCP\_CMD\_SET\_DAQ\_PTR**](syn__xcp_8h.md#define-syn_xcp_cmd_set_daq_ptr)  `0xE2U`<br> |
| define  | [**SYN\_XCP\_CMD\_SET\_MTA**](syn__xcp_8h.md#define-syn_xcp_cmd_set_mta)  `0xF6U`<br> |
| define  | [**SYN\_XCP\_CMD\_SET\_REQUEST**](syn__xcp_8h.md#define-syn_xcp_cmd_set_request)  `0xF9U`<br> |
| define  | [**SYN\_XCP\_CMD\_SHORT\_UPLOAD**](syn__xcp_8h.md#define-syn_xcp_cmd_short_upload)  `0xF4U`<br> |
| define  | [**SYN\_XCP\_CMD\_START\_STOP\_DAQ\_LIST**](syn__xcp_8h.md#define-syn_xcp_cmd_start_stop_daq_list)  `0xDEU`<br> |
| define  | [**SYN\_XCP\_CMD\_START\_STOP\_SYNCH**](syn__xcp_8h.md#define-syn_xcp_cmd_start_stop_synch)  `0xDDU`<br> |
| define  | [**SYN\_XCP\_CMD\_SYNCH**](syn__xcp_8h.md#define-syn_xcp_cmd_synch)  `0xFCU`<br> |
| define  | [**SYN\_XCP\_CMD\_UNLOCK**](syn__xcp_8h.md#define-syn_xcp_cmd_unlock)  `0xF7U`<br> |
| define  | [**SYN\_XCP\_CMD\_UPLOAD**](syn__xcp_8h.md#define-syn_xcp_cmd_upload)  `0xF5U`<br> |
| define  | [**SYN\_XCP\_CMD\_WRITE\_DAQ**](syn__xcp_8h.md#define-syn_xcp_cmd_write_daq)  `0xE1U`<br> |
| define  | [**SYN\_XCP\_ERR\_ACCESS\_DENIED**](syn__xcp_8h.md#define-syn_xcp_err_access_denied)  `0x24U`<br> |
| define  | [**SYN\_XCP\_ERR\_CMD\_SYNTAX**](syn__xcp_8h.md#define-syn_xcp_err_cmd_syntax)  `0x20U`<br> |
| define  | [**SYN\_XCP\_ERR\_CMD\_UNKNOWN**](syn__xcp_8h.md#define-syn_xcp_err_cmd_unknown)  `0x30U`<br> |
| define  | [**SYN\_XCP\_ERR\_NOT\_CONNECTED**](syn__xcp_8h.md#define-syn_xcp_err_not_connected)  `0x28U`<br> |
| define  | [**SYN\_XCP\_ERR\_OUT\_OF\_RANGE**](syn__xcp_8h.md#define-syn_xcp_err_out_of_range)  `0x22U`<br> |
| define  | [**SYN\_XCP\_ERR\_SUCCESS**](syn__xcp_8h.md#define-syn_xcp_err_success)  `0x00U`<br> |
| define  | [**SYN\_XCP\_ERR\_WRITE\_PROTECTED**](syn__xcp_8h.md#define-syn_xcp_err_write_protected)  `0x23U`<br> |
| define  | [**SYN\_XCP\_MAX\_DAQ\_LISTS**](syn__xcp_8h.md#define-syn_xcp_max_daq_lists)  `4U`<br> |
| define  | [**SYN\_XCP\_MAX\_ENTRIES\_PER\_ODT**](syn__xcp_8h.md#define-syn_xcp_max_entries_per_odt)  `7U`<br> |
| define  | [**SYN\_XCP\_MAX\_ODT\_PER\_DAQ**](syn__xcp_8h.md#define-syn_xcp_max_odt_per_daq)  `8U`<br> |
| define  | [**SYN\_XCP\_PID\_ERR**](syn__xcp_8h.md#define-syn_xcp_pid_err)  `0xFEU`<br> |
| define  | [**SYN\_XCP\_PID\_EV**](syn__xcp_8h.md#define-syn_xcp_pid_ev)  `0xFDU`<br> |
| define  | [**SYN\_XCP\_PID\_RES**](syn__xcp_8h.md#define-syn_xcp_pid_res)  `0xFFU`<br> |
| define  | [**SYN\_XCP\_PID\_SERV**](syn__xcp_8h.md#define-syn_xcp_pid_serv)  `0xFCU`<br> |
| define  | [**SYN\_XCP\_RESOURCE\_CAL\_PAG**](syn__xcp_8h.md#define-syn_xcp_resource_cal_pag)  `0x01U`<br> |
| define  | [**SYN\_XCP\_RESOURCE\_DAQ**](syn__xcp_8h.md#define-syn_xcp_resource_daq)  `0x04U`<br> |
| define  | [**SYN\_XCP\_RESOURCE\_PGM**](syn__xcp_8h.md#define-syn_xcp_resource_pgm)  `0x10U`<br> |
| define  | [**SYN\_XCP\_RESOURCE\_STIM**](syn__xcp_8h.md#define-syn_xcp_resource_stim)  `0x08U`<br> |

## Detailed Description


Provides a zero-allocation, lightweight XCP v1.x slave protocol stack for ECU calibration, live parameter measurement, memory upload/download, and DAQ (Data Acquisition) list streaming over CAN. 


    
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
## Macro Definition Documentation





### define SYN\_XCP\_CMD\_CONNECT 

```C++
#define SYN_XCP_CMD_CONNECT `0xFFU`
```



Connect command 


        

<hr>



### define SYN\_XCP\_CMD\_DISCONNECT 

```C++
#define SYN_XCP_CMD_DISCONNECT `0xFEU`
```



Disconnect command 


        

<hr>



### define SYN\_XCP\_CMD\_DOWNLOAD 

```C++
#define SYN_XCP_CMD_DOWNLOAD `0xF0U`
```



Download memory block 


        

<hr>



### define SYN\_XCP\_CMD\_DOWNLOAD\_MAX 

```C++
#define SYN_XCP_CMD_DOWNLOAD_MAX `0xEEU`
```



Fast max download 


        

<hr>



### define SYN\_XCP\_CMD\_GET\_COMM\_MODE\_INFO 

```C++
#define SYN_XCP_CMD_GET_COMM_MODE_INFO `0xFBU`
```



Get comm mode info 


        

<hr>



### define SYN\_XCP\_CMD\_GET\_ID 

```C++
#define SYN_XCP_CMD_GET_ID `0xFAU`
```



Get station ID 


        

<hr>



### define SYN\_XCP\_CMD\_GET\_SEED 

```C++
#define SYN_XCP_CMD_GET_SEED `0xF8U`
```



Get seed for unlock 


        

<hr>



### define SYN\_XCP\_CMD\_GET\_STATUS 

```C++
#define SYN_XCP_CMD_GET_STATUS `0xFDU`
```



Get status command 


        

<hr>



### define SYN\_XCP\_CMD\_SET\_DAQ\_LIST\_MODE 

```C++
#define SYN_XCP_CMD_SET_DAQ_LIST_MODE `0xE0U`
```



Set DAQ list mode 


        

<hr>



### define SYN\_XCP\_CMD\_SET\_DAQ\_PTR 

```C++
#define SYN_XCP_CMD_SET_DAQ_PTR `0xE2U`
```



Set DAQ pointer 


        

<hr>



### define SYN\_XCP\_CMD\_SET\_MTA 

```C++
#define SYN_XCP_CMD_SET_MTA `0xF6U`
```



Set memory transfer address 


        

<hr>



### define SYN\_XCP\_CMD\_SET\_REQUEST 

```C++
#define SYN_XCP_CMD_SET_REQUEST `0xF9U`
```



Set request 


        

<hr>



### define SYN\_XCP\_CMD\_SHORT\_UPLOAD 

```C++
#define SYN_XCP_CMD_SHORT_UPLOAD `0xF4U`
```



Short upload 


        

<hr>



### define SYN\_XCP\_CMD\_START\_STOP\_DAQ\_LIST 

```C++
#define SYN_XCP_CMD_START_STOP_DAQ_LIST `0xDEU`
```



Start/stop single DAQ list 


        

<hr>



### define SYN\_XCP\_CMD\_START\_STOP\_SYNCH 

```C++
#define SYN_XCP_CMD_START_STOP_SYNCH `0xDDU`
```



Start/stop synchronous DAQ 


        

<hr>



### define SYN\_XCP\_CMD\_SYNCH 

```C++
#define SYN_XCP_CMD_SYNCH `0xFCU`
```



Synchronize command 


        

<hr>



### define SYN\_XCP\_CMD\_UNLOCK 

```C++
#define SYN_XCP_CMD_UNLOCK `0xF7U`
```



Unlock resource with key 


        

<hr>



### define SYN\_XCP\_CMD\_UPLOAD 

```C++
#define SYN_XCP_CMD_UPLOAD `0xF5U`
```



Upload memory block 


        

<hr>



### define SYN\_XCP\_CMD\_WRITE\_DAQ 

```C++
#define SYN_XCP_CMD_WRITE_DAQ `0xE1U`
```



Write DAQ entry 


        

<hr>



### define SYN\_XCP\_ERR\_ACCESS\_DENIED 

```C++
#define SYN_XCP_ERR_ACCESS_DENIED `0x24U`
```



Access denied 


        

<hr>



### define SYN\_XCP\_ERR\_CMD\_SYNTAX 

```C++
#define SYN_XCP_ERR_CMD_SYNTAX `0x20U`
```



Command syntax error 


        

<hr>



### define SYN\_XCP\_ERR\_CMD\_UNKNOWN 

```C++
#define SYN_XCP_ERR_CMD_UNKNOWN `0x30U`
```



Command unknown 


        

<hr>



### define SYN\_XCP\_ERR\_NOT\_CONNECTED 

```C++
#define SYN_XCP_ERR_NOT_CONNECTED `0x28U`
```



Station not connected 


        

<hr>



### define SYN\_XCP\_ERR\_OUT\_OF\_RANGE 

```C++
#define SYN_XCP_ERR_OUT_OF_RANGE `0x22U`
```



Parameter out of range 


        

<hr>



### define SYN\_XCP\_ERR\_SUCCESS 

```C++
#define SYN_XCP_ERR_SUCCESS `0x00U`
```



Action succeeded 


        

<hr>



### define SYN\_XCP\_ERR\_WRITE\_PROTECTED 

```C++
#define SYN_XCP_ERR_WRITE_PROTECTED `0x23U`
```



Memory write protected 


        

<hr>



### define SYN\_XCP\_MAX\_DAQ\_LISTS 

```C++
#define SYN_XCP_MAX_DAQ_LISTS `4U`
```



Maximum DAQ list count 


        

<hr>



### define SYN\_XCP\_MAX\_ENTRIES\_PER\_ODT 

```C++
#define SYN_XCP_MAX_ENTRIES_PER_ODT `7U`
```



Maximum entries per ODT 


        

<hr>



### define SYN\_XCP\_MAX\_ODT\_PER\_DAQ 

```C++
#define SYN_XCP_MAX_ODT_PER_DAQ `8U`
```



Maximum ODTs per DAQ list 


        

<hr>



### define SYN\_XCP\_PID\_ERR 

```C++
#define SYN_XCP_PID_ERR `0xFEU`
```



Error packet 


        

<hr>



### define SYN\_XCP\_PID\_EV 

```C++
#define SYN_XCP_PID_EV `0xFDU`
```



Event packet 


        

<hr>



### define SYN\_XCP\_PID\_RES 

```C++
#define SYN_XCP_PID_RES `0xFFU`
```



Response packet 


        

<hr>



### define SYN\_XCP\_PID\_SERV 

```C++
#define SYN_XCP_PID_SERV `0xFCU`
```



Service packet 


        

<hr>



### define SYN\_XCP\_RESOURCE\_CAL\_PAG 

```C++
#define SYN_XCP_RESOURCE_CAL_PAG `0x01U`
```



Calibration/paging resource mask 


        

<hr>



### define SYN\_XCP\_RESOURCE\_DAQ 

```C++
#define SYN_XCP_RESOURCE_DAQ `0x04U`
```



Data acquisition resource mask 


        

<hr>



### define SYN\_XCP\_RESOURCE\_PGM 

```C++
#define SYN_XCP_RESOURCE_PGM `0x10U`
```



Flash programming resource mask 


        

<hr>



### define SYN\_XCP\_RESOURCE\_STIM 

```C++
#define SYN_XCP_RESOURCE_STIM `0x08U`
```



Data stimulation resource mask 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xcp.h`

