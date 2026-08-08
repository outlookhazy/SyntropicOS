

# File syn\_tls.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_tls.c**](syn__tls_8c.md)

[Go to the source code of this file](syn__tls_8c_source.md)

_Native Zero-Heap TLS 1.3 Protocol Engine & Transport Adapter implementation._ 

* `#include "syntropic/net/syn_tls.h"`
* `#include "syntropic/crypto/syn_ed25519.h"`
* `#include "syntropic/util/syn_hmac.h"`
* `#include "syntropic/util/syn_random.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_tls\_bind\_transport**](#function-syn_tls_bind_transport) ([**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* tls\_ctx, [**SYN\_Transport**](structSYN__Transport.md) \* tr\_out) <br>_Wrap a TLS 1.3 context into a standard_ [_**SYN\_Transport**_](structSYN__Transport.md) _abstraction._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_tls\_handshake**](#function-syn_tls_handshake) ([**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* ctx) <br>_Perform a non-blocking TLS 1.3 handshake step._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_tls\_init**](#function-syn_tls_init) ([**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* ctx, const [**SYN\_TLS\_Config**](structSYN__TLS__Config.md) \* config, [**SYN\_Transport**](structSYN__Transport.md) \* transport, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize a TLS 1.3 context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_tls\_recv**](#function-syn_tls_recv) ([**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* ctx, uint8\_t \* data, size\_t max\_len, size\_t \* out\_len) <br>_Receive and decrypt TLS 1.3 record payload into caller buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_tls\_send**](#function-syn_tls_send) ([**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* ctx, const uint8\_t \* data, size\_t len) <br>_Encrypt and transmit application data payload over TLS 1.3._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_tls\_task**](#function-syn_tls_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread task — drives the TLS 1.3 client handshake & session._  |




























## Public Functions Documentation




### function syn\_tls\_bind\_transport 

_Wrap a TLS 1.3 context into a standard_ [_**SYN\_Transport**_](structSYN__Transport.md) _abstraction._
```C++
void syn_tls_bind_transport (
    SYN_TLS_Context * tls_ctx,
    SYN_Transport * tr_out
) 
```





**Parameters:**


* `tls_ctx` Initialized TLS 1.3 context. 
* `tr_out` [out] Output transport instance. 




        

<hr>



### function syn\_tls\_handshake 

_Perform a non-blocking TLS 1.3 handshake step._ 
```C++
bool syn_tls_handshake (
    SYN_TLS_Context * ctx
) 
```





**Parameters:**


* `ctx` TLS 1.3 context. 



**Returns:**

true if handshake completed or progressing, false on fatal error. 





        

<hr>



### function syn\_tls\_init 

_Initialize a TLS 1.3 context._ 
```C++
bool syn_tls_init (
    SYN_TLS_Context * ctx,
    const SYN_TLS_Config * config,
    SYN_Transport * transport,
    uint8_t * rx_buf,
    size_t rx_buf_size,
    uint8_t * tx_buf,
    size_t tx_buf_size
) 
```





**Parameters:**


* `ctx` Context to initialize. 
* `config` Configuration. 
* `transport` Wire transport instance. 
* `rx_buf` Caller-owned RX record buffer. 
* `rx_buf_size` RX buffer capacity. 
* `tx_buf` Caller-owned TX record buffer. 
* `tx_buf_size` TX buffer capacity. 



**Returns:**

true on success. 





        

<hr>



### function syn\_tls\_recv 

_Receive and decrypt TLS 1.3 record payload into caller buffer._ 
```C++
bool syn_tls_recv (
    SYN_TLS_Context * ctx,
    uint8_t * data,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `ctx` TLS 1.3 context. 
* `data` [out] Output buffer for decrypted data. 
* `max_len` Capacity of output buffer. 
* `out_len` [out] Actual decrypted bytes written. 



**Returns:**

true if a complete record decrypted. 





        

<hr>



### function syn\_tls\_send 

_Encrypt and transmit application data payload over TLS 1.3._ 
```C++
bool syn_tls_send (
    SYN_TLS_Context * ctx,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `ctx` TLS 1.3 context. 
* `data` Application data buffer. 
* `len` Application data length. 



**Returns:**

true if record encrypted and sent successfully. 





        

<hr>



### function syn\_tls\_task 

_Cooperative protothread task — drives the TLS 1.3 client handshake & session._ 
```C++
SYN_PT_Status syn_tls_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread handle. 
* `task` Task descriptor. 



**Returns:**

PT status. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tls.c`

