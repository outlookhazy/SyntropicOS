

# File syn\_wg.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_wg.c**](syn__wg_8c.md)

[Go to the source code of this file](syn__wg_8c_source.md)

_WireGuard client — Noise\_IKpsk2 handshake + transport._ [More...](#detailed-description)

* `#include "../crypto/syn_blake2s.h"`
* `#include "../crypto/syn_chacha20poly1305.h"`
* `#include "../crypto/syn_x25519.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../sched/syn_sched.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_metrics.h"`
* `#include "../util/syn_random.h"`
* `#include "syn_wg.h"`
* `#include <string.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**WG\_StepAct**](#enum-wg_stepact)  <br>_Next cooperative scheduling action returned by_ [_**wg\_task\_step\_once()**_](syn__wg_8c.md#function-wg_task_step_once) _._ |






## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t WG\_LABEL\_COOKIE[] | [**SYN\_UNUSED**](#variable-syn_unused)   = `"cookie--"`<br> |
|  const uint8\_t | [**WG\_CONSTRUCTION**](#variable-wg_construction)   = `/* multi line expression */`<br> |
|  const uint8\_t | [**WG\_IDENTIFIER**](#variable-wg_identifier)   = `/* multi line expression */`<br> |
|  const uint8\_t | [**WG\_LABEL\_MAC1**](#variable-wg_label_mac1)   = `"mac1----"`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_wg\_disconnect**](#function-syn_wg_disconnect) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Disconnect the WireGuard tunnel and close socket._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_wg\_get\_stats**](#function-syn_wg_get_stats) (const [**SYN\_WG**](structSYN__WG.md) \* wg, [**SYN\_WgStats**](structSYN__WgStats.md) \* stats) <br>_Retrieve WireGuard tunnel metrics and statistics._  |
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
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ct\_memeq**](#function-syn_ct_memeq) (const void \* a, const void \* b, size\_t n) <br>_Constant-time byte-slice equality check._  |
|  void | [**syn\_secure\_zero**](#function-syn_secure_zero) (void \* v, size\_t n) <br>_Zero sensitive memory without dead-store elimination._  |
|  void | [**wg\_clear\_handshake\_state**](#function-wg_clear_handshake_state) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Zeroize pending Noise handshake state and mark handshake inactive._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**wg\_consume\_response\_pt**](#function-wg_consume_response_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Cooperative child protothread driving Noise\_IKpsk2 handshake response consumption._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_decrypt\_and\_hash**](#function-wg_decrypt_and_hash) (uint8\_t h, const uint8\_t k, const uint8\_t \* ct, size\_t ct\_len, const uint8\_t tag, uint8\_t \* plain) <br>_Decrypt-and-hash: verify + decrypt, mix ciphertext+tag into hash._  |
|  uint8\_t | [**wg\_dh\_finish\_mix\_key**](#function-wg_dh_finish_mix_key) (uint8\_t ck, uint8\_t k, uint8\_t dh) <br>_Mix completed X25519 DH secret into Noise chaining key, verify non-zero, and scrub._  |
|  void | [**wg\_encrypt\_and\_hash**](#function-wg_encrypt_and_hash) (uint8\_t h, const uint8\_t k, const uint8\_t \* plain, size\_t plain\_len, uint8\_t \* ct, uint8\_t tag) <br>_Encrypt-and-hash: encrypt plaintext, mix ciphertext+tag into hash._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_handle\_transport**](#function-wg_handle_transport) ([**SYN\_WG**](structSYN__WG.md) \* wg, const uint8\_t \* msg, size\_t len) <br>_Decrypt and deliver an incoming WireGuard transport message._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_handshake\_timed\_out**](#function-wg_handshake_timed_out) (const [**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Check if pending handshake initiation has exceeded rekey timeout._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_has\_work**](#function-wg_has_work) (const [**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Check if WireGuard task has pending work to process._  |
|  void | [**wg\_hkdf2**](#function-wg_hkdf2) (uint8\_t out1, uint8\_t out2, const uint8\_t ck, const uint8\_t \* input, size\_t input\_len) <br> |
|  void | [**wg\_hkdf3\_step1**](#function-wg_hkdf3_step1) (uint8\_t prk, uint8\_t out1, const uint8\_t ck, const uint8\_t \* input, size\_t input\_len) <br>_HKDF3 step 1: extract PRK and expand T1 = HMAC(PRK, 0x01)._  |
|  void | [**wg\_hkdf3\_step2**](#function-wg_hkdf3_step2) (uint8\_t out2, uint8\_t out3, uint8\_t prk, const uint8\_t out1) <br>_HKDF3 step 2: expand T2 and T3 from PRK and T1, then scrub PRK._  |
|  void | [**wg\_hs\_work\_wipe**](#function-wg_hs_work_wipe) ([**SYN\_WgHsWork**](structSYN__WgHsWork.md) \* w) <br>_Securely wipe cooperative handshake scratchpad and X25519 context._  |
|  void | [**wg\_init\_enc\_static**](#function-wg_init_enc_static) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Encrypt static public key under dh\_es AEAD key and initialize dh\_ss X25519._  |
|  void | [**wg\_init\_enc\_ts**](#function-wg_init_enc_ts) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Encrypt TAI64N timestamp under dh\_ss AEAD key into hs\_work.enc\_ts._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_init\_mac\_and\_send**](#function-wg_init_mac_and_send) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Compute MAC1, transmit 148-byte initiation datagram, and commit hs\_\* state._  |
|  void | [**wg\_init\_mix\_eph\_key**](#function-wg_init_mix_eph_key) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Mix initiator ephemeral public key into Noise chaining key._  |
|  void | [**wg\_init\_mix\_es**](#function-wg_init_mix_es) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Mix dh\_es shared secret into Noise chaining key and stage AEAD key in tmp\_key._  |
|  void | [**wg\_init\_mix\_ss**](#function-wg_init_mix_ss) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Mix dh\_ss shared secret into Noise chaining key and stage AEAD key in tmp\_key._  |
|  void | [**wg\_init\_noise\_and\_eph**](#function-wg_init_noise_and_eph) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Initialize Noise\_IKpsk2 transcript state and start ephemeral X25519 pubkey derivation._  |
|  void | [**wg\_init\_noise\_state**](#function-wg_init_noise_state) (uint8\_t ck, uint8\_t h, const uint8\_t peer\_pub) <br>_Initialize Noise\_IKpsk2 chaining key and handshake hash with responder static public key._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_init\_prepare**](#function-wg_init_prepare) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Generate ephemeral private key and sender index via CSPRNG._  |
|  void | [**wg\_init\_start**](#function-wg_init_start) ([**SYN\_WG**](structSYN__WG.md) \* wg, [**bool**](syn__defs_8h.md#enum-bool) is\_rekey) <br>_Arm the cooperative handshake initiation protothread in hs\_work._  |
|  void | [**wg\_init\_start\_dh\_es**](#function-wg_init_start_dh_es) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Mix initiator ephemeral public key into Noise hash and start dh\_es X25519._  |
|  void | [**wg\_mac1**](#function-wg_mac1) (uint8\_t mac, const uint8\_t peer\_pub, const uint8\_t \* msg, size\_t msg\_len) <br>_Compute mac1 = keyed BLAKE2s(HASH("mac1----" \|\| peer\_pub), msg)._  |
|  void | [**wg\_mix\_hash**](#function-wg_mix_hash) (uint8\_t h, const void \* data, size\_t len) <br>_Mix hash: H = BLAKE2s(H \|\| data)._  |
|  void | [**wg\_mix\_key**](#function-wg_mix_key) (uint8\_t ck, uint8\_t k, const uint8\_t \* input, size\_t len) <br>_Mix key: (CK, k) = HKDF(CK, input)._  |
|  void | [**wg\_process\_datagram**](#function-wg_process_datagram) ([**SYN\_WG**](structSYN__WG.md) \* wg, size\_t len) <br>_Dispatch a received WireGuard UDP datagram by message type._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_rekey\_due**](#function-wg_rekey_due) (const [**SYN\_WG**](structSYN__WG.md) \* wg, uint32\_t now) <br>_Check whether hitless rekey initiation or rekey retry is due on an active session._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_rekey\_rx\_due**](#function-wg_rekey_rx_due) (const [**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Check whether an inbound datagram should take turn ahead of an active rekey step._  |
|  void | [**wg\_replay\_commit**](#function-wg_replay_commit) ([**SYN\_WgSession**](structSYN__WgSession.md) \* s, uint64\_t counter) <br>_Commit counter into anti-replay sliding window bitmap._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_replay\_test**](#function-wg_replay_test) (const [**SYN\_WgSession**](structSYN__WgSession.md) \* s, uint64\_t counter) <br>_Anti-replay check without state mutation._  |
|  void | [**wg\_resp\_after\_dh\_ee**](#function-wg_resp_after_dh_ee) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Mix dh\_ee shared secret and initialize dh\_se X25519 scalar multiplication._  |
|  void | [**wg\_resp\_commit\_session**](#function-wg_resp_commit_session) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Atomically install active transport session, scrub handshake state, and emit keepalive._  |
|  void | [**wg\_resp\_derive\_keys**](#function-wg_resp_derive_keys) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Derive transport session send/recv keys into hs\_work staging buffers._  |
|  void | [**wg\_resp\_mix\_eph\_key**](#function-wg_resp_mix_eph_key) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Mix responder ephemeral key into Noise chaining key._  |
|  void | [**wg\_resp\_mix\_se**](#function-wg_resp_mix_se) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Mix dh\_se shared secret into Noise chaining key._  |
|  void | [**wg\_resp\_psk\_step1**](#function-wg_resp_psk_step1) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Extract PSK PRK into eph\_pub scratch and derive first HKDF output into chaining key._  |
|  void | [**wg\_resp\_psk\_step2**](#function-wg_resp_psk_step2) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Derive PSK tau and empty-payload AEAD key from staged PRK and mix tau into Noise hash._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_resp\_start**](#function-wg_resp_start) ([**SYN\_WG**](structSYN__WG.md) \* wg, const uint8\_t \* msg, size\_t len) <br>_Validate a 92B handshake response header + MAC1 and arm wg\_consume\_response\_pt._  |
|  void | [**wg\_resp\_start\_dh\_ee**](#function-wg_resp_start_dh_ee) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Mix responder ephemeral key into Noise hash and initialize dh\_ee X25519._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_resp\_verify\_tag**](#function-wg_resp_verify_tag) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Verify response empty-payload AEAD tag before deriving transport session keys._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**wg\_send\_initiation\_pt**](#function-wg_send_initiation_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Cooperative child protothread driving Noise\_IKpsk2 handshake initiation._  |
|  void | [**wg\_send\_keepalive**](#function-wg_send_keepalive) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Send an empty (keepalive) transport message._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_socket\_readable**](#function-wg_socket_readable) (const [**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Check if WireGuard UDP socket is open and readable._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**wg\_step\_active\_hs**](#function-wg_step_active_hs) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Step the active initiation or response protothread by one cooperative slice._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_step\_established\_timers**](#function-wg_step_established_timers) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Evaluate established-session rekey/reject expiry and keepalive timers._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_step\_hs\_or\_init**](#function-wg_step_hs_or_init) ([**SYN\_WG**](structSYN__WG.md) \* wg, [**bool**](syn__defs_8h.md#enum-bool) \* need\_delay) <br>_Step active handshake protothread or start initiation when disconnected._  |
|  [**WG\_StepAct**](syn__wg_8c.md#enum-wg_stepact) | [**wg\_step\_recv**](#function-wg_step_recv) ([**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Receive and process at most one UDP datagram on the WireGuard socket._  |
|  void | [**wg\_tai64n**](#function-wg_tai64n) (uint8\_t out, const [**SYN\_SNTP**](structSYN__SNTP.md) \* sntp) <br>_Write a 12-byte TAI64N timestamp from NTP time._  |
|  [**WG\_StepAct**](syn__wg_8c.md#enum-wg_stepact) | [**wg\_task\_step\_once**](#function-wg_task_step_once) ([**SYN\_WG**](structSYN__WG.md) \* wg, uint32\_t \* delay\_ms) <br>_Execute one bounded cooperative WireGuard slice (at most 1 crypto/packet step)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_task\_valid**](#function-wg_task_valid) (const [**SYN\_PT**](structSYN__PT.md) \* pt, const [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Validate task arguments for syn\_wg\_task._  |
|  size\_t | [**wg\_unpad\_ip\_len**](#function-wg_unpad_ip_len) (const uint8\_t \* plain, size\_t ct\_len) <br>_Strip WireGuard 16-byte transport padding using inner IPv4/IPv6 header length._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wg\_verify\_response\_mac1**](#function-wg_verify_response_mac1) (const [**SYN\_WG**](structSYN__WG.md) \* wg, const uint8\_t \* msg) <br>_Verify mac1 on an incoming 92-byte handshake response before DH work._  |


























## Detailed Description


Implements the WireGuard protocol per the whitepaper: [https://www.wireguard.com/papers/wireguard.pdf](https://www.wireguard.com/papers/wireguard.pdf)


Noise protocol pattern: IKpsk2 Initiator (us) knows responder's static public key. PSK mixed in at handshake step 2. 


    
## Public Types Documentation




### enum WG\_StepAct 

_Next cooperative scheduling action returned by_ [_**wg\_task\_step\_once()**_](syn__wg_8c.md#function-wg_task_step_once) _._
```C++
enum WG_StepAct {
    WG_ACT_WAIT = 0,
    WG_ACT_MORE,
    WG_ACT_PASS,
    WG_ACT_DELAY
};
```




<hr>
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




### function syn\_wg\_disconnect 

_Disconnect the WireGuard tunnel and close socket._ 
```C++
void syn_wg_disconnect (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function syn\_wg\_get\_stats 

_Retrieve WireGuard tunnel metrics and statistics._ 
```C++
SYN_Status syn_wg_get_stats (
    const SYN_WG * wg,
    SYN_WgStats * stats
) 
```





**Parameters:**


* `wg` Client context. 
* `stats` [out] Output statistics structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



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



### function syn\_ct\_memeq 

_Constant-time byte-slice equality check._ 
```C++
static bool syn_ct_memeq (
    const void * a,
    const void * b,
    size_t n
) 
```





**Parameters:**


* `a` First buffer. 
* `b` Second buffer. 
* `n` Byte count. 



**Returns:**

true if all n bytes match, false otherwise. 





        

<hr>



### function syn\_secure\_zero 

_Zero sensitive memory without dead-store elimination._ 
```C++
static void syn_secure_zero (
    void * v,
    size_t n
) 
```





**Parameters:**


* `v` Target buffer. 
* `n` Byte count. 




        

<hr>



### function wg\_clear\_handshake\_state 

_Zeroize pending Noise handshake state and mark handshake inactive._ 
```C++
static void wg_clear_handshake_state (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_consume\_response\_pt 

_Cooperative child protothread driving Noise\_IKpsk2 handshake response consumption._ 
```C++
static SYN_PT_Status wg_consume_response_pt (
    SYN_PT * pt,
    SYN_WG * wg
) 
```





**Parameters:**


* `pt` Handshake protothread control block. 
* `wg` Client context. 



**Returns:**

PT\_MORE while slicing, PT\_ENDED on session establishment, or PT\_EXITED on auth failure. 





        

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



### function wg\_dh\_finish\_mix\_key 

_Mix completed X25519 DH secret into Noise chaining key, verify non-zero, and scrub._ 
```C++
static uint8_t wg_dh_finish_mix_key (
    uint8_t ck,
    uint8_t k,
    uint8_t dh
) 
```





**Parameters:**


* `ck` [in,out] Running 32-byte Noise chaining key. 
* `k` [out] Derived 32-byte AEAD key. 
* `dh` [in,out] 32-byte completed X25519 shared secret (zeroized on return). 



**Returns:**

0U if the 32-byte shared secret is all-zero (RFC 7748 Section 6.1), 1U otherwise. 





        

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



### function wg\_handshake\_timed\_out 

_Check if pending handshake initiation has exceeded rekey timeout._ 
```C++
static bool wg_handshake_timed_out (
    const SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 



**Returns:**

true if in SYN\_WG\_HANDSHAKE\_INIT and timed out. 





        

<hr>



### function wg\_has\_work 

_Check if WireGuard task has pending work to process._ 
```C++
static bool wg_has_work (
    const SYN_WG * wg
) 
```





**Parameters:**


* `wg` Pointer to WireGuard state structure. 



**Returns:**

true if work pending, false otherwise. 





        

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



### function wg\_hkdf3\_step1 

_HKDF3 step 1: extract PRK and expand T1 = HMAC(PRK, 0x01)._ 
```C++
static void wg_hkdf3_step1 (
    uint8_t prk,
    uint8_t out1,
    const uint8_t ck,
    const uint8_t * input,
    size_t input_len
) 
```





**Parameters:**


* `prk` [out] 32-byte extracted pseudorandom key. 
* `out1` [out] First 32-byte output block (may alias ck). 
* `ck` 32-byte chaining key. 
* `input` Input keying material. 
* `input_len` Input length in bytes. 




        

<hr>



### function wg\_hkdf3\_step2 

_HKDF3 step 2: expand T2 and T3 from PRK and T1, then scrub PRK._ 
```C++
static void wg_hkdf3_step2 (
    uint8_t out2,
    uint8_t out3,
    uint8_t prk,
    const uint8_t out1
) 
```





**Parameters:**


* `out2` [out] Second 32-byte output block. 
* `out3` [out] Third 32-byte output block. 
* `prk` [in,out] 32-byte PRK from step 1 (zeroized on return). 
* `out1` First 32-byte output block from step 1. 




        

<hr>



### function wg\_hs\_work\_wipe 

_Securely wipe cooperative handshake scratchpad and X25519 context._ 
```C++
static void wg_hs_work_wipe (
    SYN_WgHsWork * w
) 
```





**Parameters:**


* `w` Handshake work scratchpad. 




        

<hr>



### function wg\_init\_enc\_static 

_Encrypt static public key under dh\_es AEAD key and initialize dh\_ss X25519._ 
```C++
static void wg_init_enc_static (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_init\_enc\_ts 

_Encrypt TAI64N timestamp under dh\_ss AEAD key into hs\_work.enc\_ts._ 
```C++
static void wg_init_enc_ts (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_init\_mac\_and\_send 

_Compute MAC1, transmit 148-byte initiation datagram, and commit hs\_\* state._ 
```C++
static bool wg_init_mac_and_send (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 



**Returns:**

true if DH outputs were non-zero and the 148-byte initiation datagram was transmitted. 





        

<hr>



### function wg\_init\_mix\_eph\_key 

_Mix initiator ephemeral public key into Noise chaining key._ 
```C++
static void wg_init_mix_eph_key (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_init\_mix\_es 

_Mix dh\_es shared secret into Noise chaining key and stage AEAD key in tmp\_key._ 
```C++
static void wg_init_mix_es (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_init\_mix\_ss 

_Mix dh\_ss shared secret into Noise chaining key and stage AEAD key in tmp\_key._ 
```C++
static void wg_init_mix_ss (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_init\_noise\_and\_eph 

_Initialize Noise\_IKpsk2 transcript state and start ephemeral X25519 pubkey derivation._ 
```C++
static void wg_init_noise_and_eph (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_init\_noise\_state 

_Initialize Noise\_IKpsk2 chaining key and handshake hash with responder static public key._ 
```C++
static void wg_init_noise_state (
    uint8_t ck,
    uint8_t h,
    const uint8_t peer_pub
) 
```





**Parameters:**


* `ck` [out] 32-byte initial chaining key. 
* `h` [out] 32-byte initial handshake hash. 
* `peer_pub` 32-byte responder static public key. 




        

<hr>



### function wg\_init\_prepare 

_Generate ephemeral private key and sender index via CSPRNG._ 
```C++
static bool wg_init_prepare (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 



**Returns:**

true if 36 random bytes were generated into hs\_work. 





        

<hr>



### function wg\_init\_start 

_Arm the cooperative handshake initiation protothread in hs\_work._ 
```C++
static void wg_init_start (
    SYN_WG * wg,
    bool is_rekey
) 
```





**Parameters:**


* `wg` Client context. 
* `is_rekey` true when initiating a background hitless rekey. 




        

<hr>



### function wg\_init\_start\_dh\_es 

_Mix initiator ephemeral public key into Noise hash and start dh\_es X25519._ 
```C++
static void wg_init_start_dh_es (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

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



### function wg\_process\_datagram 

_Dispatch a received WireGuard UDP datagram by message type._ 
```C++
static void wg_process_datagram (
    SYN_WG * wg,
    size_t len
) 
```





**Parameters:**


* `wg` Client context. 
* `len` Datagram byte length (&gt; 0). 




        

<hr>



### function wg\_rekey\_due 

_Check whether hitless rekey initiation or rekey retry is due on an active session._ 
```C++
static bool wg_rekey_due (
    const SYN_WG * wg,
    uint32_t now
) 
```





**Parameters:**


* `wg` Client context. 
* `now` Current millisecond tick. 



**Returns:**

true if a rekey initiation packet should be transmitted. 





        

<hr>



### function wg\_rekey\_rx\_due 

_Check whether an inbound datagram should take turn ahead of an active rekey step._ 
```C++
static bool wg_rekey_rx_due (
    const SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 



**Returns:**

true when established, mid-rekey, socket readable, and RX turn is active. 





        

<hr>



### function wg\_replay\_commit 

_Commit counter into anti-replay sliding window bitmap._ 
```C++
static void wg_replay_commit (
    SYN_WgSession * s,
    uint64_t counter
) 
```





**Parameters:**


* `s` Active session. 
* `counter` Validated counter value. 




        

<hr>



### function wg\_replay\_test 

_Anti-replay check without state mutation._ 
```C++
static bool wg_replay_test (
    const SYN_WgSession * s,
    uint64_t counter
) 
```





**Parameters:**


* `s` Active session. 
* `counter` Received counter value. 



**Returns:**

true if counter is new and within window. 





        

<hr>



### function wg\_resp\_after\_dh\_ee 

_Mix dh\_ee shared secret and initialize dh\_se X25519 scalar multiplication._ 
```C++
static void wg_resp_after_dh_ee (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_resp\_commit\_session 

_Atomically install active transport session, scrub handshake state, and emit keepalive._ 
```C++
static void wg_resp_commit_session (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_resp\_derive\_keys 

_Derive transport session send/recv keys into hs\_work staging buffers._ 
```C++
static void wg_resp_derive_keys (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_resp\_mix\_eph\_key 

_Mix responder ephemeral key into Noise chaining key._ 
```C++
static void wg_resp_mix_eph_key (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_resp\_mix\_se 

_Mix dh\_se shared secret into Noise chaining key._ 
```C++
static void wg_resp_mix_se (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_resp\_psk\_step1 

_Extract PSK PRK into eph\_pub scratch and derive first HKDF output into chaining key._ 
```C++
static void wg_resp_psk_step1 (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_resp\_psk\_step2 

_Derive PSK tau and empty-payload AEAD key from staged PRK and mix tau into Noise hash._ 
```C++
static void wg_resp_psk_step2 (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_resp\_start 

_Validate a 92B handshake response header + MAC1 and arm wg\_consume\_response\_pt._ 
```C++
static bool wg_resp_start (
    SYN_WG * wg,
    const uint8_t * msg,
    size_t len
) 
```





**Parameters:**


* `wg` Client context. 
* `msg` Received response buffer. 
* `len` Received byte count. 



**Returns:**

true if MAC1 and receiver index verified and response fields staged in hs\_work. 





        

<hr>



### function wg\_resp\_start\_dh\_ee 

_Mix responder ephemeral key into Noise hash and initialize dh\_ee X25519._ 
```C++
static void wg_resp_start_dh_ee (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 




        

<hr>



### function wg\_resp\_verify\_tag 

_Verify response empty-payload AEAD tag before deriving transport session keys._ 
```C++
static bool wg_resp_verify_tag (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 



**Returns:**

true if DH outputs were non-zero and response tag authenticated, false on forged packet. 





        

<hr>



### function wg\_send\_initiation\_pt 

_Cooperative child protothread driving Noise\_IKpsk2 handshake initiation._ 
```C++
static SYN_PT_Status wg_send_initiation_pt (
    SYN_PT * pt,
    SYN_WG * wg
) 
```





**Parameters:**


* `pt` Handshake protothread control block. 
* `wg` Client context. 



**Returns:**

PT\_MORE while slicing, PT\_ENDED on transmission, or PT\_EXITED on failure. 





        

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



### function wg\_socket\_readable 

_Check if WireGuard UDP socket is open and readable._ 
```C++
static bool wg_socket_readable (
    const SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 



**Returns:**

true if socket has pending data or error. 





        

<hr>



### function wg\_step\_active\_hs 

_Step the active initiation or response protothread by one cooperative slice._ 
```C++
static SYN_PT_Status wg_step_active_hs (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context with hs\_work.active == true. 



**Returns:**

Protothread status (PT\_MORE while running, PT\_ENDED on success, PT\_EXITED on error). 





        

<hr>



### function wg\_step\_established\_timers 

_Evaluate established-session rekey/reject expiry and keepalive timers._ 
```C++
static bool wg_step_established_timers (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 



**Returns:**

true if session expired or rekey protothread started, false otherwise. 





        

<hr>



### function wg\_step\_hs\_or\_init 

_Step active handshake protothread or start initiation when disconnected._ 
```C++
static bool wg_step_hs_or_init (
    SYN_WG * wg,
    bool * need_delay
) 
```





**Parameters:**


* `wg` Client context. 
* `need_delay` [out] Set true if initial initiation failed and requires a 5s retry delay. 



**Returns:**

true if a handshake slice was executed, false if no handshake action was needed. 





        

<hr>



### function wg\_step\_recv 

_Receive and process at most one UDP datagram on the WireGuard socket._ 
```C++
static WG_StepAct wg_step_recv (
    SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 



**Returns:**

Cooperative scheduling action after polling UDP RX. 





        

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



### function wg\_task\_step\_once 

_Execute one bounded cooperative WireGuard slice (at most 1 crypto/packet step)._ 
```C++
static WG_StepAct wg_task_step_once (
    SYN_WG * wg,
    uint32_t * delay_ms
) 
```





**Parameters:**


* `wg` Client context. 
* `delay_ms` [out] Retry backoff duration in milliseconds when WG\_ACT\_DELAY is returned. 



**Returns:**

Next protothread action for [**syn\_wg\_task()**](syn__wg_8c.md#function-syn_wg_task). 





        

<hr>



### function wg\_task\_valid 

_Validate task arguments for syn\_wg\_task._ 
```C++
static bool wg_task_valid (
    const SYN_PT * pt,
    const SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread pointer. 
* `task` Task descriptor. 



**Returns:**

true if arguments and SNTP pointer are non-NULL. 





        

<hr>



### function wg\_unpad\_ip\_len 

_Strip WireGuard 16-byte transport padding using inner IPv4/IPv6 header length._ 
```C++
static size_t wg_unpad_ip_len (
    const uint8_t * plain,
    size_t ct_len
) 
```





**Parameters:**


* `plain` Decrypted transport payload buffer. 
* `ct_len` Decrypted ciphertext byte length. 



**Returns:**

Actual IP packet length, or ct\_len for non-IP raw payloads. 





        

<hr>



### function wg\_verify\_response\_mac1 

_Verify mac1 on an incoming 92-byte handshake response before DH work._ 
```C++
static bool wg_verify_response_mac1 (
    const SYN_WG * wg,
    const uint8_t * msg
) 
```





**Parameters:**


* `wg` Client context. 
* `msg` Received 92-byte response message. 



**Returns:**

true if mac1 authenticates against our static public key. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_wg.c`

