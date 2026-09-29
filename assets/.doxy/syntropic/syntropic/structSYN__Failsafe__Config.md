

# Struct SYN\_Failsafe\_Config



[**ClassList**](annotated.md) **>** [**SYN\_Failsafe\_Config**](structSYN__Failsafe__Config.md)



[More...](#detailed-description)

* `#include <syn_rc_failsafe.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_Failsafe\_ChannelMode**](syn__rc__failsafe_8h.md#enum-syn_failsafe_channelmode) | [**channel\_modes**](#variable-channel_modes)  <br> |
|  uint16\_t | [**fallback\_us**](#variable-fallback_us)  <br> |
|  uint32\_t | [**timeout\_ms**](#variable-timeout_ms)  <br> |












































## Detailed Description


Failsafe Manager Configuration. 


    
## Public Attributes Documentation




### variable channel\_modes 

```C++
SYN_Failsafe_ChannelMode SYN_Failsafe_Config::channel_modes[SYN_RC_FAILSAFE_MAX_CHANNELS];
```



Per-channel fallback mode. 


        

<hr>



### variable fallback\_us 

```C++
uint16_t SYN_Failsafe_Config::fallback_us[SYN_RC_FAILSAFE_MAX_CHANNELS];
```



Fallback pulse widths in us. 


        

<hr>



### variable timeout\_ms 

```C++
uint32_t SYN_Failsafe_Config::timeout_ms;
```



Signal loss timeout threshold in ms (e.g. 500 ms). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_rc_failsafe.h`

