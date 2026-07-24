

# File syn\_str.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_str.h**](syn__str_8h.md)

[Go to the source code of this file](syn__str_8h_source.md)

_Zero-allocation string parsing, trimming, and tokenization utilities._ 

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_str\_split**](#function-syn_str_split) (char \* str, char delimiter, char \* tokens, size\_t max\_tokens) <br>_Split a string in-place by a delimiter character into a token array._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_str\_to\_i32**](#function-syn_str_to_i32) (const char \* str, int32\_t \* out\_val) <br>_Parse a signed 32-bit integer from a string safely._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_str\_to\_u32**](#function-syn_str_to_u32) (const char \* str, uint32\_t \* out\_val) <br>_Parse an unsigned 32-bit integer from a string safely._  |
|  char \* | [**syn\_str\_trim**](#function-syn_str_trim) (char \* str) <br>_Trim leading and trailing whitespace characters in-place._  |




























## Public Functions Documentation




### function syn\_str\_split 

_Split a string in-place by a delimiter character into a token array._ 
```C++
size_t syn_str_split (
    char * str,
    char delimiter,
    char * tokens,
    size_t max_tokens
) 
```



Replaces occurrences of delimiter with '\0' and populates the tokens array. Zero-allocation. Modifies the input string.




**Parameters:**


* `str` Null-terminated input string (modified in-place). 
* `delimiter` Character to split on (e.g. ',' or ' '). 
* `tokens` Array of char pointers to store token starts. 
* `max_tokens` Maximum capacity of tokens array. 



**Returns:**

Number of tokens stored in tokens array. 





        

<hr>



### function syn\_str\_to\_i32 

_Parse a signed 32-bit integer from a string safely._ 
```C++
bool syn_str_to_i32 (
    const char * str,
    int32_t * out_val
) 
```





**Parameters:**


* `str` Input string to parse. 
* `out_val` Pointer to store parsed int32\_t result. 



**Returns:**

true if successfully parsed, false on error or empty string. 





        

<hr>



### function syn\_str\_to\_u32 

_Parse an unsigned 32-bit integer from a string safely._ 
```C++
bool syn_str_to_u32 (
    const char * str,
    uint32_t * out_val
) 
```





**Parameters:**


* `str` Input string to parse. 
* `out_val` Pointer to store parsed uint32\_t result. 



**Returns:**

true if successfully parsed, false on error or empty string. 





        

<hr>



### function syn\_str\_trim 

_Trim leading and trailing whitespace characters in-place._ 
```C++
char * syn_str_trim (
    char * str
) 
```





**Parameters:**


* `str` Null-terminated input string (modified in-place). 



**Returns:**

Pointer to the first non-whitespace character in str. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_str.h`

