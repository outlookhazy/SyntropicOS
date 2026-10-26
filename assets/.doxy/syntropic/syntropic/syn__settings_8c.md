

# File syn\_settings.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_settings.c**](syn__settings_8c.md)

[Go to the source code of this file](syn__settings_8c_source.md)



* `#include "../util/syn_assert.h"`
* `#include "../util/syn_crc.h"`
* `#include "syn_settings.h"`
* `#include "syn_vfs.h"`
* `#include <stdio.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_settings\_changed**](#function-syn_settings_changed) (const [**SYN\_Settings**](structSYN__Settings.md) \* s) <br>_Check if settings have changed since last save/load._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_dual\_bank\_init**](#function-syn_settings_dual_bank_init) ([**SYN\_DualBankSettings**](structSYN__DualBankSettings.md) \* db, uint32\_t flash\_base\_a, uint32\_t flash\_base\_b, uint8\_t sector\_count, void \* data, uint16\_t data\_size, const void \* defaults) <br>_Initialize dual-bank transactional settings manager._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_dual\_bank\_save**](#function-syn_settings_dual_bank_save) ([**SYN\_DualBankSettings**](structSYN__DualBankSettings.md) \* db) <br>_Atomically save settings to inactive bank and switch active bank upon CRC32 verification._  |
|  int | [**syn\_settings\_export**](#function-syn_settings_export) (const [**SYN\_Settings**](structSYN__Settings.md) \* s, void \* buf, size\_t len) <br>_Export current settings to a binary buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_export\_vfs**](#function-syn_settings_export_vfs) (const [**SYN\_Settings**](structSYN__Settings.md) \* s, const char \* filepath) <br>_Export settings directly to a VFS file._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_import**](#function-syn_settings_import) ([**SYN\_Settings**](structSYN__Settings.md) \* s, const void \* buf, size\_t len, [**bool**](syn__defs_8h.md#enum-bool) save) <br>_Import settings from a binary buffer and validate._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_import\_vfs**](#function-syn_settings_import_vfs) ([**SYN\_Settings**](structSYN__Settings.md) \* s, const char \* filepath, [**bool**](syn__defs_8h.md#enum-bool) save) <br>_Import settings directly from a VFS file._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_init**](#function-syn_settings_init) ([**SYN\_Settings**](structSYN__Settings.md) \* s, uint32\_t flash\_base, uint8\_t sector\_count, void \* data, uint16\_t data\_size, const void \* defaults) <br>_Initialize the settings manager._  |
|  void | [**syn\_settings\_on\_change**](#function-syn_settings_on_change) ([**SYN\_Settings**](structSYN__Settings.md) \* s, [**SYN\_SettingsChangeCallback**](syn__settings_8h.md#typedef-syn_settingschangecallback) cb, void \* ctx) <br>_Register a change callback._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_reload**](#function-syn_settings_reload) ([**SYN\_Settings**](structSYN__Settings.md) \* s) <br>_Reload settings from flash, discarding any unsaved changes._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_reset**](#function-syn_settings_reset) ([**SYN\_Settings**](structSYN__Settings.md) \* s) <br>_Reset settings to defaults and save._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_save**](#function-syn_settings_save) ([**SYN\_Settings**](structSYN__Settings.md) \* s) <br>_Save current settings to flash._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**compute\_crc**](#function-compute_crc) (const void \* data, uint16\_t size) <br> |


























## Public Functions Documentation




### function syn\_settings\_changed 

_Check if settings have changed since last save/load._ 
```C++
bool syn_settings_changed (
    const SYN_Settings * s
) 
```



Recomputes CRC-16 and compares to stored checksum. Does NOT save — use this for polling change detection.




**Parameters:**


* `s` Settings instance. 



**Returns:**

true if data has changed. 





        

<hr>



### function syn\_settings\_dual\_bank\_init 

_Initialize dual-bank transactional settings manager._ 
```C++
SYN_Status syn_settings_dual_bank_init (
    SYN_DualBankSettings * db,
    uint32_t flash_base_a,
    uint32_t flash_base_b,
    uint8_t sector_count,
    void * data,
    uint16_t data_size,
    const void * defaults
) 
```





**Parameters:**


* `db` Dual-bank settings instance. 
* `flash_base_a` Base flash address for Bank A. 
* `flash_base_b` Base flash address for Bank B. 
* `sector_count` Sectors per bank. 
* `data` Pointer to RAM settings struct. 
* `data_size` Size of settings struct in bytes. 
* `defaults` Pointer to default values in ROM. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_settings\_dual\_bank\_save 

_Atomically save settings to inactive bank and switch active bank upon CRC32 verification._ 
```C++
SYN_Status syn_settings_dual_bank_save (
    SYN_DualBankSettings * db
) 
```





**Parameters:**


* `db` Dual-bank settings instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_settings\_export 

_Export current settings to a binary buffer._ 
```C++
int syn_settings_export (
    const SYN_Settings * s,
    void * buf,
    size_t len
) 
```





**Parameters:**


* `s` Settings instance. 
* `buf` Destination buffer. 
* `len` Capacity of buffer. 



**Returns:**

Bytes written on success, or negative error code if buffer too small. 





        

<hr>



### function syn\_settings\_export\_vfs 

_Export settings directly to a VFS file._ 
```C++
SYN_Status syn_settings_export_vfs (
    const SYN_Settings * s,
    const char * filepath
) 
```





**Parameters:**


* `s` Settings instance. 
* `filepath` Target VFS path (e.g. "/sd/config.bin"). 



**Returns:**

SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_settings\_import 

_Import settings from a binary buffer and validate._ 
```C++
SYN_Status syn_settings_import (
    SYN_Settings * s,
    const void * buf,
    size_t len,
    bool save
) 
```





**Parameters:**


* `s` Settings instance. 
* `buf` Source buffer containing exported settings. 
* `len` Length of buffer. 
* `save` If true, automatically save to flash if valid. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on size mismatch. 





        

<hr>



### function syn\_settings\_import\_vfs 

_Import settings directly from a VFS file._ 
```C++
SYN_Status syn_settings_import_vfs (
    SYN_Settings * s,
    const char * filepath,
    bool save
) 
```





**Parameters:**


* `s` Settings instance. 
* `filepath` Source VFS path (e.g. "/sd/config.bin"). 
* `save` If true, save imported settings to flash upon verification. 



**Returns:**

SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_settings\_init 

_Initialize the settings manager._ 
```C++
SYN_Status syn_settings_init (
    SYN_Settings * s,
    uint32_t flash_base,
    uint8_t sector_count,
    void * data,
    uint16_t data_size,
    const void * defaults
) 
```



Attempts to load settings from flash. If flash contains valid data (matching size + CRC), it's loaded into \*data. Otherwise, \*defaults are copied into \*data and saved to flash.




**Parameters:**


* `s` Settings instance. 
* `flash_base` Base address of the flash region for this setting. 
* `sector_count` Number of flash sectors to use (more = longer wear life). 
* `data` Pointer to the user's settings struct (RAM). 
* `data_size` Size of the settings struct in bytes. 
* `defaults` Pointer to default values (const, typically in ROM). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_settings\_on\_change 

_Register a change callback._ 
```C++
void syn_settings_on_change (
    SYN_Settings * s,
    SYN_SettingsChangeCallback cb,
    void * ctx
) 
```



Called when [**syn\_settings\_save()**](syn__settings_8h.md#function-syn_settings_save) detects that data has changed.




**Parameters:**


* `s` Settings instance. 
* `cb` Callback function. 
* `ctx` User context. 




        

<hr>



### function syn\_settings\_reload 

_Reload settings from flash, discarding any unsaved changes._ 
```C++
SYN_Status syn_settings_reload (
    SYN_Settings * s
) 
```





**Parameters:**


* `s` Settings instance. 



**Returns:**

SYN\_OK if loaded, SYN\_ERR\_NOT\_FOUND if flash has no valid data. 





        

<hr>



### function syn\_settings\_reset 

_Reset settings to defaults and save._ 
```C++
SYN_Status syn_settings_reset (
    SYN_Settings * s
) 
```



Copies defaults into the data struct, saves to flash.




**Parameters:**


* `s` Settings instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_settings\_save 

_Save current settings to flash._ 
```C++
SYN_Status syn_settings_save (
    SYN_Settings * s
) 
```



Computes CRC-16 of the current data. If it differs from the last known checksum, writes to flash and calls the change callback. If unchanged, this is a no-op (no flash write).




**Parameters:**


* `s` Settings instance. 



**Returns:**

SYN\_OK on success, or flash write error. 





        

<hr>
## Public Static Functions Documentation




### function compute\_crc 

```C++
static uint16_t compute_crc (
    const void * data,
    uint16_t size
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_settings.c`

