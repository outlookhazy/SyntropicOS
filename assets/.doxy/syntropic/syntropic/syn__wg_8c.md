

# File syn\_wg.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_wg.c**](syn__wg_8c.md)

[Go to the source code of this file](syn__wg_8c_source.md)

_WireGuard client — Noise\_IKpsk2 handshake + transport._ [More...](#detailed-description)

* `#include "syn_wg.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "../util/syn_random.h"`
* `#include "../util/syn_assert.h"`
* `#include "../crypto/syn_blake2s.h"`
* `#include "../crypto/syn_chacha20poly1305.h"`
* `#include "../crypto/syn_x25519.h"`
* `#include "../util/syn_metrics.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t WG\_LABEL\_COOKIE[] | [**SYN\_UNUSED**](#variable-syn_unused)   = `"cookie--"`<br> |
|  const uint8\_t | [**WG\_CONSTRUCTION**](#variable-wg_construction)   = `"Noise\_IKpsk2\_25519\_ChaChaPoly\_BLAKE2s"`<br> |
|  const uint8\_t | [**WG\_IDENTIFIER**](#variable-wg_identifier)   = `"WireGuard v1 zx2c4 Jason@zx2c4.com"`<br> |
|  const uint8\_t | [**WG\_LABEL\_MAC1**](#variable-wg_label_mac1)   = `"mac1----"`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_wg\_init**](#function-syn_wg_init) ([**SYN\_WG**](structSYN__WG.md) \* wg, const [**SYN\_WgConfig**](structSYN__WgConfig.md) \* config, [**SYN\_SNTP**](structSYN__SNTP.md) \* sntp, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize the WireGuard client._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_wg\_send**](#function-syn_wg_send) ([**SYN\_WG**](structSYN__WG.md) \* wg, const uint8\_t \* ip\_packet, size\_t len) <br>_Send an IP packet through the WireGuard tunnel._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_wg\_task**](#function-syn_wg_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread task — drives the WireGuard client._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**load32\_le**](#function-load32_le) (const uint8\_t \* p) <br>_Load 32-bit little-endian word._  |
|  uint64\_t | [**load64\_le**](#function-load64_le) (const uint8\_t \* p) <br>_Load 64-bit little-endian word._  |
|  void | [**store32\_be**](#function-store32_be) (uint8\_t \* p, uint32\_t v) <br>_Store 32-bit big-endian word._  |
|  void | [**store32\_le**](#function-store32_le) (uint8\_t \* p, uint32\_t v) <br>_Store 32-bit little-endian word._  |
|  void | [**store64\_be**](#function-store64_be) (uint8\_t \* p, uint64\_t v) <br>_Store 64-bit big-endian word._  |
|  void | [**store64\_le**](#function-store64_le) (uint8\_t \* p, uint64\_t v) <br>_Store 64-bit little-endian word._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_consume\_response**](#function-wg_consume_response) ([**SYN\_WG**](structSYN__WG.md) \* wg, const uint8\_t \* msg, size\_t len) <br>_Parse and validate a handshake response, deriving session keys._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_decrypt\_and\_hash**](#function-wg_decrypt_and_hash) (uint8\_t h, const uint8\_t k, const uint8\_t \* ct, size\_t ct\_len, const uint8\_t tag, uint8\_t \* plain) <br>_Decrypt-and-hash: verify + decrypt, mix ciphertext+tag into hash._  |
|  void | [**wg\_encrypt\_and\_hash**](#function-wg_encrypt_and_hash) (uint8\_t h, const uint8\_t k, const uint8\_t \* plain, size\_t plain\_len, uint8\_t \* ct, uint8\_t tag) <br>_Encrypt-and-hash: encrypt plaintext, mix ciphertext+tag into hash._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_handle\_transport**](#function-wg_handle_transport) ([**SYN\_WG**](structSYN__WG.md) \* wg, const uint8\_t \* msg, size\_t len) <br>_Decrypt and deliver an incoming WireGuard transport message._  |
|  void | [**wg\_hkdf2**](#function-wg_hkdf2) (uint8\_t out1, uint8\_t out2, const uint8\_t ck, const uint8\_t \* input, size\_t input\_len) <br> |
|  void | [**wg\_hkdf3**](#function-wg_hkdf3) (uint8\_t out1, uint8\_t out2, uint8\_t out3, const uint8\_t ck, const uint8\_t \* input, size\_t input\_len) <br> |
|  void | [**wg\_mac1**](#function-wg_mac1) (uint8\_t mac, const uint8\_t peer\_pub, const uint8\_t \* msg, size\_t msg\_len) <br>_Compute mac1 = keyed BLAKE2s(HASH("mac1----" \|\| peer\_pub), msg)._  |
|  void | [**wg\_mix\_hash**](#function-wg_mix_hash) (uint8\_t h, const void \* data, size\_t len) <br>_Mix hash: H = BLAKE2s(H \|\| data)._  |
|  void | [**wg\_mix\_key**](#function-wg_mix_key) (uint8\_t ck, uint8\_t k, const uint8\_t \* input, size\_t len) <br>_Mix key: (CK, k) = HKDF(CK, input)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_replay\_check**](#function-wg_replay_check) ([**SYN\_WgSession**](structSYN__WgSession.md) \* s, uint64\_t counter) <br>_Anti-replay check using a sliding window bitmap._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_send\_initiation**](#function-wg_send_initiation) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Build and send a Noise\_IKpsk2 handshake initiation message._  |
|  void | [**wg\_send\_keepalive**](#function-wg_send_keepalive) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Send an empty (keepalive) transport message._  |
|  void | [**wg\_tai64n**](#function-wg_tai64n) (uint8\_t out, const [**SYN\_SNTP**](structSYN__SNTP.md) \* sntp) <br>_Write a 12-byte TAI64N timestamp from NTP time._  |


























## Detailed Description


Implements the WireGuard protocol per the whitepaper: [https://www.wireguard.com/papers/wireguard.pdf](https://www.wireguard.com/papers/wireguard.pdf)


Noise protocol pattern: IKpsk2 Initiator (us) knows responder's static public key. PSK mixed in at handshake step 2. 


    
## Public Static Attributes Documentation




### variable SYN\_UNUSED 

```C++
const uint8_t WG_LABEL_COOKIE [] SYN_UNUSED;
```



Cookie label 
 


        

<hr>



### variable WG\_CONSTRUCTION 

```C++
const uint8_t WG_CONSTRUCTION[];
```



Noise construction string 


        

<hr>



### variable WG\_IDENTIFIER 

```C++
const uint8_t WG_IDENTIFIER[];
```



Protocol identifier string 


        

<hr>



### variable WG\_LABEL\_MAC1 

```C++
const uint8_t WG_LABEL_MAC1[];
```



MAC1 label 
 


        

<hr>
## Public Functions Documentation




### function syn\_wg\_init 

_Initialize the WireGuard client._ 
```C++
void syn_wg_init (
    SYN_WG * wg,
    const SYN_WgConfig * config,
    SYN_SNTP * sntp,
    uint8_t * rx_buf,
    size_t rx_buf_size,
    uint8_t * tx_buf,
    size_t tx_buf_size
) 
```



Derives the public key from the private key and prepares state. Does NOT open a socket or start a handshake — that happens when the protothread task runs.




**Parameters:**


* `wg` Client context. 
* `config` Peer configuration (copied). 
* `sntp` SNTP time source (must be initialized, may not be synced yet). 
* `rx_buf` Receive buffer (at least SYN\_WG\_MTU + SYN\_WG\_TRANSPORT\_OVERHEAD). 
* `rx_buf_size` Receive buffer capacity. 
* `tx_buf` Transmit buffer (same sizing). 
* `tx_buf_size` Transmit buffer capacity. 




        

<hr>



### function syn\_wg\_send 

_Send an IP packet through the WireGuard tunnel._ 
```C++
SYN_Status syn_wg_send (
    SYN_WG * wg,
    const uint8_t * ip_packet,
    size_t len
) 
```



Encrypts the packet with the current session key and sends it as a WireGuard transport message.




**Parameters:**


* `wg` Client context. 
* `ip_packet` Raw IP packet to send. 
* `len` Packet length. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if no session or send failed. 





        

<hr>



### function syn\_wg\_task 

_Cooperative protothread task — drives the WireGuard client._ 
```C++
SYN_PT_Status syn_wg_task (
    SYN_PT * pt,
    SYN_Task * task
) 
```



Handles: NTP sync wait, handshake, incoming packet processing, keepalive, and rekeying. Pass the [**SYN\_WG**](structSYN__WG.md) context via task-&gt;user\_data.




**Parameters:**


* `pt` Protothread. 
* `task` Task descriptor. 



**Returns:**

PT status. 





        

<hr>
## Public Static Functions Documentation




### function load32\_le 

_Load 32-bit little-endian word._ 
```C++
static inline uint32_t load32_le (
    const uint8_t * p
) 
```





**Parameters:**


* `p` Source bytes. 



**Returns:**

32-bit value. 





        

<hr>



### function load64\_le 

_Load 64-bit little-endian word._ 
```C++
static inline uint64_t load64_le (
    const uint8_t * p
) 
```





**Parameters:**


* `p` Source bytes. 



**Returns:**

64-bit value. 





        

<hr>



### function store32\_be 

_Store 32-bit big-endian word._ 
```C++
static inline void store32_be (
    uint8_t * p,
    uint32_t v
) 
```





**Parameters:**


* `p` Destination bytes. 
* `v` Value to store. 




        

<hr>



### function store32\_le 

_Store 32-bit little-endian word._ 
```C++
static inline void store32_le (
    uint8_t * p,
    uint32_t v
) 
```





**Parameters:**


* `p` Destination bytes. 
* `v` Value to store. 




        

<hr>



### function store64\_be 

_Store 64-bit big-endian word._ 
```C++
static inline void store64_be (
    uint8_t * p,
    uint64_t v
) 
```





**Parameters:**


* `p` Destination bytes. 
* `v` Value to store. 




        

<hr>



### function store64\_le 

_Store 64-bit little-endian word._ 
```C++
static inline void store64_le (
    uint8_t * p,
    uint64_t v
) 
```





**Parameters:**


* `p` Destination bytes. 
* `v` Value to store. 




        

<hr>



### function wg\_consume\_response 

_Parse and validate a handshake response, deriving session keys._ 
```C++
static bool wg_consume_response (
    SYN_WG * wg,
    const uint8_t * msg,
    size_t len
) 
```





**Parameters:**


* `wg` Client context. 
* `msg` Received message data. 
* `len` Received message length. 



**Returns:**

true if response was valid and session established. 





        

<hr>



### function wg\_decrypt\_and\_hash 

_Decrypt-and-hash: verify + decrypt, mix ciphertext+tag into hash._ 
```C++
static bool wg_decrypt_and_hash (
    uint8_t h,
    const uint8_t k,
    const uint8_t * ct,
    size_t ct_len,
    const uint8_t tag,
    uint8_t * plain
) 
```





**Parameters:**


* `h` Running hash (32 bytes). 
* `k` Decryption key (32 bytes). 
* `ct` Ciphertext input. 
* `ct_len` Ciphertext length. 
* `tag` MAC tag input (16 bytes). 
* `plain` Plaintext output. 



**Returns:**

true if decryption and verification succeeded. 





        

<hr>



### function wg\_encrypt\_and\_hash 

_Encrypt-and-hash: encrypt plaintext, mix ciphertext+tag into hash._ 
```C++
static void wg_encrypt_and_hash (
    uint8_t h,
    const uint8_t k,
    const uint8_t * plain,
    size_t plain_len,
    uint8_t * ct,
    uint8_t tag
) 
```





**Parameters:**


* `h` Running hash (32 bytes). 
* `k` Encryption key (32 bytes). 
* `plain` Plaintext input. 
* `plain_len` Plaintext length. 
* `ct` Ciphertext output. 
* `tag` MAC tag output (16 bytes). 




        

<hr>



### function wg\_handle\_transport 

_Decrypt and deliver an incoming WireGuard transport message._ 
```C++
static bool wg_handle_transport (
    SYN_WG * wg,
    const uint8_t * msg,
    size_t len
) 
```





**Parameters:**


* `wg` Client context. 
* `msg` Received message data. 
* `len` Received message length. 



**Returns:**

true if message was valid and delivered. 





        

<hr>



### function wg\_hkdf2 

```C++
static void wg_hkdf2 (
    uint8_t out1,
    uint8_t out2,
    const uint8_t ck,
    const uint8_t * input,
    size_t input_len
) 
```



HKDF-Extract + Expand producing 2 outputs. 

**Parameters:**


* `out1` Output buffers (32 bytes each). 
* `ck` Chaining key (32 bytes). 
* `input` Input data. 
* `input_len` Length of input data. 




        

<hr>



### function wg\_hkdf3 

```C++
static void wg_hkdf3 (
    uint8_t out1,
    uint8_t out2,
    uint8_t out3,
    const uint8_t ck,
    const uint8_t * input,
    size_t input_len
) 
```



HKDF-Extract + Expand producing 3 outputs. 

**Parameters:**


* `out1` Output buffers (32 bytes each). 
* `ck` Chaining key (32 bytes). 
* `input` Input data. 
* `input_len` Length of input data. 




        

<hr>



### function wg\_mac1 

_Compute mac1 = keyed BLAKE2s(HASH("mac1----" \|\| peer\_pub), msg)._ 
```C++
static void wg_mac1 (
    uint8_t mac,
    const uint8_t peer_pub,
    const uint8_t * msg,
    size_t msg_len
) 
```





**Parameters:**


* `mac` Output MAC (16 bytes). 
* `peer_pub` Peer's public key. 
* `msg` Message data. 
* `msg_len` Length of message data. 




        

<hr>



### function wg\_mix\_hash 

_Mix hash: H = BLAKE2s(H \|\| data)._ 
```C++
static void wg_mix_hash (
    uint8_t h,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `h` Running hash (32 bytes). 
* `data` Input data. 
* `len` Length of input data. 




        

<hr>



### function wg\_mix\_key 

_Mix key: (CK, k) = HKDF(CK, input)._ 
```C++
static void wg_mix_key (
    uint8_t ck,
    uint8_t k,
    const uint8_t * input,
    size_t len
) 
```





**Parameters:**


* `ck` Running chaining key (32 bytes). 
* `k` Output key (32 bytes). 
* `input` Input data. 
* `len` Length of input data. 




        

<hr>



### function wg\_replay\_check 

_Anti-replay check using a sliding window bitmap._ 
```C++
static bool wg_replay_check (
    SYN_WgSession * s,
    uint64_t counter
) 
```





**Parameters:**


* `s` Active session. 
* `counter` Received counter value. 



**Returns:**

true if counter is new and within window. 





        

<hr>



### function wg\_send\_initiation 

_Build and send a Noise\_IKpsk2 handshake initiation message._ 
```C++
static bool wg_send_initiation (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 



**Returns:**

true if message was built and sent. 





        

<hr>



### function wg\_send\_keepalive 

_Send an empty (keepalive) transport message._ 
```C++
static void wg_send_keepalive (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_tai64n 

_Write a 12-byte TAI64N timestamp from NTP time._ 
```C++
static void wg_tai64n (
    uint8_t out,
    const SYN_SNTP * sntp
) 
```





**Parameters:**


* `out` Output buffer (12 bytes). 
* `sntp` NTP time source. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_wg.c`

