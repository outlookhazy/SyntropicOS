

# File syn\_rc\_failsafe.h



[**FileList**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_rc\_failsafe.h**](syn__rc__failsafe_8h.md)

[Go to the source code of this file](syn__rc__failsafe_8h_source.md)

_Zero-Heap RC Safety Failsafe Manager & Watchdog._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Failsafe\_Config**](structSYN__Failsafe__Config.md) <br> |
| struct | [**SYN\_Failsafe\_Manager**](structSYN__Failsafe__Manager.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_Failsafe\_ChannelMode**](#enum-syn_failsafe_channelmode)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_failsafe\_feed\_frame**](#function-syn_failsafe_feed_frame) ([**SYN\_Failsafe\_Manager**](structSYN__Failsafe__Manager.md) \* mgr, const uint16\_t \* in\_channels, uint8\_t num\_channels, uint32\_t timestamp\_ms) <br>_Register valid incoming frame from RC receiver (resets watchdog timer)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_failsafe\_init**](#function-syn_failsafe_init) ([**SYN\_Failsafe\_Manager**](structSYN__Failsafe__Manager.md) \* mgr, const [**SYN\_Failsafe\_Config**](structSYN__Failsafe__Config.md) \* config) <br>_Initialize failsafe manager instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_failsafe\_step**](#function-syn_failsafe_step) ([**SYN\_Failsafe\_Manager**](structSYN__Failsafe__Manager.md) \* mgr, uint32\_t now\_ms, uint16\_t \* out\_channels) <br>_Step failsafe manager state machine and retrieve active channels (or fallback channels if timed out)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_RC\_FAILSAFE\_MAX\_CHANNELS**](syn__rc__failsafe_8h.md#define-syn_rc_failsafe_max_channels)  `16U`<br> |

## Detailed Description


Monitors receiver frame updates and enforces pre-configured failsafe fallback positions or emergency disarming if signal loss exceeds timeout threshold. 


    
## Public Types Documentation




### enum SYN\_Failsafe\_ChannelMode 

```C++
enum SYN_Failsafe_ChannelMode {
    SYN_FAILSAFE_HOLD = 0,
    SYN_FAILSAFE_FALLBACK,
    SYN_FAILSAFE_DISARM
};
```



Failsafe Channel Mode. 


        

<hr>
## Public Functions Documentation




### function syn\_failsafe\_feed\_frame 

_Register valid incoming frame from RC receiver (resets watchdog timer)._ 
```C++
SYN_Status syn_failsafe_feed_frame (
    SYN_Failsafe_Manager * mgr,
    const uint16_t * in_channels,
    uint8_t num_channels,
    uint32_t timestamp_ms
) 
```





**Parameters:**


* `mgr` Pointer to failsafe manager struct. 
* `in_channels` Pointer to array of channel pulse widths in us. 
* `num_channels` Total channels provided (max 16). 
* `timestamp_ms` Current system timestamp in ms. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_failsafe\_init 

_Initialize failsafe manager instance._ 
```C++
SYN_Status syn_failsafe_init (
    SYN_Failsafe_Manager * mgr,
    const SYN_Failsafe_Config * config
) 
```





**Parameters:**


* `mgr` Pointer to failsafe manager struct. 
* `config` Pointer to failsafe configuration struct. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_failsafe\_step 

_Step failsafe manager state machine and retrieve active channels (or fallback channels if timed out)._ 
```C++
bool syn_failsafe_step (
    SYN_Failsafe_Manager * mgr,
    uint32_t now_ms,
    uint16_t * out_channels
) 
```





**Parameters:**


* `mgr` Pointer to failsafe manager struct. 
* `now_ms` Current system timestamp in ms. 
* `out_channels` Pointer to output channel array (must hold at least 16 uint16\_t values). 



**Returns:**

true if system is currently in Failsafe state, false if active link. 





        

<hr>
## Macro Definition Documentation





### define SYN\_RC\_FAILSAFE\_MAX\_CHANNELS 

```C++
#define SYN_RC_FAILSAFE_MAX_CHANNELS `16U`
```



Maximum supported failsafe RC channels. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_rc_failsafe.h`

