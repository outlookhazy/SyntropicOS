

# File syn\_coap.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_coap.c**](syn__coap_8c.md)

[Go to the source code of this file](syn__coap_8c_source.md)

_CoAP message serialization, parsing, and cooperative client task implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "../sched/syn_sched.h"`
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


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**coap\_close\_sock**](#function-coap_close_sock) ([**SYN\_CoapRequest**](structSYN__CoapRequest.md) \* r) <br>_Close and invalidate a CoAP UDP request socket if currently open._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_decode\_nibble**](#function-coap_decode_nibble) (uint8\_t nibble, const uint8\_t \* buf, size\_t buf\_len, size\_t \* pos, size\_t \* out) <br>_Decode a 4-bit CoAP option delta or length nibble and any extended bytes._  |
|  void | [**coap\_encode\_nibble**](#function-coap_encode_nibble) (size\_t val, uint8\_t \* nibble, uint8\_t \* ext\_len, uint16\_t \* ext\_val) <br>_Encode a CoAP option delta or length into base nibble and extended value._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_hdr\_valid**](#function-coap_hdr_valid) (const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, uint8\_t ver, size\_t buf\_len) <br>_Validate CoAP header version, token length, empty rules, and reserved code classes._  |
|  void | [**coap\_init\_backoff**](#function-coap_init_backoff) ([**SYN\_Backoff**](structSYN__Backoff.md) \* b, const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, uint32\_t timeout\_ms, uint8\_t retries) <br>_Initialize exponential retransmit backoff (forcing 0 retries for NON requests)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_parse\_opt\_hdr**](#function-coap_parse_opt_hdr) (const uint8\_t \* buf, size\_t buf\_len, size\_t \* pos, uint16\_t \* prev\_num, size\_t \* len\_out) <br>_Parse one CoAP option header and validate 16-bit option-number delta and buffer bounds._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_pt\_args\_valid**](#function-coap_pt_args_valid) (const [**SYN\_PT**](structSYN__PT.md) \* pt, const [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Validate protothread and task user\_data pointers for CoAP request tasks._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_response\_is\_rst**](#function-coap_response_is_rst) (const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* req, const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* resp) <br>_Check whether a parsed CoAP message is an RFC 7252 Empty Reset matching the request MID._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_response\_matches**](#function-coap_response_matches) (const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* req, const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* resp) <br>_Check whether a parsed CoAP response token matches the active request token._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_serialize\_args\_ok**](#function-coap_serialize_args_ok) (const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* msg, const [**SYN\_CoapOption**](structSYN__CoapOption.md) \* options, size\_t option\_count, const uint8\_t \* buf, size\_t max\_buf\_len) <br>_Validate inputs to syn\_coap\_serialize before writing packet bytes._  |
|  void | [**coap\_sort\_options**](#function-coap_sort_options) ([**SYN\_CoapOption**](structSYN__CoapOption.md) \* sorted, const [**SYN\_CoapOption**](structSYN__CoapOption.md) \* options, size\_t count) <br>_Copy and sort up to 16 CoAP options by ascending option number._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_tr\_poll\_rx**](#function-coap_tr_poll_rx) ([**SYN\_CoapTransportRequest**](structSYN__CoapTransportRequest.md) \* r, [**bool**](syn__defs_8h.md#enum-bool) \* done\_out) <br>_Poll transport once and update request status on matching response or Reset._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_tr\_wait\_done**](#function-coap_tr_wait_done) (const [**SYN\_CoapTransportRequest**](structSYN__CoapTransportRequest.md) \* r) <br>_Check whether the generic-transport CoAP receive wait condition is satisfied._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_udp\_consume\_rx**](#function-coap_udp_consume_rx) ([**SYN\_CoapRequest**](structSYN__CoapRequest.md) \* r, size\_t n) <br>_Parse a received UDP CoAP datagram and update request status on response or Reset._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**coap\_udp\_wait\_done**](#function-coap_udp_wait_done) (const [**SYN\_CoapRequest**](structSYN__CoapRequest.md) \* r) <br>_Check whether the UDP CoAP receive wait condition is satisfied._  |
|  void | [**coap\_write\_ext**](#function-coap_write_ext) (uint8\_t \* buf, size\_t \* pos, uint8\_t ext\_len, uint16\_t ext\_val) <br>_Append 0, 1, or 2 extended option header bytes._  |


























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
## Public Static Functions Documentation




### function coap\_close\_sock 

_Close and invalidate a CoAP UDP request socket if currently open._ 
```C++
static void coap_close_sock (
    SYN_CoapRequest * r
) 
```





**Parameters:**


* `r` CoAP request context. 




        

<hr>



### function coap\_decode\_nibble 

_Decode a 4-bit CoAP option delta or length nibble and any extended bytes._ 
```C++
static bool coap_decode_nibble (
    uint8_t nibble,
    const uint8_t * buf,
    size_t buf_len,
    size_t * pos,
    size_t * out
) 
```





**Parameters:**


* `nibble` 4-bit option nibble. 
* `buf` Packet buffer. 
* `buf_len` Total packet length. 
* `pos` [in,out] Read cursor. 
* `out` [out] Decoded option delta or length. 



**Returns:**

true on valid encoding within buffer bounds, false otherwise. 





        

<hr>



### function coap\_encode\_nibble 

_Encode a CoAP option delta or length into base nibble and extended value._ 
```C++
static void coap_encode_nibble (
    size_t val,
    uint8_t * nibble,
    uint8_t * ext_len,
    uint16_t * ext_val
) 
```





**Parameters:**


* `val` Raw option delta or length. 
* `nibble` [out] 4-bit header nibble (0..14). 
* `ext_len` [out] Extended byte count (0, 1, or 2). 
* `ext_val` [out] Extended integer value. 




        

<hr>



### function coap\_hdr\_valid 

_Validate CoAP header version, token length, empty rules, and reserved code classes._ 
```C++
static bool coap_hdr_valid (
    const SYN_CoapMsg * msg,
    uint8_t ver,
    size_t buf_len
) 
```





**Parameters:**


* `msg` Parsed message header fields. 
* `ver` 2-bit CoAP version field. 
* `buf_len` Total datagram byte length. 



**Returns:**

true if header fields satisfy RFC 7252 format rules, false otherwise. 





        

<hr>



### function coap\_init\_backoff 

_Initialize exponential retransmit backoff (forcing 0 retries for NON requests)._ 
```C++
static void coap_init_backoff (
    SYN_Backoff * b,
    const SYN_CoapMsg * msg,
    uint32_t timeout_ms,
    uint8_t retries
) 
```





**Parameters:**


* `b` [out] Backoff state. 
* `msg` CoAP request message descriptor (may be NULL). 
* `timeout_ms` Initial retransmit timeout in milliseconds. 
* `retries` Maximum retransmission count for confirmable messages. 




        

<hr>



### function coap\_parse\_opt\_hdr 

_Parse one CoAP option header and validate 16-bit option-number delta and buffer bounds._ 
```C++
static bool coap_parse_opt_hdr (
    const uint8_t * buf,
    size_t buf_len,
    size_t * pos,
    uint16_t * prev_num,
    size_t * len_out
) 
```





**Parameters:**


* `buf` Packet buffer. 
* `buf_len` Total packet length. 
* `pos` [in,out] Read cursor (initially at option header byte). 
* `prev_num` [in,out] Running option number accumulator. 
* `len_out` [out] Decoded option value length. 



**Returns:**

true if option header and bounds are valid, false otherwise. 





        

<hr>



### function coap\_pt\_args\_valid 

_Validate protothread and task user\_data pointers for CoAP request tasks._ 
```C++
static bool coap_pt_args_valid (
    const SYN_PT * pt,
    const SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread control block. 
* `task` Scheduler task descriptor. 



**Returns:**

true if pt, task, and task-&gt;user\_data are all non-NULL. 





        

<hr>



### function coap\_response\_is\_rst 

_Check whether a parsed CoAP message is an RFC 7252 Empty Reset matching the request MID._ 
```C++
static bool coap_response_is_rst (
    const SYN_CoapMsg * req,
    const SYN_CoapMsg * resp
) 
```





**Parameters:**


* `req` Active request message header. 
* `resp` Parsed inbound message header. 



**Returns:**

true if resp is COAP\_TYPE\_RST with code 0.00 and matching msg\_id. 





        

<hr>



### function coap\_response\_matches 

_Check whether a parsed CoAP response token matches the active request token._ 
```C++
static bool coap_response_matches (
    const SYN_CoapMsg * req,
    const SYN_CoapMsg * resp
) 
```





**Parameters:**


* `req` Request message header. 
* `resp` Parsed response message header. 



**Returns:**

true if token lengths and bytes match. 





        

<hr>



### function coap\_serialize\_args\_ok 

_Validate inputs to syn\_coap\_serialize before writing packet bytes._ 
```C++
static bool coap_serialize_args_ok (
    const SYN_CoapMsg * msg,
    const SYN_CoapOption * options,
    size_t option_count,
    const uint8_t * buf,
    size_t max_buf_len
) 
```





**Parameters:**


* `msg` Message header and payload descriptor. 
* `options` Option array (may be NULL when option\_count == 0). 
* `option_count` Number of options. 
* `buf` Output buffer. 
* `max_buf_len` Output buffer capacity. 



**Returns:**

true if all pointers, counts, and header bounds are valid. 





        

<hr>



### function coap\_sort\_options 

_Copy and sort up to 16 CoAP options by ascending option number._ 
```C++
static void coap_sort_options (
    SYN_CoapOption * sorted,
    const SYN_CoapOption * options,
    size_t count
) 
```





**Parameters:**


* `sorted` [out] Destination option array (capacity 16). 
* `options` Source options array. 
* `count` Number of options to copy and sort (&lt;= 16). 




        

<hr>



### function coap\_tr\_poll\_rx 

_Poll transport once and update request status on matching response or Reset._ 
```C++
static bool coap_tr_poll_rx (
    SYN_CoapTransportRequest * r,
    bool * done_out
) 
```





**Parameters:**


* `r` Transport request context. 
* `done_out` [out] Set true when a matching response or RFC 7252 Reset completed the request. 



**Returns:**

true if a non-empty frame was read from transport, false otherwise. 





        

<hr>



### function coap\_tr\_wait\_done 

_Check whether the generic-transport CoAP receive wait condition is satisfied._ 
```C++
static bool coap_tr_wait_done (
    const SYN_CoapTransportRequest * r
) 
```





**Parameters:**


* `r` CoAP transport request context. 



**Returns:**

true if transport is NULL, has pending data, or attempt deadline elapsed. 





        

<hr>



### function coap\_udp\_consume\_rx 

_Parse a received UDP CoAP datagram and update request status on response or Reset._ 
```C++
static bool coap_udp_consume_rx (
    SYN_CoapRequest * r,
    size_t n
) 
```





**Parameters:**


* `r` Request context. 
* `n` Received byte count (&gt; 0). 



**Returns:**

true if a matching response or RFC 7252 Reset completed the request, false otherwise. 





        

<hr>



### function coap\_udp\_wait\_done 

_Check whether the UDP CoAP receive wait condition is satisfied._ 
```C++
static bool coap_udp_wait_done (
    const SYN_CoapRequest * r
) 
```





**Parameters:**


* `r` CoAP UDP request context. 



**Returns:**

true if socket closed, readable, or attempt deadline elapsed. 





        

<hr>



### function coap\_write\_ext 

_Append 0, 1, or 2 extended option header bytes._ 
```C++
static void coap_write_ext (
    uint8_t * buf,
    size_t * pos,
    uint8_t ext_len,
    uint16_t ext_val
) 
```





**Parameters:**


* `buf` Destination packet buffer. 
* `pos` [in,out] Write cursor. 
* `ext_len` Extended byte count (0, 1, or 2). 
* `ext_val` Extended value to write. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_coap.c`

