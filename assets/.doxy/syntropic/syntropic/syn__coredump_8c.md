

# File syn\_coredump.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_coredump.c**](syn__coredump_8c.md)

[Go to the source code of this file](syn__coredump_8c_source.md)

_Persistent core dump implementation._ [More...](#detailed-description)

* `#include "syn_coredump.h"`
* `#include "../util/syn_crc.h"`
* `#include "../util/syn_assert.h"`
* `#include "../port/syn_port_flash.h"`
* `#include "../port/syn_port_system.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_coredump\_clear**](#function-syn_coredump_clear) (void) <br>_Clear the core dump from flash._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_coredump\_read**](#function-syn_coredump_read) ([**SYN\_CoreDump**](structSYN__CoreDump.md) \* out) <br>_Read a core dump from flash._  |
|  void | [**syn\_coredump\_save**](#function-syn_coredump_save) (const [**SYN\_FaultContext**](structSYN__FaultContext.md) \* ctx) <br>_Save a core dump to flash._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**coredump\_compute\_crc**](#function-coredump_compute_crc) (const [**SYN\_CoreDump**](structSYN__CoreDump.md) \* dump) <br>_Compute CRC-32 over all fields preceding the crc member._  |


























## Detailed Description


Writes fault context + partial stack to a reserved flash sector. Designed to be callable from a hard fault handler (no heap, no locks, minimal stack usage). 


    
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
## Public Static Functions Documentation




### function coredump\_compute\_crc 

_Compute CRC-32 over all fields preceding the crc member._ 
```C++
static uint32_t coredump_compute_crc (
    const SYN_CoreDump * dump
) 
```





**Parameters:**


* `dump` Core dump to checksum. 



**Returns:**

CRC-32 value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_coredump.c`

