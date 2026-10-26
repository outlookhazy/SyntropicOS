

# File syn\_coap.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_coap.c**](syn__coap_8c.md)

[Go to the source code of this file](syn__coap_8c_source.md)

_CoAP message serialization, parsing, and cooperative client task implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_pack.h"`
* `#include "syn_coap.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_coap\_decode\_block\_opt**](#function-syn_coap_decode_block_opt) (const uint8\_t \* opt\_val, size\_t opt\_len, [**SYN\_CoapBlock**](structSYN__CoapBlock.md) \* block) <br>_Decode a CoAP Blockwise Option (Block1 / Block2) value (RFC 7959)._  |
|  size\_t | [**syn\_coap\_encode\_block\_opt**](#function-syn_coap_encode_block_opt) (const [**SYN\_CoapBlock**](structSYN__CoapBlock.md) \* block, uint8\_t buf) <br>_Encode a CoAP Blockwise Option (Block1 / Block2) value (RFC 7959)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_coap\_parse**](#function-syn_coap_parse) ([**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, [**SYN\_CoapOption**](structSYN__CoapOption.md) \* options, size\_t max\_options, size\_t \* option\_count, const uint8\_t \* buf, size\_t buf\_len) <br>_Parse a raw CoAP packet into a message structure._  |
|  void | [**syn\_coap\_request\_init**](#function-syn_coap_request_init) ([**SYN\_CoapRequest**](structSYN__CoapRequest.md) \* r, const [**SYN\_SockAddr**](structSYN__SockAddr.md) \* server\_addr, const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, uint32\_t timeout\_ms, uint8\_t retries) <br>_Initialize a CoAP request context._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_coap\_request\_task**](#function-syn_coap_request_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread task to execute a CoAP client request._  |
|  size\_t | [**syn\_coap\_serialize**](#function-syn_coap_serialize) (const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, const [**SYN\_CoapOption**](structSYN__CoapOption.md) \* options, size\_t option\_count, uint8\_t \* buf, size\_t max\_buf\_len) <br>_Serialize a CoAP message into a byte buffer._  |
|  void | [**syn\_coap\_transport\_request\_init**](#function-syn_coap_transport_request_init) ([**SYN\_CoapTransportRequest**](structSYN__CoapTransportRequest.md) \* r, [**SYN\_Transport**](structSYN__Transport.md) \* transport, const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, uint32\_t timeout\_ms, uint8\_t retries) <br>_Initialize a CoAP request context over a_ [_**SYN\_Transport**_](structSYN__Transport.md) _._ |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_coap\_transport\_request\_task**](#function-syn_coap_transport_request_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread task for executing a CoAP request over_ [_**SYN\_Transport**_](structSYN__Transport.md) _._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_coap\_transport\_send\_request**](#function-syn_coap_transport_send_request) ([**SYN\_Transport**](structSYN__Transport.md) \* transport, const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* req, const [**SYN\_CoapOption**](structSYN__CoapOption.md) \* req\_opts, size\_t req\_opt\_cnt, [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* resp, [**SYN\_CoapOption**](structSYN__CoapOption.md) \* resp\_opts, size\_t max\_resp\_opts, size\_t \* resp\_opt\_cnt, uint8\_t \* resp\_buf, size\_t resp\_buf\_sz) <br>_Send a CoAP request and await matching response over_ [_**SYN\_Transport**_](structSYN__Transport.md) _._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_coaps\_client\_handshake**](#function-syn_coaps_client_handshake) ([**SYN\_CoapsClient**](structSYN__CoapsClient.md) \* client) <br>_Advance or complete DTLS 1.3 handshake for CoAPS client._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_coaps\_client\_init**](#function-syn_coaps_client_init) ([**SYN\_CoapsClient**](structSYN__CoapsClient.md) \* client, const [**SYN\_DTLS\_Config**](structSYN__DTLS__Config.md) \* config, [**SYN\_Transport**](structSYN__Transport.md) \* underlying\_transport, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize a CoAP over DTLS 1.3 Client._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_coaps\_client\_send\_request**](#function-syn_coaps_client_send_request) ([**SYN\_CoapsClient**](structSYN__CoapsClient.md) \* client, const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* req, const [**SYN\_CoapOption**](structSYN__CoapOption.md) \* req\_opts, size\_t req\_opt\_cnt, [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* resp, [**SYN\_CoapOption**](structSYN__CoapOption.md) \* resp\_opts, size\_t max\_resp\_opts, size\_t \* resp\_opt\_cnt, uint8\_t \* resp\_buf, size\_t resp\_buf\_sz) <br>_Send a CoAPS request and receive response via DTLS 1.3 datagram channel._  |




























## Public Functions Documentation




### function syn\_coap\_decode\_block\_opt 

_Decode a CoAP Blockwise Option (Block1 / Block2) value (RFC 7959)._ 
```C++
bool syn_coap_decode_block_opt (
    const uint8_t * opt_val,
    size_t opt_len,
    SYN_CoapBlock * block
) 
```





**Parameters:**


* `opt_val` Pointer to option value bytes. 
* `opt_len` Option length in bytes (1, 2, or 3). 
* `block` [out] Parsed block parameters. 



**Returns:**

true on success, false on invalid length or format. 





        

<hr>



### function syn\_coap\_encode\_block\_opt 

_Encode a CoAP Blockwise Option (Block1 / Block2) value (RFC 7959)._ 
```C++
size_t syn_coap_encode_block_opt (
    const SYN_CoapBlock * block,
    uint8_t buf
) 
```





**Parameters:**


* `block` Block parameters. 
* `buf` [out] Output buffer for encoded uint (up to 3 bytes). 



**Returns:**

Number of bytes encoded (1, 2, or 3 bytes). 





        

<hr>



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



### function syn\_coap\_transport\_request\_init 

_Initialize a CoAP request context over a_ [_**SYN\_Transport**_](structSYN__Transport.md) _._
```C++
void syn_coap_transport_request_init (
    SYN_CoapTransportRequest * r,
    SYN_Transport * transport,
    const SYN_CoapMsg * msg,
    uint32_t timeout_ms,
    uint8_t retries
) 
```





**Parameters:**


* `r` Request context. 
* `transport` Transport instance. 
* `msg` Request message header. 
* `timeout_ms` Initial retransmit timeout (ms). 
* `retries` Maximum number of retransmissions. 




        

<hr>



### function syn\_coap\_transport\_request\_task 

_Cooperative protothread task for executing a CoAP request over_ [_**SYN\_Transport**_](structSYN__Transport.md) _._
```C++
SYN_PT_Status syn_coap_transport_request_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread context. 
* `task` Scheduler task (user\_data pointing to [**SYN\_CoapTransportRequest**](structSYN__CoapTransportRequest.md)). 



**Returns:**

Protothread status. 





        

<hr>



### function syn\_coap\_transport\_send\_request 

_Send a CoAP request and await matching response over_ [_**SYN\_Transport**_](structSYN__Transport.md) _._
```C++
SYN_Status syn_coap_transport_send_request (
    SYN_Transport * transport,
    const SYN_CoapMsg * req,
    const SYN_CoapOption * req_opts,
    size_t req_opt_cnt,
    SYN_CoapMsg * resp,
    SYN_CoapOption * resp_opts,
    size_t max_resp_opts,
    size_t * resp_opt_cnt,
    uint8_t * resp_buf,
    size_t resp_buf_sz
) 
```





**Parameters:**


* `transport` Transport instance. 
* `req` Request message. 
* `req_opts` Request options. 
* `req_opt_cnt` Request option count. 
* `resp` [out] Parsed response message. 
* `resp_opts` [out] Parsed response options array. 
* `max_resp_opts` Capacity of response options array. 
* `resp_opt_cnt` [out] Number of parsed response options. 
* `resp_buf` [out] Buffer for raw received packet. 
* `resp_buf_sz` Capacity of response buffer. 



**Returns:**

SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_coaps\_client\_handshake 

_Advance or complete DTLS 1.3 handshake for CoAPS client._ 
```C++
bool syn_coaps_client_handshake (
    SYN_CoapsClient * client
) 
```





**Parameters:**


* `client` CoAPS client context. 



**Returns:**

true on handshake success, false on error. 





        

<hr>



### function syn\_coaps\_client\_init 

_Initialize a CoAP over DTLS 1.3 Client._ 
```C++
bool syn_coaps_client_init (
    SYN_CoapsClient * client,
    const SYN_DTLS_Config * config,
    SYN_Transport * underlying_transport,
    uint8_t * rx_buf,
    size_t rx_buf_size,
    uint8_t * tx_buf,
    size_t tx_buf_size
) 
```





**Parameters:**


* `client` CoAPS client context. 
* `config` DTLS 1.3 configuration. 
* `underlying_transport` Underlying datagram transport (e.g. UDP socket). 
* `rx_buf` Caller-owned RX record buffer. 
* `rx_buf_size` Capacity of RX record buffer. 
* `tx_buf` Caller-owned TX record buffer. 
* `tx_buf_size` Capacity of TX record buffer. 



**Returns:**

true on success, false on invalid parameters. 





        

<hr>



### function syn\_coaps\_client\_send\_request 

_Send a CoAPS request and receive response via DTLS 1.3 datagram channel._ 
```C++
SYN_Status syn_coaps_client_send_request (
    SYN_CoapsClient * client,
    const SYN_CoapMsg * req,
    const SYN_CoapOption * req_opts,
    size_t req_opt_cnt,
    SYN_CoapMsg * resp,
    SYN_CoapOption * resp_opts,
    size_t max_resp_opts,
    size_t * resp_opt_cnt,
    uint8_t * resp_buf,
    size_t resp_buf_sz
) 
```





**Parameters:**


* `client` CoAPS client context. 
* `req` Request message. 
* `req_opts` Request options. 
* `req_opt_cnt` Request option count. 
* `resp` [out] Parsed response message. 
* `resp_opts` [out] Parsed response options array. 
* `max_resp_opts` Capacity of response options array. 
* `resp_opt_cnt` [out] Number of parsed response options. 
* `resp_buf` [out] Buffer for decrypted response payload. 
* `resp_buf_sz` Capacity of response buffer. 



**Returns:**

SYN\_OK on success, error code otherwise. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_coap.c`

