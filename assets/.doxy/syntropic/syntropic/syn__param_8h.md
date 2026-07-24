

# File syn\_param.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_param.h**](syn__param_8h.md)

[Go to the source code of this file](syn__param_8h_source.md)

_Persistent parameter store with wear leveling._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_flash.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_ParamSlotHeader**](structSYN__ParamSlotHeader.md) <br>_Slot header — stored at the beginning of each parameter slot._  |
| struct | [**SYN\_ParamStore**](structSYN__ParamStore.md) <br>_Wear-leveled flash parameter store instance._  |






















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
|  uint16\_t | [**syn\_param\_write\_count**](#function-syn_param_write_count) (const [**SYN\_ParamStore**](structSYN__ParamStore.md) \* store) <br>_Get the current write count (approximate wear indicator)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_PARAM\_MAGIC**](syn__param_8h.md#define-syn_param_magic)  `0xC0DEu`<br>_Magic number for parameter slot headers._  |

## Detailed Description


Stores key-value parameters in flash with:
* **Wear leveling**: Slot rotation across flash sectors
* **Integrity**: CRC-16 on each slot
* **Atomic writes**: Sequence numbers to identify the latest valid slot
* **No dynamic allocation**: Fixed-size parameter block, caller-owned RAM






**
**




The flash region is divided into sectors. Each sector contains N slots. Each slot stores the full parameter block + header (sequence, CRC). Writes go to the next free slot. When a sector is full, the next sector is erased and writing continues there.



```C++
Sector 0:  [slot0][slot1][slot2]...[slotN]
Sector 1:  [slot0][slot1]...
Sector 2:  ...
```



The highest sequence number with a valid CRC is the active slot.




**
**


```C++
// Define your parameters as a struct
typedef struct {
    uint16_t brightness;
    int16_t  offset;
    uint8_t  mode;
} MyParams;

static SYN_ParamStore store;
static MyParams params;

syn_param_init(&store, FLASH_PARAM_START, 2, sizeof(MyParams));
syn_param_load(&store, &params);   // loads latest valid or defaults

params.brightness = 80;
syn_param_save(&store, &params);   // writes to next slot
```
 





    
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




### function syn\_param\_write\_count 

_Get the current write count (approximate wear indicator)._ 
```C++
static inline uint16_t syn_param_write_count (
    const SYN_ParamStore * store
) 
```





**Parameters:**


* `store` Store instance. 



**Returns:**

The sequence number, which increments with each save. 





        

<hr>
## Macro Definition Documentation





### define SYN\_PARAM\_MAGIC 

_Magic number for parameter slot headers._ 
```C++
#define SYN_PARAM_MAGIC `0xC0DEu`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_param.h`

