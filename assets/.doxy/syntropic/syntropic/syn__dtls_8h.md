

# File syn\_dtls.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dtls.h**](syn__dtls_8h.md)

[Go to the source code of this file](syn__dtls_8h_source.md)

_Native Zero-Heap DTLS 1.3 Datagram Protocol Engine (RFC 9147)._ [More...](#detailed-description)

* `#include "syntropic/crypto/syn_aes.h"`
* `#include "syntropic/crypto/syn_chacha20poly1305.h"`
* `#include "syntropic/crypto/syn_hkdf.h"`
* `#include "syntropic/crypto/syn_sha256.h"`
* `#include "syntropic/crypto/syn_sha512.h"`
* `#include "syntropic/crypto/syn_x25519.h"`
* `#include "syntropic/crypto/syn_x509.h"`
* `#include "syntropic/net/syn_tls.h"`
* `#include "syntropic/net/syn_transport.h"`
* `#include "syntropic/pt/syn_pt.h"`
* `#include "syntropic/sched/syn_task.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DTLS\_Config**](structSYN__DTLS__Config.md) <br> |
| struct | [**SYN\_DTLS\_Context**](structSYN__DTLS__Context.md) <br> |
| struct | [**SYN\_DTLS\_ReplayWindow**](structSYN__DTLS__ReplayWindow.md) <br>_64-packet Sliding Window Anti-Replay Filter (RFC 9147 Section 4.5.2)._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_DTLS\_AuthMode**](#enum-syn_dtls_authmode)  <br> |
| enum  | [**SYN\_DTLS\_CipherSuite**](#enum-syn_dtls_ciphersuite)  <br> |
| enum  | [**SYN\_DTLS\_Epoch**](#enum-syn_dtls_epoch)  <br> |
| enum  | [**SYN\_DTLS\_State**](#enum-syn_dtls_state)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_dtls\_bind\_transport**](#function-syn_dtls_bind_transport) ([**SYN\_DTLS\_Context**](structSYN__DTLS__Context.md) \* dtls\_ctx, [**SYN\_Transport**](structSYN__Transport.md) \* tr\_out) <br>_Bind DTLS 1.3 engine to abstract_ [_**SYN\_Transport**_](structSYN__Transport.md) _interface._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dtls\_handshake**](#function-syn_dtls_handshake) ([**SYN\_DTLS\_Context**](structSYN__DTLS__Context.md) \* ctx) <br>_Execute DTLS 1.3 handshake._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dtls\_init**](#function-syn_dtls_init) ([**SYN\_DTLS\_Context**](structSYN__DTLS__Context.md) \* ctx, const [**SYN\_DTLS\_Config**](structSYN__DTLS__Config.md) \* config, [**SYN\_Transport**](structSYN__Transport.md) \* transport, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize DTLS 1.3 engine context with caller-allocated memory._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dtls\_recv**](#function-syn_dtls_recv) ([**SYN\_DTLS\_Context**](structSYN__DTLS__Context.md) \* ctx, uint8\_t \* data, size\_t max\_len, size\_t \* out\_len) <br>_Receive and decrypt a DTLS 1.3 datagram record._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dtls\_replay\_check**](#function-syn_dtls_replay_check) (const [**SYN\_DTLS\_ReplayWindow**](structSYN__DTLS__ReplayWindow.md) \* win, uint64\_t seq) <br>_Check whether a sequence number is acceptable by the replay window._  |
|  void | [**syn\_dtls\_replay\_update**](#function-syn_dtls_replay_update) ([**SYN\_DTLS\_ReplayWindow**](structSYN__DTLS__ReplayWindow.md) \* win, uint64\_t seq) <br>_Commit a validated sequence number to the replay window._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dtls\_send**](#function-syn_dtls_send) ([**SYN\_DTLS\_Context**](structSYN__DTLS__Context.md) \* ctx, const uint8\_t \* data, size\_t len) <br>_Send application data protected by DTLS 1.3 AEAD datagram record._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_dtls\_task**](#function-syn_dtls_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Non-blocking DTLS background task (Protothread)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_DTLS\_RECORD\_MAX\_PAYLOAD**](syn__dtls_8h.md#define-syn_dtls_record_max_payload)  `2048U`<br>_Maximum DTLS datagram record payload size in bytes (2048)._  |
| define  | [**SYN\_DTLS\_REPLAY\_WINDOW\_SIZE**](syn__dtls_8h.md#define-syn_dtls_replay_window_size)  `64U`<br>_Size of anti-replay sliding window in packets (64)._  |
| define  | [**SYN\_DTLS\_SECRET\_LEN**](syn__dtls_8h.md#define-syn_dtls_secret_len)  `48U`<br>_Length of DTLS 1.3 secret keys in bytes (48 for SHA-384 / SHA-256 capacity)._  |
| define  | [**SYN\_DTLS\_UNIFIED\_CID\_BIT**](syn__dtls_8h.md#define-syn_dtls_unified_cid_bit)  `0x10U`<br> |
| define  | [**SYN\_DTLS\_UNIFIED\_EPOCH\_MASK**](syn__dtls_8h.md#define-syn_dtls_unified_epoch_mask)  `0x03U`<br> |
| define  | [**SYN\_DTLS\_UNIFIED\_FIXED\_BIT**](syn__dtls_8h.md#define-syn_dtls_unified_fixed_bit)  `0x20U`<br> |
| define  | [**SYN\_DTLS\_UNIFIED\_LEN\_BIT**](syn__dtls_8h.md#define-syn_dtls_unified_len_bit)  `0x04U`<br> |
| define  | [**SYN\_DTLS\_UNIFIED\_SEQ\_16BIT**](syn__dtls_8h.md#define-syn_dtls_unified_seq_16bit)  `0x08U`<br> |

## Detailed Description


Implements Datagram Transport Layer Security (DTLS) version 1.3:
* Unified record header format (RFC 9147 Section 4)
* 64-packet sliding window anti-replay protection (RFC 9147 Section 4.5.2)
* Epoch-based key scheduling & AEAD record encryption/decryption
* Pre-Shared Key (PSK), Raw Public Key (RPK), and X.509 modes
* Full pluggability into [**SYN\_Transport**](structSYN__Transport.md) (UDP, wireless, serial, etc.) 




    
## Public Types Documentation




### enum SYN\_DTLS\_AuthMode 

```C++
enum SYN_DTLS_AuthMode {
    SYN_DTLS_AUTH_MODE_PSK = 0,
    SYN_DTLS_AUTH_MODE_RAW_PUBKEY,
    SYN_DTLS_AUTH_MODE_X509_SERVER,
    SYN_DTLS_AUTH_MODE_MTLS
};
```



DTLS 1.3 Authentication Mode 


        

<hr>



### enum SYN\_DTLS\_CipherSuite 

```C++
enum SYN_DTLS_CipherSuite {
    SYN_DTLS_CIPHER_SUITE_CHACHA20_POLY1305_SHA256 =
        0,
    SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256 = 1,
    SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384 = 2,
    SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256 = 3,
    SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256 = 4
};
```



DTLS 1.3 Cipher Suite 


        

<hr>



### enum SYN\_DTLS\_Epoch 

```C++
enum SYN_DTLS_Epoch {
    SYN_DTLS_EPOCH_PLAINTEXT = 0,
    SYN_DTLS_EPOCH_EARLY_DATA = 1,
    SYN_DTLS_EPOCH_HANDSHAKE = 2,
    SYN_DTLS_EPOCH_APP_DATA = 3
};
```



DTLS 1.3 Epochs 


        

<hr>



### enum SYN\_DTLS\_State 

```C++
enum SYN_DTLS_State {
    SYN_DTLS_STATE_UNINITIALIZED = 0,
    SYN_DTLS_STATE_CLIENT_HELLO_SENT,
    SYN_DTLS_STATE_SERVER_HELLO_RECEIVED,
    SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED,
    SYN_DTLS_STATE_CERTIFICATE_VERIFIED,
    SYN_DTLS_STATE_FINISHED_SENT,
    SYN_DTLS_STATE_ESTABLISHED,
    SYN_DTLS_STATE_ERROR
};
```



DTLS 1.3 Handshake State 


        

<hr>
## Public Functions Documentation




### function syn\_dtls\_bind\_transport 

_Bind DTLS 1.3 engine to abstract_ [_**SYN\_Transport**_](structSYN__Transport.md) _interface._
```C++
void syn_dtls_bind_transport (
    SYN_DTLS_Context * dtls_ctx,
    SYN_Transport * tr_out
) 
```





**Parameters:**


* `dtls_ctx` Initialized DTLS context. 
* `tr_out` [out] Transport instance to populate. 




        

<hr>



### function syn\_dtls\_handshake 

_Execute DTLS 1.3 handshake._ 
```C++
bool syn_dtls_handshake (
    SYN_DTLS_Context * ctx
) 
```





**Parameters:**


* `ctx` Initialized DTLS context. 



**Returns:**

true on successful handshake completion. 





        

<hr>



### function syn\_dtls\_init 

_Initialize DTLS 1.3 engine context with caller-allocated memory._ 
```C++
bool syn_dtls_init (
    SYN_DTLS_Context * ctx,
    const SYN_DTLS_Config * config,
    SYN_Transport * transport,
    uint8_t * rx_buf,
    size_t rx_buf_size,
    uint8_t * tx_buf,
    size_t tx_buf_size
) 
```





**Parameters:**


* `ctx` Context to initialize. 
* `config` Engine configuration. 
* `transport` Underlying transport (UDP, Socket, Serial). 
* `rx_buf` Caller-allocated buffer for incoming datagrams. 
* `rx_buf_size` Size of rx\_buf in bytes. 
* `tx_buf` Caller-allocated buffer for outgoing datagrams. 
* `tx_buf_size` Size of tx\_buf in bytes. 



**Returns:**

true on success, false on invalid parameters. 





        

<hr>



### function syn\_dtls\_recv 

_Receive and decrypt a DTLS 1.3 datagram record._ 
```C++
bool syn_dtls_recv (
    SYN_DTLS_Context * ctx,
    uint8_t * data,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `ctx` Established DTLS context. 
* `data` Output buffer for decrypted application data. 
* `max_len` Capacity of output buffer. 
* `out_len` [out] Actual decrypted payload length. 



**Returns:**

true if valid uncorrupted, non-replayed record received. 





        

<hr>



### function syn\_dtls\_replay\_check 

_Check whether a sequence number is acceptable by the replay window._ 
```C++
bool syn_dtls_replay_check (
    const SYN_DTLS_ReplayWindow * win,
    uint64_t seq
) 
```





**Parameters:**


* `win` Pointer to replay window. 
* `seq` 64-bit reconstructed packet sequence number. 



**Returns:**

true if packet is not a duplicate and is within or ahead of window. 





        

<hr>



### function syn\_dtls\_replay\_update 

_Commit a validated sequence number to the replay window._ 
```C++
void syn_dtls_replay_update (
    SYN_DTLS_ReplayWindow * win,
    uint64_t seq
) 
```





**Parameters:**


* `win` Pointer to replay window. 
* `seq` 64-bit packet sequence number. 




        

<hr>



### function syn\_dtls\_send 

_Send application data protected by DTLS 1.3 AEAD datagram record._ 
```C++
bool syn_dtls_send (
    SYN_DTLS_Context * ctx,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `ctx` Established DTLS context. 
* `data` Application data buffer. 
* `len` Length in bytes. 



**Returns:**

true if encrypted record was sent. 





        

<hr>



### function syn\_dtls\_task 

_Non-blocking DTLS background task (Protothread)._ 
```C++
SYN_PT_Status syn_dtls_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Pointer to task protothread. 
* `task` Pointer to scheduler task context. 



**Returns:**

SYN\_PT\_Status. 





        

<hr>
## Macro Definition Documentation





### define SYN\_DTLS\_RECORD\_MAX\_PAYLOAD 

_Maximum DTLS datagram record payload size in bytes (2048)._ 
```C++
#define SYN_DTLS_RECORD_MAX_PAYLOAD `2048U`
```




<hr>



### define SYN\_DTLS\_REPLAY\_WINDOW\_SIZE 

_Size of anti-replay sliding window in packets (64)._ 
```C++
#define SYN_DTLS_REPLAY_WINDOW_SIZE `64U`
```




<hr>



### define SYN\_DTLS\_SECRET\_LEN 

_Length of DTLS 1.3 secret keys in bytes (48 for SHA-384 / SHA-256 capacity)._ 
```C++
#define SYN_DTLS_SECRET_LEN `48U`
```




<hr>



### define SYN\_DTLS\_UNIFIED\_CID\_BIT 

```C++
#define SYN_DTLS_UNIFIED_CID_BIT `0x10U`
```



Bit 4: Connection ID present 


        

<hr>



### define SYN\_DTLS\_UNIFIED\_EPOCH\_MASK 

```C++
#define SYN_DTLS_UNIFIED_EPOCH_MASK `0x03U`
```



Bits 1..0: Epoch (0..3) 


        

<hr>



### define SYN\_DTLS\_UNIFIED\_FIXED\_BIT 

```C++
#define SYN_DTLS_UNIFIED_FIXED_BIT `0x20U`
```



DTLS 1.3 Unified Header Flags Bit 5: Must be 1 for DTLS 1.3 unified record 


        

<hr>



### define SYN\_DTLS\_UNIFIED\_LEN\_BIT 

```C++
#define SYN_DTLS_UNIFIED_LEN_BIT `0x04U`
```



Bit 2: Length field present 


        

<hr>



### define SYN\_DTLS\_UNIFIED\_SEQ\_16BIT 

```C++
#define SYN_DTLS_UNIFIED_SEQ_16BIT `0x08U`
```



Bit 3: 16-bit sequence number (0 = 8-bit) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dtls.h`

