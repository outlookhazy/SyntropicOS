

# File syn\_fat.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_fat.c**](syn__fat_8c.md)

[Go to the source code of this file](syn__fat_8c_source.md)



* `#include "syn_fat.h"`
* `#include "../log/syn_log.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**DirEntryLoc**](structDirEntryLoc.md) <br>_Result of a directory scan — location of a matching entry._  |
| struct | [**FAT\_Volume**](structFAT__Volume.md) <br>_Internal FAT volume metadata parsed from the boot sector._  |
| struct | [**SYN\_FatFileCtx**](structSYN__FatFileCtx.md) <br>_Open file tracking context (pooled)._  |






## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_SD**](structSYN__SD.md) | [**g\_sd**](#variable-g_sd)  <br> |


## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_FatFileCtx**](structSYN__FatFileCtx.md) | [**g\_fat\_files**](#variable-g_fat_files)  <br> |
|  const [**SYN\_VfsOps**](structSYN__VfsOps.md) | [**g\_fat\_vfs\_ops**](#variable-g_fat_vfs_ops)   = `/* multi line expression */`<br> |
|  [**FAT\_Volume**](structFAT__Volume.md) | [**g\_vol**](#variable-g_vol)  <br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_VfsOps**](structSYN__VfsOps.md) \* | [**syn\_fat\_get\_ops**](#function-syn_fat_get_ops) (void) <br>_Get the VFS operations structure for the custom FAT filesystem._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fat\_init**](#function-syn_fat_init) (uint8\_t spi\_bus, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) cs) <br>_Initialize the SD card and mount the FAT filesystem in VFS at /sd._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**cluster\_to\_sector**](#function-cluster_to_sector) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t cluster) <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**fat\_parse\_bpb**](#function-fat_parse_bpb) ([**FAT\_Volume**](structFAT__Volume.md) \* vol, const uint8\_t \* bpb, uint32\_t volume\_start) <br> |
|  uint32\_t | [**find\_free\_cluster**](#function-find_free_cluster) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol) <br> |
|  uint32\_t | [**find\_partition\_start**](#function-find_partition_start) (const uint8\_t \* sector0) <br> |
|  uint32\_t | [**get\_cluster\_for\_offset**](#function-get_cluster_for_offset) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t start\_cluster, uint32\_t offset) <br> |
|  void | [**path\_to\_fat\_name**](#function-path_to_fat_name) (const char \* path, char \* fat\_name) <br> |
|  uint32\_t | [**read\_fat\_entry**](#function-read_fat_entry) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t cluster) <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**scan\_root\_dir**](#function-scan_root_dir) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, const char \* fat\_name, [**DirEntryLoc**](structDirEntryLoc.md) \* loc, [**bool**](syn__defs_8h.md#enum-bool) find\_empty\_slot) <br> |
|  int | [**syn\_fat\_vfs\_close**](#function-syn_fat_vfs_close) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file) <br> |
|  int | [**syn\_fat\_vfs\_open**](#function-syn_fat_vfs_open) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file, const char \* path, int flags, void \* fs\_data) <br> |
|  int | [**syn\_fat\_vfs\_read**](#function-syn_fat_vfs_read) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file, void \* buf, size\_t len) <br> |
|  int32\_t | [**syn\_fat\_vfs\_seek**](#function-syn_fat_vfs_seek) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file, int32\_t offset, int whence) <br> |
|  int32\_t | [**syn\_fat\_vfs\_tell**](#function-syn_fat_vfs_tell) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file) <br> |
|  int | [**syn\_fat\_vfs\_unlink**](#function-syn_fat_vfs_unlink) (const char \* path, void \* fs\_data) <br> |
|  int | [**syn\_fat\_vfs\_write**](#function-syn_fat_vfs_write) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file, const void \* buf, size\_t len) <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**write\_fat\_entry**](#function-write_fat_entry) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t cluster, uint32\_t value) <br> |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**TAG**](syn__fat_8c.md#define-tag)  `"syn\_fat"`<br> |

## Public Attributes Documentation




### variable g\_sd 

```C++
SYN_SD g_sd;
```




<hr>
## Public Static Attributes Documentation




### variable g\_fat\_files 

```C++
SYN_FatFileCtx g_fat_files[SYN_VFS_MAX_OPEN_FILES];
```




<hr>



### variable g\_fat\_vfs\_ops 

```C++
const SYN_VfsOps g_fat_vfs_ops;
```




<hr>



### variable g\_vol 

```C++
FAT_Volume g_vol;
```




<hr>
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
## Public Static Functions Documentation




### function cluster\_to\_sector 

```C++
static uint32_t cluster_to_sector (
    const FAT_Volume * vol,
    uint32_t cluster
) 
```




<hr>



### function fat\_parse\_bpb 

```C++
static bool fat_parse_bpb (
    FAT_Volume * vol,
    const uint8_t * bpb,
    uint32_t volume_start
) 
```




<hr>



### function find\_free\_cluster 

```C++
static uint32_t find_free_cluster (
    const FAT_Volume * vol
) 
```




<hr>



### function find\_partition\_start 

```C++
static uint32_t find_partition_start (
    const uint8_t * sector0
) 
```




<hr>



### function get\_cluster\_for\_offset 

```C++
static uint32_t get_cluster_for_offset (
    const FAT_Volume * vol,
    uint32_t start_cluster,
    uint32_t offset
) 
```




<hr>



### function path\_to\_fat\_name 

```C++
static void path_to_fat_name (
    const char * path,
    char * fat_name
) 
```




<hr>



### function read\_fat\_entry 

```C++
static uint32_t read_fat_entry (
    const FAT_Volume * vol,
    uint32_t cluster
) 
```




<hr>



### function scan\_root\_dir 

```C++
static bool scan_root_dir (
    const FAT_Volume * vol,
    const char * fat_name,
    DirEntryLoc * loc,
    bool find_empty_slot
) 
```




<hr>



### function syn\_fat\_vfs\_close 

```C++
static int syn_fat_vfs_close (
    SYN_VfsFile * file
) 
```




<hr>



### function syn\_fat\_vfs\_open 

```C++
static int syn_fat_vfs_open (
    SYN_VfsFile * file,
    const char * path,
    int flags,
    void * fs_data
) 
```




<hr>



### function syn\_fat\_vfs\_read 

```C++
static int syn_fat_vfs_read (
    SYN_VfsFile * file,
    void * buf,
    size_t len
) 
```




<hr>



### function syn\_fat\_vfs\_seek 

```C++
static int32_t syn_fat_vfs_seek (
    SYN_VfsFile * file,
    int32_t offset,
    int whence
) 
```




<hr>



### function syn\_fat\_vfs\_tell 

```C++
static int32_t syn_fat_vfs_tell (
    SYN_VfsFile * file
) 
```




<hr>



### function syn\_fat\_vfs\_unlink 

```C++
static int syn_fat_vfs_unlink (
    const char * path,
    void * fs_data
) 
```




<hr>



### function syn\_fat\_vfs\_write 

```C++
static int syn_fat_vfs_write (
    SYN_VfsFile * file,
    const void * buf,
    size_t len
) 
```




<hr>



### function write\_fat\_entry 

```C++
static bool write_fat_entry (
    const FAT_Volume * vol,
    uint32_t cluster,
    uint32_t value
) 
```




<hr>
## Macro Definition Documentation





### define TAG 

```C++
#define TAG `"syn_fat"`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_fat.c`

