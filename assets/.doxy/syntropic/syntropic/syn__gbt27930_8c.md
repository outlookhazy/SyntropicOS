

# File syn\_gbt27930.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_gbt27930.c**](syn__gbt27930_8c.md)

[Go to the source code of this file](syn__gbt27930_8c_source.md)

_GB/T 27930 EV DC Fast Charging Protocol Implementation._ 

* `#include "syn_gbt27930.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_gbt27930\_init**](#function-syn_gbt27930_init) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, [**SYN\_GBT27930\_Role**](syn__gbt27930_8h.md#enum-syn_gbt27930_role) role) <br>_Initialize GB/T 27930 Charging Session._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gbt27930\_process\_rx\_frame**](#function-syn_gbt27930_process_rx_frame) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process incoming CAN frame on GB/T 27930 session._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gbt27930\_start\_handshake**](#function-syn_gbt27930_start_handshake) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session) <br>_Start handshake phase._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_gbt27930\_step**](#function-syn_gbt27930_step) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t dt\_ms, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* tx\_frame) <br>_Step session state machine timers and produce periodic CAN frames._  |
|  void | [**syn\_gbt27930\_stop\_charging**](#function-syn_gbt27930_stop_charging) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint8\_t reason) <br>_Stop charging session cleanly._  |




























## Public Functions Documentation




### function syn\_gbt27930\_init 

_Initialize GB/T 27930 Charging Session._ 
```C++
void syn_gbt27930_init (
    SYN_GBT27930_Session * session,
    SYN_GBT27930_Role role
) 
```





**Parameters:**


* `session` Pointer to session instance. 
* `role` Node role (SYN\_GBT27930\_ROLE\_BMS or SYN\_GBT27930\_ROLE\_CHARGER). 




        

<hr>



### function syn\_gbt27930\_process\_rx\_frame 

_Process incoming CAN frame on GB/T 27930 session._ 
```C++
SYN_Status syn_gbt27930_process_rx_frame (
    SYN_GBT27930_Session * session,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` Pointer to session instance. 
* `frame` Pointer to received CAN frame. 



**Returns:**

SYN\_OK if frame processed successfully. 





        

<hr>



### function syn\_gbt27930\_start\_handshake 

_Start handshake phase._ 
```C++
SYN_Status syn_gbt27930_start_handshake (
    SYN_GBT27930_Session * session
) 
```





**Parameters:**


* `session` Pointer to session instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_gbt27930\_step 

_Step session state machine timers and produce periodic CAN frames._ 
```C++
bool syn_gbt27930_step (
    SYN_GBT27930_Session * session,
    uint32_t dt_ms,
    SYN_CAN_Frame * tx_frame
) 
```





**Parameters:**


* `session` Pointer to session instance. 
* `dt_ms` Milliseconds elapsed since last call. 
* `tx_frame` Pointer to CAN frame output buffer. 



**Returns:**

true if tx\_frame contains a CAN frame to transmit, false if idle. 





        

<hr>



### function syn\_gbt27930\_stop\_charging 

_Stop charging session cleanly._ 
```C++
void syn_gbt27930_stop_charging (
    SYN_GBT27930_Session * session,
    uint8_t reason
) 
```





**Parameters:**


* `session` Pointer to session instance. 
* `reason` Stop reason code. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_gbt27930.c`

