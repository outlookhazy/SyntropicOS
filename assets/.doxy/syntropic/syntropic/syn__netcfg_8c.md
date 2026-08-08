

# File syn\_netcfg.c



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_netcfg.c**](syn__netcfg_8c.md)

[Go to the source code of this file](syn__netcfg_8c_source.md)

_Unified Zero-Heap Network IP Address Manager Implementation._ 

* `#include "syntropic/net/syn_netcfg.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netcfg\_init**](#function-syn_netcfg_init) ([**SYN\_NETCFG**](structSYN__NETCFG.md) \* netcfg, [**SYN\_NETCFG\_Mode**](syn__netcfg_8h.md#enum-syn_netcfg_mode) mode, const uint8\_t mac) <br>_Initialize Unified Network IP Manager._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netcfg\_set\_link\_callback**](#function-syn_netcfg_set_link_callback) ([**SYN\_NETCFG**](structSYN__NETCFG.md) \* netcfg, [**SYN\_NETCFG\_LinkCb**](syn__netcfg_8h.md#typedef-syn_netcfg_linkcb) cb, void \* user\_data) <br>_Register Application Link Event Callback._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netcfg\_set\_link\_state**](#function-syn_netcfg_set_link_state) ([**SYN\_NETCFG**](structSYN__NETCFG.md) \* netcfg, [**SYN\_ETH**](structSYN__ETH.md) \* eth, [**SYN\_NETCFG\_LinkState**](syn__netcfg_8h.md#enum-syn_netcfg_linkstate) state) <br>_Notify Network Manager of Physical Link State Change (Link Up / Link Down)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netcfg\_set\_static**](#function-syn_netcfg_set_static) ([**SYN\_NETCFG**](structSYN__NETCFG.md) \* netcfg, [**SYN\_ETH**](structSYN__ETH.md) \* eth, uint32\_t ip, uint32\_t netmask, uint32\_t gateway) <br>_Set Static IP configuration parameters._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_netcfg\_trigger\_autoip\_fallback**](#function-syn_netcfg_trigger_autoip_fallback) ([**SYN\_NETCFG**](structSYN__NETCFG.md) \* netcfg, [**SYN\_ETH**](structSYN__ETH.md) \* eth, const uint8\_t mac) <br>_Trigger AutoIP fallback when DHCP times out._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_netcfg.c`

