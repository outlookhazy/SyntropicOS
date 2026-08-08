

# File syn\_uds.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_uds.c**](syn__uds_8c.md)

[Go to the source code of this file](syn__uds_8c_source.md)

_ISO 14229 Unified Diagnostic Services (UDS) Server Implementation._ 

* `#include "syn_uds.h"`
* `#include "../util/syn_aes128.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_uds\_clear\_pending\_reset**](#function-syn_uds_clear_pending_reset) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Clear pending ECU reset flag after executing reset or transmitting response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_disable\_aes128\_security**](#function-syn_uds_disable_aes128_security) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Disable AES-128 security mode and revert to standard XOR security key calculation._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_get\_status**](#function-syn_uds_dtc_get_status) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dtc, uint8\_t \* out\_status) <br>_Get current 8-bit status byte for a registered DTC._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_report\_test\_result**](#function-syn_uds_dtc_report_test_result) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dtc, [**bool**](syn__defs_8h.md#enum-bool) failed) <br>_Report diagnostic test result for a registered DTC according to ISO 14229-1 state transitions._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_start\_operation\_cycle**](#function-syn_uds_dtc_start_operation_cycle) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Advance server state to a new operation cycle. Clears testFailedThisOperationCycle and sets testNotCompletedThisOperationCycle for all registered DTCs._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_enable\_aes128\_security**](#function-syn_uds_enable_aes128_security) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, const uint8\_t key) <br>_Enable AES-128 algorithm for UDS SecurityAccess (0x27) seed/key unlock._  |
|  uint8\_t | [**syn\_uds\_get\_pending\_reset**](#function-syn_uds_get_pending_reset) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Get pending ECU reset sub-function requested by 0x11 service._  |
|  uint8\_t | [**syn\_uds\_get\_security\_level**](#function-syn_uds_get_security_level) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Get currently unlocked security level (0 = locked, 1 = level 1, 2 = level 2...)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_init**](#function-syn_uds_init) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Initialize UDS Server context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_is\_sid\_functional\_supported**](#function-syn_uds_is_sid_functional_supported) (uint8\_t sid) <br>_Check if Service Identifier (SID) supports Functional Addressing (1:N Broadcast)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_process\_request**](#function-syn_uds_process_request) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, const uint8\_t \* req, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t max\_resp\_len, uint16\_t \* resp\_len, [**SYN\_UDS\_AddrMode**](syn__uds_8h.md#enum-syn_uds_addrmode) addr\_mode) <br>_Process incoming UDS request diagnostic payload and format response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_access\_timing**](#function-syn_uds_register_access_timing) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_AccessTimingHandler**](syn__uds_8h.md#typedef-syn_uds_accesstiminghandler) handler, void \* ctx) <br>_Register AccessTimingParameter (0x83) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_auth\_handler**](#function-syn_uds_register_auth_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_AuthHandler**](syn__uds_8h.md#typedef-syn_uds_authhandler) handler, void \* ctx) <br>_Register Authentication (0x29) handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_comm\_control**](#function-syn_uds_register_comm_control) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_CommControlHandler**](syn__uds_8h.md#typedef-syn_uds_commcontrolhandler) handler, void \* ctx) <br>_Register CommunicationControl (0x28) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_did**](#function-syn_uds_register_did) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint16\_t did, uint8\_t \* data, uint16\_t len, [**bool**](syn__defs_8h.md#enum-bool) writable) <br>_Register Data Identifier (DID) mapping in UDS Server table._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_did\_ext**](#function-syn_uds_register_did_ext) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint16\_t did, uint8\_t \* data, uint16\_t len, [**bool**](syn__defs_8h.md#enum-bool) writable, uint8\_t session\_mask, uint16\_t security\_mask) <br>_Register Data Identifier (DID) with custom session & security permission bitmask._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_dtc**](#function-syn_uds_register_dtc) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dtc, uint8\_t status, uint8\_t severity) <br>_Register Diagnostic Trouble Code (DTC) in UDS Server table._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_dtc\_handler**](#function-syn_uds_register_dtc_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_DTCHandler**](syn__uds_8h.md#typedef-syn_uds_dtchandler) handler, void \* ctx) <br>_Register ReadDTCInformation (0x19) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_dynamic\_did\_handler**](#function-syn_uds_register_dynamic_did_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_DynamicDIDHandler**](syn__uds_8h.md#typedef-syn_uds_dynamicdidhandler) handler, void \* ctx) <br>_Register DynamicallyDefineDataIdentifier (0x2C) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_file\_transfer**](#function-syn_uds_register_file_transfer) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_FileTransferHandler**](syn__uds_8h.md#typedef-syn_uds_filetransferhandler) handler, void \* ctx) <br>_Register RequestFileTransfer (0x38) handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_io\_control**](#function-syn_uds_register_io_control) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_IOControlHandler**](syn__uds_8h.md#typedef-syn_uds_iocontrolhandler) handler, void \* ctx) <br>_Register InputOutputControlByIdentifier (0x2F) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_link\_control**](#function-syn_uds_register_link_control) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_LinkControlHandler**](syn__uds_8h.md#typedef-syn_uds_linkcontrolhandler) handler, void \* ctx) <br>_Register LinkControl (0x87) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_memory\_handler**](#function-syn_uds_register_memory_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_MemoryHandler**](syn__uds_8h.md#typedef-syn_uds_memoryhandler) handler, void \* ctx) <br>_Register ReadMemoryByAddress (0x23) and WriteMemoryByAddress (0x3D) handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_periodic\_data\_handler**](#function-syn_uds_register_periodic_data_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_PeriodicDataHandler**](syn__uds_8h.md#typedef-syn_uds_periodicdatahandler) handler, void \* ctx) <br>_Register ReadDataByPeriodicIdentifier (0x2A) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_roe\_handler**](#function-syn_uds_register_roe_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_ResponseOnEventHandler**](syn__uds_8h.md#typedef-syn_uds_responseoneventhandler) handler, void \* ctx) <br>_Register ResponseOnEvent (0x86) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_routine\_control**](#function-syn_uds_register_routine_control) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_RoutineControlHandler**](syn__uds_8h.md#typedef-syn_uds_routinecontrolhandler) handler, void \* ctx) <br>_Register RoutineControl (0x31) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_scaling\_data\_handler**](#function-syn_uds_register_scaling_data_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_ScalingDataHandler**](syn__uds_8h.md#typedef-syn_uds_scalingdatahandler) handler, void \* ctx) <br>_Register ReadScalingDataByIdentifier (0x24) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_secured\_data**](#function-syn_uds_register_secured_data) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_SecuredDataHandler**](syn__uds_8h.md#typedef-syn_uds_secureddatahandler) handler, void \* ctx) <br>_Register SecuredDataTransmission (0x84) callback handler._  |
|  void | [**syn\_uds\_set\_reset\_handler**](#function-syn_uds_set_reset_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_ResetHandler**](syn__uds_8h.md#typedef-syn_uds_resethandler) cb, void \* ctx) <br>_Register deferred post-TX ECU reset handler callback._  |
|  void | [**syn\_uds\_set\_reset\_wait\_ms**](#function-syn_uds_set_reset_wait_ms) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint16\_t wait\_ms) <br>_Set post-TX ECU reset delay window duration in milliseconds._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_set\_security\_seed\_bytes**](#function-syn_uds_set_security_seed_bytes) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, const uint8\_t seed) <br>_Set custom 16-byte seed for AES-128 SecurityAccess._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_set\_service\_security\_mask**](#function-syn_uds_set_service_security_mask) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint8\_t sid, uint16\_t security\_mask) <br>_Configure required security level mask for a specific Service Identifier._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_set\_service\_session\_mask**](#function-syn_uds_set_service_session_mask) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint8\_t sid, uint8\_t session\_mask) <br>_Configure allowed diagnostic session mask for a specific Service Identifier._  |
|  void | [**syn\_uds\_set\_session\_transition\_handler**](#function-syn_uds_set_session_transition_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_SessionTransitionHandler**](syn__uds_8h.md#typedef-syn_uds_sessiontransitionhandler) cb, void \* ctx) <br>_Register optional session transition policy callback._  |
|  void | [**syn\_uds\_tick**](#function-syn_uds_tick) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dt\_ms) <br>_Advance periodic S3 server timer by dt\_ms milliseconds. Reverts session to DEFAULT and locks security state if S3 timer expires (5000ms)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int | [**find\_did\_index**](#function-find_did_index) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint16\_t did) <br>_Search for DID index in UDS server table._  |
|  uint16\_t | [**get\_active\_security\_mask**](#function-get_active_security_mask) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Get currently active security level bitmask for server._  |
|  uint8\_t | [**get\_sid\_allowed\_session\_mask**](#function-get_sid_allowed_session_mask) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint8\_t sid) <br>_Get allowed session bitmask for target Service Identifier._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**get\_sid\_functional\_support**](#function-get_sid_functional_support) (uint8\_t sid) <br>_Check if Service Identifier supports Functional Addressing (1:N Broadcast)._  |
|  uint16\_t | [**get\_sid\_required\_security\_mask**](#function-get_sid_required_security_mask) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint8\_t sid) <br>_Get required security level bitmask for target Service Identifier._  |
|  uint8\_t | [**session\_to\_mask**](#function-session_to_mask) ([**SYN\_UDS\_Session**](syn__uds_8h.md#enum-syn_uds_session) session) <br>_Convert UDS Session enum to session bitmask representation._  |


























## Public Functions Documentation




### function syn\_uds\_clear\_pending\_reset 

_Clear pending ECU reset flag after executing reset or transmitting response._ 
```C++
void syn_uds_clear_pending_reset (
    SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 




        

<hr>



### function syn\_uds\_disable\_aes128\_security 

_Disable AES-128 security mode and revert to standard XOR security key calculation._ 
```C++
bool syn_uds_disable_aes128_security (
    SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_dtc\_get\_status 

_Get current 8-bit status byte for a registered DTC._ 
```C++
bool syn_uds_dtc_get_status (
    SYN_UDS_Server * server,
    uint32_t dtc,
    uint8_t * out_status
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `dtc` 24-bit Diagnostic Trouble Code. 
* `out_status` Output pointer to store DTC status byte. 



**Returns:**

true if DTC found, false otherwise. 





        

<hr>



### function syn\_uds\_dtc\_report\_test\_result 

_Report diagnostic test result for a registered DTC according to ISO 14229-1 state transitions._ 
```C++
bool syn_uds_dtc_report_test_result (
    SYN_UDS_Server * server,
    uint32_t dtc,
    bool failed
) 
```



Updates DTC status bits (testFailed, testFailedThisOperationCycle, pendingDTC, confirmedDTC, etc.) and fault detection counter.




**Parameters:**


* `server` Pointer to UDS server instance. 
* `dtc` 24-bit Diagnostic Trouble Code. 
* `failed` True if test failed, false if test passed. 



**Returns:**

true if DTC found and updated, false otherwise. 





        

<hr>



### function syn\_uds\_dtc\_start\_operation\_cycle 

_Advance server state to a new operation cycle. Clears testFailedThisOperationCycle and sets testNotCompletedThisOperationCycle for all registered DTCs._ 
```C++
bool syn_uds_dtc_start_operation_cycle (
    SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_enable\_aes128\_security 

_Enable AES-128 algorithm for UDS SecurityAccess (0x27) seed/key unlock._ 
```C++
bool syn_uds_enable_aes128_security (
    SYN_UDS_Server * server,
    const uint8_t key
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `key` 16-byte AES-128 secret key. 



**Returns:**

true on success, false if server or key is NULL. 





        

<hr>



### function syn\_uds\_get\_pending\_reset 

_Get pending ECU reset sub-function requested by 0x11 service._ 
```C++
uint8_t syn_uds_get_pending_reset (
    const SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 



**Returns:**

Pending reset type (e.g. 0x01 = hard, 0x02 = keyOffOn, 0x03 = soft), or 0 if none. 





        

<hr>



### function syn\_uds\_get\_security\_level 

_Get currently unlocked security level (0 = locked, 1 = level 1, 2 = level 2...)._ 
```C++
uint8_t syn_uds_get_security_level (
    const SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 



**Returns:**

Active security level uint8\_t. 





        

<hr>



### function syn\_uds\_init 

_Initialize UDS Server context._ 
```C++
bool syn_uds_init (
    SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_is\_sid\_functional\_supported 

_Check if Service Identifier (SID) supports Functional Addressing (1:N Broadcast)._ 
```C++
bool syn_uds_is_sid_functional_supported (
    uint8_t sid
) 
```





**Parameters:**


* `sid` Service Identifier (SID). 



**Returns:**

true if service supports functional addressing, false if physical addressing only. 





        

<hr>



### function syn\_uds\_process\_request 

_Process incoming UDS request diagnostic payload and format response._ 
```C++
bool syn_uds_process_request (
    SYN_UDS_Server * server,
    const uint8_t * req,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t max_resp_len,
    uint16_t * resp_len,
    SYN_UDS_AddrMode addr_mode
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `req` Pointer to input request bytes. 
* `req_len` Length of request payload in bytes. 
* `resp_buf` Output buffer for response bytes. 
* `max_resp_len` Capacity of output response buffer. 
* `resp_len` Pointer to store generated response byte length. 
* `addr_mode` Addressing mode (SYN\_UDS\_ADDR\_PHYSICAL or SYN\_UDS\_ADDR\_FUNCTIONAL). 



**Returns:**

true if response frame generated, false on invalid parameters. 





        

<hr>



### function syn\_uds\_register\_access\_timing 

_Register AccessTimingParameter (0x83) callback handler._ 
```C++
bool syn_uds_register_access_timing (
    SYN_UDS_Server * server,
    SYN_UDS_AccessTimingHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x83 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_auth\_handler 

_Register Authentication (0x29) handler._ 
```C++
bool syn_uds_register_auth_handler (
    SYN_UDS_Server * server,
    SYN_UDS_AuthHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x29 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_comm\_control 

_Register CommunicationControl (0x28) callback handler._ 
```C++
bool syn_uds_register_comm_control (
    SYN_UDS_Server * server,
    SYN_UDS_CommControlHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x28 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_did 

_Register Data Identifier (DID) mapping in UDS Server table._ 
```C++
bool syn_uds_register_did (
    SYN_UDS_Server * server,
    uint16_t did,
    uint8_t * data,
    uint16_t len,
    bool writable
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `did` 16-bit Data Identifier code (e.g., 0xF190 for VIN). 
* `data` Pointer to data memory buffer. 
* `len` Data byte length. 
* `writable` True if DID allows WriteDataByIdentifier (0x2E). 



**Returns:**

true on success, false if table full or invalid params. 





        

<hr>



### function syn\_uds\_register\_did\_ext 

_Register Data Identifier (DID) with custom session & security permission bitmask._ 
```C++
bool syn_uds_register_did_ext (
    SYN_UDS_Server * server,
    uint16_t did,
    uint8_t * data,
    uint16_t len,
    bool writable,
    uint8_t session_mask,
    uint16_t security_mask
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `did` 16-bit Data Identifier code (e.g., 0xF190 for VIN). 
* `data` Pointer to data memory buffer. 
* `len` Data byte length. 
* `writable` True if DID allows WriteDataByIdentifier (0x2E). 
* `session_mask` Permitted session bitmask (SYN\_UDS\_SESSION\_MASK\_\*). 
* `security_mask` Permitted security level bitmask (SYN\_UDS\_SECURITY\_MASK\_\*). 



**Returns:**

true on success, false if table full or invalid params. 





        

<hr>



### function syn\_uds\_register\_dtc 

_Register Diagnostic Trouble Code (DTC) in UDS Server table._ 
```C++
bool syn_uds_register_dtc (
    SYN_UDS_Server * server,
    uint32_t dtc,
    uint8_t status,
    uint8_t severity
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `dtc` 24-bit Diagnostic Trouble Code (e.g., 0x012345). 
* `status` Initial DTCStatusByte bitmask. 
* `severity` DTCSeverityByte (severity & class). 



**Returns:**

true on success, false if table full or invalid params. 





        

<hr>



### function syn\_uds\_register\_dtc\_handler 

_Register ReadDTCInformation (0x19) callback handler._ 
```C++
bool syn_uds_register_dtc_handler (
    SYN_UDS_Server * server,
    SYN_UDS_DTCHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked for custom DTC subfunctions. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_dynamic\_did\_handler 

_Register DynamicallyDefineDataIdentifier (0x2C) callback handler._ 
```C++
bool syn_uds_register_dynamic_did_handler (
    SYN_UDS_Server * server,
    SYN_UDS_DynamicDIDHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x2C is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_file\_transfer 

_Register RequestFileTransfer (0x38) handler._ 
```C++
bool syn_uds_register_file_transfer (
    SYN_UDS_Server * server,
    SYN_UDS_FileTransferHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x38 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_io\_control 

_Register InputOutputControlByIdentifier (0x2F) callback handler._ 
```C++
bool syn_uds_register_io_control (
    SYN_UDS_Server * server,
    SYN_UDS_IOControlHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x2F is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_link\_control 

_Register LinkControl (0x87) callback handler._ 
```C++
bool syn_uds_register_link_control (
    SYN_UDS_Server * server,
    SYN_UDS_LinkControlHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x87 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_memory\_handler 

_Register ReadMemoryByAddress (0x23) and WriteMemoryByAddress (0x3D) handler._ 
```C++
bool syn_uds_register_memory_handler (
    SYN_UDS_Server * server,
    SYN_UDS_MemoryHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when memory services are processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_periodic\_data\_handler 

_Register ReadDataByPeriodicIdentifier (0x2A) callback handler._ 
```C++
bool syn_uds_register_periodic_data_handler (
    SYN_UDS_Server * server,
    SYN_UDS_PeriodicDataHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x2A is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_roe\_handler 

_Register ResponseOnEvent (0x86) callback handler._ 
```C++
bool syn_uds_register_roe_handler (
    SYN_UDS_Server * server,
    SYN_UDS_ResponseOnEventHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x86 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_routine\_control 

_Register RoutineControl (0x31) callback handler._ 
```C++
bool syn_uds_register_routine_control (
    SYN_UDS_Server * server,
    SYN_UDS_RoutineControlHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x31 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_scaling\_data\_handler 

_Register ReadScalingDataByIdentifier (0x24) callback handler._ 
```C++
bool syn_uds_register_scaling_data_handler (
    SYN_UDS_Server * server,
    SYN_UDS_ScalingDataHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x24 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_secured\_data 

_Register SecuredDataTransmission (0x84) callback handler._ 
```C++
bool syn_uds_register_secured_data (
    SYN_UDS_Server * server,
    SYN_UDS_SecuredDataHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x84 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_set\_reset\_handler 

_Register deferred post-TX ECU reset handler callback._ 
```C++
void syn_uds_set_reset_handler (
    SYN_UDS_Server * server,
    SYN_UDS_ResetHandler cb,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `cb` Callback function to execute after post-TX reset delay. 
* `ctx` Context pointer passed to callback function. 




        

<hr>



### function syn\_uds\_set\_reset\_wait\_ms 

_Set post-TX ECU reset delay window duration in milliseconds._ 
```C++
void syn_uds_set_reset_wait_ms (
    SYN_UDS_Server * server,
    uint16_t wait_ms
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `wait_ms` Delay duration in ms before executing reset callback. 




        

<hr>



### function syn\_uds\_set\_security\_seed\_bytes 

_Set custom 16-byte seed for AES-128 SecurityAccess._ 
```C++
bool syn_uds_set_security_seed_bytes (
    SYN_UDS_Server * server,
    const uint8_t seed
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `seed` 16-byte seed buffer. 



**Returns:**

true on success, false if server or seed is NULL. 





        

<hr>



### function syn\_uds\_set\_service\_security\_mask 

_Configure required security level mask for a specific Service Identifier._ 
```C++
bool syn_uds_set_service_security_mask (
    SYN_UDS_Server * server,
    uint8_t sid,
    uint16_t security_mask
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `sid` Service Identifier (e.g. 0x27, 0x34). 
* `security_mask` Required security level bitmask (SYN\_UDS\_SECURITY\_MASK\_\*). 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_set\_service\_session\_mask 

_Configure allowed diagnostic session mask for a specific Service Identifier._ 
```C++
bool syn_uds_set_service_session_mask (
    SYN_UDS_Server * server,
    uint8_t sid,
    uint8_t session_mask
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `sid` Service Identifier (e.g. 0x27, 0x34). 
* `session_mask` Allowed session bitmask (SYN\_UDS\_SESSION\_MASK\_\*). 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_set\_session\_transition\_handler 

_Register optional session transition policy callback._ 
```C++
void syn_uds_set_session_transition_handler (
    SYN_UDS_Server * server,
    SYN_UDS_SessionTransitionHandler cb,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `cb` Callback function to validate session transition graph permissions. 
* `ctx` Context pointer passed to callback function. 




        

<hr>



### function syn\_uds\_tick 

_Advance periodic S3 server timer by dt\_ms milliseconds. Reverts session to DEFAULT and locks security state if S3 timer expires (5000ms)._ 
```C++
void syn_uds_tick (
    SYN_UDS_Server * server,
    uint32_t dt_ms
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `dt_ms` Milliseconds elapsed since last tick. 




        

<hr>
## Public Static Functions Documentation




### function find\_did\_index 

_Search for DID index in UDS server table._ 
```C++
static int find_did_index (
    const SYN_UDS_Server * server,
    uint16_t did
) 
```





**Parameters:**


* `server` UDS server instance. 
* `did` 16-bit DID value to find. 



**Returns:**

Index in table, or -1 if not found. 





        

<hr>



### function get\_active\_security\_mask 

_Get currently active security level bitmask for server._ 
```C++
static uint16_t get_active_security_mask (
    const SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` UDS server instance pointer. 



**Returns:**

Currently active security level bitmask (bit 0 = level 0/none, bit N = level N). 





        

<hr>



### function get\_sid\_allowed\_session\_mask 

_Get allowed session bitmask for target Service Identifier._ 
```C++
static uint8_t get_sid_allowed_session_mask (
    const SYN_UDS_Server * server,
    uint8_t sid
) 
```





**Parameters:**


* `server` UDS server instance pointer. 
* `sid` Service Identifier (SID). 



**Returns:**

Allowed session bitmask for SID. 





        

<hr>



### function get\_sid\_functional\_support 

_Check if Service Identifier supports Functional Addressing (1:N Broadcast)._ 
```C++
static bool get_sid_functional_support (
    uint8_t sid
) 
```





**Parameters:**


* `sid` Service Identifier (SID). 



**Returns:**

true if service supports functional addressing, false if physical addressing only. 





        

<hr>



### function get\_sid\_required\_security\_mask 

_Get required security level bitmask for target Service Identifier._ 
```C++
static uint16_t get_sid_required_security_mask (
    const SYN_UDS_Server * server,
    uint8_t sid
) 
```





**Parameters:**


* `server` UDS server instance pointer. 
* `sid` Service Identifier (SID). 



**Returns:**

Required security level bitmask for SID (0 = no security check at top level). 





        

<hr>



### function session\_to\_mask 

_Convert UDS Session enum to session bitmask representation._ 
```C++
static uint8_t session_to_mask (
    SYN_UDS_Session session
) 
```





**Parameters:**


* `session` Active UDS session. 



**Returns:**

Bitmask representing active session (SYN\_UDS\_SESSION\_MASK\_\*). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_uds.c`

