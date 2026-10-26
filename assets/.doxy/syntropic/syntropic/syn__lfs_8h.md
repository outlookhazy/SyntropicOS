

# File syn\_lfs.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_lfs.h**](syn__lfs_8h.md)

[Go to the source code of this file](syn__lfs_8h_source.md)

_LittleFS filesystem VFS adapter._ 

* `#include "syn_vfs.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_LfsConfig**](structSYN__LfsConfig.md) <br>_LittleFS partition configuration._  |






















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
The documentation for this class was generated from the following file `src/syntropic/storage/syn_lfs.h`

