

# File syn\_uds\_util.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_uds\_util.c**](syn__uds__util_8c.md)

[Go to the source code of this file](syn__uds__util_8c_source.md)

_SAE J2012 / ISO 15031-6 UDS Diagnostic Trouble Code (DTC) Utilities Implementation._ 

* `#include "syntropic/proto/syn_uds_util.h"`
* `#include "syntropic/util/syn_fmt.h"`
* `#include <ctype.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_from\_str**](#function-syn_uds_dtc_from_str) (const char \* str, uint32\_t \* dtc\_out) <br>_Parse a 5, 6, or 7-character SAE J2012 DTC string into a 24-bit UDS integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_to\_str**](#function-syn_uds_dtc_to_str) (uint32\_t dtc, char \* out\_buf, size\_t buf\_size) <br>_Format a 24-bit UDS integer into a standard SAE J2012 DTC display string._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int | [**hex\_char\_to\_nibble**](#function-hex_char_to_nibble) (char c) <br>_Convert single hex character to 4-bit nibble value._  |


























## Public Functions Documentation




### function syn\_uds\_dtc\_from\_str 

_Parse a 5, 6, or 7-character SAE J2012 DTC string into a 24-bit UDS integer._ 
```C++
bool syn_uds_dtc_from_str (
    const char * str,
    uint32_t * dtc_out
) 
```



Accepts strings like:
* "P0105" or "P010500" -&gt; 0x010500
* "B111717" -&gt; 0x911717
* "U013100" -&gt; 0xC13100
* "C101000" -&gt; 0x501000




Case-insensitive for prefix (p/c/b/u or P/C/B/U) and hex digits.




**Parameters:**


* `str` Null-terminated SAE J2012 DTC string. 
* `dtc_out` [out] Destination for parsed 24-bit DTC value. 



**Returns:**

true if successfully parsed, false on invalid format. 





        

<hr>



### function syn\_uds\_dtc\_to\_str 

_Format a 24-bit UDS integer into a standard SAE J2012 DTC display string._ 
```C++
bool syn_uds_dtc_to_str (
    uint32_t dtc,
    char * out_buf,
    size_t buf_size
) 
```



Formats values like:
* 0x010500 -&gt; "P010500"
* 0x911717 -&gt; "B111717"
* 0xC13100 -&gt; "U013100"




Buffer must have a capacity of at least 8 bytes (7 chars + null terminator).




**Parameters:**


* `dtc` 24-bit UDS DTC integer value. 
* `out_buf` Destination string buffer. 
* `buf_size` Capacity of output buffer (minimum 8 bytes). 



**Returns:**

true if formatted successfully, false if buffer too small or NULL. 





        

<hr>
## Public Static Functions Documentation




### function hex\_char\_to\_nibble 

_Convert single hex character to 4-bit nibble value._ 
```C++
static int hex_char_to_nibble (
    char c
) 
```





**Parameters:**


* `c` Input character ('0'-'9', 'a'-'f', 'A'-'F'). 



**Returns:**

Nibble value (0-15), or -1 if invalid hex char. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_uds_util.c`

