

# File syn\_json\_read.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_json\_read.h**](syn__json__read_8h.md)

[Go to the source code of this file](syn__json__read_8h_source.md)

_Minimal JSON reader — in-place, zero-alloc._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_JsonReader**](structSYN__JsonReader.md) <br>_JSON reader — token array + parse state._  |
| struct | [**SYN\_JsonToken**](structSYN__JsonToken.md) <br>_Parsed JSON token — key + value + type._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_JsonType**](#enum-syn_jsontype)  <br>_JSON value types._  |




















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_JSON\_MAX\_TOKENS**](syn__json__read_8h.md#define-syn_json_max_tokens)  `32`<br>_Maximum number of parsed JSON tokens per document._  |

## Detailed Description


Parses a JSON string in-place, providing token-based access. No dynamic memory, no callbacks — just walks through the buffer and lets you query values by key path.




**
**


```C++
char json[] = "{\"ssid\":\"MyNet\",\"channel\":6,\"hidden\":false}";
SYN_JsonReader r;
syn_json_parse(&r, json, strlen(json));

char ssid[33];
if (syn_json_get_str(&r, "ssid", ssid, sizeof(ssid))) { ... }

int32_t ch;
if (syn_json_get_int(&r, "channel", &ch)) { ... }

bool hidden;
if (syn_json_get_bool(&r, "hidden", &hidden)) { ... }
```





**
**


* Top-level must be an object `{}`
* Nested objects accessed via dot notation: `"wifi.ssid"`
* No array indexing (arrays can be iterated)
* Modifies the input buffer (inserts null terminators)
* Max 32 tokens (keys + values) per parse 







    
## Public Types Documentation




### enum SYN\_JsonType 

_JSON value types._ 
```C++
enum SYN_JsonType {
    SYN_JSON_NONE = 0,
    SYN_JSON_STRING = 1,
    SYN_JSON_NUMBER = 2,
    SYN_JSON_BOOL = 3,
    SYN_JSON_NULL = 4,
    SYN_JSON_OBJECT = 5,
    SYN_JSON_ARRAY = 6
};
```




<hr>
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

SYN\_JsonType, or SYN\_JSON\_NONE if not found. 





        

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
## Macro Definition Documentation





### define SYN\_JSON\_MAX\_TOKENS 

_Maximum number of parsed JSON tokens per document._ 
```C++
#define SYN_JSON_MAX_TOKENS `32`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_json_read.h`

