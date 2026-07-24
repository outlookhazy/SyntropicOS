

# File syn\_coap.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_coap.c**](syn__coap_8c.md)

[Go to the source code of this file](syn__coap_8c_source.md)

_CoAP message serialization, parsing, and cooperative client task implementation._ 

* `#include "syn_coap.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_coap\_parse**](#function-syn_coap_parse) ([**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, [**SYN\_CoapOption**](structSYN__CoapOption.md) \* options, size\_t max\_options, size\_t \* option\_count, const uint8\_t \* buf, size\_t buf\_len) <br>_Parse a raw CoAP packet into a message structure._  |
|  void | [**syn\_coap\_request\_init**](#function-syn_coap_request_init) ([**SYN\_CoapRequest**](structSYN__CoapRequest.md) \* r, const [**SYN\_SockAddr**](structSYN__SockAddr.md) \* server\_addr, const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, uint32\_t timeout\_ms, uint8\_t retries) <br>_Initialize a CoAP request context._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_coap\_request\_task**](#function-syn_coap_request_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread task to execute a CoAP client request._  |
|  size\_t | [**syn\_coap\_serialize**](#function-syn_coap_serialize) (const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, const [**SYN\_CoapOption**](structSYN__CoapOption.md) \* options, size\_t option\_count, uint8\_t \* buf, size\_t max\_buf\_len) <br>_Serialize a CoAP message into a byte buffer._  |




























## Public Functions Documentation




### function syn\_coap\_parse 

_Parse a raw CoAP packet into a message structure._ 
```C++
SYN_Status syn_coap_parse (
    SYN_CoapMsg * msg,
    SYN_CoapOption * options,
    size_t max_options,
    size_t * option_count,
    const uint8_t * buf,
    size_t buf_len
) 
```





**Parameters:**


* `msg` [out] Parsed message header. 
* `options` [out] Array to receive parsed options. 
* `max_options` Capacity of the options array. 
* `option_count` [out] Number of options actually parsed. 
* `buf` Raw packet buffer. 
* `buf_len` Length of the raw packet in bytes. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on malformed input. 





        

<hr>



### function syn\_coap\_request\_init 

_Initialize a CoAP request context._ 
```C++
void syn_coap_request_init (
    SYN_CoapRequest * r,
    const SYN_SockAddr * server_addr,
    const SYN_CoapMsg * msg,
    uint32_t timeout_ms,
    uint8_t retries
) 
```





**Parameters:**


* `r` Request context. 
* `server_addr` Target server address. 
* `msg` Request header/token/payload. 
* `timeout_ms` Initial retransmit timeout (ms). 
* `retries` Maximum number of retransmissions. 




        

<hr>



### function syn\_coap\_request\_task 

_Cooperative protothread task to execute a CoAP client request._ 
```C++
SYN_PT_Status syn_coap_request_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread state. 
* `task` Scheduler task (user\_data must point to a [**SYN\_CoapRequest**](structSYN__CoapRequest.md)). 



**Returns:**

Protothread status. 





        

<hr>



### function syn\_coap\_serialize 

_Serialize a CoAP message into a byte buffer._ 
```C++
size_t syn_coap_serialize (
    const SYN_CoapMsg * msg,
    const SYN_CoapOption * options,
    size_t option_count,
    uint8_t * buf,
    size_t max_buf_len
) 
```





**Parameters:**


* `msg` Message header to serialize. 
* `options` Options array (will be sorted by option number). 
* `option_count` Number of options. 
* `buf` [out] Destination buffer. 
* `max_buf_len` Capacity of the destination buffer. 



**Returns:**

Number of bytes written, or 0 on error (buffer too small). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_coap.c`

