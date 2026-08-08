

# File syn\_tls.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_tls.h**](syn__tls_8h.md)

[Go to the source code of this file](syn__tls_8h_source.md)

_Native Zero-Heap TLS 1.3 Protocol Engine & Transport Adapter (RFC 8446)._ [More...](#detailed-description)

* `#include "syntropic/crypto/syn_chacha20poly1305.h"`
* `#include "syntropic/crypto/syn_hkdf.h"`
* `#include "syntropic/crypto/syn_x25519.h"`
* `#include "syntropic/crypto/syn_x509.h"`
* `#include "syntropic/net/syn_transport.h"`
* `#include "syntropic/pt/syn_pt.h"`
* `#include "syntropic/sched/syn_task.h"`
* `#include "syntropic/util/syn_sha256.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_TLS\_Config**](structSYN__TLS__Config.md) <br> |
| struct | [**SYN\_TLS\_Context**](structSYN__TLS__Context.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_TLS\_AuthMode**](#enum-syn_tls_authmode)  <br> |
| enum  | [**SYN\_TLS\_State**](#enum-syn_tls_state)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_tls\_bind\_transport**](#function-syn_tls_bind_transport) ([**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* tls\_ctx, [**SYN\_Transport**](structSYN__Transport.md) \* tr\_out) <br>_Wrap a TLS 1.3 context into a standard_ [_**SYN\_Transport**_](structSYN__Transport.md) _abstraction._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_tls\_handshake**](#function-syn_tls_handshake) ([**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* ctx) <br>_Perform a non-blocking TLS 1.3 handshake step._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_tls\_init**](#function-syn_tls_init) ([**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* ctx, const [**SYN\_TLS\_Config**](structSYN__TLS__Config.md) \* config, [**SYN\_Transport**](structSYN__Transport.md) \* transport, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize a TLS 1.3 context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_tls\_recv**](#function-syn_tls_recv) ([**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* ctx, uint8\_t \* data, size\_t max\_len, size\_t \* out\_len) <br>_Receive and decrypt TLS 1.3 record payload into caller buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_tls\_send**](#function-syn_tls_send) ([**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* ctx, const uint8\_t \* data, size\_t len) <br>_Encrypt and transmit application data payload over TLS 1.3._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_tls\_task**](#function-syn_tls_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread task — drives the TLS 1.3 client handshake & session._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_TLS\_State**](syn__tls_8h.md#enum-syn_tls_state) | [**syn\_tls\_get\_state**](#function-syn_tls_get_state) (const [**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* ctx) <br>_Retrieve current TLS 1.3 state machine state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_tls\_is\_established**](#function-syn_tls_is_established) (const [**SYN\_TLS\_Context**](structSYN__TLS__Context.md) \* ctx) <br>_Check if the TLS 1.3 session is established and ready for app data._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_TLS\_RECORD\_MAX\_PAYLOAD**](syn__tls_8h.md#define-syn_tls_record_max_payload)  `2048U`<br>_Maximum TLS record payload size in bytes (2048)._  |
| define  | [**SYN\_TLS\_SECRET\_LEN**](syn__tls_8h.md#define-syn_tls_secret_len)  `32U`<br>_Length of TLS 1.3 secret keys in bytes (32)._  |

## Detailed Description


Implements TLS 1.3 Client & Server handshakes and encrypted record layer. Supported modes:
* Pre-Shared Key (PSK / PSK-DHE)
* Raw Public Key (RPK) - RFC 7250
* X.509 Server Authentication & Mutual TLS (mTLS)




Plugs into standard [**SYN\_Transport**](structSYN__Transport.md) interface. 


    
## Public Types Documentation




### enum SYN\_TLS\_AuthMode 

```C++
enum SYN_TLS_AuthMode {
    SYN_TLS_AUTH_MODE_PSK = 0,
    SYN_TLS_AUTH_MODE_RAW_PUBKEY,
    SYN_TLS_AUTH_MODE_X509_SERVER,
    SYN_TLS_AUTH_MODE_MTLS
};
```



TLS 1.3 Authentication Mode 


        

<hr>



### enum SYN\_TLS\_State 

```C++
enum SYN_TLS_State {
    SYN_TLS_STATE_UNINITIALIZED = 0,
    SYN_TLS_STATE_CLIENT_HELLO_SENT,
    SYN_TLS_STATE_SERVER_HELLO_RECEIVED,
    SYN_TLS_STATE_HANDSHAKE_KEYS_DERIVED,
    SYN_TLS_STATE_CERTIFICATE_VERIFIED,
    SYN_TLS_STATE_FINISHED_SENT,
    SYN_TLS_STATE_ESTABLISHED,
    SYN_TLS_STATE_ERROR
};
```



TLS 1.3 Handshake State Machine States 


        

<hr>
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
## Public Static Functions Documentation




### function syn\_tls\_get\_state 

_Retrieve current TLS 1.3 state machine state._ 
```C++
static inline SYN_TLS_State syn_tls_get_state (
    const SYN_TLS_Context * ctx
) 
```





**Parameters:**


* `ctx` TLS 1.3 context. 



**Returns:**

Current SYN\_TLS\_State. 





        

<hr>



### function syn\_tls\_is\_established 

_Check if the TLS 1.3 session is established and ready for app data._ 
```C++
static inline bool syn_tls_is_established (
    const SYN_TLS_Context * ctx
) 
```





**Parameters:**


* `ctx` TLS 1.3 context. 



**Returns:**

true if established. 





        

<hr>
## Macro Definition Documentation





### define SYN\_TLS\_RECORD\_MAX\_PAYLOAD 

_Maximum TLS record payload size in bytes (2048)._ 
```C++
#define SYN_TLS_RECORD_MAX_PAYLOAD `2048U`
```




<hr>



### define SYN\_TLS\_SECRET\_LEN 

_Length of TLS 1.3 secret keys in bytes (32)._ 
```C++
#define SYN_TLS_SECRET_LEN `32U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tls.h`

