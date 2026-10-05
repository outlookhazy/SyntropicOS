

# File syn\_at\_parser.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_at\_parser.c**](syn__at__parser_8c.md)

[Go to the source code of this file](syn__at__parser_8c_source.md)

_Implementation of stream-oriented AT command parser._ 

* `#include "syn_at_parser.h"`
* `#include <ctype.h>`
* `#include <stdlib.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_AtRespType**](syn__at__parser_8h.md#enum-syn_atresptype) | [**syn\_at\_parser\_feed\_char**](#function-syn_at_parser_feed_char) ([**SYN\_AtParser**](structSYN__AtParser.md) \* parser, char c) <br>_Feed a single byte into the AT parser._  |
|  [**SYN\_AtRespType**](syn__at__parser_8h.md#enum-syn_atresptype) | [**syn\_at\_parser\_feed\_stream**](#function-syn_at_parser_feed_stream) ([**SYN\_AtParser**](structSYN__AtParser.md) \* parser, [**SYN\_Stream**](structSYN__Stream.md) \* stream) <br>_Feed available bytes from a_ [_**SYN\_Stream**_](structSYN__Stream.md) _into the AT parser._ |
|  int | [**syn\_at\_parser\_get\_cme\_error**](#function-syn_at_parser_get_cme_error) (const [**SYN\_AtParser**](structSYN__AtParser.md) \* parser) <br>_Get the parsed error code for SYN\_AT\_RESP\_CME\_ERROR._  |
|  const char \* | [**syn\_at\_parser\_get\_line**](#function-syn_at_parser_get_line) (const [**SYN\_AtParser**](structSYN__AtParser.md) \* parser) <br>_Get the current accumulated response line string._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_at\_parser\_get\_param\_int**](#function-syn_at_parser_get_param_int) (const char \* line, size\_t param\_idx, int \* out\_val) <br>_Helper: Extract an integer parameter from a comma-delimited AT response line._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_at\_parser\_get\_param\_str**](#function-syn_at_parser_get_param_str) (const char \* line, size\_t param\_idx, char \* out\_buf, size\_t max\_len) <br>_Helper: Extract a string parameter from a comma-delimited AT response line._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_at\_parser\_init**](#function-syn_at_parser_init) ([**SYN\_AtParser**](structSYN__AtParser.md) \* parser, char \* buf, size\_t buf\_size) <br>_Initialize an AT command parser._  |
|  void | [**syn\_at\_parser\_reset**](#function-syn_at_parser_reset) ([**SYN\_AtParser**](structSYN__AtParser.md) \* parser) <br>_Reset internal parser state._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  const char \* | [**find\_param\_start**](#function-find_param_start) (const char \* line, size\_t param\_idx) <br>_Find starting position of parameter by index._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**starts\_with**](#function-starts_with) (const char \* str, const char \* prefix) <br>_Check if string starts with given prefix._  |


























## Public Functions Documentation




### function syn\_at\_parser\_feed\_char 

_Feed a single byte into the AT parser._ 
```C++
SYN_AtRespType syn_at_parser_feed_char (
    SYN_AtParser * parser,
    char c
) 
```



Processes a single character. Returns response type if a complete line (CRLF) or prompt ('&gt;') has been parsed.




**Parameters:**


* `parser` Parser instance. 
* `c` Byte to process. 



**Returns:**

Decoded response type, or SYN\_AT\_RESP\_NONE if line incomplete. 





        

<hr>



### function syn\_at\_parser\_feed\_stream 

_Feed available bytes from a_ [_**SYN\_Stream**_](structSYN__Stream.md) _into the AT parser._
```C++
SYN_AtRespType syn_at_parser_feed_stream (
    SYN_AtParser * parser,
    SYN_Stream * stream
) 
```



Reads bytes non-blockingly from the stream until a complete response line/prompt is parsed or the stream is empty.




**Parameters:**


* `parser` Parser instance. 
* `stream` Pointer to input stream. 



**Returns:**

Decoded response type, or SYN\_AT\_RESP\_NONE if line incomplete. 





        

<hr>



### function syn\_at\_parser\_get\_cme\_error 

_Get the parsed error code for SYN\_AT\_RESP\_CME\_ERROR._ 
```C++
int syn_at_parser_get_cme_error (
    const SYN_AtParser * parser
) 
```





**Parameters:**


* `parser` Parser instance. 



**Returns:**

Numerical CME/CMS error code, or -1 if unavailable. 





        

<hr>



### function syn\_at\_parser\_get\_line 

_Get the current accumulated response line string._ 
```C++
const char * syn_at_parser_get_line (
    const SYN_AtParser * parser
) 
```





**Parameters:**


* `parser` Parser instance. 



**Returns:**

Null-terminated line string. 





        

<hr>



### function syn\_at\_parser\_get\_param\_int 

_Helper: Extract an integer parameter from a comma-delimited AT response line._ 
```C++
bool syn_at_parser_get_param_int (
    const char * line,
    size_t param_idx,
    int * out_val
) 
```



Example line: "+CSQ: 20,0" -&gt; param 0 is 20, param 1 is 0.




**Parameters:**


* `line` Response text line. 
* `param_idx` 0-based parameter index. 
* `out_val` [out] Extracted integer value. 



**Returns:**

true if parameter was found and successfully parsed. 





        

<hr>



### function syn\_at\_parser\_get\_param\_str 

_Helper: Extract a string parameter from a comma-delimited AT response line._ 
```C++
bool syn_at_parser_get_param_str (
    const char * line,
    size_t param_idx,
    char * out_buf,
    size_t max_len
) 
```



Handles quoted strings (e.g. "+CPIN: \"READY\"") and strips quotes.




**Parameters:**


* `line` Response text line. 
* `param_idx` 0-based parameter index. 
* `out_buf` Destination buffer. 
* `max_len` Destination capacity. 



**Returns:**

true if parameter was found and extracted. 





        

<hr>



### function syn\_at\_parser\_init 

_Initialize an AT command parser._ 
```C++
SYN_Status syn_at_parser_init (
    SYN_AtParser * parser,
    char * buf,
    size_t buf_size
) 
```





**Parameters:**


* `parser` Parser instance. 
* `buf` Caller-owned buffer for line accumulation. 
* `buf_size` Capacity of buffer in bytes. 



**Returns:**

SYN\_OK on success, or SYN\_ERR\_INVALID\_PARAM. 





        

<hr>



### function syn\_at\_parser\_reset 

_Reset internal parser state._ 
```C++
void syn_at_parser_reset (
    SYN_AtParser * parser
) 
```



Clears accumulated line buffer and last decoded response.




**Parameters:**


* `parser` Parser instance. 




        

<hr>
## Public Static Functions Documentation




### function find\_param\_start 

_Find starting position of parameter by index._ 
```C++
static const char * find_param_start (
    const char * line,
    size_t param_idx
) 
```





**Parameters:**


* `line` Input line string. 
* `param_idx` 0-based parameter index. 



**Returns:**

Pointer to start of parameter, or NULL if index out of range. 





        

<hr>



### function starts\_with 

_Check if string starts with given prefix._ 
```C++
static bool starts_with (
    const char * str,
    const char * prefix
) 
```





**Parameters:**


* `str` Input string. 
* `prefix` Prefix string. 



**Returns:**

True if str starts with prefix. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_at_parser.c`

