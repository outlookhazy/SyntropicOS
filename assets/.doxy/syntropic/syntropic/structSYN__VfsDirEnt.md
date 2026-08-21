

# Struct SYN\_VfsDirEnt



[**ClassList**](annotated.md) **>** [**SYN\_VfsDirEnt**](structSYN__VfsDirEnt.md)



_Directory entry returned by_ [_**syn\_vfs\_readdir()**_](syn__vfs_8c.md#function-syn_vfs_readdir) _._

* `#include <syn_vfs.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_dir**](#variable-is_dir)  <br> |
|  char | [**name**](#variable-name)  <br> |
|  uint32\_t | [**size**](#variable-size)  <br> |












































## Public Attributes Documentation




### variable is\_dir 

```C++
bool SYN_VfsDirEnt::is_dir;
```



true if the entry is a directory 
 


        

<hr>



### variable name 

```C++
char SYN_VfsDirEnt::name[64];
```



Entry name (null-terminated) 
 


        

<hr>



### variable size 

```C++
uint32_t SYN_VfsDirEnt::size;
```



File size in bytes (0 for dirs) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_vfs.h`

