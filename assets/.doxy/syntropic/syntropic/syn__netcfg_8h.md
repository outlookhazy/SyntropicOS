

# File syn\_netcfg.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_netcfg.h**](syn__netcfg_8h.md)

[Go to the source code of this file](syn__netcfg_8h_source.md)

_Unified Zero-Heap Network IP Address Manager._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/net/syn_autoip.h"`
* `#include "syntropic/net/syn_dhcp.h"`
* `#include "syntropic/net/syn_eth.h"`
* `#include "syntropic/pt/syn_pt.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_NETCFG**](structSYN__NETCFG.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_NETCFG\_LinkCb**](#typedef-syn_netcfg_linkcb)  <br> |
| enum  | [**SYN\_NETCFG\_LinkState**](#enum-syn_netcfg_linkstate)  <br> |
| enum  | [**SYN\_NETCFG\_Mode**](#enum-syn_netcfg_mode)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netcfg\_init**](#function-syn_netcfg_init) ([**SYN\_NETCFG**](structSYN__NETCFG.md) \* netcfg, [**SYN\_NETCFG\_Mode**](syn__netcfg_8h.md#enum-syn_netcfg_mode) mode, const uint8\_t mac) <br>_Initialize Unified Network IP Manager._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netcfg\_set\_link\_callback**](#function-syn_netcfg_set_link_callback) ([**SYN\_NETCFG**](structSYN__NETCFG.md) \* netcfg, [**SYN\_NETCFG\_LinkCb**](syn__netcfg_8h.md#typedef-syn_netcfg_linkcb) cb, void \* user\_data) <br>_Register Application Link Event Callback._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netcfg\_set\_link\_state**](#function-syn_netcfg_set_link_state) ([**SYN\_NETCFG**](structSYN__NETCFG.md) \* netcfg, [**SYN\_ETH**](structSYN__ETH.md) \* eth, [**SYN\_NETCFG\_LinkState**](syn__netcfg_8h.md#enum-syn_netcfg_linkstate) state) <br>_Notify Network Manager of Physical Link State Change (Link Up / Link Down)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netcfg\_set\_static**](#function-syn_netcfg_set_static) ([**SYN\_NETCFG**](structSYN__NETCFG.md) \* netcfg, [**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t ip, uint32\_t netmask, uint32\_t gateway) <br>_Set Static IP configuration parameters._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netcfg\_trigger\_autoip\_fallback**](#function-syn_netcfg_trigger_autoip_fallback) ([**SYN\_NETCFG**](structSYN__NETCFG.md) \* netcfg, [**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t mac) <br>_Trigger AutoIP fallback when DHCP times out._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_NETCFG\_BLOCK\_BOUND**](syn__netcfg_8h.md#define-pt_netcfg_block_bound) (pt, task, netcfg) `[**PT\_BLOCK\_CONDITION**](syn__pt_8h.md#define-pt_block_condition)(pt, task, (netcfg)-&gt;is\_bound == true)`<br>_Block task execution (SYN\_TASK\_BLOCKED) until IP configuration binding completes._  |
| define  | [**PT\_NETCFG\_BLOCK\_LINK\_UP**](syn__netcfg_8h.md#define-pt_netcfg_block_link_up) (pt, task, netcfg) `[**PT\_BLOCK\_CONDITION**](syn__pt_8h.md#define-pt_block_condition)(pt, task, (netcfg)-&gt;link\_state == SYN\_NETCFG\_LINK\_UP)`<br>_Block task execution (SYN\_TASK\_BLOCKED) until physical link is UP._  |
| define  | [**PT\_NETCFG\_WAIT\_BOUND**](syn__netcfg_8h.md#define-pt_netcfg_wait_bound) (pt, netcfg) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, (netcfg)-&gt;is\_bound == true)`<br>_Block a protothread coroutine until IP configuration binding completes._  |
| define  | [**PT\_NETCFG\_WAIT\_LINK\_UP**](syn__netcfg_8h.md#define-pt_netcfg_wait_link_up) (pt, netcfg) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, (netcfg)-&gt;link\_state == SYN\_NETCFG\_LINK\_UP)`<br>_Block a protothread coroutine until physical link is UP._  |

## Detailed Description


Unifies Static IP, DHCP Client, RFC 3927 AutoIP fallback, and Physical Link Event handling. 


    
## Public Types Documentation




### typedef SYN\_NETCFG\_LinkCb 

```C++
typedef void(* SYN_NETCFG_LinkCb) (struct SYN_NETCFG_s *netcfg, SYN_NETCFG_LinkState state, void *user_data);
```



Application Link Event Callback Signature. 


        

<hr>



### enum SYN\_NETCFG\_LinkState 

```C++
enum SYN_NETCFG_LinkState {
    SYN_NETCFG_LINK_DOWN = 0,
    SYN_NETCFG_LINK_UP
};
```



Physical Link States. 


        

<hr>



### enum SYN\_NETCFG\_Mode 

```C++
enum SYN_NETCFG_Mode {
    SYN_NETCFG_MODE_STATIC = 0,
    SYN_NETCFG_MODE_DHCP,
    SYN_NETCFG_MODE_AUTO
};
```



IP Manager Modes. 


        

<hr>
## Public Functions Documentation




### function syn\_netcfg\_init 

_Initialize Unified Network IP Manager._ 
```C++
SYN_Status syn_netcfg_init (
    SYN_NETCFG * netcfg,
    SYN_NETCFG_Mode mode,
    const uint8_t mac
) 
```





**Parameters:**


* `netcfg` Pointer to Netcfg context. 
* `mode` Configuration mode (STATIC, DHCP, or AUTO). 
* `mac` 6-byte client MAC address. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_netcfg\_set\_link\_callback 

_Register Application Link Event Callback._ 
```C++
SYN_Status syn_netcfg_set_link_callback (
    SYN_NETCFG * netcfg,
    SYN_NETCFG_LinkCb cb,
    void * user_data
) 
```





**Parameters:**


* `netcfg` Pointer to Netcfg context. 
* `cb` Callback function pointer. 
* `user_data` User data context passed to callback. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_netcfg\_set\_link\_state 

_Notify Network Manager of Physical Link State Change (Link Up / Link Down)._ 
```C++
SYN_Status syn_netcfg_set_link_state (
    SYN_NETCFG * netcfg,
    SYN_ETH * eth,
    SYN_NETCFG_LinkState state
) 
```



On LINK\_DOWN: Invalidates IP binding, resets state machines back to INIT, and triggers callback. On LINK\_UP: Automatically re-triggers IP acquisition state machine and triggers callback.




**Parameters:**


* `netcfg` Pointer to Netcfg context. 
* `eth` Pointer to Ethernet context. 
* `state` Physical Link State (SYN\_NETCFG\_LINK\_DOWN or SYN\_NETCFG\_LINK\_UP). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_netcfg\_set\_static 

_Set Static IP configuration parameters._ 
```C++
SYN_Status syn_netcfg_set_static (
    SYN_NETCFG * netcfg,
    SYN_ETH * eth,
    uint32_t ip,
    uint32_t netmask,
    uint32_t gateway
) 
```





**Parameters:**


* `netcfg` Pointer to Netcfg context. 
* `eth` Pointer to Ethernet context. 
* `ip` 32-bit IPv4 address. 
* `netmask` 32-bit netmask. 
* `gateway` 32-bit router gateway IP. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_netcfg\_trigger\_autoip\_fallback 

_Trigger AutoIP fallback when DHCP times out._ 
```C++
SYN_Status syn_netcfg_trigger_autoip_fallback (
    SYN_NETCFG * netcfg,
    SYN_ETH * eth,
    const uint8_t mac
) 
```





**Parameters:**


* `netcfg` Pointer to Netcfg context. 
* `eth` Pointer to Ethernet context. 
* `mac` 6-byte client MAC address. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define PT\_NETCFG\_BLOCK\_BOUND 

_Block task execution (SYN\_TASK\_BLOCKED) until IP configuration binding completes._ 
```C++
#define PT_NETCFG_BLOCK_BOUND (
    pt,
    task,
    netcfg
) `PT_BLOCK_CONDITION (pt, task, (netcfg)->is_bound == true)`
```




<hr>



### define PT\_NETCFG\_BLOCK\_LINK\_UP 

_Block task execution (SYN\_TASK\_BLOCKED) until physical link is UP._ 
```C++
#define PT_NETCFG_BLOCK_LINK_UP (
    pt,
    task,
    netcfg
) `PT_BLOCK_CONDITION (pt, task, (netcfg)->link_state == SYN_NETCFG_LINK_UP)`
```




<hr>



### define PT\_NETCFG\_WAIT\_BOUND 

_Block a protothread coroutine until IP configuration binding completes._ 
```C++
#define PT_NETCFG_WAIT_BOUND (
    pt,
    netcfg
) `PT_WAIT_UNTIL (pt, (netcfg)->is_bound == true)`
```





**Parameters:**


* `pt` Protothread context. 
* `netcfg` Pointer to Netcfg context. 




        

<hr>



### define PT\_NETCFG\_WAIT\_LINK\_UP 

_Block a protothread coroutine until physical link is UP._ 
```C++
#define PT_NETCFG_WAIT_LINK_UP (
    pt,
    netcfg
) `PT_WAIT_UNTIL (pt, (netcfg)->link_state == SYN_NETCFG_LINK_UP)`
```





**Parameters:**


* `pt` Protothread context. 
* `netcfg` Pointer to Netcfg context. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_netcfg.h`

