

# File syn\_coredump.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_coredump.h**](syn__coredump_8h.md)

[Go to the source code of this file](syn__coredump_8h_source.md)

_Persistent core dump — saves fault context to flash._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "syn_fault.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CoreDump**](structSYN__CoreDump.md) <br>_Persistent core dump — written to flash on fault._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_coredump\_clear**](#function-syn_coredump_clear) (void) <br>_Clear the core dump from flash._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_coredump\_read**](#function-syn_coredump_read) ([**SYN\_CoreDump**](structSYN__CoreDump.md) \* out) <br>_Read a core dump from flash._  |
|  void | [**syn\_coredump\_save**](#function-syn_coredump_save) (const [**SYN\_FaultContext**](structSYN__FaultContext.md) \* ctx) <br>_Save a core dump to flash._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_COREDUMP\_MAGIC**](syn__coredump_8h.md#define-syn_coredump_magic)  `0x43444D50u`<br>_Magic number identifying a valid core dump ("CDMP")._  |
| define  | [**SYN\_COREDUMP\_STACK\_SIZE**](syn__coredump_8h.md#define-syn_coredump_stack_size)  `128`<br> |

## Detailed Description


Captures CPU register state and a partial stack snapshot during a hard fault and persists them to a reserved flash sector. On the next boot, the dump can be read back for post-mortem diagnostics.


Requires: SYN\_USE\_FAULT, CRC module, flash port.




**
**

Define these in syn\_config.h:
* `SYN_COREDUMP_FLASH_ADDR` — Start address of the reserved flash sector.
* `SYN_COREDUMP_STACK_SIZE` — Bytes of stack to capture (default 128).






**
**


```C++
// In your HardFault handler:
syn_coredump_save(&fault_context);

// At boot:
SYN_CoreDump dump;
if (syn_coredump_read(&dump)) {
    printf("Previous crash at PC=0x%08x, uptime=%lu ms\n",
           dump.regs.pc, dump.uptime_ms);
    syn_coredump_clear();
}
```
 





    
## Public Functions Documentation




### function syn\_coredump\_clear 

_Clear the core dump from flash._ 
```C++
void syn_coredump_clear (
    void
) 
```



Erases the reserved sector so subsequent reads return false. 


        

<hr>



### function syn\_coredump\_read 

_Read a core dump from flash._ 
```C++
bool syn_coredump_read (
    SYN_CoreDump * out
) 
```



Validates the magic number and CRC. If valid, copies the dump into the output buffer.




**Parameters:**


* `out` Output buffer for the core dump. 



**Returns:**

true if a valid core dump was found. 





        

<hr>



### function syn\_coredump\_save 

_Save a core dump to flash._ 
```C++
void syn_coredump_save (
    const SYN_FaultContext * ctx
) 
```



Safe to call from a hard fault handler: no heap, no locks. Erases the reserved sector and writes the dump.




**Parameters:**


* `ctx` Register context captured at fault time. 




        

<hr>
## Macro Definition Documentation





### define SYN\_COREDUMP\_MAGIC 

_Magic number identifying a valid core dump ("CDMP")._ 
```C++
#define SYN_COREDUMP_MAGIC `0x43444D50u`
```




<hr>



### define SYN\_COREDUMP\_STACK\_SIZE 

```C++
#define SYN_COREDUMP_STACK_SIZE `128`
```



Bytes of stack to capture 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_coredump.h`

