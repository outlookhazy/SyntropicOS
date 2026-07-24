

# File syn\_power.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_power.c**](syn__power_8c.md)

[Go to the source code of this file](syn__power_8c_source.md)

_Power / voltage monitor implementation._ 

* `#include "syn_power.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_power\_init**](#function-syn_power_init) ([**SYN\_Power**](structSYN__Power.md) \* pwr, const [**SYN\_Power\_Config**](structSYN__Power__Config.md) \* cfg) <br>_Initialize power monitor._  |
|  void | [**syn\_power\_set\_errlog**](#function-syn_power_set_errlog) ([**SYN\_Power**](structSYN__Power.md) \* pwr, [**SYN\_ErrLog**](structSYN__ErrLog.md) \* errlog) <br>_Attach error log for brownout events._  |
|  void | [**syn\_power\_set\_stats**](#function-syn_power_set_stats) ([**SYN\_Power**](structSYN__Power.md) \* pwr, [**SYN\_Signal**](structSYN__Signal.md) \* stats) <br>_Attach signal stats window._  |
|  void | [**syn\_power\_update**](#function-syn_power_update) ([**SYN\_Power**](structSYN__Power.md) \* pwr) <br>_Update — read ADC and check thresholds._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_power.c`

