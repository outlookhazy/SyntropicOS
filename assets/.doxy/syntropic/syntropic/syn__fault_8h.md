

# File syn\_fault.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fault.h**](syn__fault_8h.md)

[Go to the source code of this file](syn__fault_8h_source.md)

_CPU Hard Fault / Post-Mortem Diagnostic log collector._ 

* `#include "../common/syn_defs.h"`
* `#include "syn_errlog.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_FaultContext**](structSYN__FaultContext.md) <br>_Register context dumped on CPU hard faults (ARM Cortex-M style)._  |
| struct | [**SYN\_FaultDump**](structSYN__FaultDump.md) <br>_Fault dump stored in noinit RAM across resets._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_fault\_capture**](#function-syn_fault_capture) (const [**SYN\_FaultContext**](structSYN__FaultContext.md) \* ctx) <br>_Capture a CPU fault register state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_fault\_check\_and\_log**](#function-syn_fault_check_and_log) ([**SYN\_ErrLog**](structSYN__ErrLog.md) \* errlog) <br>_Checks if a crash occurred in a previous boot, logs it, and clears the signature._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_FAULT\_SIGNATURE**](syn__fault_8h.md#define-syn_fault_signature)  `0xFA17C0DE`<br> |

## Public Functions Documentation




### function syn\_fault\_capture 

_Capture a CPU fault register state._ 
```C++
void syn_fault_capture (
    const SYN_FaultContext * ctx
) 
```



Saves register dump to uninitialized RAM and sets the signature. Safe to call from a hard fault handler (no heap, no locks).




**Parameters:**


* `ctx` Pointer to the register context to save (NULL is ignored). 




        

<hr>



### function syn\_fault\_check\_and\_log 

_Checks if a crash occurred in a previous boot, logs it, and clears the signature._ 
```C++
bool syn_fault_check_and_log (
    SYN_ErrLog * errlog
) 
```





**Parameters:**


* `errlog` Pointer to the active error log. 



**Returns:**

true if a fault was checked, logged, and cleared. 





        

<hr>
## Macro Definition Documentation





### define SYN\_FAULT\_SIGNATURE 

```C++
#define SYN_FAULT_SIGNATURE `0xFA17C0DE`
```



Magic value indicating a valid fault dump 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fault.h`

