

# File syn\_gbt27930.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_gbt27930.h**](syn__gbt27930_8h.md)

[Go to the source code of this file](syn__gbt27930_8h_source.md)

_GB/T 27930 (EV DC Fast Charging Protocol on SAE J1939 CAN Bus)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_can.h"`
* `#include "syn_j1939.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_GBT27930\_BMS\_Config**](structSYN__GBT27930__BMS__Config.md) <br>_BMS Static Parameters (BRM / BCP)._  |
| struct | [**SYN\_GBT27930\_Charger\_Config**](structSYN__GBT27930__Charger__Config.md) <br>_Charger Static Parameters (CML)._  |
| struct | [**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) <br>_GB/T 27930 Session Context._  |
| struct | [**SYN\_GBT27930\_Telemetry**](structSYN__GBT27930__Telemetry.md) <br>_Real-time Charging Telemetry (BCL / BCS / CCS / BSM)._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_GBT27930\_Role**](#enum-syn_gbt27930_role)  <br>_GB/T 27930 Node Role._  |
| enum  | [**SYN\_GBT27930\_State**](#enum-syn_gbt27930_state)  <br>_GB/T 27930 State Machine Phases._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_gbt27930\_init**](#function-syn_gbt27930_init) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, [**SYN\_GBT27930\_Role**](syn__gbt27930_8h.md#enum-syn_gbt27930_role) role) <br>_Initialize GB/T 27930 Charging Session._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gbt27930\_process\_rx\_frame**](#function-syn_gbt27930_process_rx_frame) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process incoming CAN frame on GB/T 27930 session._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_gbt27930\_start\_handshake**](#function-syn_gbt27930_start_handshake) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session) <br>_Start handshake phase._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_gbt27930\_step**](#function-syn_gbt27930_step) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t dt\_ms, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* tx\_frame) <br>_Step session state machine timers and produce periodic CAN frames._  |
|  void | [**syn\_gbt27930\_stop\_charging**](#function-syn_gbt27930_stop_charging) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint8\_t reason) <br>_Stop charging session cleanly._  |




























## Detailed Description


Implements GB/T 27930-2015 / GB/T 27930-2024 EV DC Fast Charging Protocol state machine and framing for BMS (Vehicle, 0xF4) and Charger (EVSE, 0x56) nodes. 


    
## Public Types Documentation




### enum SYN\_GBT27930\_Role 

_GB/T 27930 Node Role._ 
```C++
enum SYN_GBT27930_Role {
    SYN_GBT27930_ROLE_BMS = 0,
    SYN_GBT27930_ROLE_CHARGER = 1
};
```




<hr>



### enum SYN\_GBT27930\_State 

_GB/T 27930 State Machine Phases._ 
```C++
enum SYN_GBT27930_State {
    SYN_GBT27930_STATE_IDLE = 0,
    SYN_GBT27930_STATE_HANDSHAKE = 1,
    SYN_GBT27930_STATE_PARAM_CONFIG = 2,
    SYN_GBT27930_STATE_CHARGING = 3,
    SYN_GBT27930_STATE_STOPPING = 4,
    SYN_GBT27930_STATE_ERROR = 5
};
```




<hr>
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
The documentation for this class was generated from the following file `src/syntropic/proto/syn_gbt27930.h`

