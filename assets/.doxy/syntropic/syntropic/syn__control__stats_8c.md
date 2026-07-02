

# File syn\_control\_stats.c



[**FileList**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_control\_stats.c**](syn__control__stats_8c.md)

[Go to the source code of this file](syn__control__stats_8c_source.md)



* `#include "syntropic/control/syn_control_stats.h"`
* `#include "syntropic/util/syn_assert.h"`
* `#include <math.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_control\_stats\_report**](#function-syn_control_stats_report) (const [**SYN\_ControlStats**](structSYN__ControlStats.md) \* stats, [**SYN\_ControlReport**](structSYN__ControlReport.md) \* report) <br>_Calculate a report from the current accumulators._  |
|  void | [**syn\_control\_stats\_reset**](#function-syn_control_stats_reset) ([**SYN\_ControlStats**](structSYN__ControlStats.md) \* stats) <br>_Reset all accumulators._  |
|  void | [**syn\_control\_stats\_update**](#function-syn_control_stats_update) ([**SYN\_ControlStats**](structSYN__ControlStats.md) \* stats, int32\_t error, int32\_t output) <br>_Update stats with a new sample._  |




























## Public Functions Documentation




### function syn\_control\_stats\_report 

_Calculate a report from the current accumulators._ 
```C++
void syn_control_stats_report (
    const SYN_ControlStats * stats,
    SYN_ControlReport * report
) 
```





**Parameters:**


* `stats` Stats instance. 
* `report` Output report structure. 




        

<hr>



### function syn\_control\_stats\_reset 

_Reset all accumulators._ 
```C++
void syn_control_stats_reset (
    SYN_ControlStats * stats
) 
```





**Parameters:**


* `stats` Stats instance to reset. 




        

<hr>



### function syn\_control\_stats\_update 

_Update stats with a new sample._ 
```C++
void syn_control_stats_update (
    SYN_ControlStats * stats,
    int32_t error,
    int32_t output
) 
```





**Parameters:**


* `stats` Stats instance. 
* `error` Current servo error (target - measured). 
* `output` Current motor output (-100 to 100, or raw counts). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_control_stats.c`

