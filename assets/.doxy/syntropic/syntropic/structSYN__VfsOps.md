

# Struct SYN\_VfsOps



[**ClassList**](annotated.md) **>** [**SYN\_VfsOps**](structSYN__VfsOps.md)



_VFS operations structure. Implement these for your filesystem (e.g. LittleFS)._ 

* `#include <syn_vfs.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int(\* | [**close**](#variable-close)  <br> |
|  int(\* | [**closedir**](#variable-closedir)  <br> |
|  int(\* | [**mkdir**](#variable-mkdir)  <br> |
|  int(\* | [**open**](#variable-open)  <br> |
|  int(\* | [**opendir**](#variable-opendir)  <br> |
|  int(\* | [**read**](#variable-read)  <br> |
|  int(\* | [**readdir**](#variable-readdir)  <br> |
|  int32\_t(\* | [**seek**](#variable-seek)  <br> |
|  int32\_t(\* | [**tell**](#variable-tell)  <br> |
|  int(\* | [**unlink**](#variable-unlink)  <br> |
|  int(\* | [**write**](#variable-write)  <br> |












































## Public Attributes Documentation




### variable close 

```C++
int(* SYN_VfsOps::close) (SYN_VfsFile *file);
```



Close a file 
 


        

<hr>



### variable closedir 

```C++
int(* SYN_VfsOps::closedir) (SYN_VfsDir *dir);
```



Close a directory 
 


        

<hr>



### variable mkdir 

```C++
int(* SYN_VfsOps::mkdir) (const char *path, void *fs_data);
```



Create a directory 
 


        

<hr>



### variable open 

```C++
int(* SYN_VfsOps::open) (SYN_VfsFile *file, const char *path, int flags, void *fs_data);
```



Open a file 
 


        

<hr>



### variable opendir 

```C++
int(* SYN_VfsOps::opendir) (SYN_VfsDir *dir, const char *path, void *fs_data);
```



Open a directory 
 


        

<hr>



### variable read 

```C++
int(* SYN_VfsOps::read) (SYN_VfsFile *file, void *buf, size_t len);
```



Read from a file 
 


        

<hr>



### variable readdir 

```C++
int(* SYN_VfsOps::readdir) (SYN_VfsDir *dir, SYN_VfsDirEnt *ent);
```



Read directory entry 


        

<hr>



### variable seek 

```C++
int32_t(* SYN_VfsOps::seek) (SYN_VfsFile *file, int32_t offset, int whence);
```



Seek in a file 
 


        

<hr>



### variable tell 

```C++
int32_t(* SYN_VfsOps::tell) (SYN_VfsFile *file);
```



Get file position 
 


        

<hr>



### variable unlink 

```C++
int(* SYN_VfsOps::unlink) (const char *path, void *fs_data);
```



Delete a file 
 


        

<hr>



### variable write 

```C++
int(* SYN_VfsOps::write) (SYN_VfsFile *file, const void *buf, size_t len);
```



Write to a file 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_vfs.h`

