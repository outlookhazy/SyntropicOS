

# File syn\_vfs.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_vfs.h**](syn__vfs_8h.md)

[Go to the source code of this file](syn__vfs_8h_source.md)

_Virtual File System (VFS) abstraction layer._ 

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_VfsDir**](structSYN__VfsDir.md) <br>_Open directory descriptor._  |
| struct | [**SYN\_VfsDirEnt**](structSYN__VfsDirEnt.md) <br>_Directory entry returned by_ [_**syn\_vfs\_readdir()**_](syn__vfs_8c.md#function-syn_vfs_readdir) _._ |
| struct | [**SYN\_VfsFile**](structSYN__VfsFile.md) <br>_Open file descriptor._  |
| struct | [**SYN\_VfsMount**](structSYN__VfsMount.md) <br>_Mount point binding a path prefix to a filesystem._  |
| struct | [**SYN\_VfsOps**](structSYN__VfsOps.md) <br>_VFS operations structure. Implement these for your filesystem (e.g. LittleFS)._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  int | [**syn\_vfs\_close**](#function-syn_vfs_close) (int fd) <br>_Close an open file descriptor._  |
|  int | [**syn\_vfs\_closedir**](#function-syn_vfs_closedir) (int dd) <br>_Close an open directory descriptor._  |
|  void | [**syn\_vfs\_init**](#function-syn_vfs_init) (void) <br>_Initialize the VFS registry. Clears all mounts and descriptors._  |
|  int | [**syn\_vfs\_mkdir**](#function-syn_vfs_mkdir) (const char \* path) <br>_Create a directory._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_vfs\_mount**](#function-syn_vfs_mount) (const char \* prefix, const [**SYN\_VfsOps**](structSYN__VfsOps.md) \* ops, void \* fs\_data) <br>_Mount a filesystem at a path prefix._  |
|  int | [**syn\_vfs\_open**](#function-syn_vfs_open) (const char \* path, int flags) <br>_Open a file._  |
|  int | [**syn\_vfs\_opendir**](#function-syn_vfs_opendir) (const char \* path) <br>_Open a directory for iteration._  |
|  int | [**syn\_vfs\_read**](#function-syn_vfs_read) (int fd, void \* buf, size\_t len) <br>_Read from an open file._  |
|  int | [**syn\_vfs\_readdir**](#function-syn_vfs_readdir) (int dd, [**SYN\_VfsDirEnt**](structSYN__VfsDirEnt.md) \* ent) <br>_Read the next directory entry._  |
|  int32\_t | [**syn\_vfs\_seek**](#function-syn_vfs_seek) (int fd, int32\_t offset, int whence) <br>_Seek in an open file._  |
|  int32\_t | [**syn\_vfs\_tell**](#function-syn_vfs_tell) (int fd) <br>_Get current position in an open file._  |
|  int | [**syn\_vfs\_unlink**](#function-syn_vfs_unlink) (const char \* path) <br>_Delete a file._  |
|  int | [**syn\_vfs\_write**](#function-syn_vfs_write) (int fd, const void \* buf, size\_t len) <br>_Write to an open file._  |




























## Public Functions Documentation




### function syn\_vfs\_close 

_Close an open file descriptor._ 
```C++
int syn_vfs_close (
    int fd
) 
```





**Parameters:**


* `fd` File descriptor returned by [**syn\_vfs\_open()**](syn__vfs_8h.md#function-syn_vfs_open). 



**Returns:**

0 on success, or negative error code. 





        

<hr>



### function syn\_vfs\_closedir 

_Close an open directory descriptor._ 
```C++
int syn_vfs_closedir (
    int dd
) 
```





**Parameters:**


* `dd` Directory descriptor. 



**Returns:**

0 on success, or negative error code. 





        

<hr>



### function syn\_vfs\_init 

_Initialize the VFS registry. Clears all mounts and descriptors._ 
```C++
void syn_vfs_init (
    void
) 
```




<hr>



### function syn\_vfs\_mkdir 

_Create a directory._ 
```C++
int syn_vfs_mkdir (
    const char * path
) 
```





**Parameters:**


* `path` Absolute path including mount prefix. 



**Returns:**

0 on success, or negative error code. 





        

<hr>



### function syn\_vfs\_mount 

_Mount a filesystem at a path prefix._ 
```C++
SYN_Status syn_vfs_mount (
    const char * prefix,
    const SYN_VfsOps * ops,
    void * fs_data
) 
```





**Parameters:**


* `prefix` Mount path prefix (e.g. "/flash"). 
* `ops` Filesystem operations vtable. 
* `fs_data` Opaque filesystem context (e.g. lfs\_t \*). 



**Returns:**

SYN\_OK on success, SYN\_ERROR if table is full or prefix exists. 





        

<hr>



### function syn\_vfs\_open 

_Open a file._ 
```C++
int syn_vfs_open (
    const char * path,
    int flags
) 
```





**Parameters:**


* `path` Absolute path including mount prefix. 
* `flags` Combination of SYN\_O\_\* flags. 



**Returns:**

File descriptor (&gt;= 0) on success, or negative error code. 





        

<hr>



### function syn\_vfs\_opendir 

_Open a directory for iteration._ 
```C++
int syn_vfs_opendir (
    const char * path
) 
```





**Parameters:**


* `path` Absolute path including mount prefix. 



**Returns:**

Directory descriptor (&gt;= 0) on success, or negative error code. 





        

<hr>



### function syn\_vfs\_read 

_Read from an open file._ 
```C++
int syn_vfs_read (
    int fd,
    void * buf,
    size_t len
) 
```





**Parameters:**


* `fd` File descriptor. 
* `buf` Destination buffer. 
* `len` Maximum bytes to read. 



**Returns:**

Bytes read on success, or negative error code. 





        

<hr>



### function syn\_vfs\_readdir 

_Read the next directory entry._ 
```C++
int syn_vfs_readdir (
    int dd,
    SYN_VfsDirEnt * ent
) 
```





**Parameters:**


* `dd` Directory descriptor returned by [**syn\_vfs\_opendir()**](syn__vfs_8h.md#function-syn_vfs_opendir). 
* `ent` [out] Filled with the next entry. 



**Returns:**

1 if an entry was read, 0 at end-of-directory, or negative error. 





        

<hr>



### function syn\_vfs\_seek 

_Seek in an open file._ 
```C++
int32_t syn_vfs_seek (
    int fd,
    int32_t offset,
    int whence
) 
```





**Parameters:**


* `fd` File descriptor. 
* `offset` Byte offset. 
* `whence` SYN\_SEEK\_SET, SYN\_SEEK\_CUR, or SYN\_SEEK\_END. 



**Returns:**

New absolute offset on success, or negative error code. 





        

<hr>



### function syn\_vfs\_tell 

_Get current position in an open file._ 
```C++
int32_t syn_vfs_tell (
    int fd
) 
```





**Parameters:**


* `fd` File descriptor. 



**Returns:**

Current offset, or negative error code. 





        

<hr>



### function syn\_vfs\_unlink 

_Delete a file._ 
```C++
int syn_vfs_unlink (
    const char * path
) 
```





**Parameters:**


* `path` Absolute path including mount prefix. 



**Returns:**

0 on success, or negative error code. 





        

<hr>



### function syn\_vfs\_write 

_Write to an open file._ 
```C++
int syn_vfs_write (
    int fd,
    const void * buf,
    size_t len
) 
```





**Parameters:**


* `fd` File descriptor. 
* `buf` Source buffer. 
* `len` Bytes to write. 



**Returns:**

Bytes written on success, or negative error code. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_vfs.h`

