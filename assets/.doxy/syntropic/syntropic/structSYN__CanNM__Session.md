

# Struct SYN\_CanNM\_Session



[**ClassList**](annotated.md) **>** [**SYN\_CanNM\_Session**](structSYN__CanNM__Session.md)



_AUTOSAR CAN Network Management Session Context._ 

* `#include <syn_cannm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_CanNM\_Config**](structSYN__CanNM__Config.md) | [**config**](#variable-config)  <br> |
|  uint32\_t | [**msg\_cycle\_timer**](#variable-msg_cycle_timer)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**node\_comm\_req**](#variable-node_comm_req)  <br> |
|  [**SYN\_CanNM\_State**](syn__cannm_8h.md#enum-syn_cannm_state) | [**prev\_state**](#variable-prev_state)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**repeat\_msg\_req**](#variable-repeat_msg_req)  <br> |
|  uint32\_t | [**repeat\_msg\_timer**](#variable-repeat_msg_timer)  <br> |
|  uint8\_t | [**rx\_cbv**](#variable-rx_cbv)  <br> |
|  uint8\_t | [**rx\_source\_node\_id**](#variable-rx_source_node_id)  <br> |
|  uint8\_t | [**rx\_user\_data**](#variable-rx_user_data)  <br> |
|  [**SYN\_CanNM\_State**](syn__cannm_8h.md#enum-syn_cannm_state) | [**state**](#variable-state)  <br> |
|  uint32\_t | [**timeout\_timer**](#variable-timeout_timer)  <br> |
|  uint8\_t | [**user\_data**](#variable-user_data)  <br> |
|  uint32\_t | [**wait\_bus\_sleep\_timer**](#variable-wait_bus_sleep_timer)  <br> |












































## Public Attributes Documentation




### variable config 

```C++
SYN_CanNM_Config SYN_CanNM_Session::config;
```



Session timer & ID configuration 


        

<hr>



### variable msg\_cycle\_timer 

```C++
uint32_t SYN_CanNM_Session::msg_cycle_timer;
```



Message transmission period timer in ms 


        

<hr>



### variable node\_comm\_req 

```C++
bool SYN_CanNM_Session::node_comm_req;
```



Local application network request 


        

<hr>



### variable prev\_state 

```C++
SYN_CanNM_State SYN_CanNM_Session::prev_state;
```



Previous CAN NM FSM state 


        

<hr>



### variable repeat\_msg\_req 

```C++
bool SYN_CanNM_Session::repeat_msg_req;
```



Repeat message request flag 


        

<hr>



### variable repeat\_msg\_timer 

```C++
uint32_t SYN_CanNM_Session::repeat_msg_timer;
```



Repeat Message state duration timer in ms 


        

<hr>



### variable rx\_cbv 

```C++
uint8_t SYN_CanNM_Session::rx_cbv;
```



CBV of last received NM message 


        

<hr>



### variable rx\_source\_node\_id 

```C++
uint8_t SYN_CanNM_Session::rx_source_node_id;
```



Node ID of last received NM message 


        

<hr>



### variable rx\_user\_data 

```C++
uint8_t SYN_CanNM_Session::rx_user_data[6];
```



User data from last received NM message 


        

<hr>



### variable state 

```C++
SYN_CanNM_State SYN_CanNM_Session::state;
```



Current CAN NM FSM state 


        

<hr>



### variable timeout\_timer 

```C++
uint32_t SYN_CanNM_Session::timeout_timer;
```



NM Timeout timer in ms 


        

<hr>



### variable user\_data 

```C++
uint8_t SYN_CanNM_Session::user_data[6];
```



Local user data bytes 


        

<hr>



### variable wait\_bus\_sleep\_timer 

```C++
uint32_t SYN_CanNM_Session::wait_bus_sleep_timer;
```



Wait Bus Sleep timer in ms 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cannm.h`

