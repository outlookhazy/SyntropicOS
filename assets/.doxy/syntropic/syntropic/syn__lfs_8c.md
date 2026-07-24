

# File syn\_lfs.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_lfs.c**](syn__lfs_8c.md)

[Go to the source code of this file](syn__lfs_8c_source.md)

_LittleFS filesystem VFS adapter implementation._ 

* `#include "syn_lfs.h"`
* `#include "../port/syn_port_flash.h"`
* `#include "../util/syn_assert.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_VfsOps**](structSYN__VfsOps.md) \* | [**syn\_lfs\_get\_ops**](#function-syn_lfs_get_ops) (void) <br>_Get the VFS operations structure for LittleFS._  |




























## Public Functions Documentation




### function syn\_lfs\_get\_ops 

_Get the VFS operations structure for LittleFS._ 
```C++
const SYN_VfsOps * syn_lfs_get_ops (
    void
) 
```





**Returns:**

Pointer to the static [**SYN\_VfsOps**](structSYN__VfsOps.md) vtable, or NULL if LittleFS is not available. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_lfs.c`

