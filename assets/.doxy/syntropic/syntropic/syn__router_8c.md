

# File syn\_router.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_router.c**](syn__router_8c.md)

[Go to the source code of this file](syn__router_8c_source.md)

_Packet router implementation._ 

* `#include "syn_router.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_router\_enable\_ack**](#function-syn_router_enable_ack) ([**SYN\_Router**](structSYN__Router.md) \* r, [**SYN\_PendingAck**](structSYN__PendingAck.md) \* pending, uint8\_t cap, uint16\_t timeout\_ms, uint8\_t max\_retries) <br>_Enable reliable delivery (ACK/retry)._  |
|  void | [**syn\_router\_init**](#function-syn_router_init) ([**SYN\_Router**](structSYN__Router.md) \* r, uint8\_t node\_id, [**SYN\_Transport**](structSYN__Transport.md) \* transport, [**SYN\_RouterHandler**](structSYN__RouterHandler.md) \* handlers, uint8\_t handler\_cap) <br>_Initialize packet router._  |
|  void | [**syn\_router\_poll**](#function-syn_router_poll) ([**SYN\_Router**](structSYN__Router.md) \* r) <br>_Poll for incoming packets and dispatch handlers._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_router\_register**](#function-syn_router_register) ([**SYN\_Router**](structSYN__Router.md) \* r, uint8\_t type, [**SYN\_RouterHandlerFn**](syn__router_8h.md#typedef-syn_routerhandlerfn) handler, void \* ctx) <br>_Register a handler for a message type._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_router\_send**](#function-syn_router_send) ([**SYN\_Router**](structSYN__Router.md) \* r, uint8\_t dst, uint8\_t type, const uint8\_t \* data, uint8\_t len, [**bool**](syn__defs_8h.md#enum-bool) reliable) <br>_Send a message._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**check\_retries**](#function-check_retries) ([**SYN\_Router**](structSYN__Router.md) \* r) <br>_Retry pending packets that have timed out._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**deserialize\_packet**](#function-deserialize_packet) (const uint8\_t \* buf, size\_t len, [**SYN\_Packet**](structSYN__Packet.md) \* pkt) <br>_Deserialize a wire-format buffer into a packet._  |
|  void | [**handle\_ack**](#function-handle_ack) ([**SYN\_Router**](structSYN__Router.md) \* r, uint8\_t src, uint8\_t seq) <br>_Handle an incoming ACK by clearing the pending entry._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**queue\_pending**](#function-queue_pending) ([**SYN\_Router**](structSYN__Router.md) \* r, uint8\_t dst, uint8\_t type, uint8\_t seq, const uint8\_t \* data, uint8\_t len) <br>_Queue a pending ACK entry for reliable delivery._  |
|  void | [**send\_ack**](#function-send_ack) ([**SYN\_Router**](structSYN__Router.md) \* r, uint8\_t dst, uint8\_t seq) <br>_Send an ACK packet to the source._  |
|  size\_t | [**serialize\_packet**](#function-serialize_packet) (const [**SYN\_Packet**](structSYN__Packet.md) \* pkt, uint8\_t \* buf) <br>_Serialize a packet into a wire-format buffer._  |


























## Public Functions Documentation




### function syn\_router\_enable\_ack 

_Enable reliable delivery (ACK/retry)._ 
```C++
void syn_router_enable_ack (
    SYN_Router * r,
    SYN_PendingAck * pending,
    uint8_t cap,
    uint16_t timeout_ms,
    uint8_t max_retries
) 
```





**Parameters:**


* `r` Router. 
* `pending` Caller-owned pending ACK array. 
* `cap` Array capacity. 
* `timeout_ms` ACK timeout per attempt. 
* `max_retries` Max retransmissions before giving up. 




        

<hr>



### function syn\_router\_init 

_Initialize packet router._ 
```C++
void syn_router_init (
    SYN_Router * r,
    uint8_t node_id,
    SYN_Transport * transport,
    SYN_RouterHandler * handlers,
    uint8_t handler_cap
) 
```





**Parameters:**


* `r` Router instance. 
* `node_id` This node's ID (0-254). 
* `transport` Transport to send/receive over. 
* `handlers` Caller-owned handler array. 
* `handler_cap` Array capacity. 




        

<hr>



### function syn\_router\_poll 

_Poll for incoming packets and dispatch handlers._ 
```C++
void syn_router_poll (
    SYN_Router * r
) 
```



Also checks for ACK timeouts and retransmits if needed. Call from your main loop.




**Parameters:**


* `r` Router. 




        

<hr>



### function syn\_router\_register 

_Register a handler for a message type._ 
```C++
bool syn_router_register (
    SYN_Router * r,
    uint8_t type,
    SYN_RouterHandlerFn handler,
    void * ctx
) 
```





**Parameters:**


* `r` Router. 
* `type` Message type to handle. 
* `handler` Handler function. 
* `ctx` User context. 



**Returns:**

true if registered, false if handler table is full. 





        

<hr>



### function syn\_router\_send 

_Send a message._ 
```C++
bool syn_router_send (
    SYN_Router * r,
    uint8_t dst,
    uint8_t type,
    const uint8_t * data,
    uint8_t len,
    bool reliable
) 
```





**Parameters:**


* `r` Router. 
* `dst` Destination node ID (0xFF = broadcast). 
* `type` Message type. 
* `data` Payload. 
* `len` Payload length. 
* `reliable` If true and ACK is enabled, wait for ACK with retry. 



**Returns:**

true if sent (or queued for retry). 





        

<hr>
## Public Static Functions Documentation




### function check\_retries 

_Retry pending packets that have timed out._ 
```C++
static void check_retries (
    SYN_Router * r
) 
```





**Parameters:**


* `r` Router instance. 




        

<hr>



### function deserialize\_packet 

_Deserialize a wire-format buffer into a packet._ 
```C++
static bool deserialize_packet (
    const uint8_t * buf,
    size_t len,
    SYN_Packet * pkt
) 
```





**Parameters:**


* `buf` Input buffer. 
* `len` Buffer length. 
* `pkt` [out] Parsed packet. 



**Returns:**

true on success. 





        

<hr>



### function handle\_ack 

_Handle an incoming ACK by clearing the pending entry._ 
```C++
static void handle_ack (
    SYN_Router * r,
    uint8_t src,
    uint8_t seq
) 
```





**Parameters:**


* `r` Router instance. 
* `src` Source node ID. 
* `seq` Sequence number. 




        

<hr>



### function queue\_pending 

_Queue a pending ACK entry for reliable delivery._ 
```C++
static bool queue_pending (
    SYN_Router * r,
    uint8_t dst,
    uint8_t type,
    uint8_t seq,
    const uint8_t * data,
    uint8_t len
) 
```





**Parameters:**


* `r` Router instance. 
* `dst` Destination node ID. 
* `type` Message type. 
* `seq` Sequence number. 
* `data` Payload data. 
* `len` Payload length. 



**Returns:**

true if queued successfully. 





        

<hr>



### function send\_ack 

_Send an ACK packet to the source._ 
```C++
static void send_ack (
    SYN_Router * r,
    uint8_t dst,
    uint8_t seq
) 
```





**Parameters:**


* `r` Router instance. 
* `dst` Destination node to acknowledge. 
* `seq` Sequence number being acknowledged. 




        

<hr>



### function serialize\_packet 

_Serialize a packet into a wire-format buffer._ 
```C++
static size_t serialize_packet (
    const SYN_Packet * pkt,
    uint8_t * buf
) 
```





**Parameters:**


* `pkt` Packet to serialize. 
* `buf` [out] Output buffer (header + payload). 



**Returns:**

Total bytes written. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_router.c`

