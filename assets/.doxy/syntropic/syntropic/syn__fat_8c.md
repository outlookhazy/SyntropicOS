

# File syn\_fat.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_fat.c**](syn__fat_8c.md)

[Go to the source code of this file](syn__fat_8c_source.md)

_Custom lightweight FAT16/FAT32 VFS adapter implementation._ 

* `#include "../log/syn_log.h"`
* `#include "../util/syn_pack.h"`
* `#include "syn_fat.h"`
* `#include <limits.h>`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**DirEntryLoc**](structDirEntryLoc.md) <br>_Result of a directory scan — location of a matching or free entry._  |
| struct | [**FAT\_Volume**](structFAT__Volume.md) <br>_Internal FAT volume metadata parsed from the boot sector._  |
| struct | [**SYN\_FatFileCtx**](structSYN__FatFileCtx.md) <br>_Open file tracking context (pooled)._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**DirScanResult**](#enum-dirscanresult)  <br>_Per-sector directory inspection outcome._  |






## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_FatFileCtx**](structSYN__FatFileCtx.md) | [**g\_fat\_files**](#variable-g_fat_files)  <br>_Static pool of open FAT file contexts._  |
|  const [**SYN\_VfsOps**](structSYN__VfsOps.md) | [**g\_fat\_vfs\_ops**](#variable-g_fat_vfs_ops)   = `/* multi line expression */`<br>_Static VFS operations table for the FAT16/FAT32 adapter._  |
|  [**SYN\_SD**](structSYN__SD.md) | [**g\_sd**](#variable-g_sd)  <br>_Underlying SPI SD card driver instance._  |
|  [**FAT\_Volume**](structFAT__Volume.md) | [**g\_vol**](#variable-g_vol)  <br>_Mounted FAT volume state singleton._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_VfsOps**](structSYN__VfsOps.md) \* | [**syn\_fat\_get\_ops**](#function-syn_fat_get_ops) (void) <br>_Get the VFS operations structure for the custom FAT filesystem._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_fat\_init**](#function-syn_fat_init) (uint8\_t spi\_bus, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) cs) <br>_Initialize the SD card and mount the FAT filesystem in VFS at /sd._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int | [**alloc\_file\_slot**](#function-alloc_file_slot) (void) <br>_Allocate an unused file context slot from the static pool._  |
|  uint32\_t | [**cluster\_to\_sector**](#function-cluster_to_sector) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t cluster) <br>_Convert a valid data cluster index (&gt;= 2) to its first LBA sector._  |
|  int | [**fat\_alloc\_initial\_cluster**](#function-fat_alloc_initial_cluster) ([**SYN\_FatFileCtx**](structSYN__FatFileCtx.md) \* ctx) <br>_Allocate the first data cluster (&gt;= 2) for a 0-byte file and persist to directory entry._  |
|  int | [**fat\_check\_open\_conflict**](#function-fat_check_open_conflict) (const char fat\_name, int flags) <br>_Validate open_ `flags` _and check for conflicting open handles on_`fat_name` _._ |
|  size\_t | [**fat\_clamp\_io\_len**](#function-fat_clamp_io_len) (size\_t len, uint32\_t max\_rem) <br>_Clamp a requested I/O transfer length to INT\_MAX and an unsigned 32-bit remainder._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**fat\_compute\_layout**](#function-fat_compute_layout) ([**FAT\_Volume**](structFAT__Volume.md) \* vol, const uint8\_t \* bpb, uint32\_t volume\_start) <br>_Compute derived sector offsets and total allocatable clusters for_ `vol` _._ |
|  int | [**fat\_create\_file**](#function-fat_create_file) (const char \* fat\_name, [**DirEntryLoc**](structDirEntryLoc.md) \* loc) <br>_Create a new empty 0-byte 8.3 file entry in the root directory (start\_cluster = 0)._  |
|  void | [**fat\_entry\_location**](#function-fat_entry_location) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t cluster, uint32\_t \* fat\_sector, uint32\_t \* offset) <br>_Compute FAT table sector and byte offset within that sector for a cluster._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**fat\_flags\_wants\_write**](#function-fat_flags_wants_write) (int flags) <br>_Check whether open_ `flags` _request write access._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**fat\_is\_name\_open**](#function-fat_is_name_open) (const char fat\_name) <br>_Check whether any open FAT file slot currently references_ `fat_name` _._ |
|  void | [**fat\_name\_to\_cstr**](#function-fat_name_to_cstr) (const char fat\_name, char out) <br>_Convert an 11-byte space-padded 8.3 FAT name into a NUL-terminated C string._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**fat\_open\_attr\_allowed**](#function-fat_open_attr_allowed) (uint8\_t attr, int flags) <br>_Check whether_ `flags` _are permitted for an existing directory entry's FAT attributes._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**fat\_parse\_bpb**](#function-fat_parse_bpb) ([**FAT\_Volume**](structFAT__Volume.md) \* vol, const uint8\_t \* bpb, uint32\_t volume\_start) <br>_Parse FAT16/FAT32 BIOS Parameter Block (BPB) from a volume boot sector._  |
|  int | [**fat\_resolve\_write\_cluster**](#function-fat_resolve_write_cluster) (uint32\_t start\_cluster, uint32\_t target\_idx, uint32\_t \* out\_cluster) <br>_Traverse or extend a file's cluster chain to reach cluster index_ `target_idx` _._ |
|  int | [**fat\_sync\_dir\_size**](#function-fat_sync_dir_size) (const [**SYN\_FatFileCtx**](structSYN__FatFileCtx.md) \* ctx, uint8\_t sec\_buf) <br>_Persist updated file size to the file's directory entry on SD media._  |
|  char | [**fat\_toupper**](#function-fat_toupper) (char c) <br>_Convert an ASCII lowercase letter to uppercase._  |
|  int | [**fat\_truncate\_file**](#function-fat_truncate_file) ([**SYN\_FatFileCtx**](structSYN__FatFileCtx.md) \* ctx) <br>_Truncate an open file to 0 bytes, resetting directory cluster fields and freeing chain._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**fat\_valid\_spc**](#function-fat_valid_spc) (uint8\_t spc) <br>_Validate that_ `spc` _is a power-of-two FAT sectors-per-cluster value in [1, 128]._ |
|  int | [**fat\_write\_sector\_step**](#function-fat_write_sector_step) ([**SYN\_FatFileCtx**](structSYN__FatFileCtx.md) \* ctx, const uint8\_t \* src, size\_t rem, uint8\_t sec\_buf, uint32\_t \* out\_chunk) <br>_Write a single sector slice during sequential file write._  |
|  uint32\_t | [**find\_free\_cluster**](#function-find_free_cluster) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol) <br>_Scan the FAT for the first unallocated cluster within [2, total\_clusters + 1]._  |
|  uint32\_t | [**find\_partition\_start**](#function-find_partition_start) (const uint8\_t \* sector0) <br>_Inspect sector 0 for an MBR partition table entry and return its LBA start._  |
|  void | [**format\_83\_chars**](#function-format_83_chars) (const char \* name, char fat\_name) <br>_Encode a single validated filename component into an 11-byte 8.3 buffer._  |
|  void | [**free\_cluster\_chain**](#function-free_cluster_chain) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t start\_cluster) <br>_Free an entire cluster chain starting at_ `start_cluster` _._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**get\_cluster\_for\_offset**](#function-get_cluster_for_offset) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t start\_cluster, uint32\_t offset, uint32\_t \* out\_cluster) <br>_Traverse a file's cluster chain to the cluster covering_ `offset` _._ |
|  [**DirScanResult**](syn__fat_8c.md#enum-dirscanresult) | [**inspect\_dir\_sector**](#function-inspect_dir_sector) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, const uint8\_t \* sec\_buf, uint32\_t sec, const char \* fat\_name, [**DirEntryLoc**](structDirEntryLoc.md) \* loc, [**bool**](syn__defs_8h.md#enum-bool) empty) <br>_Inspect the 16 directory entries within a single 512-byte sector._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_fat\_partition\_type**](#function-is_fat_partition_type) (uint8\_t type) <br>_Check whether an MBR partition type code corresponds to FAT16/FAT32._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_valid\_cluster**](#function-is_valid_cluster) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t cluster) <br>_Check whether_ `cluster` _is within the volume's allocatable cluster range._ |
|  int | [**path\_to\_fat\_name**](#function-path_to_fat_name) (const char \* path, char fat\_name) <br>_Validate a root-relative VFS path and convert it into an 11-byte 8.3 FAT name._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**read\_fat\_entry**](#function-read_fat_entry) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t cluster, uint32\_t \* out\_entry) <br>_Read the FAT table entry for a given cluster._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**scan\_fat16\_root**](#function-scan_fat16_root) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, const char \* fat\_name, [**DirEntryLoc**](structDirEntryLoc.md) \* loc, [**bool**](syn__defs_8h.md#enum-bool) empty) <br>_Scan a FAT16 fixed-sector root directory region._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**scan\_fat32\_root**](#function-scan_fat32_root) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, const char \* fat\_name, [**DirEntryLoc**](structDirEntryLoc.md) \* loc, [**bool**](syn__defs_8h.md#enum-bool) empty) <br>_Scan a FAT32 cluster-chained root directory._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**scan\_root\_dir**](#function-scan_root_dir) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, const char \* name, [**DirEntryLoc**](structDirEntryLoc.md) \* loc, [**bool**](syn__defs_8h.md#enum-bool) empty) <br>_Scan the volume root directory for a matching 8.3 entry or empty slot._  |
|  int | [**syn\_fat\_vfs\_close**](#function-syn_fat_vfs_close) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file) <br>_VFS close callback releasing the pooled FAT file context._  |
|  int | [**syn\_fat\_vfs\_open**](#function-syn_fat_vfs_open) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file, const char \* path, int flags, void \* fs\_data) <br>_VFS open callback for FAT16/FAT32 files._  |
|  int | [**syn\_fat\_vfs\_read**](#function-syn_fat_vfs_read) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file, void \* buf, size\_t len) <br>_VFS read callback reading sequential bytes from a FAT file._  |
|  int | [**syn\_fat\_vfs\_rename**](#function-syn_fat_vfs_rename) (const char \* old\_path, const char \* new\_path, void \* fs\_data) <br>_VFS rename callback renaming an 8.3 root directory entry in place._  |
|  int32\_t | [**syn\_fat\_vfs\_seek**](#function-syn_fat_vfs_seek) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file, int32\_t offset, int whence) <br>_VFS seek callback updating the current read/write cursor._  |
|  int | [**syn\_fat\_vfs\_stat**](#function-syn_fat_vfs_stat) (const char \* path, [**SYN\_VfsDirEnt**](structSYN__VfsDirEnt.md) \* ent, void \* fs\_data) <br>_VFS stat callback retrieving 8.3 entry metadata from the root directory._  |
|  int32\_t | [**syn\_fat\_vfs\_tell**](#function-syn_fat_vfs_tell) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file) <br>_VFS tell callback returning the current file offset._  |
|  int | [**syn\_fat\_vfs\_unlink**](#function-syn_fat_vfs_unlink) (const char \* path, void \* fs\_data) <br>_VFS unlink callback marking a directory entry deleted and freeing its cluster chain._  |
|  int | [**syn\_fat\_vfs\_write**](#function-syn_fat_vfs_write) ([**SYN\_VfsFile**](structSYN__VfsFile.md) \* file, const void \* buf, size\_t len) <br>_VFS write callback writing sequential bytes and extending clusters as needed._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**write\_fat\_entry**](#function-write_fat_entry) (const [**FAT\_Volume**](structFAT__Volume.md) \* vol, uint32\_t cluster, uint32\_t value) <br>_Write a FAT table entry (preserving FAT32 top 4 bits and updating FAT2)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**TAG**](syn__fat_8c.md#define-tag)  `"syn\_fat"`<br>_Logging tag for the FAT filesystem adapter._  |

## Public Types Documentation




### enum DirScanResult 

_Per-sector directory inspection outcome._ 
```C++
enum DirScanResult {
    DIR_SCAN_CONTINUE = 0,
    DIR_SCAN_FOUND = 1,
    DIR_SCAN_END = 2
};
```




<hr>
## Public Static Attributes Documentation




### variable g\_fat\_files 

_Static pool of open FAT file contexts._ 
```C++
SYN_FatFileCtx g_fat_files[SYN_VFS_MAX_OPEN_FILES];
```




<hr>



### variable g\_fat\_vfs\_ops 

_Static VFS operations table for the FAT16/FAT32 adapter._ 
```C++
const SYN_VfsOps g_fat_vfs_ops;
```




<hr>



### variable g\_sd 

_Underlying SPI SD card driver instance._ 
```C++
SYN_SD g_sd;
```




<hr>



### variable g\_vol 

_Mounted FAT volume state singleton._ 
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




### function alloc\_file\_slot 

_Allocate an unused file context slot from the static pool._ 
```C++
static int alloc_file_slot (
    void
) 
```





**Returns:**

Pool index (0 .. SYN\_VFS\_MAX\_OPEN\_FILES - 1), or -1 if full. 





        

<hr>



### function cluster\_to\_sector 

_Convert a valid data cluster index (&gt;= 2) to its first LBA sector._ 
```C++
static uint32_t cluster_to_sector (
    const FAT_Volume * vol,
    uint32_t cluster
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `cluster` Cluster number (&gt;= 2). 



**Returns:**

First LBA sector of the cluster. 





        

<hr>



### function fat\_alloc\_initial\_cluster 

_Allocate the first data cluster (&gt;= 2) for a 0-byte file and persist to directory entry._ 
```C++
static int fat_alloc_initial_cluster (
    SYN_FatFileCtx * ctx
) 
```





**Parameters:**


* `ctx` Open file context whose start\_cluster is 0. 



**Returns:**

0 on success, or negative error code (-2, -3, -5, -6). 





        

<hr>



### function fat\_check\_open\_conflict 

_Validate open_ `flags` _and check for conflicting open handles on_`fat_name` _._
```C++
static int fat_check_open_conflict (
    const char fat_name,
    int flags
) 
```





**Parameters:**


* `fat_name` 11-byte 8.3 directory name. 
* `flags` Requested SYN\_O\_\* open flags. 



**Returns:**

SYN\_OK if allowed, SYN\_INVALID\_PARAM on invalid flags, SYN\_BUSY on open write conflict. 





        

<hr>



### function fat\_clamp\_io\_len 

_Clamp a requested I/O transfer length to INT\_MAX and an unsigned 32-bit remainder._ 
```C++
static size_t fat_clamp_io_len (
    size_t len,
    uint32_t max_rem
) 
```





**Parameters:**


* `len` Caller-requested byte count. 
* `max_rem` Maximum allowable 32-bit byte span from current file offset. 



**Returns:**

Clamped byte count guaranteed to fit in both int and max\_rem. 





        

<hr>



### function fat\_compute\_layout 

_Compute derived sector offsets and total allocatable clusters for_ `vol` _._
```C++
static bool fat_compute_layout (
    FAT_Volume * vol,
    const uint8_t * bpb,
    uint32_t volume_start
) 
```





**Parameters:**


* `vol` [in,out] Volume metadata structure. 
* `bpb` 512-byte boot sector buffer. 
* `volume_start` LBA sector offset of the volume boot sector. 



**Returns:**

true if geometry and cluster count are valid, false otherwise. 





        

<hr>



### function fat\_create\_file 

_Create a new empty 0-byte 8.3 file entry in the root directory (start\_cluster = 0)._ 
```C++
static int fat_create_file (
    const char * fat_name,
    DirEntryLoc * loc
) 
```





**Parameters:**


* `fat_name` 11-byte 8.3 filename. 
* `loc` [out] Populated with the newly created directory entry metadata. 



**Returns:**

0 on success, or negative error code (-3, -6, -7). 





        

<hr>



### function fat\_entry\_location 

_Compute FAT table sector and byte offset within that sector for a cluster._ 
```C++
static void fat_entry_location (
    const FAT_Volume * vol,
    uint32_t cluster,
    uint32_t * fat_sector,
    uint32_t * offset
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `cluster` Cluster number. 
* `fat_sector` [out] LBA sector in primary FAT. 
* `offset` [out] Byte offset within the 512-byte FAT sector. 




        

<hr>



### function fat\_flags\_wants\_write 

_Check whether open_ `flags` _request write access._
```C++
static bool fat_flags_wants_write (
    int flags
) 
```





**Parameters:**


* `flags` Open flags (SYN\_O\_\*). 



**Returns:**

true if write mode is requested, false for SYN\_O\_RDONLY. 





        

<hr>



### function fat\_is\_name\_open 

_Check whether any open FAT file slot currently references_ `fat_name` _._
```C++
static bool fat_is_name_open (
    const char fat_name
) 
```





**Parameters:**


* `fat_name` 11-byte 8.3 directory name. 



**Returns:**

true if an open file slot matches `fat_name`, false otherwise. 





        

<hr>



### function fat\_name\_to\_cstr 

_Convert an 11-byte space-padded 8.3 FAT name into a NUL-terminated C string._ 
```C++
static void fat_name_to_cstr (
    const char fat_name,
    char out
) 
```





**Parameters:**


* `fat_name` 11-byte 8.3 FAT name. 
* `out` [out] Output buffer (at least 13 bytes). 




        

<hr>



### function fat\_open\_attr\_allowed 

_Check whether_ `flags` _are permitted for an existing directory entry's FAT attributes._
```C++
static bool fat_open_attr_allowed (
    uint8_t attr,
    int flags
) 
```





**Parameters:**


* `attr` FAT directory attribute byte. 
* `flags` Requested SYN\_O\_\* open flags. 



**Returns:**

true if opening is allowed, false if directory or read-only violation. 





        

<hr>



### function fat\_parse\_bpb 

_Parse FAT16/FAT32 BIOS Parameter Block (BPB) from a volume boot sector._ 
```C++
static bool fat_parse_bpb (
    FAT_Volume * vol,
    const uint8_t * bpb,
    uint32_t volume_start
) 
```





**Parameters:**


* `vol` [out] Volume metadata structure to populate. 
* `bpb` 512-byte boot sector buffer. 
* `volume_start` LBA sector offset of the volume boot sector. 



**Returns:**

true if valid 512-byte-sector FAT16/FAT32 BPB, false otherwise. 





        

<hr>



### function fat\_resolve\_write\_cluster 

_Traverse or extend a file's cluster chain to reach cluster index_ `target_idx` _._
```C++
static int fat_resolve_write_cluster (
    uint32_t start_cluster,
    uint32_t target_idx,
    uint32_t * out_cluster
) 
```





**Parameters:**


* `start_cluster` First cluster of the file. 
* `target_idx` Zero-based sequential cluster offset within the file. 
* `out_cluster` [out] Resolved or newly allocated cluster number. 



**Returns:**

0 on success, or negative error code (-2 .. -5). 





        

<hr>



### function fat\_sync\_dir\_size 

_Persist updated file size to the file's directory entry on SD media._ 
```C++
static int fat_sync_dir_size (
    const SYN_FatFileCtx * ctx,
    uint8_t sec_buf
) 
```





**Parameters:**


* `ctx` Open file context. 
* `sec_buf` Caller-owned 512-byte sector scratch buffer. 



**Returns:**

0 on success, or -7 on SD read/write failure. 





        

<hr>



### function fat\_toupper 

_Convert an ASCII lowercase letter to uppercase._ 
```C++
static char fat_toupper (
    char c
) 
```





**Parameters:**


* `c` Input character. 



**Returns:**

Uppercase equivalent if 'a'..'z', else `c` unchanged. 





        

<hr>



### function fat\_truncate\_file 

_Truncate an open file to 0 bytes, resetting directory cluster fields and freeing chain._ 
```C++
static int fat_truncate_file (
    SYN_FatFileCtx * ctx
) 
```





**Parameters:**


* `ctx` Open file context. 



**Returns:**

0 on success, or negative error code (-6, -7). 





        

<hr>



### function fat\_valid\_spc 

_Validate that_ `spc` _is a power-of-two FAT sectors-per-cluster value in [1, 128]._
```C++
static bool fat_valid_spc (
    uint8_t spc
) 
```





**Parameters:**


* `spc` BPB sectors\_per\_cluster byte. 



**Returns:**

true if valid power-of-two in [1, 128], false otherwise. 





        

<hr>



### function fat\_write\_sector\_step 

_Write a single sector slice during sequential file write._ 
```C++
static int fat_write_sector_step (
    SYN_FatFileCtx * ctx,
    const uint8_t * src,
    size_t rem,
    uint8_t sec_buf,
    uint32_t * out_chunk
) 
```





**Parameters:**


* `ctx` Open file context. 
* `src` Source data pointer for the current chunk. 
* `rem` Remaining bytes to write in the call. 
* `sec_buf` Caller-owned 512-byte sector scratch buffer. 
* `out_chunk` [out] Bytes written in this step. 



**Returns:**

0 on success, or negative error code. 





        

<hr>



### function find\_free\_cluster 

_Scan the FAT for the first unallocated cluster within [2, total\_clusters + 1]._ 
```C++
static uint32_t find_free_cluster (
    const FAT_Volume * vol
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 



**Returns:**

Free cluster number (&gt;= 2), or 0 if none available. 





        

<hr>



### function find\_partition\_start 

_Inspect sector 0 for an MBR partition table entry and return its LBA start._ 
```C++
static uint32_t find_partition_start (
    const uint8_t * sector0
) 
```





**Parameters:**


* `sector0` 512-byte buffer containing LBA 0. 



**Returns:**

Partition start sector, or 0 if sector 0 is a superfloppy or invalid. 





        

<hr>



### function format\_83\_chars 

_Encode a single validated filename component into an 11-byte 8.3 buffer._ 
```C++
static void format_83_chars (
    const char * name,
    char fat_name
) 
```





**Parameters:**


* `name` Validated filename without slashes. 
* `fat_name` [out] 11-byte space-padded 8.3 name buffer. 




        

<hr>



### function free\_cluster\_chain 

_Free an entire cluster chain starting at_ `start_cluster` _._
```C++
static void free_cluster_chain (
    const FAT_Volume * vol,
    uint32_t start_cluster
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `start_cluster` First cluster in the chain (no-op if &lt; 2). 




        

<hr>



### function get\_cluster\_for\_offset 

_Traverse a file's cluster chain to the cluster covering_ `offset` _._
```C++
static bool get_cluster_for_offset (
    const FAT_Volume * vol,
    uint32_t start_cluster,
    uint32_t offset,
    uint32_t * out_cluster
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `start_cluster` First cluster of the file. 
* `offset` Byte offset within the file. 
* `out_cluster` [out] Cluster index covering `offset`. 



**Returns:**

true on success, false on SD FAT read failure or corrupt/truncated cluster chain. 





        

<hr>



### function inspect\_dir\_sector 

_Inspect the 16 directory entries within a single 512-byte sector._ 
```C++
static DirScanResult inspect_dir_sector (
    const FAT_Volume * vol,
    const uint8_t * sec_buf,
    uint32_t sec,
    const char * fat_name,
    DirEntryLoc * loc,
    bool empty
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `sec_buf` 512-byte directory sector buffer. 
* `sec` LBA sector number of `sec_buf`. 
* `fat_name` 11-byte target 8.3 name (ignored if `empty` is true). 
* `loc` [out] Populated on match. 
* `empty` true to locate a free/deleted slot, false to match `fat_name`. 



**Returns:**

DIR\_SCAN\_FOUND, DIR\_SCAN\_END, or DIR\_SCAN\_CONTINUE. 





        

<hr>



### function is\_fat\_partition\_type 

_Check whether an MBR partition type code corresponds to FAT16/FAT32._ 
```C++
static bool is_fat_partition_type (
    uint8_t type
) 
```





**Parameters:**


* `type` MBR partition type byte. 



**Returns:**

true if recognized FAT partition type, false otherwise. 





        

<hr>



### function is\_valid\_cluster 

_Check whether_ `cluster` _is within the volume's allocatable cluster range._
```C++
static bool is_valid_cluster (
    const FAT_Volume * vol,
    uint32_t cluster
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `cluster` Cluster index to validate. 



**Returns:**

true if `cluster` is in [2, total\_clusters + 1] and below reserved markers. 





        

<hr>



### function path\_to\_fat\_name 

_Validate a root-relative VFS path and convert it into an 11-byte 8.3 FAT name._ 
```C++
static int path_to_fat_name (
    const char * path,
    char fat_name
) 
```





**Parameters:**


* `path` Relative file path (with or without single leading '/'). 
* `fat_name` [out] 11-byte 8.3 name buffer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on empty/dot path, SYN\_NOT\_FOUND on subdirectory. 





        

<hr>



### function read\_fat\_entry 

_Read the FAT table entry for a given cluster._ 
```C++
static bool read_fat_entry (
    const FAT_Volume * vol,
    uint32_t cluster,
    uint32_t * out_entry
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `cluster` Cluster index to query. 
* `out_entry` [out] Next cluster index, 0 if free, or 0x0FFFFFFF on EOF. 



**Returns:**

true on success, false on SD sector read error. 





        

<hr>



### function scan\_fat16\_root 

_Scan a FAT16 fixed-sector root directory region._ 
```C++
static bool scan_fat16_root (
    const FAT_Volume * vol,
    const char * fat_name,
    DirEntryLoc * loc,
    bool empty
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `fat_name` 11-byte 8.3 filename. 
* `loc` [out] Resulting entry location. 
* `empty` true to locate a free slot, false to find `fat_name`. 



**Returns:**

true if found, false otherwise. 





        

<hr>



### function scan\_fat32\_root 

_Scan a FAT32 cluster-chained root directory._ 
```C++
static bool scan_fat32_root (
    const FAT_Volume * vol,
    const char * fat_name,
    DirEntryLoc * loc,
    bool empty
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `fat_name` 11-byte 8.3 filename. 
* `loc` [out] Resulting entry location. 
* `empty` true to locate a free slot, false to find `fat_name`. 



**Returns:**

true if found, false otherwise. 





        

<hr>



### function scan\_root\_dir 

_Scan the volume root directory for a matching 8.3 entry or empty slot._ 
```C++
static bool scan_root_dir (
    const FAT_Volume * vol,
    const char * name,
    DirEntryLoc * loc,
    bool empty
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `name` 11-byte 8.3 filename. 
* `loc` [out] Entry location descriptor. 
* `empty` true to search for an unused slot, false to match `name`. 



**Returns:**

true if found, false otherwise. 





        

<hr>



### function syn\_fat\_vfs\_close 

_VFS close callback releasing the pooled FAT file context._ 
```C++
static int syn_fat_vfs_close (
    SYN_VfsFile * file
) 
```





**Parameters:**


* `file` VFS file descriptor. 



**Returns:**

0 on success, -1 if invalid. 





        

<hr>



### function syn\_fat\_vfs\_open 

_VFS open callback for FAT16/FAT32 files._ 
```C++
static int syn_fat_vfs_open (
    SYN_VfsFile * file,
    const char * path,
    int flags,
    void * fs_data
) 
```





**Parameters:**


* `file` VFS file descriptor. 
* `path` Mount-relative file path. 
* `flags` Open flags (SYN\_O\_\*). 
* `fs_data` Unused filesystem context pointer. 



**Returns:**

0 on success, or negative error code. 





        

<hr>



### function syn\_fat\_vfs\_read 

_VFS read callback reading sequential bytes from a FAT file._ 
```C++
static int syn_fat_vfs_read (
    SYN_VfsFile * file,
    void * buf,
    size_t len
) 
```





**Parameters:**


* `file` VFS file descriptor. 
* `buf` Destination buffer. 
* `len` Maximum bytes to read. 



**Returns:**

Bytes read (&gt;= 0), or negative error code. 





        

<hr>



### function syn\_fat\_vfs\_rename 

_VFS rename callback renaming an 8.3 root directory entry in place._ 
```C++
static int syn_fat_vfs_rename (
    const char * old_path,
    const char * new_path,
    void * fs_data
) 
```





**Parameters:**


* `old_path` Mount-relative source path. 
* `new_path` Mount-relative destination path. 
* `fs_data` Unused filesystem context pointer. 



**Returns:**

0 on success, or negative error code. 





        

<hr>



### function syn\_fat\_vfs\_seek 

_VFS seek callback updating the current read/write cursor._ 
```C++
static int32_t syn_fat_vfs_seek (
    SYN_VfsFile * file,
    int32_t offset,
    int whence
) 
```





**Parameters:**


* `file` VFS file descriptor. 
* `offset` Signed byte offset relative to `whence`. 
* `whence` SYN\_SEEK\_SET, SYN\_SEEK\_CUR, or SYN\_SEEK\_END. 



**Returns:**

New absolute offset, or negative error code. 





        

<hr>



### function syn\_fat\_vfs\_stat 

_VFS stat callback retrieving 8.3 entry metadata from the root directory._ 
```C++
static int syn_fat_vfs_stat (
    const char * path,
    SYN_VfsDirEnt * ent,
    void * fs_data
) 
```





**Parameters:**


* `path` Mount-relative file path. 
* `ent` [out] Directory entry metadata. 
* `fs_data` Unused filesystem context pointer. 



**Returns:**

0 on success, or negative error code. 





        

<hr>



### function syn\_fat\_vfs\_tell 

_VFS tell callback returning the current file offset._ 
```C++
static int32_t syn_fat_vfs_tell (
    SYN_VfsFile * file
) 
```





**Parameters:**


* `file` VFS file descriptor. 



**Returns:**

Current byte offset, or -1 if invalid. 





        

<hr>



### function syn\_fat\_vfs\_unlink 

_VFS unlink callback marking a directory entry deleted and freeing its cluster chain._ 
```C++
static int syn_fat_vfs_unlink (
    const char * path,
    void * fs_data
) 
```





**Parameters:**


* `path` Mount-relative file path. 
* `fs_data` Unused filesystem context pointer. 



**Returns:**

0 on success, or negative error code. 





        

<hr>



### function syn\_fat\_vfs\_write 

_VFS write callback writing sequential bytes and extending clusters as needed._ 
```C++
static int syn_fat_vfs_write (
    SYN_VfsFile * file,
    const void * buf,
    size_t len
) 
```





**Parameters:**


* `file` VFS file descriptor. 
* `buf` Source buffer. 
* `len` Number of bytes to write. 



**Returns:**

Bytes written (&gt;= 0), or negative error code. 





        

<hr>



### function write\_fat\_entry 

_Write a FAT table entry (preserving FAT32 top 4 bits and updating FAT2)._ 
```C++
static SYN_Status write_fat_entry (
    const FAT_Volume * vol,
    uint32_t cluster,
    uint32_t value
) 
```





**Parameters:**


* `vol` Mounted FAT volume metadata. 
* `cluster` Cluster index to update. 
* `value` New cluster chain value (0 for free, 0x0FFFFFFF for EOF). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if cluster out of range, SYN\_ERROR on I/O failure. 





        

<hr>
## Macro Definition Documentation





### define TAG 

_Logging tag for the FAT filesystem adapter._ 
```C++
#define TAG `"syn_fat"`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_fat.c`

