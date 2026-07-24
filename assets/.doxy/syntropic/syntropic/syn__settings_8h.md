

# File syn\_settings.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**storage**](dir_6dc37c1fd63f78e2ff9ec2810c54ee77.md) **>** [**syn\_settings.h**](syn__settings_8h.md)

[Go to the source code of this file](syn__settings_8h_source.md)

_Persistent settings manager with change detection._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../storage/syn_param.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Settings**](structSYN__Settings.md) <br>_Persistent settings instance — flash-backed with change detection._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_SettingsChangeCallback**](#typedef-syn_settingschangecallback)  <br>_Called when_ [_**syn\_settings\_save()**_](syn__settings_8h.md#function-syn_settings_save) _detects the data has changed._ |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_settings\_changed**](#function-syn_settings_changed) (const [**SYN\_Settings**](structSYN__Settings.md) \* s) <br>_Check if settings have changed since last save/load._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_init**](#function-syn_settings_init) ([**SYN\_Settings**](structSYN__Settings.md) \* s, uint32\_t flash\_base, uint8\_t sector\_count, void \* data, uint16\_t data\_size, const void \* defaults) <br>_Initialize the settings manager._  |
|  void | [**syn\_settings\_on\_change**](#function-syn_settings_on_change) ([**SYN\_Settings**](structSYN__Settings.md) \* s, [**SYN\_SettingsChangeCallback**](syn__settings_8h.md#typedef-syn_settingschangecallback) cb, void \* ctx) <br>_Register a change callback._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_reload**](#function-syn_settings_reload) ([**SYN\_Settings**](structSYN__Settings.md) \* s) <br>_Reload settings from flash, discarding any unsaved changes._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_reset**](#function-syn_settings_reset) ([**SYN\_Settings**](structSYN__Settings.md) \* s) <br>_Reset settings to defaults and save._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_settings\_save**](#function-syn_settings_save) ([**SYN\_Settings**](structSYN__Settings.md) \* s) <br>_Save current settings to flash._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_settings\_checksum**](#function-syn_settings_checksum) (const [**SYN\_Settings**](structSYN__Settings.md) \* s) <br>_Get the current CRC-16 checksum of the settings data._  |


























## Detailed Description


Wraps syn\_param (wear-leveled flash storage) with:
* Automatic CRC-16 checksum for change detection
* Default values initialization
* Load-or-default on init
* Dirty tracking (has data changed since last save?)
* Optional change callback




Eliminates the repetitive InitSettings / LoadSettings / SaveSettings / update\_checksum boilerplate seen in every firmware module.




**
**


```C++
typedef struct {
    int32_t velocity_max;
    int32_t acceleration;
    int32_t power_max;
} MoverSettings;

static const MoverSettings defaults = { 500, 200, 80 };
static MoverSettings settings;
static SYN_Settings store;

// Init — loads from flash or applies defaults:
syn_settings_init(&store, FLASH_MOVER_BASE, 2,
                  &settings, sizeof(settings), &defaults);

// Modify and save:
settings.velocity_max = 600;
syn_settings_save(&store);  // CRC updated, writes flash

// Remote sync — check if remote copy is stale:
if (remote_checksum != syn_settings_checksum(&store)) {
    send_settings_to_remote(&settings);
}
```
 





    
## Public Types Documentation




### typedef SYN\_SettingsChangeCallback 

_Called when_ [_**syn\_settings\_save()**_](syn__settings_8h.md#function-syn_settings_save) _detects the data has changed._
```C++
typedef void(* SYN_SettingsChangeCallback) (void *data, void *ctx);
```





**Parameters:**


* `data` Pointer to the settings struct. 
* `ctx` User context. 




        

<hr>
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




### function syn\_settings\_checksum 

_Get the current CRC-16 checksum of the settings data._ 
```C++
static inline uint16_t syn_settings_checksum (
    const SYN_Settings * s
) 
```



Useful for comparing against a remote copy to detect staleness. This returns the checksum computed at the last save/load, NOT a live recomputation. Call [**syn\_settings\_save()**](syn__settings_8h.md#function-syn_settings_save) first to update.




**Parameters:**


* `s` Settings instance. 



**Returns:**

CRC-16 checksum. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_settings.h`

