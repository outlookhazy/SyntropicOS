

# File syn\_kwp2000.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_kwp2000.h**](syn__kwp2000_8h.md)

[Go to the source code of this file](syn__kwp2000_8h_source.md)

_ISO 14230-3 Key Word Protocol 2000 (KWP2000) Server Implementation._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_KWP2000\_CID**](structSYN__KWP2000__CID.md) <br>_Registered Common Identifier (CID) descriptor._  |
| struct | [**SYN\_KWP2000\_LID**](structSYN__KWP2000__LID.md) <br>_Registered Local Identifier (LID) descriptor._  |
| struct | [**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) <br>_KWP2000 Server instance state._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_KWP2000\_KeyHandler**](#typedef-syn_kwp2000_keyhandler)  <br>_Security Access key verification callback._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_KWP2000\_MemoryHandler**](#typedef-syn_kwp2000_memoryhandler)  <br>_Memory access callback for 0x34 RequestDownload & 0x36 TransferData._  |
| typedef void(\* | [**SYN\_KWP2000\_ResetHandler**](#typedef-syn_kwp2000_resethandler)  <br>_ECU reset callback function pointer._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_KWP2000\_RoutineHandler**](#typedef-syn_kwp2000_routinehandler)  <br>_Routine control callback._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_KWP2000\_SeedHandler**](#typedef-syn_kwp2000_seedhandler)  <br>_Security Access seed request callback._  |
| typedef uint8\_t | [**SYN\_KWP2000\_Session**](#typedef-syn_kwp2000_session)  <br>_KWP2000 session type._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_KWP2000\_SessionHandler**](#typedef-syn_kwp2000_sessionhandler)  <br>_Session transition callback function pointer._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_init**](#function-syn_kwp2000_init) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server) <br>_Initialize a KWP2000 diagnostic server instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_process\_request**](#function-syn_kwp2000_process_request) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t max\_resp\_len, uint16\_t \* resp\_len) <br>_Process an incoming KWP2000 request message PDU and generate response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_register\_cid**](#function-syn_kwp2000_register_cid) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, uint16\_t cid, const void \* data, uint16\_t len, [**bool**](syn__defs_8h.md#enum-bool) writable) <br>_Register a Common Identifier (CID, 2-byte) for Service 0x22 reading._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_register\_lid**](#function-syn_kwp2000_register_lid) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, uint8\_t lid, const void \* data, uint16\_t len, [**bool**](syn__defs_8h.md#enum-bool) writable) <br>_Register a Local Identifier (LID, 1-byte) for Service 0x21 reading._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_set\_memory\_handler**](#function-syn_kwp2000_set_memory_handler) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, [**SYN\_KWP2000\_MemoryHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_memoryhandler) cb, void \* user\_ctx) <br>_Register memory access handler for 0x34 RequestDownload & 0x36 TransferData._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_set\_reset\_handler**](#function-syn_kwp2000_set_reset_handler) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, [**SYN\_KWP2000\_ResetHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_resethandler) cb, void \* user\_ctx) <br>_Set ECU reset callback handler and deferred wait delay._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_set\_routine\_handler**](#function-syn_kwp2000_set_routine_handler) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, [**SYN\_KWP2000\_RoutineHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_routinehandler) cb, void \* user\_ctx) <br>_Register RoutineControl handler._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_set\_security\_handler**](#function-syn_kwp2000_set_security_handler) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, [**SYN\_KWP2000\_SeedHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_seedhandler) seed\_cb, [**SYN\_KWP2000\_KeyHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_keyhandler) key\_cb, void \* user\_ctx) <br>_Set security access seed and key callbacks._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_set\_session\_handler**](#function-syn_kwp2000_set_session_handler) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, [**SYN\_KWP2000\_SessionHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_sessionhandler) cb, void \* user\_ctx) <br>_Set session transition callback handler._  |
|  void | [**syn\_kwp2000\_tick**](#function-syn_kwp2000_tick) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, uint32\_t dt\_ms) <br>_Tick KWP2000 server S3 timer and deferred reset state._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_KWP2000\_MAX\_CIDS**](syn__kwp2000_8h.md#define-syn_kwp2000_max_cids)  `16U`<br> |
| define  | [**SYN\_KWP2000\_MAX\_LIDS**](syn__kwp2000_8h.md#define-syn_kwp2000_max_lids)  `16U`<br> |
| define  | [**SYN\_KWP2000\_NRC\_CONDITIONS\_NOT\_CORRECT**](syn__kwp2000_8h.md#define-syn_kwp2000_nrc_conditions_not_correct)  `0x22U`<br> |
| define  | [**SYN\_KWP2000\_NRC\_EXCEEDED\_NUMBER\_OF\_ATTEMPTS**](syn__kwp2000_8h.md#define-syn_kwp2000_nrc_exceeded_number_of_attempts)  `0x36U`<br> |
| define  | [**SYN\_KWP2000\_NRC\_INCORRECT\_MESSAGE\_LENGTH**](syn__kwp2000_8h.md#define-syn_kwp2000_nrc_incorrect_message_length)  `0x13U`<br> |
| define  | [**SYN\_KWP2000\_NRC\_INVALID\_KEY**](syn__kwp2000_8h.md#define-syn_kwp2000_nrc_invalid_key)  `0x35U`<br> |
| define  | [**SYN\_KWP2000\_NRC\_REQUEST\_OUT\_OF\_RANGE**](syn__kwp2000_8h.md#define-syn_kwp2000_nrc_request_out_of_range)  `0x31U`<br> |
| define  | [**SYN\_KWP2000\_NRC\_REQUIRED\_TIME\_DELAY\_NOT\_EXPIRED**](syn__kwp2000_8h.md#define-syn_kwp2000_nrc_required_time_delay_not_expired)  `0x37U`<br> |
| define  | [**SYN\_KWP2000\_NRC\_RESPONSE\_PENDING**](syn__kwp2000_8h.md#define-syn_kwp2000_nrc_response_pending)  `0x78U`<br> |
| define  | [**SYN\_KWP2000\_NRC\_SECURITY\_ACCESS\_DENIED**](syn__kwp2000_8h.md#define-syn_kwp2000_nrc_security_access_denied)  `0x33U`<br> |
| define  | [**SYN\_KWP2000\_NRC\_SERVICE\_NOT\_SUPPORTED**](syn__kwp2000_8h.md#define-syn_kwp2000_nrc_service_not_supported)  `0x11U`<br> |
| define  | [**SYN\_KWP2000\_NRC\_SUBFUNCTION\_NOT\_SUPPORTED**](syn__kwp2000_8h.md#define-syn_kwp2000_nrc_subfunction_not_supported)  `0x12U`<br> |
| define  | [**SYN\_KWP2000\_SESSION\_DEFAULT**](syn__kwp2000_8h.md#define-syn_kwp2000_session_default)  `0x81U`<br> |
| define  | [**SYN\_KWP2000\_SESSION\_EXTENDED**](syn__kwp2000_8h.md#define-syn_kwp2000_session_extended)  `0x86U`<br> |
| define  | [**SYN\_KWP2000\_SESSION\_PROGRAMMING**](syn__kwp2000_8h.md#define-syn_kwp2000_session_programming)  `0x85U`<br> |
| define  | [**SYN\_KWP2000\_SID\_CLEAR\_DIAGNOSTIC\_INFORMATION**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_clear_diagnostic_information)  `0x14U`<br> |
| define  | [**SYN\_KWP2000\_SID\_ECU\_RESET**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_ecu_reset)  `0x11U`<br> |
| define  | [**SYN\_KWP2000\_SID\_NEGATIVE\_RESPONSE**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_negative_response)  `0x7FU`<br> |
| define  | [**SYN\_KWP2000\_SID\_READ\_DATA\_BY\_COMMON\_IDENTIFIER**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_read_data_by_common_identifier)  `0x22U`<br> |
| define  | [**SYN\_KWP2000\_SID\_READ\_DATA\_BY\_LOCAL\_IDENTIFIER**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_read_data_by_local_identifier)  `0x21U`<br> |
| define  | [**SYN\_KWP2000\_SID\_READ\_DTC\_BY\_STATUS**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_read_dtc_by_status)  `0x18U`<br> |
| define  | [**SYN\_KWP2000\_SID\_READ\_ECU\_IDENTIFICATION**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_read_ecu_identification)  `0x1AU`<br> |
| define  | [**SYN\_KWP2000\_SID\_REQUEST\_DOWNLOAD**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_request_download)  `0x34U`<br> |
| define  | [**SYN\_KWP2000\_SID\_REQUEST\_TRANSFER\_EXIT**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_request_transfer_exit)  `0x37U`<br> |
| define  | [**SYN\_KWP2000\_SID\_SECURITY\_ACCESS**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_security_access)  `0x27U`<br> |
| define  | [**SYN\_KWP2000\_SID\_START\_DIAGNOSTIC\_SESSION**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_start_diagnostic_session)  `0x10U`<br> |
| define  | [**SYN\_KWP2000\_SID\_START\_ROUTINE\_BY\_LOCAL\_IDENTIFIER**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_start_routine_by_local_identifier)  `0x31U`<br> |
| define  | [**SYN\_KWP2000\_SID\_TESTER\_PRESENT**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_tester_present)  `0x3EU`<br> |
| define  | [**SYN\_KWP2000\_SID\_TRANSFER\_DATA**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_transfer_data)  `0x36U`<br> |
| define  | [**SYN\_KWP2000\_SID\_WRITE\_DATA\_BY\_COMMON\_IDENTIFIER**](syn__kwp2000_8h.md#define-syn_kwp2000_sid_write_data_by_common_identifier)  `0x2EU`<br> |

## Detailed Description


Provides a zero-allocation, lightweight KWP2000 server protocol stack over ISO 15765-2 ISO-TP for automotive diagnostic session management, Local/Common Identifier (LID/CID) read/write, security access seed/key unlocks, routine control, memory transfer, and ECU reset. 


    
## Public Types Documentation




### typedef SYN\_KWP2000\_KeyHandler 

_Security Access key verification callback._ 
```C++
typedef bool(* SYN_KWP2000_KeyHandler) (uint8_t level, const uint8_t *key, uint16_t key_len, void *user_ctx);
```




<hr>



### typedef SYN\_KWP2000\_MemoryHandler 

_Memory access callback for 0x34 RequestDownload & 0x36 TransferData._ 
```C++
typedef bool(* SYN_KWP2000_MemoryHandler) (bool is_write, uint32_t address, uint32_t size, uint8_t *data_buf, void *user_ctx);
```




<hr>



### typedef SYN\_KWP2000\_ResetHandler 

_ECU reset callback function pointer._ 
```C++
typedef void(* SYN_KWP2000_ResetHandler) (uint8_t reset_type, void *user_ctx);
```




<hr>



### typedef SYN\_KWP2000\_RoutineHandler 

_Routine control callback._ 
```C++
typedef bool(* SYN_KWP2000_RoutineHandler) (uint8_t routine_id, const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *user_ctx);
```




<hr>



### typedef SYN\_KWP2000\_SeedHandler 

_Security Access seed request callback._ 
```C++
typedef bool(* SYN_KWP2000_SeedHandler) (uint8_t level, uint8_t *seed_buf, uint16_t *seed_len, void *user_ctx);
```




<hr>



### typedef SYN\_KWP2000\_Session 

_KWP2000 session type._ 
```C++
typedef uint8_t SYN_KWP2000_Session;
```




<hr>



### typedef SYN\_KWP2000\_SessionHandler 

_Session transition callback function pointer._ 
```C++
typedef bool(* SYN_KWP2000_SessionHandler) (SYN_KWP2000_Session from_session, SYN_KWP2000_Session to_session, void *user_ctx);
```




<hr>
## Public Functions Documentation




### function syn\_kwp2000\_init 

_Initialize a KWP2000 diagnostic server instance._ 
```C++
SYN_Status syn_kwp2000_init (
    SYN_KWP2000_Server * server
) 
```





**Parameters:**


* `server` Pointer to server instance. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if server is NULL. 





        

<hr>



### function syn\_kwp2000\_process\_request 

_Process an incoming KWP2000 request message PDU and generate response._ 
```C++
SYN_Status syn_kwp2000_process_request (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t max_resp_len,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `req_buf` Pointer to request payload buffer. 
* `req_len` Length of request payload in bytes. 
* `resp_buf` Pointer to output response buffer. 
* `max_resp_len` Capacity of output response buffer. 
* `resp_len` Pointer to store output response length. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on failure. 





        

<hr>



### function syn\_kwp2000\_register\_cid 

_Register a Common Identifier (CID, 2-byte) for Service 0x22 reading._ 
```C++
SYN_Status syn_kwp2000_register_cid (
    SYN_KWP2000_Server * server,
    uint16_t cid,
    const void * data,
    uint16_t len,
    bool writable
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `cid` 2-byte Common Identifier. 
* `data` Pointer to data buffer. 
* `len` Data length in bytes. 
* `writable` Is CID writable? 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on error. 





        

<hr>



### function syn\_kwp2000\_register\_lid 

_Register a Local Identifier (LID, 1-byte) for Service 0x21 reading._ 
```C++
SYN_Status syn_kwp2000_register_lid (
    SYN_KWP2000_Server * server,
    uint8_t lid,
    const void * data,
    uint16_t len,
    bool writable
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `lid` 1-byte Local Identifier. 
* `data` Pointer to data buffer. 
* `len` Data length in bytes. 
* `writable` Is LID writable? 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on error. 





        

<hr>



### function syn\_kwp2000\_set\_memory\_handler 

_Register memory access handler for 0x34 RequestDownload & 0x36 TransferData._ 
```C++
SYN_Status syn_kwp2000_set_memory_handler (
    SYN_KWP2000_Server * server,
    SYN_KWP2000_MemoryHandler cb,
    void * user_ctx
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `cb` Memory handler callback. 
* `user_ctx` User context pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if server is NULL. 





        

<hr>



### function syn\_kwp2000\_set\_reset\_handler 

_Set ECU reset callback handler and deferred wait delay._ 
```C++
SYN_Status syn_kwp2000_set_reset_handler (
    SYN_KWP2000_Server * server,
    SYN_KWP2000_ResetHandler cb,
    void * user_ctx
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `cb` Reset handler callback. 
* `user_ctx` User context pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if server is NULL. 





        

<hr>



### function syn\_kwp2000\_set\_routine\_handler 

_Register RoutineControl handler._ 
```C++
SYN_Status syn_kwp2000_set_routine_handler (
    SYN_KWP2000_Server * server,
    SYN_KWP2000_RoutineHandler cb,
    void * user_ctx
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `cb` Routine handler callback. 
* `user_ctx` User context pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if server is NULL. 





        

<hr>



### function syn\_kwp2000\_set\_security\_handler 

_Set security access seed and key callbacks._ 
```C++
SYN_Status syn_kwp2000_set_security_handler (
    SYN_KWP2000_Server * server,
    SYN_KWP2000_SeedHandler seed_cb,
    SYN_KWP2000_KeyHandler key_cb,
    void * user_ctx
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `seed_cb` Seed request callback. 
* `key_cb` Key verification callback. 
* `user_ctx` User context pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if server is NULL. 





        

<hr>



### function syn\_kwp2000\_set\_session\_handler 

_Set session transition callback handler._ 
```C++
SYN_Status syn_kwp2000_set_session_handler (
    SYN_KWP2000_Server * server,
    SYN_KWP2000_SessionHandler cb,
    void * user_ctx
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `cb` Session handler callback. 
* `user_ctx` User context pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if server is NULL. 





        

<hr>



### function syn\_kwp2000\_tick 

_Tick KWP2000 server S3 timer and deferred reset state._ 
```C++
void syn_kwp2000_tick (
    SYN_KWP2000_Server * server,
    uint32_t dt_ms
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `dt_ms` Elapsed time step in milliseconds. 




        

<hr>
## Macro Definition Documentation





### define SYN\_KWP2000\_MAX\_CIDS 

```C++
#define SYN_KWP2000_MAX_CIDS `16U`
```



Maximum registered Common Identifiers (CID) 


        

<hr>



### define SYN\_KWP2000\_MAX\_LIDS 

```C++
#define SYN_KWP2000_MAX_LIDS `16U`
```



Maximum registered Local Identifiers (LID) 


        

<hr>



### define SYN\_KWP2000\_NRC\_CONDITIONS\_NOT\_CORRECT 

```C++
#define SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT `0x22U`
```



Conditions not correct 


        

<hr>



### define SYN\_KWP2000\_NRC\_EXCEEDED\_NUMBER\_OF\_ATTEMPTS 

```C++
#define SYN_KWP2000_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS `0x36U`
```



Exceeded number of attempts 


        

<hr>



### define SYN\_KWP2000\_NRC\_INCORRECT\_MESSAGE\_LENGTH 

```C++
#define SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH `0x13U`
```



Incorrect message length or invalid format 


        

<hr>



### define SYN\_KWP2000\_NRC\_INVALID\_KEY 

```C++
#define SYN_KWP2000_NRC_INVALID_KEY `0x35U`
```



Invalid key 


        

<hr>



### define SYN\_KWP2000\_NRC\_REQUEST\_OUT\_OF\_RANGE 

```C++
#define SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE `0x31U`
```



Request out of range 


        

<hr>



### define SYN\_KWP2000\_NRC\_REQUIRED\_TIME\_DELAY\_NOT\_EXPIRED 

```C++
#define SYN_KWP2000_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED `0x37U`
```



Required time delay not expired 


        

<hr>



### define SYN\_KWP2000\_NRC\_RESPONSE\_PENDING 

```C++
#define SYN_KWP2000_NRC_RESPONSE_PENDING `0x78U`
```



Request correctly received, response pending \ 


        

<hr>



### define SYN\_KWP2000\_NRC\_SECURITY\_ACCESS\_DENIED 

```C++
#define SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED `0x33U`
```



Security access denied 


        

<hr>



### define SYN\_KWP2000\_NRC\_SERVICE\_NOT\_SUPPORTED 

```C++
#define SYN_KWP2000_NRC_SERVICE_NOT_SUPPORTED `0x11U`
```



Service not supported 


        

<hr>



### define SYN\_KWP2000\_NRC\_SUBFUNCTION\_NOT\_SUPPORTED 

```C++
#define SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED `0x12U`
```



Subfunction not supported 


        

<hr>



### define SYN\_KWP2000\_SESSION\_DEFAULT 

```C++
#define SYN_KWP2000_SESSION_DEFAULT `0x81U`
```



Standard diagnostic session 


        

<hr>



### define SYN\_KWP2000\_SESSION\_EXTENDED 

```C++
#define SYN_KWP2000_SESSION_EXTENDED `0x86U`
```



Extended diagnostic session 


        

<hr>



### define SYN\_KWP2000\_SESSION\_PROGRAMMING 

```C++
#define SYN_KWP2000_SESSION_PROGRAMMING `0x85U`
```



ECU programming / bootloader session 


        

<hr>



### define SYN\_KWP2000\_SID\_CLEAR\_DIAGNOSTIC\_INFORMATION 

```C++
#define SYN_KWP2000_SID_CLEAR_DIAGNOSTIC_INFORMATION `0x14U`
```



ClearDiagnosticInformation 


        

<hr>



### define SYN\_KWP2000\_SID\_ECU\_RESET 

```C++
#define SYN_KWP2000_SID_ECU_RESET `0x11U`
```



ECUReset 


        

<hr>



### define SYN\_KWP2000\_SID\_NEGATIVE\_RESPONSE 

```C++
#define SYN_KWP2000_SID_NEGATIVE_RESPONSE `0x7FU`
```



NegativeResponse 


        

<hr>



### define SYN\_KWP2000\_SID\_READ\_DATA\_BY\_COMMON\_IDENTIFIER 

```C++
#define SYN_KWP2000_SID_READ_DATA_BY_COMMON_IDENTIFIER `0x22U`
```



ReadDataByCommonIdentifier 


        

<hr>



### define SYN\_KWP2000\_SID\_READ\_DATA\_BY\_LOCAL\_IDENTIFIER 

```C++
#define SYN_KWP2000_SID_READ_DATA_BY_LOCAL_IDENTIFIER `0x21U`
```



ReadDataByLocalIdentifier 


        

<hr>



### define SYN\_KWP2000\_SID\_READ\_DTC\_BY\_STATUS 

```C++
#define SYN_KWP2000_SID_READ_DTC_BY_STATUS `0x18U`
```



ReadDiagnosticTroubleCodesByStatus 


        

<hr>



### define SYN\_KWP2000\_SID\_READ\_ECU\_IDENTIFICATION 

```C++
#define SYN_KWP2000_SID_READ_ECU_IDENTIFICATION `0x1AU`
```



ReadECUIdentification 


        

<hr>



### define SYN\_KWP2000\_SID\_REQUEST\_DOWNLOAD 

```C++
#define SYN_KWP2000_SID_REQUEST_DOWNLOAD `0x34U`
```



RequestDownload 


        

<hr>



### define SYN\_KWP2000\_SID\_REQUEST\_TRANSFER\_EXIT 

```C++
#define SYN_KWP2000_SID_REQUEST_TRANSFER_EXIT `0x37U`
```



RequestTransferExit 


        

<hr>



### define SYN\_KWP2000\_SID\_SECURITY\_ACCESS 

```C++
#define SYN_KWP2000_SID_SECURITY_ACCESS `0x27U`
```



SecurityAccess 


        

<hr>



### define SYN\_KWP2000\_SID\_START\_DIAGNOSTIC\_SESSION 

```C++
#define SYN_KWP2000_SID_START_DIAGNOSTIC_SESSION `0x10U`
```



StartDiagnosticSession 


        

<hr>



### define SYN\_KWP2000\_SID\_START\_ROUTINE\_BY\_LOCAL\_IDENTIFIER 

```C++
#define SYN_KWP2000_SID_START_ROUTINE_BY_LOCAL_IDENTIFIER `0x31U`
```



StartRoutineByLocalIdentifier 


        

<hr>



### define SYN\_KWP2000\_SID\_TESTER\_PRESENT 

```C++
#define SYN_KWP2000_SID_TESTER_PRESENT `0x3EU`
```



TesterPresent 


        

<hr>



### define SYN\_KWP2000\_SID\_TRANSFER\_DATA 

```C++
#define SYN_KWP2000_SID_TRANSFER_DATA `0x36U`
```



TransferData 


        

<hr>



### define SYN\_KWP2000\_SID\_WRITE\_DATA\_BY\_COMMON\_IDENTIFIER 

```C++
#define SYN_KWP2000_SID_WRITE_DATA_BY_COMMON_IDENTIFIER `0x2EU`
```



WriteDataByCommonIdentifier 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_kwp2000.h`

