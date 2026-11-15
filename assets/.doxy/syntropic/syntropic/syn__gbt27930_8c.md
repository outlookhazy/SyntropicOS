

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


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**gbt\_check\_phase\_bounds**](#function-gbt_check_phase_bounds) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session) <br>_Evaluate phase timeout and bounded STATE\_END completion._  |
|  uint32\_t | [**gbt\_get\_tx\_interval\_ms**](#function-gbt_get_tx_interval_ms) (const [**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session) <br>_Compute periodic TX interval in milliseconds for current session state._  |
|  void | [**gbt\_handle\_error\_rx**](#function-gbt_handle_error_rx) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Handle received error message frame (BEM or CEM) and transition to error state._  |
|  void | [**gbt\_handle\_stop\_rx**](#function-gbt_handle_stop_rx) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Handle received stop charging frame (BST or CST) and update session state._  |
|  void | [**gbt\_pack\_u16\_le**](#function-gbt_pack_u16_le) (uint8\_t \* dst, uint16\_t val) <br>_Pack 16-bit unsigned integer into little-endian byte buffer._  |
|  void | [**gbt\_rx\_bms**](#function-gbt_rx_bms) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t pgn, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Dispatch inbound CAN frame for a BMS-role GB/T 27930 session._  |
|  void | [**gbt\_rx\_bms\_config**](#function-gbt_rx_bms_config) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t pgn, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process BMS-side parameter configuration frames (CTS, CML, CRO) from charger._  |
|  void | [**gbt\_rx\_bms\_handshake**](#function-gbt_rx_bms_handshake) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t pgn, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process BMS-side handshake frames (CHM, CRM) from charger._  |
|  void | [**gbt\_rx\_bms\_runtime**](#function-gbt_rx_bms_runtime) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t pgn, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process BMS-side charging, stop, statistics, and error frames (CCS, CST, CSD, CEM)._  |
|  void | [**gbt\_rx\_charger**](#function-gbt_rx_charger) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t pgn, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Dispatch inbound CAN frame for a charger-role GB/T 27930 session._  |
|  void | [**gbt\_rx\_charger\_charging**](#function-gbt_rx_charger_charging) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t pgn, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process charger-side active charging telemetry frames (BCL, BCS, BSM)._  |
|  void | [**gbt\_rx\_charger\_config**](#function-gbt_rx_charger_config) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t pgn, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process charger-side parameter configuration frames (duplicate BRM, BCP, BRO)._  |
|  void | [**gbt\_rx\_charger\_handshake**](#function-gbt_rx_charger_handshake) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t pgn, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process charger-side handshake frames (BHM, BRM)._  |
|  void | [**gbt\_rx\_charger\_runtime**](#function-gbt_rx_charger_runtime) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint32\_t pgn, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process charger stop, statistics, and error frames (BST, BSD, BEM)._  |
|  uint32\_t | [**gbt\_sat\_add\_u32**](#function-gbt_sat_add_u32) (uint32\_t a, uint32\_t b) <br>_Saturating 32-bit unsigned millisecond accumulator._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**gbt\_tx\_bms**](#function-gbt_tx_bms) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint8\_t sa, uint8\_t da, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* tx\_frame) <br>_Build next periodic transmit frame for a BMS-role GB/T 27930 session._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**gbt\_tx\_bms\_charging**](#function-gbt_tx_bms_charging) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint8\_t sa, uint8\_t da, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* tx\_frame) <br>_Build periodic BMS charging telemetry frame (BCL -&gt; BCS -&gt; BSM)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**gbt\_tx\_bms\_early**](#function-gbt_tx_bms_early) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint8\_t sa, uint8\_t da, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* tx\_frame) <br>_Build periodic BMS frame for handshake and parameter configuration phases._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**gbt\_tx\_charger**](#function-gbt_tx_charger) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint8\_t sa, uint8\_t da, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* tx\_frame) <br>_Build next periodic transmit frame for a charger-role GB/T 27930 session._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**gbt\_tx\_charger\_early**](#function-gbt_tx_charger_early) ([**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md) \* session, uint8\_t sa, uint8\_t da, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* tx\_frame) <br>_Build periodic charger frame for handshake and parameter configuration phases._  |
|  uint16\_t | [**gbt\_unpack\_u16\_le**](#function-gbt_unpack_u16_le) (const uint8\_t \* src) <br>_Unpack 16-bit unsigned integer from little-endian byte buffer._  |


























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
## Public Static Functions Documentation




### function gbt\_check\_phase\_bounds 

_Evaluate phase timeout and bounded STATE\_END completion._ 
```C++
static bool gbt_check_phase_bounds (
    SYN_GBT27930_Session * session
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 



**Returns:**

false if session transitioned to IDLE and needs no TX, true otherwise. 





        

<hr>



### function gbt\_get\_tx\_interval\_ms 

_Compute periodic TX interval in milliseconds for current session state._ 
```C++
static uint32_t gbt_get_tx_interval_ms (
    const SYN_GBT27930_Session * session
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 



**Returns:**

Periodic transmit interval in milliseconds. 





        

<hr>



### function gbt\_handle\_error\_rx 

_Handle received error message frame (BEM or CEM) and transition to error state._ 
```C++
static void gbt_handle_error_rx (
    SYN_GBT27930_Session * session,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_handle\_stop\_rx 

_Handle received stop charging frame (BST or CST) and update session state._ 
```C++
static void gbt_handle_stop_rx (
    SYN_GBT27930_Session * session,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_pack\_u16\_le 

_Pack 16-bit unsigned integer into little-endian byte buffer._ 
```C++
static void gbt_pack_u16_le (
    uint8_t * dst,
    uint16_t val
) 
```





**Parameters:**


* `dst` Target 2-byte output buffer. 
* `val` 16-bit unsigned value to encode. 




        

<hr>



### function gbt\_rx\_bms 

_Dispatch inbound CAN frame for a BMS-role GB/T 27930 session._ 
```C++
static void gbt_rx_bms (
    SYN_GBT27930_Session * session,
    uint32_t pgn,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `pgn` Unpacked J1939 parameter group number. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_rx\_bms\_config 

_Process BMS-side parameter configuration frames (CTS, CML, CRO) from charger._ 
```C++
static void gbt_rx_bms_config (
    SYN_GBT27930_Session * session,
    uint32_t pgn,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `pgn` Unpacked J1939 parameter group number. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_rx\_bms\_handshake 

_Process BMS-side handshake frames (CHM, CRM) from charger._ 
```C++
static void gbt_rx_bms_handshake (
    SYN_GBT27930_Session * session,
    uint32_t pgn,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `pgn` Unpacked J1939 parameter group number. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_rx\_bms\_runtime 

_Process BMS-side charging, stop, statistics, and error frames (CCS, CST, CSD, CEM)._ 
```C++
static void gbt_rx_bms_runtime (
    SYN_GBT27930_Session * session,
    uint32_t pgn,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `pgn` Unpacked J1939 parameter group number. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_rx\_charger 

_Dispatch inbound CAN frame for a charger-role GB/T 27930 session._ 
```C++
static void gbt_rx_charger (
    SYN_GBT27930_Session * session,
    uint32_t pgn,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `pgn` Unpacked J1939 parameter group number. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_rx\_charger\_charging 

_Process charger-side active charging telemetry frames (BCL, BCS, BSM)._ 
```C++
static void gbt_rx_charger_charging (
    SYN_GBT27930_Session * session,
    uint32_t pgn,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `pgn` Unpacked J1939 parameter group number. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_rx\_charger\_config 

_Process charger-side parameter configuration frames (duplicate BRM, BCP, BRO)._ 
```C++
static void gbt_rx_charger_config (
    SYN_GBT27930_Session * session,
    uint32_t pgn,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `pgn` Unpacked J1939 parameter group number. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_rx\_charger\_handshake 

_Process charger-side handshake frames (BHM, BRM)._ 
```C++
static void gbt_rx_charger_handshake (
    SYN_GBT27930_Session * session,
    uint32_t pgn,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `pgn` Unpacked J1939 parameter group number. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_rx\_charger\_runtime 

_Process charger stop, statistics, and error frames (BST, BSD, BEM)._ 
```C++
static void gbt_rx_charger_runtime (
    SYN_GBT27930_Session * session,
    uint32_t pgn,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `pgn` Unpacked J1939 parameter group number. 
* `frame` Received CAN frame. 




        

<hr>



### function gbt\_sat\_add\_u32 

_Saturating 32-bit unsigned millisecond accumulator._ 
```C++
static uint32_t gbt_sat_add_u32 (
    uint32_t a,
    uint32_t b
) 
```





**Parameters:**


* `a` Current timer value in ms. 
* `b` Elapsed step delta in ms. 



**Returns:**

Saturated sum (capped at UINT32\_MAX). 





        

<hr>



### function gbt\_tx\_bms 

_Build next periodic transmit frame for a BMS-role GB/T 27930 session._ 
```C++
static bool gbt_tx_bms (
    SYN_GBT27930_Session * session,
    uint8_t sa,
    uint8_t da,
    SYN_CAN_Frame * tx_frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `sa` Source CAN node address. 
* `da` Destination CAN node address. 
* `tx_frame` Output CAN frame buffer. 



**Returns:**

true when a CAN frame is ready for transmission, false otherwise. 





        

<hr>



### function gbt\_tx\_bms\_charging 

_Build periodic BMS charging telemetry frame (BCL -&gt; BCS -&gt; BSM)._ 
```C++
static bool gbt_tx_bms_charging (
    SYN_GBT27930_Session * session,
    uint8_t sa,
    uint8_t da,
    SYN_CAN_Frame * tx_frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `sa` Source CAN node address. 
* `da` Destination CAN node address. 
* `tx_frame` Output CAN frame buffer. 



**Returns:**

true when a CAN frame was populated for transmission. 





        

<hr>



### function gbt\_tx\_bms\_early 

_Build periodic BMS frame for handshake and parameter configuration phases._ 
```C++
static bool gbt_tx_bms_early (
    SYN_GBT27930_Session * session,
    uint8_t sa,
    uint8_t da,
    SYN_CAN_Frame * tx_frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `sa` Source CAN node address. 
* `da` Destination CAN node address. 
* `tx_frame` Output CAN frame buffer. 



**Returns:**

true when a CAN frame was populated for transmission. 





        

<hr>



### function gbt\_tx\_charger 

_Build next periodic transmit frame for a charger-role GB/T 27930 session._ 
```C++
static bool gbt_tx_charger (
    SYN_GBT27930_Session * session,
    uint8_t sa,
    uint8_t da,
    SYN_CAN_Frame * tx_frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `sa` Source CAN node address. 
* `da` Destination CAN node address. 
* `tx_frame` Output CAN frame buffer. 



**Returns:**

true when a CAN frame is ready for transmission, false otherwise. 





        

<hr>



### function gbt\_tx\_charger\_early 

_Build periodic charger frame for handshake and parameter configuration phases._ 
```C++
static bool gbt_tx_charger_early (
    SYN_GBT27930_Session * session,
    uint8_t sa,
    uint8_t da,
    SYN_CAN_Frame * tx_frame
) 
```





**Parameters:**


* `session` GB/T 27930 session instance. 
* `sa` Source CAN node address. 
* `da` Destination CAN node address. 
* `tx_frame` Output CAN frame buffer. 



**Returns:**

true when a CAN frame was populated for transmission. 





        

<hr>



### function gbt\_unpack\_u16\_le 

_Unpack 16-bit unsigned integer from little-endian byte buffer._ 
```C++
static uint16_t gbt_unpack_u16_le (
    const uint8_t * src
) 
```





**Parameters:**


* `src` Source 2-byte input buffer. 



**Returns:**

Decoded 16-bit unsigned value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_gbt27930.c`

