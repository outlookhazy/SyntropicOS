# Storage

| Module | Header | Config | Description |
|---|---|---|---|
| Param Store | `storage/syn_param.h` | `SYN_USE_PARAM` | Wear-leveled flash parameter store with CRC integrity checking and sequence numbering. Uses `syn_crc16_ccitt()`. |
| Settings | `storage/syn_settings.h` | `SYN_USE_SETTINGS` | Settings manager built on the param store (requires `SYN_USE_PARAM` and CRC) |
| Virtual File System | `storage/syn_vfs.h` | `SYN_USE_VFS` | Mountable VFS abstraction layer — register filesystem backends at mount points |
| LittleFS | `storage/syn_lfs.h` | `SYN_USE_LFS` | LittleFS integration for SPI flash storage (requires `SYN_USE_VFS`). The port layer exposes raw sector-level operations; SyntropicOS handles wear leveling. |
| FAT | `storage/syn_fat.h` | `SYN_USE_FAT` | FAT filesystem adapter (requires `SYN_USE_VFS`) |

## Sensor Framework

| Module | Header | Config | Description |
|---|---|---|---|
| Sensor | `sensor/syn_sensor.h` | `SYN_USE_SENSOR` | Sensor polling pipeline: poll → filter → hysteresis → signal stats → callback. Attach a `SYN_Signal` window for automatic min/max/mean/variance tracking. |
