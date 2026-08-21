

# Struct SYN\_VfsFile



[**ClassList**](annotated.md) **>** [**SYN\_VfsFile**](structSYN__VfsFile.md)



_Open file descriptor._ [More...](#detailed-description)

* `#include <syn_vfs.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**fs\_file**](#variable-fs_file)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_open**](#variable-is_open)  <br> |
|  const [**SYN\_VfsOps**](structSYN__VfsOps.md) \* | [**ops**](#variable-ops)  <br> |












































## Detailed Description


Forward declaration of file descriptor 
 


    
## Public Attributes Documentation




### variable fs\_file 

```C++
void* SYN_VfsFile::fs_file;
```



Filesystem-specific file context pointer 
 


        

<hr>



### variable is\_open 

```C++
bool SYN_VfsFile::is_open;
```



true while the descriptor is in use 
 


        

<hr>



### variable ops 

```C++
const SYN_VfsOps* SYN_VfsFile::ops;
```



Filesystem operations vtable 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_vfs.h`

