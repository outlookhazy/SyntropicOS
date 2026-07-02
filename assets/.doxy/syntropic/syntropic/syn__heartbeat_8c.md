

# File syn\_heartbeat.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_heartbeat.c**](syn__heartbeat_8c.md)

[Go to the source code of this file](syn__heartbeat_8c_source.md)

_Heartbeat / keep-alive implementation._ 

* `#include "syn_heartbeat.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_heartbeat\_add\_peer**](#function-syn_heartbeat_add_peer) ([**SYN\_Heartbeat**](structSYN__Heartbeat.md) \* hb, uint8\_t node\_id) <br>_Add a peer to monitor._  |
|  void | [**syn\_heartbeat\_init**](#function-syn_heartbeat_init) ([**SYN\_Heartbeat**](structSYN__Heartbeat.md) \* hb, [**SYN\_Router**](structSYN__Router.md) \* router, [**SYN\_HB\_Peer**](structSYN__HB__Peer.md) \* peers, uint8\_t peer\_cap, uint32\_t interval\_ms, uint32\_t timeout\_ms) <br>_Initialize heartbeat system._  |
|  void | [**syn\_heartbeat\_on\_peer\_found**](#function-syn_heartbeat_on_peer_found) ([**SYN\_Heartbeat**](structSYN__Heartbeat.md) \* hb, [**SYN\_HB\_Callback**](syn__heartbeat_8h.md#typedef-syn_hb_callback) cb, void \* ctx) <br>_Register peer-found callback._  |
|  void | [**syn\_heartbeat\_on\_peer\_lost**](#function-syn_heartbeat_on_peer_lost) ([**SYN\_Heartbeat**](structSYN__Heartbeat.md) \* hb, [**SYN\_HB\_Callback**](syn__heartbeat_8h.md#typedef-syn_hb_callback) cb, void \* ctx) <br>_Register peer-lost callback._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_heartbeat\_peer\_alive**](#function-syn_heartbeat_peer_alive) (const [**SYN\_Heartbeat**](structSYN__Heartbeat.md) \* hb, uint8\_t node\_id) <br>_Check if a specific peer is alive._  |
|  void | [**syn\_heartbeat\_set\_errlog**](#function-syn_heartbeat_set_errlog) ([**SYN\_Heartbeat**](structSYN__Heartbeat.md) \* hb, [**SYN\_ErrLog**](structSYN__ErrLog.md) \* errlog) <br>_Attach error log for peer-lost/found events._  |
|  void | [**syn\_heartbeat\_update**](#function-syn_heartbeat_update) ([**SYN\_Heartbeat**](structSYN__Heartbeat.md) \* hb) <br>_Update — send heartbeat + check peers._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**hb\_rx\_handler**](#function-hb_rx_handler) (const [**SYN\_Packet**](structSYN__Packet.md) \* pkt, void \* ctx) <br>_Router callback for incoming heartbeat packets._  |


























## Public Functions Documentation




### function syn\_heartbeat\_add\_peer 

_Add a peer to monitor._ 
```C++
bool syn_heartbeat_add_peer (
    SYN_Heartbeat * hb,
    uint8_t node_id
) 
```





**Parameters:**


* `hb` Heartbeat instance. 
* `node_id` Peer node ID. 



**Returns:**

true if added, false if peer table is full. 





        

<hr>



### function syn\_heartbeat\_init 

_Initialize heartbeat system._ 
```C++
void syn_heartbeat_init (
    SYN_Heartbeat * hb,
    SYN_Router * router,
    SYN_HB_Peer * peers,
    uint8_t peer_cap,
    uint32_t interval_ms,
    uint32_t timeout_ms
) 
```



Automatically registers a handler for SYN\_MSG\_HEARTBEAT on the router.




**Parameters:**


* `hb` Heartbeat instance. 
* `router` Packet router. 
* `peers` Caller-owned peer array. 
* `peer_cap` Array capacity. 
* `interval_ms` How often to send our heartbeat. 
* `timeout_ms` How long before a peer is "dead". 




        

<hr>



### function syn\_heartbeat\_on\_peer\_found 

_Register peer-found callback._ 
```C++
void syn_heartbeat_on_peer_found (
    SYN_Heartbeat * hb,
    SYN_HB_Callback cb,
    void * ctx
) 
```





**Parameters:**


* `hb` Heartbeat instance. 
* `cb` Callback function. 
* `ctx` User context. 




        

<hr>



### function syn\_heartbeat\_on\_peer\_lost 

_Register peer-lost callback._ 
```C++
void syn_heartbeat_on_peer_lost (
    SYN_Heartbeat * hb,
    SYN_HB_Callback cb,
    void * ctx
) 
```





**Parameters:**


* `hb` Heartbeat instance. 
* `cb` Callback function. 
* `ctx` User context. 




        

<hr>



### function syn\_heartbeat\_peer\_alive 

_Check if a specific peer is alive._ 
```C++
bool syn_heartbeat_peer_alive (
    const SYN_Heartbeat * hb,
    uint8_t node_id
) 
```





**Parameters:**


* `hb` Heartbeat instance. 
* `node_id` Peer node ID. 



**Returns:**

true if the peer is alive. 





        

<hr>



### function syn\_heartbeat\_set\_errlog 

_Attach error log for peer-lost/found events._ 
```C++
void syn_heartbeat_set_errlog (
    SYN_Heartbeat * hb,
    SYN_ErrLog * errlog
) 
```





**Parameters:**


* `hb` Heartbeat instance. 
* `errlog` Error log instance. 




        

<hr>



### function syn\_heartbeat\_update 

_Update — send heartbeat + check peers._ 
```C++
void syn_heartbeat_update (
    SYN_Heartbeat * hb
) 
```



Call from your main loop.




**Parameters:**


* `hb` Heartbeat instance. 




        

<hr>
## Public Static Functions Documentation




### function hb\_rx\_handler 

_Router callback for incoming heartbeat packets._ 
```C++
static void hb_rx_handler (
    const SYN_Packet * pkt,
    void * ctx
) 
```





**Parameters:**


* `pkt` Received packet. 
* `ctx` Heartbeat context ([**SYN\_Heartbeat**](structSYN__Heartbeat.md) \*). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_heartbeat.c`

