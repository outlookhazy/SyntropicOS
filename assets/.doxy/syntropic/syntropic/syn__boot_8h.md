

# File syn\_boot.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_boot.h**](syn__boot_8h.md)

[Go to the source code of this file](syn__boot_8h_source.md)

_Boot manager — reset reason, boot counter, safe-mode detection._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../storage/syn_param.h"`
* `#include "../system/syn_errlog.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Boot**](structSYN__Boot.md) <br>_Boot manager — crash-loop detection and safe-mode state._  |
| struct | [**SYN\_BootData**](structSYN__BootData.md) <br>_Boot data persisted to flash across resets._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_ResetReason**](#enum-syn_resetreason)  <br>_Reset reason codes — application can extend from SYN\_RESET\_USER._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_boot\_clear\_safe\_mode**](#function-syn_boot_clear_safe_mode) ([**SYN\_Boot**](structSYN__Boot.md) \* boot) <br>_Force clear safe mode and reset fail counter._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_boot\_init**](#function-syn_boot_init) ([**SYN\_Boot**](structSYN__Boot.md) \* boot, [**SYN\_ParamStore**](structSYN__ParamStore.md) \* store, uint16\_t safe\_threshold) <br>_Initialize the boot manager._  |
|  void | [**syn\_boot\_log\_events**](#function-syn_boot_log_events) ([**SYN\_Boot**](structSYN__Boot.md) \* boot) <br>_Log boot events to errlog._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_boot\_mark\_healthy**](#function-syn_boot_mark_healthy) ([**SYN\_Boot**](structSYN__Boot.md) \* boot) <br>_Mark this boot as healthy._  |
|  void | [**syn\_boot\_set\_errlog**](#function-syn_boot_set_errlog) ([**SYN\_Boot**](structSYN__Boot.md) \* boot, [**SYN\_ErrLog**](structSYN__ErrLog.md) \* errlog) <br>_Set the errlog instance for boot event recording._  |
|  void | [**syn\_boot\_set\_reset\_reason**](#function-syn_boot_set_reset_reason) ([**SYN\_Boot**](structSYN__Boot.md) \* boot, uint8\_t reason) <br>_Set the reset reason for the current boot._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_boot\_count**](#function-syn_boot_count) (const [**SYN\_Boot**](structSYN__Boot.md) \* boot) <br>_Get total boot count._  |
|  uint16\_t | [**syn\_boot\_fail\_count**](#function-syn_boot_fail_count) (const [**SYN\_Boot**](structSYN__Boot.md) \* boot) <br>_Get consecutive fail count._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_boot\_is\_safe\_mode**](#function-syn_boot_is_safe_mode) (const [**SYN\_Boot**](structSYN__Boot.md) \* boot) <br>_Check if we're in safe mode._  |
|  uint8\_t | [**syn\_boot\_reset\_reason**](#function-syn_boot_reset_reason) (const [**SYN\_Boot**](structSYN__Boot.md) \* boot) <br>_Get last reset reason._  |


























## Detailed Description


Uses the parameter store to persist boot state across resets. Detects crash loops (too many resets without a successful "mark healthy") and can trigger safe-mode behavior.




**
**


```C++
static SYN_Boot boot;
static SYN_ParamStore boot_store;

// In early init:
syn_param_init(&boot_store, BOOT_FLASH_ADDR, 2, sizeof(SYN_BootData));
syn_boot_init(&boot, &boot_store, 3);  // safe mode after 3 failures

if (syn_boot_is_safe_mode(&boot)) {
    // Load minimal config, disable risky features
}

// After successful startup:
syn_boot_mark_healthy(&boot);
```
 





    
## Public Types Documentation




### enum SYN\_ResetReason 

_Reset reason codes — application can extend from SYN\_RESET\_USER._ 
```C++
enum SYN_ResetReason {
    SYN_RESET_UNKNOWN = 0,
    SYN_RESET_POWER_ON = 1,
    SYN_RESET_WATCHDOG = 2,
    SYN_RESET_SOFTWARE = 3,
    SYN_RESET_EXTERNAL = 4,
    SYN_RESET_BROWNOUT = 5,
    SYN_RESET_ASSERT = 6,
    SYN_RESET_USER = 0x80
};
```




<hr>
## Public Functions Documentation




### function syn\_boot\_clear\_safe\_mode 

_Force clear safe mode and reset fail counter._ 
```C++
SYN_Status syn_boot_clear_safe_mode (
    SYN_Boot * boot
) 
```



Use from CLI to recover from safe mode.




**Parameters:**


* `boot` Boot manager. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_boot\_init 

_Initialize the boot manager._ 
```C++
SYN_Status syn_boot_init (
    SYN_Boot * boot,
    SYN_ParamStore * store,
    uint16_t safe_threshold
) 
```



Loads boot data from flash, increments boot counter, checks for crash loops. Call this very early in boot.




**Parameters:**


* `boot` Boot manager instance. 
* `store` Initialized parameter store for boot data. 
* `safe_threshold` Number of consecutive failed boots before safe mode. Set to 0 to disable safe mode detection. 



**Returns:**

SYN\_OK on normal boot, SYN\_ERROR on first-ever boot (no data). 





        

<hr>



### function syn\_boot\_log\_events 

_Log boot events to errlog._ 
```C++
void syn_boot_log_events (
    SYN_Boot * boot
) 
```



Records crash-loop (non-healthy previous boot) and safe-mode entry events. Call after [**syn\_boot\_set\_errlog()**](syn__boot_8h.md#function-syn_boot_set_errlog).




**Parameters:**


* `boot` Boot manager. 




        

<hr>



### function syn\_boot\_mark\_healthy 

_Mark this boot as healthy._ 
```C++
SYN_Status syn_boot_mark_healthy (
    SYN_Boot * boot
) 
```



Call after your application has started successfully (e.g., after all peripherals initialized, comms established). Resets the fail counter.




**Parameters:**


* `boot` Boot manager. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_boot\_set\_errlog 

_Set the errlog instance for boot event recording._ 
```C++
void syn_boot_set_errlog (
    SYN_Boot * boot,
    SYN_ErrLog * errlog
) 
```



Call after both boot and errlog are initialized, then call [**syn\_boot\_log\_events()**](syn__boot_8h.md#function-syn_boot_log_events) to retroactively log boot-related events.




**Parameters:**


* `boot` Boot manager. 
* `errlog` Initialized error log instance. 




        

<hr>



### function syn\_boot\_set\_reset\_reason 

_Set the reset reason for the current boot._ 
```C++
void syn_boot_set_reset_reason (
    SYN_Boot * boot,
    uint8_t reason
) 
```



Call before saving (or it gets saved on next boot\_init).




**Parameters:**


* `boot` Boot manager. 
* `reason` Reset reason code (SYN\_ResetReason or custom). 




        

<hr>
## Public Static Functions Documentation




### function syn\_boot\_count 

_Get total boot count._ 
```C++
static inline uint32_t syn_boot_count (
    const SYN_Boot * boot
) 
```





**Parameters:**


* `boot` Boot manager. 



**Returns:**

Monotonic boot count. 





        

<hr>



### function syn\_boot\_fail\_count 

_Get consecutive fail count._ 
```C++
static inline uint16_t syn_boot_fail_count (
    const SYN_Boot * boot
) 
```





**Parameters:**


* `boot` Boot manager. 



**Returns:**

Number of boots without a healthy mark. 





        

<hr>



### function syn\_boot\_is\_safe\_mode 

_Check if we're in safe mode._ 
```C++
static inline bool syn_boot_is_safe_mode (
    const SYN_Boot * boot
) 
```





**Parameters:**


* `boot` Boot manager. 



**Returns:**

true if in safe mode. 





        

<hr>



### function syn\_boot\_reset\_reason 

_Get last reset reason._ 
```C++
static inline uint8_t syn_boot_reset_reason (
    const SYN_Boot * boot
) 
```





**Parameters:**


* `boot` Boot manager. 



**Returns:**

SYN\_ResetReason or custom code. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_boot.h`

