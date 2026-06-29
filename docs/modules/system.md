# System

| Module | Header | Config | Description |
|---|---|---|---|
| Boot Manager | `system/syn_boot.h` | `SYN_USE_BOOT` | Crash-loop detection, safe mode, and boot counter. Events auto-record to errlog via `syn_boot_log_events()`. |
| Error Log | `system/syn_errlog.h` | `SYN_USE_ERRLOG` | Persistent error registry storing severity, error code, context, and timestamp for each entry |
| Version | `system/syn_version.h` | Always available | Compile-time build info: semantic version (`SYN_VERSION_MAJOR/MINOR/PATCH`), build date/time, git hash, app name. Access via `syn_version()` struct or individual defines. |
| Sleep | `system/syn_sleep.h` | Always available | Sleep coordinator with wakelock reference counting |
| Power | `system/syn_power.h` | `SYN_USE_POWER` | Power management: sleep and wake with reference counting |
| Firmware Boot | `system/syn_fwboot.h` | Always available | Firmware boot slot validation |
| Firmware Image | `system/syn_fwimage.h` | Always available | Firmware image header definitions |
| Firmware Update | `system/syn_fwupdate.h` | Always available | Streaming, transport-agnostic firmware updater |
| System Faults | `system/syn_fault.h` | `SYN_USE_FAULT` | Exception and hardware fault handler with diagnostic logging (e.g., HardFault on Cortex-M) |

## Version System

The version system provides both compile-time defines and a runtime struct:

```c
// Compile-time
printf("SyntropicOS v%d.%d.%d built %s\n",
       SYN_VERSION_MAJOR, SYN_VERSION_MINOR, SYN_VERSION_PATCH,
       SYN_BUILD_DATE);

// Runtime struct
const SYN_Version *v = syn_version();
printf("%s v%d.%d.%d [%s]\n",
       v->app_name, v->major, v->minor, v->patch, v->git_hash);
```

Override in your build system:

```
-DSYN_VERSION_MAJOR=1 -DSYN_VERSION_MINOR=2 -DSYN_VERSION_PATCH=0
-DSYN_GIT_HASH=\"abc1234\"
-DSYN_APP_NAME=\"MyFirmware\"
```
