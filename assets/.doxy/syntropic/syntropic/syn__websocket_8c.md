

# File syn\_websocket.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_websocket.c**](syn__websocket_8c.md)

[Go to the source code of this file](syn__websocket_8c_source.md)

_WebSocket protocol implementation._ 

* `#include "../sched/syn_sched.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_pack.h"`
* `#include "syn_websocket.h"`
* `#include <stdio.h>`
* `#include <string.h>`
* `#include "../util/syn_base64.h"`
* `#include "../util/syn_fmt.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SHA1\_Ctx**](structSYN__SHA1__Ctx.md) <br>_SHA-1 hashing algorithm context._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_websocket\_close**](#function-syn_websocket_close) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws) <br>_Close an active WebSocket session socket and reset parser state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_websocket\_send**](#function-syn_websocket_send) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, uint8\_t opcode, const void \* data, size\_t len) <br>_Send a frame over WebSocket._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_websocket\_task**](#function-syn_websocket_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Background task for polling active WebSockets._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_websocket\_upgrade**](#function-syn_websocket_upgrade) (const [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) \* req, [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, [**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, void(\*)(const uint8\_t \*payload, size\_t len, uint8\_t opcode, void \*ctx) on\_message, void \* ctx) <br>_Handle upgrading a connection in an httpd route handler to WebSocket._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**dispatch\_ws\_frame**](#function-dispatch_ws_frame) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws) <br>_Dispatch a completely received WebSocket frame to handlers._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**prefix\_icase**](#function-prefix_icase) (const char \* str, const char \* prefix) <br>_Case-insensitive string prefix check._  |
|  void | [**sha1\_final**](#function-sha1_final) ([**SYN\_SHA1\_Ctx**](structSYN__SHA1__Ctx.md) \* ctx, uint8\_t digest) <br>_Finalize SHA-1 and write the 20-byte digest._  |
|  void | [**sha1\_init**](#function-sha1_init) ([**SYN\_SHA1\_Ctx**](structSYN__SHA1__Ctx.md) \* ctx) <br>_Initialize SHA-1 context._  |
|  void | [**sha1\_transform**](#function-sha1_transform) (uint32\_t state, const uint8\_t buffer) <br>_Process a 64-byte SHA-1 block._  |
|  void | [**sha1\_update**](#function-sha1_update) ([**SYN\_SHA1\_Ctx**](structSYN__SHA1__Ctx.md) \* ctx, const uint8\_t \* data, uint32\_t len) <br>_Feed data into the SHA-1 context._  |
|  void | [**ws\_begin\_payload\_phase**](#function-ws_begin_payload_phase) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws) <br>_Transition after payload\_len is known into mask read, payload read, or immediate dispatch._  |
|  void | [**ws\_close\_session**](#function-ws_close_session) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws) <br>_Close a WebSocket session socket and reset frame parser state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ws\_copy\_trimmed\_key**](#function-ws_copy_trimmed_key) (const char \* key\_hdr, char \* key\_out, size\_t key\_sz) <br>_Copy Sec-WebSocket-Key value while trimming trailing SP/HTAB whitespace._  |
|  void | [**ws\_fail\_close**](#function-ws_fail_close) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, uint16\_t status\_code) <br>_Send an RFC 6455 Close status frame and terminate the WebSocket session._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ws\_feed\_byte**](#function-ws_feed_byte) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, uint8\_t b) <br>_Advance the WebSocket frame state machine by one byte._  |
|  void | [**ws\_feed\_len\_byte**](#function-ws_feed_len_byte) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, uint8\_t b) <br>_Parse second header byte (MASK flag + 7-bit base length indicator)._  |
|  void | [**ws\_feed\_mask\_byte**](#function-ws_feed_mask_byte) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, uint8\_t b) <br>_Accumulate 4-byte masking key and transition to payload or zero-length dispatch._  |
|  void | [**ws\_feed\_payload\_byte**](#function-ws_feed_payload_byte) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, uint8\_t b) <br>_Accumulate and unmask one payload byte and dispatch when frame completes._  |
|  const char \* | [**ws\_find\_key\_header**](#function-ws_find_key_header) (const char \* headers) <br>_Locate the trimmed value of Sec-WebSocket-Key within raw HTTP request headers._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ws\_has\_work**](#function-ws_has_work) (const [**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws) <br>_Check if WebSocket task has actionable work._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ws\_header0\_valid**](#function-ws_header0_valid) (uint8\_t b0) <br>_Validate first WebSocket frame byte (FIN=1, RSV1..3=0, known non-continuation opcode)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ws\_poll\_chunk**](#function-ws_poll_chunk) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws) <br>_Poll and feed buffered upgrade bytes or up to 64 bytes from an active WebSocket socket._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SHA1\_ROL**](syn__websocket_8c.md#define-sha1_rol) (value, bits) `(((value) &lt;&lt; (bits)) \| ((value) &gt;&gt; (32 - (bits))))`<br>_SHA-1 rotate-left helper._  |

## Public Functions Documentation




### function syn\_websocket\_close 

_Close an active WebSocket session socket and reset parser state._ 
```C++
void syn_websocket_close (
    SYN_WebsocketSession * ws
) 
```





**Parameters:**


* `ws` WebSocket session context (safe if NULL or already closed). 




        

<hr>



### function syn\_websocket\_send 

_Send a frame over WebSocket._ 
```C++
SYN_Status syn_websocket_send (
    SYN_WebsocketSession * ws,
    uint8_t opcode,
    const void * data,
    size_t len
) 
```



Formats a WebSocket packet header and writes the payload to the socket.




**Parameters:**


* `ws` WebSocket session context. 
* `opcode` Frame type opcode (e.g., text, binary, ping). 
* `data` Payload data to send. 
* `len` Length of payload in bytes. 



**Returns:**

SYN\_OK on success, or socket error code. 





        

<hr>



### function syn\_websocket\_task 

_Background task for polling active WebSockets._ 
```C++
SYN_PT_Status syn_websocket_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Cooperative protothread state tracker. 
* `task` Scheduler task control block. 



**Returns:**

PT\_IDLE while waiting, PT\_MORE when draining queued frames, or PT\_EXITED on invalid args. 





        

<hr>



### function syn\_websocket\_upgrade 

_Handle upgrading a connection in an httpd route handler to WebSocket._ 
```C++
SYN_Status syn_websocket_upgrade (
    const SYN_HttpdRequest * req,
    SYN_HttpdResponse * resp,
    SYN_WebsocketSession * ws,
    void(*)(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx) on_message,
    void * ctx
) 
```



Performs handshake, sends 101 status, sets upgraded = true on the response.




**Parameters:**


* `req` Incoming HTTP upgrade request. 
* `resp` HTTP response interface to write upgrade headers to. 
* `ws` WebSocket session context to initialize. 
* `on_message` Handled frame callback function. 
* `ctx` User context pointer passed through to callback. 



**Returns:**

SYN\_OK on successful handshake, SYN\_ERROR on negotiation failure. 





        

<hr>
## Public Static Functions Documentation




### function dispatch\_ws\_frame 

_Dispatch a completely received WebSocket frame to handlers._ 
```C++
static void dispatch_ws_frame (
    SYN_WebsocketSession * ws
) 
```





**Parameters:**


* `ws` Pointer to WebSocket session instance. 




        

<hr>



### function prefix\_icase 

_Case-insensitive string prefix check._ 
```C++
static inline bool prefix_icase (
    const char * str,
    const char * prefix
) 
```





**Parameters:**


* `str` Input string. 
* `prefix` Prefix to test. 



**Returns:**

True if str starts with prefix (case-insensitive). 





        

<hr>



### function sha1\_final 

_Finalize SHA-1 and write the 20-byte digest._ 
```C++
static void sha1_final (
    SYN_SHA1_Ctx * ctx,
    uint8_t digest
) 
```





**Parameters:**


* `ctx` SHA-1 context. 
* `digest` [out] 20-byte digest. 




        

<hr>



### function sha1\_init 

_Initialize SHA-1 context._ 
```C++
static void sha1_init (
    SYN_SHA1_Ctx * ctx
) 
```





**Parameters:**


* `ctx` SHA-1 context to initialize. 




        

<hr>



### function sha1\_transform 

_Process a 64-byte SHA-1 block._ 
```C++
static void sha1_transform (
    uint32_t state,
    const uint8_t buffer
) 
```





**Parameters:**


* `state` Running hash state. 
* `buffer` 64-byte input block. 




        

<hr>



### function sha1\_update 

_Feed data into the SHA-1 context._ 
```C++
static void sha1_update (
    SYN_SHA1_Ctx * ctx,
    const uint8_t * data,
    uint32_t len
) 
```





**Parameters:**


* `ctx` SHA-1 context. 
* `data` Input data. 
* `len` Input length. 




        

<hr>



### function ws\_begin\_payload\_phase 

_Transition after payload\_len is known into mask read, payload read, or immediate dispatch._ 
```C++
static void ws_begin_payload_phase (
    SYN_WebsocketSession * ws
) 
```





**Parameters:**


* `ws` WebSocket session context. 




        

<hr>



### function ws\_close\_session 

_Close a WebSocket session socket and reset frame parser state._ 
```C++
static void ws_close_session (
    SYN_WebsocketSession * ws
) 
```





**Parameters:**


* `ws` Pointer to WebSocket session instance. 




        

<hr>



### function ws\_copy\_trimmed\_key 

_Copy Sec-WebSocket-Key value while trimming trailing SP/HTAB whitespace._ 
```C++
static bool ws_copy_trimmed_key (
    const char * key_hdr,
    char * key_out,
    size_t key_sz
) 
```





**Parameters:**


* `key_hdr` Start of key value (after leading OWS). 
* `key_out` [out] Destination NUL-terminated buffer. 
* `key_sz` Capacity of key\_out in bytes. 



**Returns:**

true if non-empty trimmed key extracted, false if empty. 





        

<hr>



### function ws\_fail\_close 

_Send an RFC 6455 Close status frame and terminate the WebSocket session._ 
```C++
static void ws_fail_close (
    SYN_WebsocketSession * ws,
    uint16_t status_code
) 
```





**Parameters:**


* `ws` WebSocket session context. 
* `status_code` 16-bit RFC 6455 close status code (e.g. 1002, 1009). 




        

<hr>



### function ws\_feed\_byte 

_Advance the WebSocket frame state machine by one byte._ 
```C++
static bool ws_feed_byte (
    SYN_WebsocketSession * ws,
    uint8_t b
) 
```





**Parameters:**


* `ws` Pointer to WebSocket session instance. 
* `b` Received byte. 



**Returns:**

false if the session was closed due to protocol error or CLOSE frame. 





        

<hr>



### function ws\_feed\_len\_byte 

_Parse second header byte (MASK flag + 7-bit base length indicator)._ 
```C++
static void ws_feed_len_byte (
    SYN_WebsocketSession * ws,
    uint8_t b
) 
```





**Parameters:**


* `ws` WebSocket session context. 
* `b` Second frame byte. 




        

<hr>



### function ws\_feed\_mask\_byte 

_Accumulate 4-byte masking key and transition to payload or zero-length dispatch._ 
```C++
static void ws_feed_mask_byte (
    SYN_WebsocketSession * ws,
    uint8_t b
) 
```





**Parameters:**


* `ws` WebSocket session context. 
* `b` Masking key byte. 




        

<hr>



### function ws\_feed\_payload\_byte 

_Accumulate and unmask one payload byte and dispatch when frame completes._ 
```C++
static void ws_feed_payload_byte (
    SYN_WebsocketSession * ws,
    uint8_t b
) 
```





**Parameters:**


* `ws` WebSocket session context. 
* `b` Payload byte. 




        

<hr>



### function ws\_find\_key\_header 

_Locate the trimmed value of Sec-WebSocket-Key within raw HTTP request headers._ 
```C++
static const char * ws_find_key_header (
    const char * headers
) 
```





**Parameters:**


* `headers` Raw NUL-terminated HTTP headers block. 



**Returns:**

Pointer to key value start, or NULL if absent. 





        

<hr>



### function ws\_has\_work 

_Check if WebSocket task has actionable work._ 
```C++
static bool ws_has_work (
    const SYN_WebsocketSession * ws
) 
```





**Parameters:**


* `ws` Pointer to WebSocket session instance. 



**Returns:**

true if work pending, false otherwise. 





        

<hr>



### function ws\_header0\_valid 

_Validate first WebSocket frame byte (FIN=1, RSV1..3=0, known non-continuation opcode)._ 
```C++
static bool ws_header0_valid (
    uint8_t b0
) 
```





**Parameters:**


* `b0` First header byte. 



**Returns:**

true if header byte is valid, false on protocol error. 





        

<hr>



### function ws\_poll\_chunk 

_Poll and feed buffered upgrade bytes or up to 64 bytes from an active WebSocket socket._ 
```C++
static bool ws_poll_chunk (
    SYN_WebsocketSession * ws
) 
```





**Parameters:**


* `ws` WebSocket session context. 



**Returns:**

true if at least one byte was processed while session remains connected. 





        

<hr>
## Macro Definition Documentation





### define SHA1\_ROL 

_SHA-1 rotate-left helper._ 
```C++
#define SHA1_ROL (
    value,
    bits
) `(((value) << (bits)) | ((value) >> (32 - (bits))))`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_websocket.c`

