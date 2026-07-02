

# File syn\_boot.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_boot.c**](syn__boot_8c.md)

[Go to the source code of this file](syn__boot_8c_source.md)

_Boot manager implementation._ 

* `#include "syn_boot.h"`
* `#include "../util/syn_assert.h"`
* `#include "../system/syn_errlog.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_boot\_clear\_safe\_mode**](#function-syn_boot_clear_safe_mode) ([**SYN\_Boot**](structSYN__Boot.md) \* boot) <br>_Force clear safe mode and reset fail counter._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_boot\_init**](#function-syn_boot_init) ([**SYN\_Boot**](structSYN__Boot.md) \* boot, [**SYN\_ParamStore**](structSYN__ParamStore.md) \* store, uint16\_t safe\_threshold) <br>_Initialize the boot manager._  |
|  void | [**syn\_boot\_log\_events**](#function-syn_boot_log_events) ([**SYN\_Boot**](structSYN__Boot.md) \* boot) <br>_Log boot events to errlog._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_boot\_mark\_healthy**](#function-syn_boot_mark_healthy) ([**SYN\_Boot**](structSYN__Boot.md) \* boot) <br>_Mark this boot as healthy._  |
|  void | [**syn\_boot\_set\_errlog**](#function-syn_boot_set_errlog) ([**SYN\_Boot**](structSYN__Boot.md) \* boot, [**SYN\_ErrLog**](structSYN__ErrLog.md) \* errlog) <br>_Set the errlog instance for boot event recording._  |
|  void | [**syn\_boot\_set\_reset\_reason**](#function-syn_boot_set_reset_reason) ([**SYN\_Boot**](structSYN__Boot.md) \* boot, uint8\_t reason) <br>_Set the reset reason for the current boot._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_BOOT\_ERR\_CRASH\_LOOP**](syn__boot_8c.md#define-syn_boot_err_crash_loop)  `0x0300`<br> |
| define  | [**SYN\_BOOT\_ERR\_SAFE\_MODE**](syn__boot_8c.md#define-syn_boot_err_safe_mode)  `0x0301`<br> |

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
## Macro Definition Documentation





### define SYN\_BOOT\_ERR\_CRASH\_LOOP 

```C++
#define SYN_BOOT_ERR_CRASH_LOOP `0x0300`
```



Repeated crash detected. 
 


        

<hr>



### define SYN\_BOOT\_ERR\_SAFE\_MODE 

```C++
#define SYN_BOOT_ERR_SAFE_MODE `0x0301`
```



Entered safe mode. 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_boot.c`

