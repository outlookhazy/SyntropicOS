

# Struct SYN\_GBT27930\_Session



[**ClassList**](annotated.md) **>** [**SYN\_GBT27930\_Session**](structSYN__GBT27930__Session.md)



_GB/T 27930 Session Context._ 

* `#include <syn_gbt27930.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_GBT27930\_BMS\_Config**](structSYN__GBT27930__BMS__Config.md) | [**bms\_cfg**](#variable-bms_cfg)  <br> |
|  [**SYN\_GBT27930\_Charger\_Config**](structSYN__GBT27930__Charger__Config.md) | [**charger\_cfg**](#variable-charger_cfg)  <br> |
|  uint8\_t | [**fault\_code**](#variable-fault_code)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**peer\_ready\_for\_charging**](#variable-peer_ready_for_charging)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**ready\_for\_charging**](#variable-ready_for_charging)  <br> |
|  [**SYN\_GBT27930\_Role**](syn__gbt27930_8h.md#enum-syn_gbt27930_role) | [**role**](#variable-role)  <br> |
|  [**SYN\_GBT27930\_State**](syn__gbt27930_8h.md#enum-syn_gbt27930_state) | [**state**](#variable-state)  <br> |
|  uint8\_t | [**stop\_reason**](#variable-stop_reason)  <br> |
|  [**SYN\_GBT27930\_Telemetry**](structSYN__GBT27930__Telemetry.md) | [**telemetry**](#variable-telemetry)  <br> |
|  uint32\_t | [**timeout\_ms**](#variable-timeout_ms)  <br> |
|  uint32\_t | [**timer\_ms**](#variable-timer_ms)  <br> |












































## Public Attributes Documentation




### variable bms\_cfg 

```C++
SYN_GBT27930_BMS_Config SYN_GBT27930_Session::bms_cfg;
```



BMS Config 


        

<hr>



### variable charger\_cfg 

```C++
SYN_GBT27930_Charger_Config SYN_GBT27930_Session::charger_cfg;
```



Charger Config 


        

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



### variable ready\_for\_charging 

```C++
bool SYN_GBT27930_Session::ready_for_charging;
```



Local ready state flag 


        

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



### variable stop\_reason 

```C++
uint8_t SYN_GBT27930_Session::stop_reason;
```



Stop reason code 


        

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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_gbt27930.h`

