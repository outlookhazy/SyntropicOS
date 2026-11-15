

# Struct SYN\_GBT27930\_Session



[**ClassList**](annotated.md) **>** [**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md)



_GB/T 27930 Session Context._ 

* `#include <syn_gbt27930.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**bcl\_timeout\_ms**](#variable-bcl_timeout_ms)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**bcp\_received**](#variable-bcp_received)  <br> |
|  [**SYN\_GBT27930\_BMS\_Config**](structSYN__GBT27930__BMS__Config.md) | [**bms\_cfg**](#variable-bms_cfg)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**brm\_received**](#variable-brm_received)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**bro\_received**](#variable-bro_received)  <br> |
|  [**SYN\_GBT27930\_Charger\_Config**](structSYN__GBT27930__Charger__Config.md) | [**charger\_cfg**](#variable-charger_cfg)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**cml\_received**](#variable-cml_received)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**cml\_sent**](#variable-cml_sent)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**crm00\_received**](#variable-crm00_received)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**crm\_aa\_retransmit**](#variable-crm_aa_retransmit)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**cts\_received**](#variable-cts_received)  <br> |
|  uint8\_t | [**fault\_code**](#variable-fault_code)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**peer\_ready\_for\_charging**](#variable-peer_ready_for_charging)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**peer\_stopped**](#variable-peer_stopped)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ready\_for\_charging**](#variable-ready_for_charging)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ready\_tx\_sent**](#variable-ready_tx_sent)  <br> |
|  [**SYN\_GBT27930\_Role**](syn__gbt27930_8h.md#enum-syn_gbt27930_role) | [**role**](#variable-role)  <br> |
|  [**SYN\_GBT27930\_State**](syn__gbt27930_8h.md#enum-syn_gbt27930_state) | [**state**](#variable-state)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**stats\_received**](#variable-stats_received)  <br> |
|  uint8\_t | [**stop\_reason**](#variable-stop_reason)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**stop\_tx\_sent**](#variable-stop_tx_sent)  <br> |
|  uint8\_t | [**sync\_time\_bcd**](#variable-sync_time_bcd)  <br> |
|  [**SYN\_GBT27930\_Telemetry**](structSYN__GBT27930__Telemetry.md) | [**telemetry**](#variable-telemetry)  <br> |
|  uint32\_t | [**timeout\_ms**](#variable-timeout_ms)  <br> |
|  uint32\_t | [**timer\_ms**](#variable-timer_ms)  <br> |
|  uint8\_t | [**tx\_substep**](#variable-tx_substep)  <br> |












































## Public Attributes Documentation




### variable bcl\_timeout\_ms 

```C++
uint32_t SYN_GBT27930_Session::bcl_timeout_ms;
```



Charger 1000 ms BCL demand watchdog timer 


        

<hr>



### variable bcp\_received 

```C++
bool SYN_GBT27930_Session::bcp_received;
```



Charger received BCP battery parameters 


        

<hr>



### variable bms\_cfg 

```C++
SYN_GBT27930_BMS_Config SYN_GBT27930_Session::bms_cfg;
```



BMS Config 


        

<hr>



### variable brm\_received 

```C++
bool SYN_GBT27930_Session::brm_received;
```



Charger received BRM recognition data 


        

<hr>



### variable bro\_received 

```C++
bool SYN_GBT27930_Session::bro_received;
```



Charger received BRO (0x00 or 0xAA) from BMS 


        

<hr>



### variable charger\_cfg 

```C++
SYN_GBT27930_Charger_Config SYN_GBT27930_Session::charger_cfg;
```



Charger Config 


        

<hr>



### variable cml\_received 

```C++
bool SYN_GBT27930_Session::cml_received;
```



BMS received CML charger capability 


        

<hr>



### variable cml\_sent 

```C++
bool SYN_GBT27930_Session::cml_sent;
```



Charger transmitted CML capability frame 


        

<hr>



### variable crm00\_received 

```C++
bool SYN_GBT27930_Session::crm00_received;
```



BMS received CRM(0x00) recognition prompt 


        

<hr>



### variable crm\_aa\_retransmit 

```C++
bool SYN_GBT27930_Session::crm_aa_retransmit;
```



Charger queued CRM(0xAA) retransmit on BRM 


        

<hr>



### variable cts\_received 

```C++
bool SYN_GBT27930_Session::cts_received;
```



BMS received CTS time sync frame 


        

<hr>



### variable fault\_code 

```C++
uint8_t SYN_GBT27930_Session::fault_code;
```



Error/Fault code 


        

<hr>



### variable peer\_ready\_for\_charging 

```C++
bool SYN_GBT27930_Session::peer_ready_for_charging;
```



Peer ready state flag 


        

<hr>



### variable peer\_stopped 

```C++
bool SYN_GBT27930_Session::peer_stopped;
```



Peer BST/CST stop frame received 


        

<hr>



### variable ready\_for\_charging 

```C++
bool SYN_GBT27930_Session::ready_for_charging;
```



Local ready state flag 


        

<hr>



### variable ready\_tx\_sent 

```C++
bool SYN_GBT27930_Session::ready_tx_sent;
```



Local BRO(0xAA)/CRO(0xAA) transmitted 


        

<hr>



### variable role 

```C++
SYN_GBT27930_Role SYN_GBT27930_Session::role;
```



Node Role (BMS or CHARGER) 


        

<hr>



### variable state 

```C++
SYN_GBT27930_State SYN_GBT27930_Session::state;
```



Current State Machine Phase 


        

<hr>



### variable stats\_received 

```C++
bool SYN_GBT27930_Session::stats_received;
```



Peer BSD/CSD statistics frame received 


        

<hr>



### variable stop\_reason 

```C++
uint8_t SYN_GBT27930_Session::stop_reason;
```



Stop reason code 


        

<hr>



### variable stop\_tx\_sent 

```C++
bool SYN_GBT27930_Session::stop_tx_sent;
```



Local BST/CST stop frame transmitted 


        

<hr>



### variable sync\_time\_bcd 

```C++
uint8_t SYN_GBT27930_Session::sync_time_bcd[7];
```



Charger BCD timestamp for CTS (optional) 


        

<hr>



### variable telemetry 

```C++
SYN_GBT27930_Telemetry SYN_GBT27930_Session::telemetry;
```



Real-time telemetry 


        

<hr>



### variable timeout\_ms 

```C++
uint32_t SYN_GBT27930_Session::timeout_ms;
```



Phase timeout timer 


        

<hr>



### variable timer\_ms 

```C++
uint32_t SYN_GBT27930_Session::timer_ms;
```



Periodic transmit timer 


        

<hr>



### variable tx\_substep 

```C++
uint8_t SYN_GBT27930_Session::tx_substep;
```



Periodic multi-PGN schedule index 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_gbt27930.h`

