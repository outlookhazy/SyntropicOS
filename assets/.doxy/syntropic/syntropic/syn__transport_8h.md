

# File syn\_transport.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_transport.h**](syn__transport_8h.md)

[Go to the source code of this file](syn__transport_8h_source.md)

_Transport abstraction — pluggable send/receive (header-only)._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Transport**](structSYN__Transport.md) <br>_Abstract transport interface (send/receive function pointers)._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_transport\_recv**](#function-syn_transport_recv) ([**SYN\_Transport**](structSYN__Transport.md) \* t, uint8\_t \* data, size\_t max\_len, size\_t \* out\_len) <br>_Receive a packet via transport (non-blocking)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_transport\_send**](#function-syn_transport_send) ([**SYN\_Transport**](structSYN__Transport.md) \* t, const uint8\_t \* data, size\_t len) <br>_Send a packet via transport._  |


























## Detailed Description


Defines a uniform interface for sending/receiving byte buffers over any physical layer: UART, CAN, SPI, USB, etc. Higher-level modules (router, heartbeat) consume transports without knowing the wire.


Usage: 
```C++
// Option A: UART transport via COBS framing
static SYN_Transport tr;
syn_transport_from_uart(&tr, &uart, &cobs_dec, cobs_buf, sizeof(cobs_buf));

// Option B: Custom transport
static SYN_Transport tr = {
    .send = my_send_fn,
    .recv = my_recv_fn,
    .ctx  = &my_device,
};
```
 


    
## Public Static Functions Documentation




### function syn\_transport\_recv 

_Receive a packet via transport (non-blocking)._ 
```C++
static inline bool syn_transport_recv (
    SYN_Transport * t,
    uint8_t * data,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `t` Transport instance. 
* `data` Buffer for received data. 
* `max_len` Buffer capacity. 
* `out_len` [out] Actual received length. 



**Returns:**

true if a complete packet was received. 





        

<hr>



### function syn\_transport\_send 

_Send a packet via transport._ 
```C++
static inline bool syn_transport_send (
    SYN_Transport * t,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `t` Transport instance. 
* `data` Packet data. 
* `len` Packet length. 



**Returns:**

true if sent successfully. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_transport.h`

