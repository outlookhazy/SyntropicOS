

# File syn\_wg.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_wg.h**](syn__wg_8h.md)

[Go to the source code of this file](syn__wg_8h_source.md)

_WireGuard VPN client — Noise IK, pure C99, cooperative._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_socket.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_task.h"`
* `#include "syn_sntp.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_WG**](structSYN__WG.md) <br>_WireGuard client context — caller-owned._  |
| struct | [**SYN\_WgConfig**](structSYN__WgConfig.md) <br>_WireGuard peer configuration — set once at init._  |
| struct | [**SYN\_WgSession**](structSYN__WgSession.md) <br>_Active session derived from a completed handshake._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_WgState**](#enum-syn_wgstate)  <br>_WireGuard client connection state._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_wg\_init**](#function-syn_wg_init) ([**SYN\_WG**](structSYN__WG.md) \* wg, const [**SYN\_WgConfig**](structSYN__WgConfig.md) \* config, [**SYN\_SNTP**](structSYN__SNTP.md) \* sntp, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize the WireGuard client._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_wg\_send**](#function-syn_wg_send) ([**SYN\_WG**](structSYN__WG.md) \* wg, const uint8\_t \* ip\_packet, size\_t len) <br>_Send an IP packet through the WireGuard tunnel._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_wg\_task**](#function-syn_wg_task) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Cooperative protothread task — drives the WireGuard client._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_wg\_is\_established**](#function-syn_wg_is_established) (const [**SYN\_WG**](structSYN__WG.md) \* wg) <br>_Check if the tunnel is established and ready for data._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_WG\_INITIATION\_SIZE**](syn__wg_8h.md#define-syn_wg_initiation_size)  `148`<br> |
| define  | [**SYN\_WG\_KEEPALIVE\_TIMEOUT**](syn__wg_8h.md#define-syn_wg_keepalive_timeout)  `10`<br> |
| define  | [**SYN\_WG\_MSG\_COOKIE**](syn__wg_8h.md#define-syn_wg_msg_cookie)  `3`<br> |
| define  | [**SYN\_WG\_MSG\_INITIATION**](syn__wg_8h.md#define-syn_wg_msg_initiation)  `1`<br> |
| define  | [**SYN\_WG\_MSG\_RESPONSE**](syn__wg_8h.md#define-syn_wg_msg_response)  `2`<br> |
| define  | [**SYN\_WG\_MSG\_TRANSPORT**](syn__wg_8h.md#define-syn_wg_msg_transport)  `4`<br> |
| define  | [**SYN\_WG\_MTU**](syn__wg_8h.md#define-syn_wg_mtu)  `1420`<br> |
| define  | [**SYN\_WG\_REJECT\_AFTER\_TIME**](syn__wg_8h.md#define-syn_wg_reject_after_time)  `180`<br> |
| define  | [**SYN\_WG\_REKEY\_AFTER\_TIME**](syn__wg_8h.md#define-syn_wg_rekey_after_time)  `120`<br> |
| define  | [**SYN\_WG\_REKEY\_TIMEOUT**](syn__wg_8h.md#define-syn_wg_rekey_timeout)  `5`<br> |
| define  | [**SYN\_WG\_RESPONSE\_SIZE**](syn__wg_8h.md#define-syn_wg_response_size)  `92`<br> |
| define  | [**SYN\_WG\_TRANSPORT\_OVERHEAD**](syn__wg_8h.md#define-syn_wg_transport_overhead)  `32`<br> |

## Detailed Description


Implements a WireGuard client as a cooperative protothread task. Handles the Noise\_IKpsk2 handshake, ChaCha20-Poly1305 transport encryption, keepalive timers, and rekeying.


The client connects to a single WireGuard peer (server) over UDP. Decrypted IP packets are delivered to user code via a callback. User code sends packets into the tunnel via [**syn\_wg\_send()**](syn__wg_8h.md#function-syn_wg_send).


No TUN device, no host routing — all traffic is handled in userspace. The host OS only sees a single UDP socket.


Requires: BLAKE2S, CHACHA20POLY1305, X25519, SNTP.




**
**


```C++
static SYN_WG wg;
static uint8_t rx[1500], tx[1500];

SYN_WgConfig cfg = {
    .endpoint = { .ip = {1,2,3,4}, .port = 51820 },
    .keepalive_interval_s = 25,
};
memcpy(cfg.private_key, my_key, 32);
memcpy(cfg.peer_public_key, server_key, 32);

syn_wg_init(&wg, &cfg, &sntp, rx, sizeof(rx), tx, sizeof(tx));
wg.on_recv = my_handler;

syn_task_create(&tasks[1], "wg", syn_wg_task, 1, &wg);
```
 





    
## Public Types Documentation




### enum SYN\_WgState 

_WireGuard client connection state._ 
```C++
enum SYN_WgState {
    SYN_WG_DISCONNECTED,
    SYN_WG_HANDSHAKE_INIT,
    SYN_WG_ESTABLISHED
};
```




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




### function syn\_wg\_is\_established 

_Check if the tunnel is established and ready for data._ 
```C++
static inline bool syn_wg_is_established (
    const SYN_WG * wg
) 
```





**Parameters:**


* `wg` Client context. 



**Returns:**

true if the session is active. 





        

<hr>
## Macro Definition Documentation





### define SYN\_WG\_INITIATION\_SIZE 

```C++
#define SYN_WG_INITIATION_SIZE `148`
```



Message sizes. Size of initiation message 


        

<hr>



### define SYN\_WG\_KEEPALIVE\_TIMEOUT 

```C++
#define SYN_WG_KEEPALIVE_TIMEOUT `10`
```



Send keepalive if no outbound (s) 
 


        

<hr>



### define SYN\_WG\_MSG\_COOKIE 

```C++
#define SYN_WG_MSG_COOKIE `3`
```



Cookie reply message 
 


        

<hr>



### define SYN\_WG\_MSG\_INITIATION 

```C++
#define SYN_WG_MSG_INITIATION `1`
```



WireGuard message types. Handshake initiation message 


        

<hr>



### define SYN\_WG\_MSG\_RESPONSE 

```C++
#define SYN_WG_MSG_RESPONSE `2`
```



Handshake response message 
 


        

<hr>



### define SYN\_WG\_MSG\_TRANSPORT 

```C++
#define SYN_WG_MSG_TRANSPORT `4`
```



Encrypted transport message 


        

<hr>



### define SYN\_WG\_MTU 

```C++
#define SYN_WG_MTU `1420`
```



Default tunnel MTU (inner IP packet before encryption). 


        

<hr>



### define SYN\_WG\_REJECT\_AFTER\_TIME 

```C++
#define SYN_WG_REJECT_AFTER_TIME `180`
```



Drop session after this many seconds 
 


        

<hr>



### define SYN\_WG\_REKEY\_AFTER\_TIME 

```C++
#define SYN_WG_REKEY_AFTER_TIME `120`
```



WireGuard protocol timers (seconds). Initiate rekey after this many seconds 


        

<hr>



### define SYN\_WG\_REKEY\_TIMEOUT 

```C++
#define SYN_WG_REKEY_TIMEOUT `5`
```



Retry handshake if no response (s) 
 


        

<hr>



### define SYN\_WG\_RESPONSE\_SIZE 

```C++
#define SYN_WG_RESPONSE_SIZE `92`
```



Size of response message 
 


        

<hr>



### define SYN\_WG\_TRANSPORT\_OVERHEAD 

```C++
#define SYN_WG_TRANSPORT_OVERHEAD `32`
```



WireGuard transport overhead: type(4) + receiver(4) + counter(8) + tag(16) = 32 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_wg.h`

