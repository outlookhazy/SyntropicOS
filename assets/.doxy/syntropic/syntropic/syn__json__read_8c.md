

# File syn\_json\_read.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_json\_read.c**](syn__json__read_8c.md)

[Go to the source code of this file](syn__json__read_8c_source.md)

_Minimal JSON reader — in-place tokenizer._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_json_read.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_JsonToken**](structSYN__JsonToken.md) \* | [**syn\_json\_find**](#function-syn_json_find) (const [**SYN\_JsonReader**](structSYN__JsonReader.md) \* r, const char \* key) <br>_Find a token by key name._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_json\_get\_bool**](#function-syn_json_get_bool) (const [**SYN\_JsonReader**](structSYN__JsonReader.md) \* r, const char \* key, [**bool**](syn__defs_8h.md#enum-bool) \* out) <br>_Get a boolean value by key._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_json\_get\_int**](#function-syn_json_get_int) (const [**SYN\_JsonReader**](structSYN__JsonReader.md) \* r, const char \* key, int32\_t \* out) <br>_Get an integer value by key._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_json\_get\_str**](#function-syn_json_get_str) (const [**SYN\_JsonReader**](structSYN__JsonReader.md) \* r, const char \* key, char \* out, size\_t out\_sz) <br>_Get a string value by key._  |
|  [**SYN\_JsonType**](syn__json__read_8h.md#enum-syn_jsontype) | [**syn\_json\_get\_type**](#function-syn_json_get_type) (const [**SYN\_JsonReader**](structSYN__JsonReader.md) \* r, const char \* key) <br>_Get the type of a value by key._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_json\_is\_null**](#function-syn_json_is_null) (const [**SYN\_JsonReader**](structSYN__JsonReader.md) \* r, const char \* key) <br>_Check if a key exists and is null._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_json\_parse**](#function-syn_json_parse) ([**SYN\_JsonReader**](structSYN__JsonReader.md) \* r, char \* json, size\_t len) <br>_Parse a JSON string in-place._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  char \* | [**json\_consume\_delimiter**](#function-json_consume_delimiter) (char \* p, const char \* end, uint8\_t \* depth, [**bool**](syn__defs_8h.md#enum-bool) \* done) <br>_Advance past post-value delimiters and pop closed object frames iteratively._  |
|  char \* | [**json\_parse\_member\_header**](#function-json_parse_member_header) (char \* p, const char \* end, const char \*\* key\_out) <br>_Parse a JSON object member key and colon separator, positioning at value start._  |
|  char \* | [**json\_parse\_non\_object\_value**](#function-json_parse_non_object_value) ([**SYN\_JsonReader**](structSYN__JsonReader.md) \* r, char \* p, const char \* end, const char \* key, uint8\_t depth) <br>_Parse or skip a non-object JSON member value (array, string, or scalar)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**json\_parse\_scalar\_token**](#function-json_parse_scalar_token) ([**SYN\_JsonToken**](structSYN__JsonToken.md) \* tok, const char \* val\_start, size\_t vlen) <br>_Classify and parse a JSON scalar span (true, false, null, or integer)._  |
|  int32\_t | [**parse\_int\_span**](#function-parse_int_span) (const char \* s, const char \* s\_end) <br>_Parse a signed 32-bit integer from a bounded span with saturation._  |
|  char \* | [**parse\_object**](#function-parse_object) ([**SYN\_JsonReader**](structSYN__JsonReader.md) \* r, char \* p, const char \* end) <br>_Iterative bounded-depth JSON object parser._  |
|  char \* | [**parse\_string**](#function-parse_string) (char \* p, const char \* end, const char \*\* out) <br>_Parse a JSON string starting at p._  |
|  char \* | [**skip\_value**](#function-skip_value) (char \* p, const char \* end) <br>_Skip a JSON value (any type)._  |
|  char \* | [**skip\_ws**](#function-skip_ws) (char \* p, const char \* end) <br>_Skip whitespace._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_JSON\_MAX\_DEPTH**](syn__json__read_8c.md#define-syn_json_max_depth)  `8U`<br>_Maximum supported JSON object nesting depth (iterative state bound)._  |

## Public Functions Documentation




### function syn\_json\_find 

_Find a token by key name._ 
```C++
const SYN_JsonToken * syn_json_find (
    const SYN_JsonReader * r,
    const char * key
) 
```



Use dot notation for nested keys: "wifi.ssid"




**Parameters:**


* `r` Reader. 
* `key` Key to find. 



**Returns:**

Pointer to token, or NULL if not found. 





        

<hr>



### function syn\_json\_get\_bool 

_Get a boolean value by key._ 
```C++
bool syn_json_get_bool (
    const SYN_JsonReader * r,
    const char * key,
    bool * out
) 
```





**Parameters:**


* `r` Reader. 
* `key` Key to find. 
* `out` Output value. 



**Returns:**

true if key found and value is a boolean. 





        

<hr>



### function syn\_json\_get\_int 

_Get an integer value by key._ 
```C++
bool syn_json_get_int (
    const SYN_JsonReader * r,
    const char * key,
    int32_t * out
) 
```





**Parameters:**


* `r` Reader. 
* `key` Key to find. 
* `out` Output value. 



**Returns:**

true if key found and value is a number. 





        

<hr>



### function syn\_json\_get\_str 

_Get a string value by key._ 
```C++
bool syn_json_get_str (
    const SYN_JsonReader * r,
    const char * key,
    char * out,
    size_t out_sz
) 
```





**Parameters:**


* `r` Reader. 
* `key` Key to find. 
* `out` Output buffer for the string. 
* `out_sz` Buffer capacity. 



**Returns:**

true if key found and value is a string. 





        

<hr>



### function syn\_json\_get\_type 

_Get the type of a value by key._ 
```C++
SYN_JsonType syn_json_get_type (
    const SYN_JsonReader * r,
    const char * key
) 
```





**Parameters:**


* `r` Reader. 
* `key` Key to find. 



**Returns:**

[**SYN\_JsonType**](syn__json__read_8h.md#enum-syn_jsontype), or SYN\_JSON\_NONE if not found. 





        

<hr>



### function syn\_json\_is\_null 

_Check if a key exists and is null._ 
```C++
bool syn_json_is_null (
    const SYN_JsonReader * r,
    const char * key
) 
```





**Parameters:**


* `r` Reader. 
* `key` Key to find. 



**Returns:**

true if key exists and value is null. 





        

<hr>



### function syn\_json\_parse 

_Parse a JSON string in-place._ 
```C++
bool syn_json_parse (
    SYN_JsonReader * r,
    char * json,
    size_t len
) 
```



Tokenizes the JSON into key-value pairs. Modifies the input buffer (inserts null terminators at string boundaries).




**Parameters:**


* `r` Reader instance. 
* `json` JSON string (will be modified). 
* `len` Length of JSON string. 



**Returns:**

true if parsing succeeded. 





        

<hr>
## Public Static Functions Documentation




### function json\_consume\_delimiter 

_Advance past post-value delimiters and pop closed object frames iteratively._ 
```C++
static char * json_consume_delimiter (
    char * p,
    const char * end,
    uint8_t * depth,
    bool * done
) 
```





**Parameters:**


* `p` Current buffer cursor after a member value or at an empty object brace. 
* `end` One-past-end of JSON buffer. 
* `depth` [in,out] Current open-object nesting depth. 
* `done` [out] Set to true when the root object closes. 



**Returns:**

Updated cursor after ',' or root '}', or NULL on syntax/truncation error. 





        

<hr>



### function json\_parse\_member\_header 

_Parse a JSON object member key and colon separator, positioning at value start._ 
```C++
static char * json_parse_member_header (
    char * p,
    const char * end,
    const char ** key_out
) 
```





**Parameters:**


* `p` Current buffer cursor at start of member key. 
* `end` One-past-end of JSON buffer. 
* `key_out` [out] Null-terminated key string pointer. 



**Returns:**

Cursor positioned at start of member value, or NULL on syntax error. 





        

<hr>



### function json\_parse\_non\_object\_value 

_Parse or skip a non-object JSON member value (array, string, or scalar)._ 
```C++
static char * json_parse_non_object_value (
    SYN_JsonReader * r,
    char * p,
    const char * end,
    const char * key,
    uint8_t depth
) 
```





**Parameters:**


* `r` JSON reader instance. 
* `p` Current buffer cursor at start of value. 
* `end` One-past-end of JSON buffer. 
* `key` Null-terminated member key string. 
* `depth` Current object nesting depth. 



**Returns:**

Cursor past the parsed value, or NULL on syntax error. 





        

<hr>



### function json\_parse\_scalar\_token 

_Classify and parse a JSON scalar span (true, false, null, or integer)._ 
```C++
static bool json_parse_scalar_token (
    SYN_JsonToken * tok,
    const char * val_start,
    size_t vlen
) 
```





**Parameters:**


* `tok` [out] Destination token. 
* `val_start` Start of scalar characters. 
* `vlen` Exact length of scalar span. 



**Returns:**

true if valid JSON scalar token, false otherwise. 





        

<hr>



### function parse\_int\_span 

_Parse a signed 32-bit integer from a bounded span with saturation._ 
```C++
static int32_t parse_int_span (
    const char * s,
    const char * s_end
) 
```





**Parameters:**


* `s` Start of numeric token. 
* `s_end` One-past-end of numeric token. 



**Returns:**

Parsed int32\_t value (saturated to [INT32\_MIN, INT32\_MAX]). 





        

<hr>



### function parse\_object 

_Iterative bounded-depth JSON object parser._ 
```C++
static char * parse_object (
    SYN_JsonReader * r,
    char * p,
    const char * end
) 
```



Tokenizes key-value pairs up to SYN\_JSON\_MAX\_DEPTH without recursion.




**Parameters:**


* `r` JSON reader instance. 
* `p` Current position (caller verified leading '{'). 
* `end` End of buffer. 



**Returns:**

Pointer past the root closing '}', or NULL on error. 





        

<hr>



### function parse\_string 

_Parse a JSON string starting at p._ 
```C++
static char * parse_string (
    char * p,
    const char * end,
    const char ** out
) 
```



Must point to opening '"'. Null-terminates in-place.




**Parameters:**


* `p` Current position (at opening '"'). 
* `end` End of buffer. 
* `out` [out] Start of the string content. 



**Returns:**

Pointer past the closing '"', or NULL on error. 





        

<hr>



### function skip\_value 

_Skip a JSON value (any type)._ 
```C++
static char * skip_value (
    char * p,
    const char * end
) 
```





**Parameters:**


* `p` Current position. 
* `end` End of buffer. 



**Returns:**

Pointer past the value. 





        

<hr>



### function skip\_ws 

_Skip whitespace._ 
```C++
static char * skip_ws (
    char * p,
    const char * end
) 
```





**Parameters:**


* `p` Current position. 
* `end` End of buffer. 



**Returns:**

Pointer to next non-whitespace char. 





        

<hr>
## Macro Definition Documentation





### define SYN\_JSON\_MAX\_DEPTH 

_Maximum supported JSON object nesting depth (iterative state bound)._ 
```C++
#define SYN_JSON_MAX_DEPTH `8U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_json_read.c`

