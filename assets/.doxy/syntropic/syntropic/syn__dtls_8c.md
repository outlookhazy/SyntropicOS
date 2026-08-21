

# File syn\_dtls.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dtls.c**](syn__dtls_8c.md)

[Go to the source code of this file](syn__dtls_8c_source.md)

_Native Zero-Heap DTLS 1.3 Datagram Protocol Engine (RFC 9147)._ 

* `#include "syn_dtls.h"`
* `#include "syntropic/crypto/syn_asn1.h"`
* `#include "syntropic/crypto/syn_ed25519.h"`
* `#include "syntropic/crypto/syn_hkdf.h"`
* `#include "syntropic/crypto/syn_p256.h"`
* `#include <string.h>`





































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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dtls.c`

