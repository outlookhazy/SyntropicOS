

# File syn\_fat.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_fat.h**](syn__fat_8h.md)

[Go to the source code of this file](syn__fat_8h_source.md)

_Custom lightweight FAT16/FAT32 VFS adapter._ 

* `#include "syn_vfs.h"`
* `#include "../drivers/syn_sd.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_VfsOps**](structSYN__VfsOps.md) \* | [**syn\_fat\_get\_ops**](#function-syn_fat_get_ops) (void) <br>_Get the VFS operations structure for the custom FAT filesystem._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fat\_init**](#function-syn_fat_init) (uint8\_t spi\_bus, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) cs) <br>_Initialize the SD card and mount the FAT filesystem in VFS at /sd._  |




























## Public Functions Documentation




### function syn\_fat\_get\_ops 

_Get the VFS operations structure for the custom FAT filesystem._ 
```C++
const SYN_VfsOps * syn_fat_get_ops (
    void
) 
```





**Returns:**

Pointer to the static [**SYN\_VfsOps**](structSYN__VfsOps.md) vtable. 





        

<hr>



### function syn\_fat\_init 

_Initialize the SD card and mount the FAT filesystem in VFS at /sd._ 
```C++
SYN_Status syn_fat_init (
    uint8_t spi_bus,
    SYN_GPIO_Pin cs
) 
```





**Parameters:**


* `spi_bus` SPI bus index. 
* `cs` GPIO CS pin index. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on failure. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_fat.h`

