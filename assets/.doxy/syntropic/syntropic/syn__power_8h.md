

# File syn\_power.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_power.h**](syn__power_8h.md)

[Go to the source code of this file](syn__power_8h_source.md)

_Power / voltage monitor._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_adc.h"`
* `#include "../util/syn_hysteresis.h"`
* `#include "../dsp/syn_signal.h"`
* `#include "../system/syn_errlog.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Power**](structSYN__Power.md) <br>_Power monitor instance — ADC + hysteresis + brownout state._  |
| struct | [**SYN\_Power\_Config**](structSYN__Power__Config.md) <br>_Power monitor configuration._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_PowerCallback**](#typedef-syn_powercallback)  <br>_Power event callback._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_power\_init**](#function-syn_power_init) ([**SYN\_Power**](structSYN__Power.md) \* pwr, const [**SYN\_Power\_Config**](structSYN__Power__Config.md) \* cfg) <br>_Initialize power monitor._  |
|  void | [**syn\_power\_set\_errlog**](#function-syn_power_set_errlog) ([**SYN\_Power**](structSYN__Power.md) \* pwr, [**SYN\_ErrLog**](structSYN__ErrLog.md) \* errlog) <br>_Attach error log for brownout events._  |
|  void | [**syn\_power\_set\_stats**](#function-syn_power_set_stats) ([**SYN\_Power**](structSYN__Power.md) \* pwr, [**SYN\_Signal**](structSYN__Signal.md) \* stats) <br>_Attach signal stats window._  |
|  void | [**syn\_power\_update**](#function-syn_power_update) ([**SYN\_Power**](structSYN__Power.md) \* pwr) <br>_Update — read ADC and check thresholds._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_power\_is\_brownout**](#function-syn_power_is_brownout) (const [**SYN\_Power**](structSYN__Power.md) \* pwr) <br>_Check if currently in brownout state._  |
|  int32\_t | [**syn\_power\_voltage\_mv**](#function-syn_power_voltage_mv) (const [**SYN\_Power**](structSYN__Power.md) \* pwr) <br>_Get last voltage reading in millivolts._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_POWER\_ERR\_BROWNOUT**](syn__power_8h.md#define-syn_power_err_brownout)  `0x0400`<br>_Brownout voltage detected (errlog code)._  |

## Detailed Description


Wraps ADC + hysteresis to provide voltage monitoring with brownout detection and recovery callbacks. Optionally pushes readings to a [**SYN\_Signal**](structSYN__Signal.md) window for noise statistics.


Usage: 
```C++
static SYN_Power pwr;
static SYN_ADC   vbat_adc;

SYN_Power_Config cfg = {
    .adc           = &vbat_adc,
    .brownout_mv   = 3000,   // 3.0V
    .restore_mv    = 3200,   // 3.2V (hysteresis)
    .on_brownout   = my_brownout_handler,
    .on_restore    = my_restore_handler,
};
syn_power_init(&pwr, &cfg);

// In your main loop:
syn_power_update(&pwr);
int32_t mv = syn_power_voltage_mv(&pwr);
```
 


    
## Public Types Documentation




### typedef SYN\_PowerCallback 

_Power event callback._ 
```C++
typedef void(* SYN_PowerCallback) (struct SYN_Power *pwr, void *ctx);
```





**Parameters:**


* `pwr` Power monitor that fired. 
* `ctx` User context. 




        

<hr>
## Public Functions Documentation




### function syn\_power\_init 

_Initialize power monitor._ 
```C++
void syn_power_init (
    SYN_Power * pwr,
    const SYN_Power_Config * cfg
) 
```





**Parameters:**


* `pwr` Power monitor instance. 
* `cfg` Configuration. 




        

<hr>



### function syn\_power\_set\_errlog 

_Attach error log for brownout events._ 
```C++
void syn_power_set_errlog (
    SYN_Power * pwr,
    SYN_ErrLog * errlog
) 
```





**Parameters:**


* `pwr` Power monitor. 
* `errlog` Error log instance. 




        

<hr>



### function syn\_power\_set\_stats 

_Attach signal stats window._ 
```C++
void syn_power_set_stats (
    SYN_Power * pwr,
    SYN_Signal * stats
) 
```





**Parameters:**


* `pwr` Power monitor. 
* `stats` Initialized [**SYN\_Signal**](structSYN__Signal.md), or NULL to detach. 




        

<hr>



### function syn\_power\_update 

_Update — read ADC and check thresholds._ 
```C++
void syn_power_update (
    SYN_Power * pwr
) 
```



Call from your main loop or scheduler task.




**Parameters:**


* `pwr` Power monitor. 




        

<hr>
## Public Static Functions Documentation




### function syn\_power\_is\_brownout 

_Check if currently in brownout state._ 
```C++
static inline bool syn_power_is_brownout (
    const SYN_Power * pwr
) 
```





**Parameters:**


* `pwr` Power monitor. 



**Returns:**

true if in brownout. 





        

<hr>



### function syn\_power\_voltage\_mv 

_Get last voltage reading in millivolts._ 
```C++
static inline int32_t syn_power_voltage_mv (
    const SYN_Power * pwr
) 
```





**Parameters:**


* `pwr` Power monitor. 



**Returns:**

Voltage in mV. 





        

<hr>
## Macro Definition Documentation





### define SYN\_POWER\_ERR\_BROWNOUT 

_Brownout voltage detected (errlog code)._ 
```C++
#define SYN_POWER_ERR_BROWNOUT `0x0400`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_power.h`

