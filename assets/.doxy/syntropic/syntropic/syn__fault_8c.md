

# File syn\_fault.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fault.c**](syn__fault_8c.md)

[Go to the source code of this file](syn__fault_8c_source.md)

_CPU Hard Fault / Post-Mortem Diagnostics implementation._ 

* `#include "syn_fault.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_FaultDump**](structSYN__FaultDump.md) g\_fault\_dump | [**SYN\_NOINIT**](#variable-syn_noinit)  <br>_Global fault dump stored in uninitialized RAM._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_fault\_capture**](#function-syn_fault_capture) (const [**SYN\_FaultContext**](structSYN__FaultContext.md) \* ctx) <br>_Capture a CPU fault register state._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_fault\_check\_and\_log**](#function-syn_fault_check_and_log) ([**SYN\_ErrLog**](structSYN__ErrLog.md) \* errlog) <br>_Checks if a crash occurred in a previous boot, logs it, and clears the signature._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_NOINIT**](syn__fault_8c.md#define-syn_noinit)  <br> |

## Public Static Attributes Documentation




### variable SYN\_NOINIT 

_Global fault dump stored in uninitialized RAM._ 
```C++
SYN_FaultDump g_fault_dump SYN_NOINIT;
```




<hr>
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





### define SYN\_NOINIT 

```C++
#define SYN_NOINIT 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fault.c`

