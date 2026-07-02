

# File syn\_fwboot.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fwboot.c**](syn__fwboot_8c.md)

[Go to the source code of this file](syn__fwboot_8c_source.md)

_A/B firmware boot manager implementation._ 

* `#include "syn_fwboot.h"`
* `#include "syn_fwimage.h"`
* `#include "../util/syn_assert.h"`
* `#include "../port/syn_port_flash.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fwboot\_confirm**](#function-syn_fwboot_confirm) ([**SYN\_FwBootManager**](structSYN__FwBootManager.md) \* mgr) <br>_Confirm the currently active slot as good._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fwboot\_init**](#function-syn_fwboot_init) ([**SYN\_FwBootManager**](structSYN__FwBootManager.md) \* mgr, uint32\_t slot\_a, uint32\_t slot\_b) <br>_Initialize the boot manager._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fwboot\_refresh**](#function-syn_fwboot_refresh) ([**SYN\_FwBootManager**](structSYN__FwBootManager.md) \* mgr) <br>_Reload slot headers from flash._  |
|  uint8\_t | [**syn\_fwboot\_select**](#function-syn_fwboot_select) ([**SYN\_FwBootManager**](structSYN__FwBootManager.md) \* mgr, [**bool**](syn__defs_8h.md#enum-bool) rollback) <br>_Select the best slot to boot._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**read\_header**](#function-read_header) (uint32\_t addr, [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) \* hdr) <br>_Read a firmware image header from flash._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**write\_state**](#function-write_state) (uint32\_t addr, const [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) \* hdr, uint8\_t new\_state) <br>_Write a new state to a firmware header (erase + rewrite)._  |


























## Public Functions Documentation




### function syn\_fwboot\_confirm 

_Confirm the currently active slot as good._ 
```C++
SYN_Status syn_fwboot_confirm (
    SYN_FwBootManager * mgr
) 
```



Promotes state from TESTING to CONFIRMED. Call after the application has started successfully (after syn\_boot\_mark\_healthy).




**Parameters:**


* `mgr` Boot manager. 



**Returns:**

SYN\_OK if confirmed, SYN\_ERROR if not in TESTING state. 





        

<hr>



### function syn\_fwboot\_init 

_Initialize the boot manager._ 
```C++
SYN_Status syn_fwboot_init (
    SYN_FwBootManager * mgr,
    uint32_t slot_a,
    uint32_t slot_b
) 
```



Reads image headers from both slots.




**Parameters:**


* `mgr` Boot manager instance. 
* `slot_a` Flash base address of slot A. 
* `slot_b` Flash base address of slot B. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_fwboot\_refresh 

_Reload slot headers from flash._ 
```C++
SYN_Status syn_fwboot_refresh (
    SYN_FwBootManager * mgr
) 
```



Call after an OTA update to refresh the cached state.




**Parameters:**


* `mgr` Boot manager. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_fwboot\_select 

_Select the best slot to boot._ 
```C++
uint8_t syn_fwboot_select (
    SYN_FwBootManager * mgr,
    bool rollback
) 
```



Priority:
* TESTING slot (continue testing a new image)
* NEW slot (first boot of a new image — marks as TESTING)
* CONFIRMED slot with highest version
* Any CONFIRMED slot




If a TESTING slot failed (called with rollback=true), it's marked INVALID and the other CONFIRMED slot is selected.




**Parameters:**


* `mgr` Boot manager. 
* `rollback` true if the previous boot was unhealthy (trigger rollback). 



**Returns:**

Slot index (SYN\_FW\_SLOT\_A or SYN\_FW\_SLOT\_B), or SYN\_FW\_SLOT\_NONE. 





        

<hr>
## Public Static Functions Documentation




### function read\_header 

_Read a firmware image header from flash._ 
```C++
static SYN_Status read_header (
    uint32_t addr,
    SYN_FwImageHeader * hdr
) 
```





**Parameters:**


* `addr` Flash address. 
* `hdr` [out] Header struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function write\_state 

_Write a new state to a firmware header (erase + rewrite)._ 
```C++
static SYN_Status write_state (
    uint32_t addr,
    const SYN_FwImageHeader * hdr,
    uint8_t new_state
) 
```





**Parameters:**


* `addr` Flash address of the header. 
* `hdr` Current header. 
* `new_state` New state value. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fwboot.c`

