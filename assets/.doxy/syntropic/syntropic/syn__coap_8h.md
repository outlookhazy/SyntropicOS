

# File syn\_coap.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_coap.h**](syn__coap_8h.md)

[Go to the source code of this file](syn__coap_8h_source.md)

_Constrained Application Protocol (CoAP - RFC 7252) implementation._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_task.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CoapMsg**](structSYN__CoapMsg.md) <br>_Parsed or to-be-serialized CoAP message header._  |
| struct | [**SYN\_CoapOption**](structSYN__CoapOption.md) <br>_Single CoAP option (number + opaque value)._  |
| struct | [**SYN\_CoapRequest**](structSYN__CoapRequest.md) <br>_CoAP client request context._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_coap\_parse**](#function-syn_coap_parse) ([**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, [**SYN\_CoapOption**](structSYN__CoapOption.md) \* options, size\_t max\_options, size\_t \* option\_count, const uint8\_t \* buf, size\_t buf\_len) <br>_Parse a raw CoAP packet into a message structure._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_coap\_request\_task**](#function-syn_coap_request_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread task to execute a CoAP client request._  |
|  size\_t | [**syn\_coap\_serialize**](#function-syn_coap_serialize) (const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, const [**SYN\_CoapOption**](structSYN__CoapOption.md) \* options, size\_t option\_count, uint8\_t \* buf, size\_t max\_buf\_len) <br>_Serialize a CoAP message into a byte buffer._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**COAP\_VERSION**](syn__coap_8h.md#define-coap_version)  `1`<br> |

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
## Macro Definition Documentation





### define COAP\_VERSION 

```C++
#define COAP_VERSION `1`
```



CoAP protocol version 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_coap.h`

