

# File syn\_fwboot.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_fwboot.h**](syn__fwboot_8h.md)

[Go to the source code of this file](syn__fwboot_8h_source.md)

_A/B firmware boot manager — slot selection, rollback, confirm._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "syn_fwimage.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_FwBootManager**](structSYN__FwBootManager.md) <br>_A/B firmware boot manager — slot selection and rollback state._  |






















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
|  uint8\_t | [**syn\_fwboot\_active\_slot**](#function-syn_fwboot_active_slot) (const [**SYN\_FwBootManager**](structSYN__FwBootManager.md) \* mgr) <br>_Get the currently active slot._  |
|  uint32\_t | [**syn\_fwboot\_slot\_addr**](#function-syn_fwboot_slot_addr) (const [**SYN\_FwBootManager**](structSYN__FwBootManager.md) \* mgr, uint8\_t slot) <br>_Get the flash base address of a slot._  |
|  const [**SYN\_FwImageHeader**](structSYN__FwImageHeader.md) \* | [**syn\_fwboot\_slot\_header**](#function-syn_fwboot_slot_header) (const [**SYN\_FwBootManager**](structSYN__FwBootManager.md) \* mgr, uint8\_t slot) <br>_Get the cached header for a slot._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_FW\_SLOT\_A**](syn__fwboot_8h.md#define-syn_fw_slot_a)  `0`<br> |
| define  | [**SYN\_FW\_SLOT\_B**](syn__fwboot_8h.md#define-syn_fw_slot_b)  `1`<br> |
| define  | [**SYN\_FW\_SLOT\_NONE**](syn__fwboot_8h.md#define-syn_fw_slot_none)  `0xFF`<br> |

## Detailed Description


Works with syn\_fwimage headers to manage dual firmware slots. Integrates with syn\_boot for crash-loop detection.




**
**


* Bootloader reads both slot headers
* [**syn\_fwboot\_select()**](syn__fwboot_8h.md#function-syn_fwboot_select) picks the best slot to boot
* If selected slot is NEW → mark as TESTING before jumping
* Application calls [**syn\_fwboot\_confirm()**](syn__fwboot_8h.md#function-syn_fwboot_confirm) after healthy startup
* If boot fails → next boot sees TESTING + unhealthy → rollback






**
**


```C++
SYN_FwBootManager mgr;
syn_fwboot_init(&mgr, SLOT_A_ADDR, SLOT_B_ADDR);

// In bootloader: pick which slot to run
uint8_t slot = syn_fwboot_select(&mgr);
uint32_t entry = syn_fwboot_slot_addr(&mgr, slot)
                 + sizeof(SYN_FwImageHeader);
// Jump to entry...

// In application: after successful startup
syn_fwboot_confirm(&mgr);
```
 





    
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




### function syn\_fwboot\_active\_slot 

_Get the currently active slot._ 
```C++
static inline uint8_t syn_fwboot_active_slot (
    const SYN_FwBootManager * mgr
) 
```





**Parameters:**


* `mgr` Boot manager. 



**Returns:**

Active slot index. 





        

<hr>



### function syn\_fwboot\_slot\_addr 

_Get the flash base address of a slot._ 
```C++
static inline uint32_t syn_fwboot_slot_addr (
    const SYN_FwBootManager * mgr,
    uint8_t slot
) 
```





**Parameters:**


* `mgr` Boot manager. 
* `slot` Slot index (SYN\_FW\_SLOT\_A or SYN\_FW\_SLOT\_B). 



**Returns:**

Flash base address, or 0 if invalid slot. 





        

<hr>



### function syn\_fwboot\_slot\_header 

_Get the cached header for a slot._ 
```C++
static inline const SYN_FwImageHeader * syn_fwboot_slot_header (
    const SYN_FwBootManager * mgr,
    uint8_t slot
) 
```





**Parameters:**


* `mgr` Boot manager. 
* `slot` Slot index. 



**Returns:**

Pointer to cached header, or NULL if invalid slot. 





        

<hr>
## Macro Definition Documentation





### define SYN\_FW\_SLOT\_A 

```C++
#define SYN_FW_SLOT_A `0`
```



Firmware slot A index. 
 


        

<hr>



### define SYN\_FW\_SLOT\_B 

```C++
#define SYN_FW_SLOT_B `1`
```



Firmware slot B index. 
 


        

<hr>



### define SYN\_FW\_SLOT\_NONE 

```C++
#define SYN_FW_SLOT_NONE `0xFF`
```



No valid slot. 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fwboot.h`

