

# File syn\_kwp2000.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_kwp2000.c**](syn__kwp2000_8c.md)

[Go to the source code of this file](syn__kwp2000_8c_source.md)

_ISO 14230-3 Key Word Protocol 2000 (KWP2000) Server Engine Implementation._ 

* `#include "syn_kwp2000.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_init**](#function-syn_kwp2000_init) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server) <br>_Initialize a KWP2000 diagnostic server instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_process\_request**](#function-syn_kwp2000_process_request) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t max\_resp\_len, uint16\_t \* resp\_len) <br>_Process an incoming KWP2000 request message PDU and generate response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_register\_cid**](#function-syn_kwp2000_register_cid) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, uint16\_t cid, void \* data, uint16\_t len, [**bool**](syn__defs_8h.md#enum-bool) writable) <br>_Register a Common Identifier (CID, 2-byte) for Service 0x22/0x2E._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_register\_lid**](#function-syn_kwp2000_register_lid) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, uint8\_t lid, void \* data, uint16\_t len, [**bool**](syn__defs_8h.md#enum-bool) writable) <br>_Register a Local Identifier (LID, 1-byte) for Service 0x21/0x3B._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_set\_memory\_handler**](#function-syn_kwp2000_set_memory_handler) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, [**SYN\_KWP2000\_MemoryHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_memoryhandler) cb, void \* user\_ctx) <br>_Register memory access handler for 0x34 RequestDownload & 0x36 TransferData._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_set\_reset\_handler**](#function-syn_kwp2000_set_reset_handler) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, [**SYN\_KWP2000\_ResetHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_resethandler) cb, void \* user\_ctx) <br>_Set ECU reset callback handler and deferred wait delay._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_set\_routine\_handler**](#function-syn_kwp2000_set_routine_handler) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, [**SYN\_KWP2000\_RoutineHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_routinehandler) cb, void \* user\_ctx) <br>_Register RoutineControl handler._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_set\_security\_handler**](#function-syn_kwp2000_set_security_handler) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, [**SYN\_KWP2000\_SeedHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_seedhandler) seed\_cb, [**SYN\_KWP2000\_KeyHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_keyhandler) key\_cb, void \* user\_ctx) <br>_Set security access seed and key callbacks._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kwp2000\_set\_session\_handler**](#function-syn_kwp2000_set_session_handler) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, [**SYN\_KWP2000\_SessionHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_sessionhandler) cb, void \* user\_ctx) <br>_Set session transition callback handler._  |
|  void | [**syn\_kwp2000\_tick**](#function-syn_kwp2000_tick) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, uint32\_t dt\_ms) <br>_Tick KWP2000 server S3 timer and deferred reset state._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**kwp\_dispatch\_transfer\_or\_misc**](#function-kwp_dispatch_transfer_or_misc) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t max\_resp\_len, uint16\_t \* resp\_len) <br>_Dispatch routine, download, transfer, tester present, or unsupported KWP2000 services._  |
|  const [**SYN\_KWP2000\_CID**](structSYN__KWP2000__CID.md) \* | [**kwp\_find\_cid**](#function-kwp_find_cid) (const [**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, uint16\_t target\_cid) <br>_Look up a registered Common Identifier (CID) entry in the KWP2000 server._  |
|  const [**SYN\_KWP2000\_LID**](structSYN__KWP2000__LID.md) \* | [**kwp\_find\_lid**](#function-kwp_find_lid) (const [**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, uint8\_t target\_lid) <br>_Look up a registered Local Identifier (LID) entry in the KWP2000 server._  |
|  void | [**kwp\_handle\_download**](#function-kwp_handle_download) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle RequestDownload (0x34) request._  |
|  void | [**kwp\_handle\_read\_cid**](#function-kwp_handle_read_cid) (const [**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t max\_resp\_len, uint16\_t \* resp\_len) <br>_Handle ReadDataByCommonIdentifier (0x22) request._  |
|  void | [**kwp\_handle\_read\_lid**](#function-kwp_handle_read_lid) (const [**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t max\_resp\_len, uint16\_t \* resp\_len) <br>_Handle ReadDataByLocalIdentifier (0x21) request._  |
|  void | [**kwp\_handle\_reset**](#function-kwp_handle_reset) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle ECUReset (0x11) request._  |
|  void | [**kwp\_handle\_routine**](#function-kwp_handle_routine) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t max\_resp\_len, uint16\_t \* resp\_len) <br>_Handle StartRoutineByLocalIdentifier (0x31) request._  |
|  void | [**kwp\_handle\_security**](#function-kwp_handle_security) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t max\_resp\_len, uint16\_t \* resp\_len) <br>_Handle SecurityAccess (0x27) request and dispatch seed or key step._  |
|  void | [**kwp\_handle\_security\_key**](#function-kwp_handle_security_key) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle SecurityAccess sendKey (even sub-function) step._  |
|  void | [**kwp\_handle\_security\_seed**](#function-kwp_handle_security_seed) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, uint8\_t sid, uint8\_t sub\_func, uint8\_t \* resp\_buf, uint16\_t max\_resp\_len, uint16\_t \* resp\_len) <br>_Handle SecurityAccess requestSeed (odd sub-function) step._  |
|  void | [**kwp\_handle\_session**](#function-kwp_handle_session) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle StartDiagnosticSession (0x10) request._  |
|  void | [**kwp\_handle\_transfer**](#function-kwp_handle_transfer) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle TransferData (0x36) block write request._  |
|  void | [**kwp\_handle\_write\_cid**](#function-kwp_handle_write_cid) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle WriteDataByCommonIdentifier (0x2E) request._  |
|  void | [**kwp\_handle\_write\_lid**](#function-kwp_handle_write_lid) ([**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md) \* server, const uint8\_t \* req\_buf, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle WriteDataByLocalIdentifier (0x3B) request._  |
|  void | [**make\_nrc**](#function-make_nrc) (uint8\_t sid, uint8\_t nrc, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Helper to format a KWP2000 Negative Response Code (NRC)._  |


























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

_Register a Common Identifier (CID, 2-byte) for Service 0x22/0x2E._ 
```C++
SYN_Status syn_kwp2000_register_cid (
    SYN_KWP2000_Server * server,
    uint16_t cid,
    void * data,
    uint16_t len,
    bool writable
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `cid` 2-byte Common Identifier. 
* `data` Pointer to mutable data buffer. 
* `len` Data length in bytes. 
* `writable` Is CID writable? 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on error. 





        

<hr>



### function syn\_kwp2000\_register\_lid 

_Register a Local Identifier (LID, 1-byte) for Service 0x21/0x3B._ 
```C++
SYN_Status syn_kwp2000_register_lid (
    SYN_KWP2000_Server * server,
    uint8_t lid,
    void * data,
    uint16_t len,
    bool writable
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `lid` 1-byte Local Identifier. 
* `data` Pointer to mutable data buffer. 
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
## Public Static Functions Documentation




### function kwp\_dispatch\_transfer\_or\_misc 

_Dispatch routine, download, transfer, tester present, or unsupported KWP2000 services._ 
```C++
static void kwp_dispatch_transfer_or_misc (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t max_resp_len,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `max_resp_len` Capacity of resp\_buf in bytes. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_find\_cid 

_Look up a registered Common Identifier (CID) entry in the KWP2000 server._ 
```C++
static const SYN_KWP2000_CID * kwp_find_cid (
    const SYN_KWP2000_Server * server,
    uint16_t target_cid
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `target_cid` 16-bit Common Identifier to match. 



**Returns:**

Pointer to matching CID entry, or NULL if not registered. 





        

<hr>



### function kwp\_find\_lid 

_Look up a registered Local Identifier (LID) entry in the KWP2000 server._ 
```C++
static const SYN_KWP2000_LID * kwp_find_lid (
    const SYN_KWP2000_Server * server,
    uint8_t target_lid
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `target_lid` Local Identifier byte to match. 



**Returns:**

Pointer to matching LID entry, or NULL if not registered. 





        

<hr>



### function kwp\_handle\_download 

_Handle RequestDownload (0x34) request._ 
```C++
static void kwp_handle_download (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_read\_cid 

_Handle ReadDataByCommonIdentifier (0x22) request._ 
```C++
static void kwp_handle_read_cid (
    const SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t max_resp_len,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `max_resp_len` Capacity of resp\_buf in bytes. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_read\_lid 

_Handle ReadDataByLocalIdentifier (0x21) request._ 
```C++
static void kwp_handle_read_lid (
    const SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t max_resp_len,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `max_resp_len` Capacity of resp\_buf in bytes. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_reset 

_Handle ECUReset (0x11) request._ 
```C++
static void kwp_handle_reset (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_routine 

_Handle StartRoutineByLocalIdentifier (0x31) request._ 
```C++
static void kwp_handle_routine (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t max_resp_len,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `max_resp_len` Capacity of resp\_buf in bytes. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_security 

_Handle SecurityAccess (0x27) request and dispatch seed or key step._ 
```C++
static void kwp_handle_security (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t max_resp_len,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `max_resp_len` Capacity of resp\_buf in bytes. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_security\_key 

_Handle SecurityAccess sendKey (even sub-function) step._ 
```C++
static void kwp_handle_security_key (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_security\_seed 

_Handle SecurityAccess requestSeed (odd sub-function) step._ 
```C++
static void kwp_handle_security_seed (
    SYN_KWP2000_Server * server,
    uint8_t sid,
    uint8_t sub_func,
    uint8_t * resp_buf,
    uint16_t max_resp_len,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `sid` Request service identifier. 
* `sub_func` Odd SecurityAccess sub-function byte. 
* `resp_buf` Response PDU buffer. 
* `max_resp_len` Capacity of resp\_buf in bytes. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_session 

_Handle StartDiagnosticSession (0x10) request._ 
```C++
static void kwp_handle_session (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_transfer 

_Handle TransferData (0x36) block write request._ 
```C++
static void kwp_handle_transfer (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_write\_cid 

_Handle WriteDataByCommonIdentifier (0x2E) request._ 
```C++
static void kwp_handle_write_cid (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function kwp\_handle\_write\_lid 

_Handle WriteDataByLocalIdentifier (0x3B) request._ 
```C++
static void kwp_handle_write_lid (
    SYN_KWP2000_Server * server,
    const uint8_t * req_buf,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `server` KWP2000 server instance. 
* `req_buf` Request PDU buffer. 
* `req_len` Request PDU length in bytes. 
* `resp_buf` Response PDU buffer. 
* `resp_len` Output response PDU length in bytes. 




        

<hr>



### function make\_nrc 

_Helper to format a KWP2000 Negative Response Code (NRC)._ 
```C++
static void make_nrc (
    uint8_t sid,
    uint8_t nrc,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `sid` Service Identifier. 
* `nrc` Negative Response Code. 
* `resp_buf` Response buffer. 
* `resp_len` Pointer to store response length. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_kwp2000.c`

