

# Struct SYN\_VfsMount



[**ClassList**](annotated.md) **>** [**SYN\_VfsMount**](structSYN__VfsMount.md)



_Mount point binding a path prefix to a filesystem._ 

* `#include <syn_vfs.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**fs\_data**](#variable-fs_data)  <br> |
|  const [**SYN\_VfsOps**](structSYN__VfsOps.md) \* | [**ops**](#variable-ops)  <br> |
|  const char \* | [**prefix**](#variable-prefix)  <br> |
|  size\_t | [**prefix\_len**](#variable-prefix_len)  <br> |












































## Public Attributes Documentation




### variable fs\_data 

```C++
void* SYN_VfsMount::fs_data;
```



Filesystem instance pointer (e.g. lfs\_t \*) 
 


        

<hr>



### variable ops 

```C++
const SYN_VfsOps* SYN_VfsMount::ops;
```



Filesystem operations for this mount 
 


        

<hr>



### variable prefix 

```C++
const char* SYN_VfsMount::prefix;
```



Mount path prefix (e.g. "/flash") 
 


        

<hr>



### variable prefix\_len 

```C++
size_t SYN_VfsMount::prefix_len;
```



Cached strlen(prefix) for fast matching 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_vfs.h`

