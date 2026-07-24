

# File syn\_log.c



[**FileList**](files.md) **>** [**log**](dir_1a216ccd2a39b426a7ff78fb20f852eb.md) **>** [**syn\_log.c**](syn__log_8c.md)

[Go to the source code of this file](syn__log_8c_source.md)

_Logging system implementation._ 

* `#include "syn_log.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../port/syn_port_serial.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_fmt.h"`
* `#include <stdio.h>`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**s\_inited**](#variable-s_inited)   = `false`<br> |
|  [**SYN\_LogLevel**](syn__log_8h.md#enum-syn_loglevel) | [**s\_level**](#variable-s_level)   = `SYN\_LOG\_DEBUG`<br> |
|  const char \*const | [**s\_level\_chars**](#variable-s_level_chars)   = `"TDIWEF"`<br>_Single-char level labels: T=Trace, D=Debug, I=Info, W=Warn, E=Error, F=Fatal._  |
|  const char \*const | [**s\_level\_colors**](#variable-s_level_colors)   = `/* multi line expression */`<br> |














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
| define  | [**LOG\_COLOR\_RESET**](syn__log_8c.md#define-log_color_reset)  `"\033[0m"`<br> |

## Public Static Attributes Documentation




### variable s\_inited 

```C++
bool s_inited;
```



Init flag. 
 


        

<hr>



### variable s\_level 

```C++
SYN_LogLevel s_level;
```



Current minimum log level. 
 


        

<hr>



### variable s\_level\_chars 

_Single-char level labels: T=Trace, D=Debug, I=Info, W=Warn, E=Error, F=Fatal._ 
```C++
const char* const s_level_chars;
```




<hr>



### variable s\_level\_colors 

```C++
const char* const s_level_colors[];
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





### define LOG\_COLOR\_RESET 

```C++
#define LOG_COLOR_RESET `"\033[0m"`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/log/syn_log.c`

