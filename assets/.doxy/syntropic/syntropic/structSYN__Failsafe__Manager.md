

# Struct SYN\_Failsafe\_Manager



[**ClassList**](annotated.md) **>** [**SYN\_Failsafe\_Manager**](structSYN__Failsafe__Manager.md)



[More...](#detailed-description)

* `#include <syn_rc_failsafe.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**channels**](#variable-channels)  <br> |
|  [**SYN\_Failsafe\_Config**](structSYN__Failsafe__Config.md) | [**config**](#variable-config)  <br> |
|  uint32\_t | [**failsafe\_events**](#variable-failsafe_events)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**in\_failsafe**](#variable-in_failsafe)  <br> |
|  uint32\_t | [**last\_frame\_ms**](#variable-last_frame_ms)  <br> |












































## Detailed Description


Failsafe Manager State Instance. 


    
## Public Attributes Documentation




### variable channels 

```C++
uint16_t SYN_Failsafe_Manager::channels[SYN_RC_FAILSAFE_MAX_CHANNELS];
```



Output channel pulse widths in us 


        

<hr>



### variable config 

```C++
SYN_Failsafe_Config SYN_Failsafe_Manager::config;
```



Active configuration settings 


        

<hr>



### variable failsafe\_events 

```C++
uint32_t SYN_Failsafe_Manager::failsafe_events;
```



Total failsafe trigger events count 


        

<hr>



### variable in\_failsafe 

```C++
bool SYN_Failsafe_Manager::in_failsafe;
```



True if currently in failsafe mode 


        

<hr>



### variable last\_frame\_ms 

```C++
uint32_t SYN_Failsafe_Manager::last_frame_ms;
```



Timestamp of last valid frame in ms 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_rc_failsafe.h`

