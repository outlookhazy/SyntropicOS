

# File syn\_param.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_param.c**](syn__param_8c.md)

[Go to the source code of this file](syn__param_8c_source.md)

_Persistent parameter store with wear leveling._ 

* `#include "syn_param.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_crc.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_param\_erase\_all**](#function-syn_param_erase_all) ([**SYN\_ParamStore**](structSYN__ParamStore.md) \* store) <br>_Erase all parameter data (factory reset)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_param\_init**](#function-syn_param_init) ([**SYN\_ParamStore**](structSYN__ParamStore.md) \* store, uint32\_t flash\_base, uint8\_t sector\_count, uint16\_t data\_size) <br>_Initialize the parameter store._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_param\_load**](#function-syn_param_load) (const [**SYN\_ParamStore**](structSYN__ParamStore.md) \* store, void \* data) <br>_Load parameters from the latest valid slot._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_param\_save**](#function-syn_param_save) ([**SYN\_ParamStore**](structSYN__ParamStore.md) \* store, const void \* data) <br>_Save parameters to the next slot (wear-leveled)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**align16**](#function-align16) (uint16\_t size) <br>_Align size up to 16-byte boundary for flash write alignment._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**read\_slot**](#function-read_slot) (const [**SYN\_ParamStore**](structSYN__ParamStore.md) \* store, uint8\_t sector, uint16\_t slot, [**SYN\_ParamSlotHeader**](structSYN__ParamSlotHeader.md) \* hdr, void \* data) <br>_Read a slot header and data from flash._  |
|  uint32\_t | [**slot\_addr**](#function-slot_addr) (const [**SYN\_ParamStore**](structSYN__ParamStore.md) \* store, uint8\_t sector, uint16\_t slot) <br>_Compute the flash address of a parameter slot._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**verify\_slot\_crc**](#function-verify_slot_crc) (const [**SYN\_ParamStore**](structSYN__ParamStore.md) \* store, uint8\_t sector, uint16\_t slot, uint16\_t expected\_crc) <br>_Verify CRC of a stored slot's data._  |


























## Public Functions Documentation




### function syn\_param\_erase\_all 

_Erase all parameter data (factory reset)._ 
```C++
SYN_Status syn_param_erase_all (
    SYN_ParamStore * store
) 
```





**Parameters:**


* `store` Store instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_param\_init 

_Initialize the parameter store._ 
```C++
SYN_Status syn_param_init (
    SYN_ParamStore * store,
    uint32_t flash_base,
    uint8_t sector_count,
    uint16_t data_size
) 
```



Scans flash to find the latest valid slot. Must be called before load or save.




**Parameters:**


* `store` Store instance. 
* `flash_base` Start address of the flash region for params. 
* `sector_count` Number of flash sectors to use for wear leveling. 
* `data_size` Size of the parameter struct in bytes. 



**Returns:**

SYN\_OK if at least one valid slot was found, SYN\_ERROR if flash is blank (use defaults). 





        

<hr>



### function syn\_param\_load 

_Load parameters from the latest valid slot._ 
```C++
SYN_Status syn_param_load (
    const SYN_ParamStore * store,
    void * data
) 
```





**Parameters:**


* `store` Initialized store. 
* `data` Buffer to receive the parameter data (must be data\_size bytes). 



**Returns:**

SYN\_OK on success, SYN\_ERROR if no valid data found. 





        

<hr>



### function syn\_param\_save 

_Save parameters to the next slot (wear-leveled)._ 
```C++
SYN_Status syn_param_save (
    SYN_ParamStore * store,
    const void * data
) 
```



Writes to the next free slot, rotating across sectors. When a sector fills up, the next sector is erased and writing continues there.




**Parameters:**


* `store` Initialized store. 
* `data` Parameter data to save (must be data\_size bytes). 



**Returns:**

SYN\_OK on success, SYN\_ERROR on flash write failure. 





        

<hr>
## Public Static Functions Documentation




### function align16 

_Align size up to 16-byte boundary for flash write alignment._ 
```C++
static uint16_t align16 (
    uint16_t size
) 
```





**Parameters:**


* `size` Size to align. 



**Returns:**

Aligned size. 





        

<hr>



### function read\_slot 

_Read a slot header and data from flash._ 
```C++
static bool read_slot (
    const SYN_ParamStore * store,
    uint8_t sector,
    uint16_t slot,
    SYN_ParamSlotHeader * hdr,
    void * data
) 
```





**Parameters:**


* `store` Parameter store. 
* `sector` Sector index. 
* `slot` Slot index. 
* `hdr` [out] Slot header. 
* `data` [out] Slot data. 



**Returns:**

true on success. 





        

<hr>



### function slot\_addr 

_Compute the flash address of a parameter slot._ 
```C++
static uint32_t slot_addr (
    const SYN_ParamStore * store,
    uint8_t sector,
    uint16_t slot
) 
```





**Parameters:**


* `store` Parameter store. 
* `sector` Sector index. 
* `slot` Slot index within sector. 



**Returns:**

Absolute flash address. 





        

<hr>



### function verify\_slot\_crc 

_Verify CRC of a stored slot's data._ 
```C++
static bool verify_slot_crc (
    const SYN_ParamStore * store,
    uint8_t sector,
    uint16_t slot,
    uint16_t expected_crc
) 
```





**Parameters:**


* `store` Parameter store. 
* `sector` Sector index. 
* `slot` Slot index. 
* `expected_crc` Expected CRC value. 



**Returns:**

true if CRC matches. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_param.c`

