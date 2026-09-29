

# Struct SYN\_VfsDir



[**ClassList**](annotated.md) **>** [**SYN\_VfsDir**](structSYN__VfsDir.md)



_Open directory descriptor._ [More...](#detailed-description)

* `#include <syn_vfs.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**fs\_dir**](#variable-fs_dir)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_open**](#variable-is_open)  <br> |
|  const [**SYN\_VfsOps**](structSYN__VfsOps.md) \* | [**ops**](#variable-ops)  <br> |












































## Detailed Description


Forward declaration of directory descriptor 


    
## Public Attributes Documentation




### variable fs\_dir 

```C++
void* SYN_VfsDir::fs_dir;
```



Filesystem-specific directory context pointer 
 


        

<hr>



### variable is\_open 

```C++
bool SYN_VfsDir::is_open;
```



true while the descriptor is in use 
 


        

<hr>



### variable ops 

```C++
const SYN_VfsOps* SYN_VfsDir::ops;
```



Filesystem operations vtable 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_vfs.h`

