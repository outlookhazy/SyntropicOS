

# File syn\_websocket.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_websocket.h**](syn__websocket_8h.md)

[Go to the source code of this file](syn__websocket_8h_source.md)

_WebSocket protocol support on httpd._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_task.h"`
* `#include "syn_httpd.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) <br>_WebSocket session context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_WebsocketState**](#enum-syn_websocketstate)  <br>_WebSocket connection states._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_websocket\_send**](#function-syn_websocket_send) ([**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, uint8\_t opcode, const void \* data, size\_t len) <br>_Send a frame over WebSocket._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_websocket\_task**](#function-syn_websocket_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Background task for polling active WebSockets._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_websocket\_upgrade**](#function-syn_websocket_upgrade) (const [**SYN\_HttpdRequest**](structSYN__HttpdRequest.md) \* req, [**SYN\_HttpdResponse**](structSYN__HttpdResponse.md) \* resp, [**SYN\_WebsocketSession**](structSYN__WebsocketSession.md) \* ws, void(\*)(const uint8\_t \*payload, size\_t len, uint8\_t opcode, void \*ctx) on\_message, void \* ctx) <br>_Handle upgrading a connection in an httpd route handler to WebSocket._  |




























## Public Types Documentation




### enum SYN\_WebsocketState 

_WebSocket connection states._ 
```C++
enum SYN_WebsocketState {
    SYN_WS_STATE_CLOSED,
    SYN_WS_STATE_CONNECTED
};
```




<hr>
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_websocket.h`

