

# File syn\_errlog.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_errlog.c**](syn__errlog_8c.md)

[Go to the source code of this file](syn__errlog_8c_source.md)

_Persistent error registry implementation._ 

* `#include "syn_errlog.h"`
* `#include "../util/syn_assert.h"`
* `#include "../port/syn_port_system.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_errlog\_clear**](#function-syn_errlog_clear) ([**SYN\_ErrLog**](structSYN__ErrLog.md) \* log) <br>_Clear all entries._  |
|  size\_t | [**syn\_errlog\_count\_severity**](#function-syn_errlog_count_severity) (const [**SYN\_ErrLog**](structSYN__ErrLog.md) \* log, [**SYN\_ErrSeverity**](syn__errlog_8h.md#enum-syn_errseverity) severity) <br>_Count errors with a specific severity level._  |
|  void | [**syn\_errlog\_init**](#function-syn_errlog_init) ([**SYN\_ErrLog**](structSYN__ErrLog.md) \* log, [**SYN\_ErrEntry**](structSYN__ErrEntry.md) \* entries, size\_t capacity, uint32\_t boot\_count) <br>_Initialize the error log._  |
|  const [**SYN\_ErrEntry**](structSYN__ErrEntry.md) \* | [**syn\_errlog\_latest**](#function-syn_errlog_latest) (const [**SYN\_ErrLog**](structSYN__ErrLog.md) \* log) <br>_Get the most recent error entry._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_errlog\_read**](#function-syn_errlog_read) (const [**SYN\_ErrLog**](structSYN__ErrLog.md) \* log, size\_t index, [**SYN\_ErrEntry**](structSYN__ErrEntry.md) \* out) <br>_Read an error entry by index (0 = oldest available)._  |
|  void | [**syn\_errlog\_record**](#function-syn_errlog_record) ([**SYN\_ErrLog**](structSYN__ErrLog.md) \* log, uint16\_t code, [**SYN\_ErrSeverity**](syn__errlog_8h.md#enum-syn_errseverity) severity, uint32\_t context) <br>_Record an error._  |




























## Public Functions Documentation




### function syn\_errlog\_clear 

_Clear all entries._ 
```C++
void syn_errlog_clear (
    SYN_ErrLog * log
) 
```





**Parameters:**


* `log` Error log. 




        

<hr>



### function syn\_errlog\_count\_severity 

_Count errors with a specific severity level._ 
```C++
size_t syn_errlog_count_severity (
    const SYN_ErrLog * log,
    SYN_ErrSeverity severity
) 
```





**Parameters:**


* `log` Error log. 
* `severity` Severity to count. 



**Returns:**

Number of matching entries. 





        

<hr>



### function syn\_errlog\_init 

_Initialize the error log._ 
```C++
void syn_errlog_init (
    SYN_ErrLog * log,
    SYN_ErrEntry * entries,
    size_t capacity,
    uint32_t boot_count
) 
```





**Parameters:**


* `log` Error log instance. 
* `entries` Caller-provided entry buffer. 
* `capacity` Number of entries in buffer. 
* `boot_count` Current boot count (from boot manager or 0). 




        

<hr>



### function syn\_errlog\_latest 

_Get the most recent error entry._ 
```C++
const SYN_ErrEntry * syn_errlog_latest (
    const SYN_ErrLog * log
) 
```





**Parameters:**


* `log` Error log. 



**Returns:**

Pointer to latest entry, or NULL if empty. 





        

<hr>



### function syn\_errlog\_read 

_Read an error entry by index (0 = oldest available)._ 
```C++
bool syn_errlog_read (
    const SYN_ErrLog * log,
    size_t index,
    SYN_ErrEntry * out
) 
```





**Parameters:**


* `log` Error log. 
* `index` Entry index. 
* `out` Output entry. 



**Returns:**

true if entry exists. 





        

<hr>



### function syn\_errlog\_record 

_Record an error._ 
```C++
void syn_errlog_record (
    SYN_ErrLog * log,
    uint16_t code,
    SYN_ErrSeverity severity,
    uint32_t context
) 
```





**Parameters:**


* `log` Error log. 
* `code` Application-defined error code. 
* `severity` Severity level. 
* `context` Application-defined context value. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_errlog.c`

