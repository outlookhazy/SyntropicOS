

# File syn\_router.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_router.h**](syn__router_8h.md)

[Go to the source code of this file](syn__router_8h_source.md)

_Packet router — addressed message dispatch._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "syn_transport.h"`
* `#include "../port/syn_port_system.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Packet**](structSYN__Packet.md) <br>_Router packet — wire format header + payload buffer._  |
| struct | [**SYN\_PendingAck**](structSYN__PendingAck.md) <br>_Pending ACK entry for reliable delivery tracking._  |
| struct | [**SYN\_Router**](structSYN__Router.md) <br>_Router instance — node ID, transport, handler table, ACK tracking._  |
| struct | [**SYN\_RouterHandler**](structSYN__RouterHandler.md) <br>_Handler registration entry — message type + callback._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_RouterHandlerFn**](#typedef-syn_routerhandlerfn)  <br>_Packet handler callback._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_router\_enable\_ack**](#function-syn_router_enable_ack) ([**SYN\_Router**](structSYN__Router.md) \* r, [**SYN\_PendingAck**](structSYN__PendingAck.md) \* pending, uint8\_t cap, uint16\_t timeout\_ms, uint8\_t max\_retries) <br>_Enable reliable delivery (ACK/retry)._  |
|  void | [**syn\_router\_init**](#function-syn_router_init) ([**SYN\_Router**](structSYN__Router.md) \* r, uint8\_t node\_id, [**SYN\_Transport**](structSYN__Transport.md) \* transport, [**SYN\_RouterHandler**](structSYN__RouterHandler.md) \* handlers, uint8\_t handler\_cap) <br>_Initialize packet router._  |
|  void | [**syn\_router\_poll**](#function-syn_router_poll) ([**SYN\_Router**](structSYN__Router.md) \* r) <br>_Poll for incoming packets and dispatch handlers._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_router\_register**](#function-syn_router_register) ([**SYN\_Router**](structSYN__Router.md) \* r, uint8\_t type, [**SYN\_RouterHandlerFn**](syn__router_8h.md#typedef-syn_routerhandlerfn) handler, void \* ctx) <br>_Register a handler for a message type._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_router\_send**](#function-syn_router_send) ([**SYN\_Router**](structSYN__Router.md) \* r, uint8\_t dst, uint8\_t type, const uint8\_t \* data, uint8\_t len, [**bool**](syn__defs_8h.md#enum-bool) reliable) <br>_Send a message._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_MSG\_ACK**](syn__router_8h.md#define-syn_msg_ack)  `0x01`<br> |
| define  | [**SYN\_MSG\_DISCOVER**](syn__router_8h.md#define-syn_msg_discover)  `0x02`<br> |
| define  | [**SYN\_MSG\_HEARTBEAT**](syn__router_8h.md#define-syn_msg_heartbeat)  `0x00`<br> |
| define  | [**SYN\_PKT\_FLAG\_ACK\_REQ**](syn__router_8h.md#define-syn_pkt_flag_ack_req)  `0x01`<br> |
| define  | [**SYN\_PKT\_FLAG\_IS\_ACK**](syn__router_8h.md#define-syn_pkt_flag_is_ack)  `0x02`<br> |
| define  | [**SYN\_ROUTER\_HEADER\_SIZE**](syn__router_8h.md#define-syn_router_header_size)  `6`<br> |
| define  | [**SYN\_ROUTER\_MAX\_PAYLOAD**](syn__router_8h.md#define-syn_router_max_payload)  `64`<br>_Maximum router payload bytes._  |
| define  | [**SYN\_ROUTER\_MAX\_PENDING**](syn__router_8h.md#define-syn_router_max_pending)  `4`<br>_Maximum pending ACK entries._  |

## Detailed Description


Routes packets between nodes over any [**SYN\_Transport**](structSYN__Transport.md). Each node has an ID (0-254, 0xFF = broadcast). Messages have a type + payload. Handlers are registered per message type.


Supports optional reliable delivery (ACK/retry) per-send.


Usage: 
```C++
static SYN_RouterHandler handlers[8];
static SYN_Router router;

syn_router_init(&router, 0x01, &transport,
                  handlers, SYN_ARRAY_SIZE(handlers));

syn_router_register(&router, MSG_SENSOR_DATA, on_sensor, NULL);

// Send
uint8_t payload[] = {0x42, 0x00};
syn_router_send(&router, 0x02, MSG_SENSOR_DATA,
                  payload, sizeof(payload), false);

// In main loop:
syn_router_poll(&router);
```
 


    
## Public Types Documentation




### typedef SYN\_RouterHandlerFn 

_Packet handler callback._ 
```C++
typedef void(* SYN_RouterHandlerFn) (const SYN_Packet *pkt, void *ctx);
```





**Parameters:**


* `pkt` Received packet. 
* `ctx` User context. 




        

<hr>
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
## Macro Definition Documentation





### define SYN\_MSG\_ACK 

```C++
#define SYN_MSG_ACK `0x01`
```



Acknowledgment 
 


        

<hr>



### define SYN\_MSG\_DISCOVER 

```C++
#define SYN_MSG_DISCOVER `0x02`
```



Node discovery 
 


        

<hr>



### define SYN\_MSG\_HEARTBEAT 

```C++
#define SYN_MSG_HEARTBEAT `0x00`
```



Keepalive ping 
 


        

<hr>



### define SYN\_PKT\_FLAG\_ACK\_REQ 

```C++
#define SYN_PKT_FLAG_ACK_REQ `0x01`
```



Sender wants an ACK 
 


        

<hr>



### define SYN\_PKT\_FLAG\_IS\_ACK 

```C++
#define SYN_PKT_FLAG_IS_ACK `0x02`
```



This packet is an ACK 
 


        

<hr>



### define SYN\_ROUTER\_HEADER\_SIZE 

```C++
#define SYN_ROUTER_HEADER_SIZE `6`
```



Wire format overhead: src(1) + dst(1) + type(1) + seq(1) + flags(1) + len(1) = 6 bytes 


        

<hr>



### define SYN\_ROUTER\_MAX\_PAYLOAD 

_Maximum router payload bytes._ 
```C++
#define SYN_ROUTER_MAX_PAYLOAD `64`
```




<hr>



### define SYN\_ROUTER\_MAX\_PENDING 

_Maximum pending ACK entries._ 
```C++
#define SYN_ROUTER_MAX_PENDING `4`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_router.h`

