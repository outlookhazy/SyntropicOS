

# File syn\_port\_flash.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_flash.h**](syn__port__flash_8h.md)

[Go to the source code of this file](syn__port__flash_8h_source.md)

_Flash storage port interface — implement these for your platform._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_flash\_erase**](#function-syn_port_flash_erase) (uint32\_t addr) <br>_Erase a flash sector._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_flash\_read**](#function-syn_port_flash_read) (uint32\_t addr, void \* buf, size\_t len) <br>_Read from flash._  |
|  uint32\_t | [**syn\_port\_flash\_sector\_size**](#function-syn_port_flash_sector_size) (uint32\_t addr) <br>_Get the sector size for the given address._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_flash\_write**](#function-syn_port_flash_write) (uint32\_t addr, const void \* buf, size\_t len) <br>_Write to flash._  |




























## Detailed Description


Used by the parameter store module for wear-leveled persistent storage. The port exposes raw sector-level operations; SyntropicOS handles wear leveling. 


    
## Public Functions Documentation




### function syn\_port\_flash\_erase 

_Erase a flash sector._ 
```C++
SYN_Status syn_port_flash_erase (
    uint32_t addr
) 
```





**Parameters:**


* `addr` Start address of the sector (must be sector-aligned). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_flash\_read 

_Read from flash._ 
```C++
SYN_Status syn_port_flash_read (
    uint32_t addr,
    void * buf,
    size_t len
) 
```





**Parameters:**


* `addr` Source address in flash. 
* `buf` Destination buffer. 
* `len` Number of bytes to read. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_flash\_sector\_size 

_Get the sector size for the given address._ 
```C++
uint32_t syn_port_flash_sector_size (
    uint32_t addr
) 
```





**Parameters:**


* `addr` Address within the sector. 



**Returns:**

Sector size in bytes. 





        

<hr>



### function syn\_port\_flash\_write 

_Write to flash._ 
```C++
SYN_Status syn_port_flash_write (
    uint32_t addr,
    const void * buf,
    size_t len
) 
```



Flash must be erased before writing (writes can only clear bits). The implementation should handle any alignment requirements.




**Parameters:**


* `addr` Destination address in flash. 
* `buf` Source data. 
* `len` Number of bytes to write. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_flash.h`

