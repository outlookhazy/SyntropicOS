

# Struct SYN\_CanNM\_Config



[**ClassList**](annotated.md) **>** [**SYN\_CanNM\_Config**](structSYN__CanNM__Config.md)



_AUTOSAR CAN Network Management Configuration._ 

* `#include <syn_cannm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**can\_id\_base**](#variable-can_id_base)  <br> |
|  uint32\_t | [**can\_id\_mask**](#variable-can_id_mask)  <br> |
|  uint32\_t | [**msg\_cycle\_ms**](#variable-msg_cycle_ms)  <br> |
|  uint32\_t | [**nm\_timeout\_ms**](#variable-nm_timeout_ms)  <br> |
|  uint8\_t | [**node\_id**](#variable-node_id)  <br> |
|  uint32\_t | [**repeat\_msg\_time\_ms**](#variable-repeat_msg_time_ms)  <br> |
|  uint32\_t | [**wait\_bus\_sleep\_ms**](#variable-wait_bus_sleep_ms)  <br> |












































## Public Attributes Documentation




### variable can\_id\_base 

```C++
uint32_t SYN_CanNM_Config::can_id_base;
```



CAN NM ID Base (e.g. 0x400) 


        

<hr>



### variable can\_id\_mask 

```C++
uint32_t SYN_CanNM_Config::can_id_mask;
```



CAN NM ID Filter Mask (e.g. 0x7F0) 


        

<hr>



### variable msg\_cycle\_ms 

```C++
uint32_t SYN_CanNM_Config::msg_cycle_ms;
```



Transmission period (default: 100ms) 


        

<hr>



### variable nm\_timeout\_ms 

```C++
uint32_t SYN_CanNM_Config::nm_timeout_ms;
```



NM Timeout (default: 1000ms) 


        

<hr>



### variable node\_id 

```C++
uint8_t SYN_CanNM_Config::node_id;
```



Local ECU Node ID (e.g. 0x01) 


        

<hr>



### variable repeat\_msg\_time\_ms 

```C++
uint32_t SYN_CanNM_Config::repeat_msg_time_ms;
```



Repeat Message State Duration (default: 1600ms) 


        

<hr>



### variable wait\_bus\_sleep\_ms 

```C++
uint32_t SYN_CanNM_Config::wait_bus_sleep_ms;
```



Wait Bus Sleep Timeout (default: 1500ms) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cannm.h`

