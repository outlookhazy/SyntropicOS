

# File syn\_log.h



[**FileList**](files.md) **>** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md) **>** [**syn\_log.h**](syn__log_8h.md)

[Go to the source code of this file](syn__log_8h_source.md)

_Severity-filtered logging system._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../common/syn_compiler.h"`
* `#include <stddef.h>`
* `#include <stdarg.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_LogLevel**](#enum-syn_loglevel)  <br>_Log severity levels._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_log**](#function-syn_log) ([**SYN\_LogLevel**](syn__log_8h.md#enum-syn_loglevel) level, const char \* tag, const char \* fmt, ...) <br>_Core log function (printf-style)._  |
|  [**SYN\_LogLevel**](syn__log_8h.md#enum-syn_loglevel) | [**syn\_log\_get\_level**](#function-syn_log_get_level) (void) <br>_Get the current runtime minimum log level._  |
|  void | [**syn\_log\_hexdump**](#function-syn_log_hexdump) (const char \* tag, const void \* data, size\_t len) <br>_Hex dump utility — output a buffer as hex + ASCII._  |
|  void | [**syn\_log\_init**](#function-syn_log_init) ([**SYN\_LogLevel**](syn__log_8h.md#enum-syn_loglevel) min\_level) <br>_Initialize the logging system._  |
|  void | [**syn\_log\_raw**](#function-syn_log_raw) (const char \* str) <br>_Output a raw string with no formatting or prefix._  |
|  void | [**syn\_log\_set\_level**](#function-syn_log_set_level) ([**SYN\_LogLevel**](syn__log_8h.md#enum-syn_loglevel) level) <br>_Change the runtime minimum log level._  |
|  void | [**syn\_log\_va**](#function-syn_log_va) ([**SYN\_LogLevel**](syn__log_8h.md#enum-syn_loglevel) level, const char \* tag, const char \* fmt, va\_list args) <br>_Core log function (va\_list variant)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_LOG\_BUF\_SIZE**](syn__log_8h.md#define-syn_log_buf_size)  `192`<br>_Size of the log formatting buffer in bytes._  |
| define  | [**SYN\_LOG\_D**](syn__log_8h.md#define-syn_log_d) (tag, fmt, ...) `[**syn\_log**](syn__log_8c.md#function-syn_log)(SYN\_LOG\_DEBUG, tag, fmt, ##\_\_VA\_ARGS\_\_)`<br>_Log at DEBUG level._  |
| define  | [**SYN\_LOG\_E**](syn__log_8h.md#define-syn_log_e) (tag, fmt, ...) `[**syn\_log**](syn__log_8c.md#function-syn_log)(SYN\_LOG\_ERROR, tag, fmt, ##\_\_VA\_ARGS\_\_)`<br>_Log at ERROR level._  |
| define  | [**SYN\_LOG\_F**](syn__log_8h.md#define-syn_log_f) (tag, fmt, ...) `[**syn\_log**](syn__log_8c.md#function-syn_log)(SYN\_LOG\_FATAL, tag, fmt, ##\_\_VA\_ARGS\_\_)`<br>_Log at FATAL level._  |
| define  | [**SYN\_LOG\_I**](syn__log_8h.md#define-syn_log_i) (tag, fmt, ...) `[**syn\_log**](syn__log_8c.md#function-syn_log)(SYN\_LOG\_INFO, tag, fmt, ##\_\_VA\_ARGS\_\_)`<br>_Log at INFO level._  |
| define  | [**SYN\_LOG\_LEVEL**](syn__log_8h.md#define-syn_log_level)  `SYN\_LOG\_DEBUG`<br>_Compile-time minimum log level (defaults to SYN\_LOG\_DEBUG)._  |
| define  | [**SYN\_LOG\_T**](syn__log_8h.md#define-syn_log_t) (tag, fmt, ...) `[**syn\_log**](syn__log_8c.md#function-syn_log)(SYN\_LOG\_TRACE, tag, fmt, ##\_\_VA\_ARGS\_\_)`<br>_Log at TRACE level._  |
| define  | [**SYN\_LOG\_TIMESTAMP**](syn__log_8h.md#define-syn_log_timestamp)  `1`<br> |
| define  | [**SYN\_LOG\_W**](syn__log_8h.md#define-syn_log_w) (tag, fmt, ...) `[**syn\_log**](syn__log_8c.md#function-syn_log)(SYN\_LOG\_WARN, tag, fmt, ##\_\_VA\_ARGS\_\_)`<br>_Log at WARN level._  |

## Detailed Description


Provides printf-style logging with severity levels, per-module tags, optional timestamps, and ANSI color output. Logging calls below the compile-time minimum level are stripped entirely (zero overhead).




**
**

The user provides an output function at init time (typically wrapping UART transmit). If no init is called, a weak default discards output.




**
**

Set `SYN_LOG_LEVEL` in syn\_config.h to the minimum level. Any macro call below that level compiles to `((void)0)`.




**
**


```C++
static void my_output(const char *str, size_t len) {
    syn_port_uart_transmit(0, (const uint8_t *)str, len, 0);
}

syn_log_init(my_output, SYN_LOG_DEBUG);

#define TAG "main"
SYN_LOG_I(TAG, "System started, version %d.%d", 1, 0);
SYN_LOG_D(TAG, "Free memory: %u bytes", free_mem);
SYN_LOG_E(TAG, "Sensor read failed: %d", status);
```



Output: 
```C++
[   1234] I/main: System started, version 1.0
[   1235] D/main: Free memory: 8192 bytes
[   1240] E/main: Sensor read failed: -1
```
 


    
## Public Types Documentation




### enum SYN\_LogLevel 

_Log severity levels._ 
```C++
enum SYN_LogLevel {
    SYN_LOG_TRACE = 0,
    SYN_LOG_DEBUG = 1,
    SYN_LOG_INFO = 2,
    SYN_LOG_WARN = 3,
    SYN_LOG_ERROR = 4,
    SYN_LOG_FATAL = 5,
    SYN_LOG_NONE = 6
};
```




<hr>
## Public Functions Documentation




### function syn\_log 

_Core log function (printf-style)._ 
```C++
void syn_log (
    SYN_LogLevel level,
    const char * tag,
    const char * fmt,
    ...
) 
```



Normally called via the SYN\_LOG\_\* macros rather than directly.




**Parameters:**


* `level` Severity level. 
* `tag` Module/component name (short string). 
* `fmt` printf-style format string. 
* `...` Format arguments. 




        

<hr>



### function syn\_log\_get\_level 

_Get the current runtime minimum log level._ 
```C++
SYN_LogLevel syn_log_get_level (
    void
) 
```





**Returns:**

Current minimum level. 





        

<hr>



### function syn\_log\_hexdump 

_Hex dump utility — output a buffer as hex + ASCII._ 
```C++
void syn_log_hexdump (
    const char * tag,
    const void * data,
    size_t len
) 
```





**Parameters:**


* `tag` Log tag. 
* `data` Buffer to dump. 
* `len` Number of bytes. 




        

<hr>



### function syn\_log\_init 

_Initialize the logging system._ 
```C++
void syn_log_init (
    SYN_LogLevel min_level
) 
```



Set to 1 in syn\_config.h to enable ANSI color codes. 


Output goes directly to syn\_port\_serial\_write — no callback needed.




**Parameters:**


* `min_level` Minimum runtime log level (messages below this are suppressed at runtime). Compile-time level still applies. 




        

<hr>



### function syn\_log\_raw 

_Output a raw string with no formatting or prefix._ 
```C++
void syn_log_raw (
    const char * str
) 
```



Useful for printing hex dumps, tables, or other pre-formatted data that shouldn't get a timestamp/level prefix.




**Parameters:**


* `str` Null-terminated string to output. 




        

<hr>



### function syn\_log\_set\_level 

_Change the runtime minimum log level._ 
```C++
void syn_log_set_level (
    SYN_LogLevel level
) 
```





**Parameters:**


* `level` New minimum level. 




        

<hr>



### function syn\_log\_va 

_Core log function (va\_list variant)._ 
```C++
void syn_log_va (
    SYN_LogLevel level,
    const char * tag,
    const char * fmt,
    va_list args
) 
```





**Parameters:**


* `level` Severity level. 
* `tag` Module/component name. 
* `fmt` printf-style format string. 
* `args` va\_list of format arguments. 




        

<hr>
## Macro Definition Documentation





### define SYN\_LOG\_BUF\_SIZE 

_Size of the log formatting buffer in bytes._ 
```C++
#define SYN_LOG_BUF_SIZE `192`
```




<hr>



### define SYN\_LOG\_D 

_Log at DEBUG level._ 
```C++
#define SYN_LOG_D (
    tag,
    fmt,
    ...
) `syn_log ( SYN_LOG_DEBUG , tag, fmt, ##__VA_ARGS__)`
```




<hr>



### define SYN\_LOG\_E 

_Log at ERROR level._ 
```C++
#define SYN_LOG_E (
    tag,
    fmt,
    ...
) `syn_log ( SYN_LOG_ERROR , tag, fmt, ##__VA_ARGS__)`
```




<hr>



### define SYN\_LOG\_F 

_Log at FATAL level._ 
```C++
#define SYN_LOG_F (
    tag,
    fmt,
    ...
) `syn_log ( SYN_LOG_FATAL , tag, fmt, ##__VA_ARGS__)`
```




<hr>



### define SYN\_LOG\_I 

_Log at INFO level._ 
```C++
#define SYN_LOG_I (
    tag,
    fmt,
    ...
) `syn_log ( SYN_LOG_INFO , tag, fmt, ##__VA_ARGS__)`
```




<hr>



### define SYN\_LOG\_LEVEL 

_Compile-time minimum log level (defaults to SYN\_LOG\_DEBUG)._ 
```C++
#define SYN_LOG_LEVEL `SYN_LOG_DEBUG`
```




<hr>



### define SYN\_LOG\_T 

_Log at TRACE level._ 
```C++
#define SYN_LOG_T (
    tag,
    fmt,
    ...
) `syn_log ( SYN_LOG_TRACE , tag, fmt, ##__VA_ARGS__)`
```




<hr>



### define SYN\_LOG\_TIMESTAMP 

```C++
#define SYN_LOG_TIMESTAMP `1`
```



Set to 0 in syn\_config.h to disable timestamp prefix. 


        

<hr>



### define SYN\_LOG\_W 

_Log at WARN level._ 
```C++
#define SYN_LOG_W (
    tag,
    fmt,
    ...
) `syn_log ( SYN_LOG_WARN , tag, fmt, ##__VA_ARGS__)`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/log/syn_log.h`

