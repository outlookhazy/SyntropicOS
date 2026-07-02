

# File syn\_websocket.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_websocket.c**](syn__websocket_8c.md)

[Go to the source code of this file](syn__websocket_8c_source.md)

_WebSocket protocol implementation._ 

* `#include "syn_websocket.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SHA1\_Ctx**](structSYN__SHA1__Ctx.md) <br>_SHA-1 hashing algorithm context._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_websocket\_send**](#function-syn_websocket_send) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, uint8\_t opcode, const void \* data, size\_t len) <br>_Send a frame over WebSocket._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_websocket\_task**](#function-syn_websocket_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Background task for polling active WebSockets._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_websocket\_upgrade**](#function-syn_websocket_upgrade) (const [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) \* req, [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, [**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, void(\*)(const uint8\_t \*payload, size\_t len, uint8\_t opcode, void \*ctx) on\_message, void \* ctx) <br>_Handle upgrading a connection in an httpd route handler to WebSocket._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**base64\_encode**](#function-base64_encode) (const uint8\_t \* src, size\_t len, char \* dst) <br>_Base64 encode a byte array._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**prefix\_icase**](#function-prefix_icase) (const char \* str, const char \* prefix) <br>_Case-insensitive prefix match._  |
|  void | [**sha1\_final**](#function-sha1_final) ([**SYN\_SHA1\_Ctx**](structSYN__SHA1__Ctx.md) \* ctx, uint8\_t digest) <br>_Finalize SHA-1 and write the 20-byte digest._  |
|  void | [**sha1\_init**](#function-sha1_init) ([**SYN\_SHA1\_Ctx**](structSYN__SHA1__Ctx.md) \* ctx) <br>_Initialize SHA-1 context._  |
|  void | [**sha1\_transform**](#function-sha1_transform) (uint32\_t state, const uint8\_t buffer) <br>_Process a 64-byte SHA-1 block._  |
|  void | [**sha1\_update**](#function-sha1_update) ([**SYN\_SHA1\_Ctx**](structSYN__SHA1__Ctx.md) \* ctx, const uint8\_t \* data, uint32\_t len) <br>_Feed data into the SHA-1 context._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SHA1\_ROL**](syn__websocket_8c.md#define-sha1_rol) (value, bits) `(((value) &lt;&lt; (bits)) \| ((value) &gt;&gt; (32 - (bits))))`<br>_SHA-1 rotate-left helper._  |

## Public Functions Documentation




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

PT\_WAITING or PT\_EXITED status. 





        

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




### function base64\_encode 

_Base64 encode a byte array._ 
```C++
static void base64_encode (
    const uint8_t * src,
    size_t len,
    char * dst
) 
```





**Parameters:**


* `src` Source bytes. 
* `len` Number of bytes. 
* `dst` [out] Null-terminated base64 string. 




        

<hr>



### function prefix\_icase 

_Case-insensitive prefix match._ 
```C++
static bool prefix_icase (
    const char * str,
    const char * prefix
) 
```





**Parameters:**


* `str` String to test. 
* `prefix` Prefix to match. 



**Returns:**

true if match. 





        

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

