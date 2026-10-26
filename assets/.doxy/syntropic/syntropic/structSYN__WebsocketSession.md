

# Struct SYN\_WebsocketSession



[**ClassList**](annotated.md) **>** [**SYN\_WebsocketSession**](structSYN__WebsocketSession.md)



_WebSocket session context._ 

* `#include <syn_websocket.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**bytes\_read**](#variable-bytes_read)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  uint8\_t | [**mask\_key**](#variable-mask_key)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**masked**](#variable-masked)  <br> |
|  void(\* | [**on\_message**](#variable-on_message)  <br>_User callback invoked when a complete frame is received._  |
|  uint8\_t | [**opcode**](#variable-opcode)  <br> |
|  uint32\_t | [**payload\_len**](#variable-payload_len)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  uint8\_t | [**rx\_state**](#variable-rx_state)  <br> |
|  [**SYN\_Socket**](syn__port__socket_8h.md#typedef-syn_socket) | [**sock**](#variable-sock)  <br> |
|  [**SYN\_WebsocketState**](syn__websocket_8h.md#enum-syn_websocketstate) | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable bytes\_read 

```C++
uint32_t SYN_WebsocketSession::bytes_read;
```



Accumulated payload bytes read so far 


        

<hr>



### variable ctx 

```C++
void* SYN_WebsocketSession::ctx;
```



User context pointer for message callback 


        

<hr>



### variable mask\_key 

```C++
uint8_t SYN_WebsocketSession::mask_key[4];
```



Client-to-server frame masking key 


        

<hr>



### variable masked 

```C++
bool SYN_WebsocketSession::masked;
```



True if the incoming frame is masked 


        

<hr>



### variable on\_message 

_User callback invoked when a complete frame is received._ 
```C++
void(* SYN_WebsocketSession::on_message) (const uint8_t *payload, size_t len, uint8_t opcode, void *ctx);
```





**Parameters:**


* `payload` Received frame data. 
* `len` Frame length in bytes. 
* `opcode` Frame type opcode. 
* `ctx` User context pointer. 




        

<hr>



### variable opcode 

```C++
uint8_t SYN_WebsocketSession::opcode;
```



WebSocket opcode (e.g. text, binary, close, ping) 


        

<hr>



### variable payload\_len 

```C++
uint32_t SYN_WebsocketSession::payload_len;
```



Size of the current incoming frame payload 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_WebsocketSession::rx_buf[128];
```



Payload receive packet storage 


        

<hr>



### variable rx\_state 

```C++
uint8_t SYN_WebsocketSession::rx_state;
```



Frame parsing state: 0=header, 1=length, 2=mask, 3=payload 


        

<hr>



### variable sock 

```C++
SYN_Socket SYN_WebsocketSession::sock;
```



Network socket handle 


        

<hr>



### variable state 

```C++
SYN_WebsocketState SYN_WebsocketSession::state;
```



WebSocket connection state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_websocket.h`

