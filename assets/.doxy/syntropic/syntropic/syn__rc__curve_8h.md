

# File syn\_rc\_curve.h



[**FileList**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_rc\_curve.h**](syn__rc__curve_8h.md)

[Go to the source code of this file](syn__rc__curve_8h_source.md)

_Zero-Heap RC Stick Exponential (Expo) & Deadband Curve Mapper._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/util/syn_qmath.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_RCCurve\_Config**](structSYN__RCCurve__Config.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_rc\_curve\_apply**](#function-syn_rc_curve_apply) (uint16\_t input\_us, const [**SYN\_RCCurve\_Config**](structSYN__RCCurve__Config.md) \* config) <br>_Apply deadband, exponential response, and dual-rate scaling to raw RC channel (1000..2000 us)._  |




























## Detailed Description


Provides stick input response shaping for drones/RC models:
* Deadband filtering: Ignores micro-stick jitter around neutral center (1500 us +/- deadband).
* Exponential curve: y = (1 - expo) \* x + expo \* x^3 (softens center stick precision).
* Dual Rate: Scales output range (0..100%).




All operations use Q16.16 fixed-point arithmetic. Zero float / zero heap. 


    
## Public Functions Documentation




### function syn\_rc\_curve\_apply 

_Apply deadband, exponential response, and dual-rate scaling to raw RC channel (1000..2000 us)._ 
```C++
uint16_t syn_rc_curve_apply (
    uint16_t input_us,
    const SYN_RCCurve_Config * config
) 
```





**Parameters:**


* `input_us` Raw input pulse width in microseconds (1000..2000 us). 
* `config` Pointer to RC curve configuration. 



**Returns:**

Scaled & shaped output in microseconds (clamped 1000..2000 us). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_rc_curve.h`

