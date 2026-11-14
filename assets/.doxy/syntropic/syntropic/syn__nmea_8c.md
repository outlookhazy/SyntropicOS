

# File syn\_nmea.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_nmea.c**](syn__nmea_8c.md)

[Go to the source code of this file](syn__nmea_8c_source.md)

_NMEA 0183 / GNSS sentence parser and encoder implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_nmea.h"`
* `#include <stddef.h>`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_nmea\_checksum**](#function-syn_nmea_checksum) (const char \* sentence) <br>_Calculate XOR checksum of an NMEA sentence payload (between '$' and '\*')._  |
|  [**SYN\_NMEA\_SentenceType**](syn__nmea_8h.md#enum-syn_nmea_sentencetype) | [**syn\_nmea\_get\_type**](#function-syn_nmea_get_type) (const char \* sentence) <br>_Identify sentence type from NMEA string._  |
|  double | [**syn\_nmea\_parse\_coord**](#function-syn_nmea_parse_coord) (const char \* nmea\_coord, char dir) <br>_Parse NMEA DDMM.MMMM coordinate string and direction indicator._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_nmea\_parse\_gga**](#function-syn_nmea_parse_gga) (const char \* sentence, [**SYN\_NMEA\_GGA**](structSYN__NMEA__GGA.md) \* gga) <br>_Parse a $GPGGA / $GNGGA sentence._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_nmea\_parse\_gsa**](#function-syn_nmea_parse_gsa) (const char \* sentence, [**SYN\_NMEA\_GSA**](structSYN__NMEA__GSA.md) \* gsa) <br>_Parse a $GPGSA / $GNGSA sentence._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_nmea\_parse\_rmc**](#function-syn_nmea_parse_rmc) (const char \* sentence, [**SYN\_NMEA\_RMC**](structSYN__NMEA__RMC.md) \* rmc) <br>_Parse a $GPRMC / $GNRMC sentence._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_nmea\_parse\_vtg**](#function-syn_nmea_parse_vtg) (const char \* sentence, [**SYN\_NMEA\_VTG**](structSYN__NMEA__VTG.md) \* vtg) <br>_Parse a $GPVTG / $GNVTG sentence._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_nmea\_parse\_zda**](#function-syn_nmea_parse_zda) (const char \* sentence, [**SYN\_NMEA\_ZDA**](structSYN__NMEA__ZDA.md) \* zda) <br>_Parse a $GPZDA / $GNZDA sentence._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_nmea\_parser\_feed**](#function-syn_nmea_parser_feed) ([**SYN\_NMEA\_Parser**](structSYN__NMEA__Parser.md) \* parser, char byte, char \* out\_sentence) <br>_Feed a byte into the NMEA streaming parser._  |
|  void | [**syn\_nmea\_parser\_init**](#function-syn_nmea_parser_init) ([**SYN\_NMEA\_Parser**](structSYN__NMEA__Parser.md) \* parser) <br>_Initialize an NMEA streaming parser._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_nmea\_validate**](#function-syn_nmea_validate) (const char \* sentence) <br>_Validate an NMEA sentence string (checks start '$', '\*XX' checksum, and CRLF)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**get\_field**](#function-get_field) (const char \* sentence, uint8\_t index, char \* dest, size\_t max\_len) <br>_Extract comma-separated NMEA field from sentence._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**hex\_nibble**](#function-hex_nibble) (char c, uint8\_t \* out) <br>_Decode a single hexadecimal ASCII digit into its 4-bit nibble value._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_nmea\_end**](#function-is_nmea_end) (char c) <br>_Check whether character terminates NMEA sentence payload._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**nmea\_digits\_span\_valid**](#function-nmea_digits_span_valid) (const char \* s, size\_t len) <br>_Verify that a character span is non-empty and consists only of ASCII decimal digits._  |
|  const char \* | [**nmea\_find\_valid\_star**](#function-nmea_find_valid_star) (const char \* sentence) <br>_Locate first '\*' delimiter and verify no embedded CR/LF/'$' precedes it._  |
|  void | [**nmea\_gga\_parse\_metrics**](#function-nmea_gga_parse_metrics) (const char \* sentence, [**SYN\_NMEA\_GGA**](structSYN__NMEA__GGA.md) \* gga) <br>_Parse GGA quality, satellite count, HDOP, and altitude fields._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**nmea\_is\_digit**](#function-nmea_is_digit) (char c) <br>_Check whether a character is an ASCII decimal digit ('0'..'9')._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**nmea\_parse\_coord\_parts**](#function-nmea_parse_coord_parts) (const char \* nmea\_coord, int32\_t \* out\_deg, int32\_t \* out\_min\_whole, const char \*\* out\_frac) <br>_Validate and split an NMEA coordinate string (ddmm.mmmm / dddmm.mmmm) into components._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**nmea\_parse\_u32\_bounded**](#function-nmea_parse_u32_bounded) (const char \* s, uint32\_t min\_val, uint32\_t max\_val, uint32\_t \* out) <br>_Parse a non-empty unsigned decimal integer string within [min\_val, max\_val]._  |
|  void | [**parse\_rmc\_date**](#function-parse_rmc_date) (const char \* f, [**SYN\_NMEA\_RMC**](structSYN__NMEA__RMC.md) \* rmc) <br>_Parse NMEA 6-character date field (ddmmyy) into bounded day, month, and full year._  |
|  void | [**parse\_time**](#function-parse_time) (const char \* field, uint8\_t \* h, uint8\_t \* m, uint8\_t \* s, uint16\_t \* ms) <br>_Parse NMEA UTC time field (hhmmss[.sss]) into bounded components._  |
|  double | [**syn\_atof**](#function-syn_atof) (const char \* s) <br>_Lightweight ASCII string to double conversion for non-empty NMEA numerical fields._  |


























## Public Functions Documentation




### function syn\_nmea\_checksum 

_Calculate XOR checksum of an NMEA sentence payload (between '$' and '\*')._ 
```C++
uint8_t syn_nmea_checksum (
    const char * sentence
) 
```





**Parameters:**


* `sentence` NMEA sentence string. 



**Returns:**

8-bit XOR checksum value. 





        

<hr>



### function syn\_nmea\_get\_type 

_Identify sentence type from NMEA string._ 
```C++
SYN_NMEA_SentenceType syn_nmea_get_type (
    const char * sentence
) 
```





**Parameters:**


* `sentence` NMEA sentence string. 



**Returns:**

[**SYN\_NMEA\_SentenceType**](syn__nmea_8h.md#enum-syn_nmea_sentencetype) enum. 





        

<hr>



### function syn\_nmea\_parse\_coord 

_Parse NMEA DDMM.MMMM coordinate string and direction indicator._ 
```C++
double syn_nmea_parse_coord (
    const char * nmea_coord,
    char dir
) 
```





**Parameters:**


* `nmea_coord` Coordinate string (e.g. "4807.038"). 
* `dir` Direction char ('N', 'S', 'E', 'W'). 



**Returns:**

Decimal degrees (+N/+E, -S/-W). 





        

<hr>



### function syn\_nmea\_parse\_gga 

_Parse a $GPGGA / $GNGGA sentence._ 
```C++
bool syn_nmea_parse_gga (
    const char * sentence,
    SYN_NMEA_GGA * gga
) 
```





**Parameters:**


* `sentence` NMEA sentence string. 
* `gga` Pointer to destination GGA struct. 



**Returns:**

true on success. 





        

<hr>



### function syn\_nmea\_parse\_gsa 

_Parse a $GPGSA / $GNGSA sentence._ 
```C++
bool syn_nmea_parse_gsa (
    const char * sentence,
    SYN_NMEA_GSA * gsa
) 
```





**Parameters:**


* `sentence` NMEA sentence string. 
* `gsa` Pointer to destination GSA struct. 



**Returns:**

true on success. 





        

<hr>



### function syn\_nmea\_parse\_rmc 

_Parse a $GPRMC / $GNRMC sentence._ 
```C++
bool syn_nmea_parse_rmc (
    const char * sentence,
    SYN_NMEA_RMC * rmc
) 
```





**Parameters:**


* `sentence` NMEA sentence string. 
* `rmc` Pointer to destination RMC struct. 



**Returns:**

true on success. 





        

<hr>



### function syn\_nmea\_parse\_vtg 

_Parse a $GPVTG / $GNVTG sentence._ 
```C++
bool syn_nmea_parse_vtg (
    const char * sentence,
    SYN_NMEA_VTG * vtg
) 
```





**Parameters:**


* `sentence` NMEA sentence string. 
* `vtg` Pointer to destination VTG struct. 



**Returns:**

true on success. 





        

<hr>



### function syn\_nmea\_parse\_zda 

_Parse a $GPZDA / $GNZDA sentence._ 
```C++
bool syn_nmea_parse_zda (
    const char * sentence,
    SYN_NMEA_ZDA * zda
) 
```





**Parameters:**


* `sentence` NMEA sentence string. 
* `zda` Pointer to destination ZDA struct. 



**Returns:**

true on success. 





        

<hr>



### function syn\_nmea\_parser\_feed 

_Feed a byte into the NMEA streaming parser._ 
```C++
bool syn_nmea_parser_feed (
    SYN_NMEA_Parser * parser,
    char byte,
    char * out_sentence
) 
```





**Parameters:**


* `parser` Pointer to parser instance. 
* `byte` Received character. 
* `out_sentence` Output buffer for completed NMEA sentence string (if returns true). 



**Returns:**

true if a complete, valid NMEA sentence was received. 





        

<hr>



### function syn\_nmea\_parser\_init 

_Initialize an NMEA streaming parser._ 
```C++
void syn_nmea_parser_init (
    SYN_NMEA_Parser * parser
) 
```





**Parameters:**


* `parser` Pointer to parser state machine. 




        

<hr>



### function syn\_nmea\_validate 

_Validate an NMEA sentence string (checks start '$', '\*XX' checksum, and CRLF)._ 
```C++
bool syn_nmea_validate (
    const char * sentence
) 
```





**Parameters:**


* `sentence` NMEA sentence string. 



**Returns:**

true if valid sentence and matching checksum. 





        

<hr>
## Public Static Functions Documentation




### function get\_field 

_Extract comma-separated NMEA field from sentence._ 
```C++
static bool get_field (
    const char * sentence,
    uint8_t index,
    char * dest,
    size_t max_len
) 
```





**Parameters:**


* `sentence` Input NMEA sentence string (non-NULL). 
* `index` Field index (0-based). 
* `dest` Destination string buffer (non-NULL, max\_len &gt;= 2). 
* `max_len` Maximum capacity of destination buffer. 



**Returns:**

true if non-empty field extracted, false otherwise. 





        

<hr>



### function hex\_nibble 

_Decode a single hexadecimal ASCII digit into its 4-bit nibble value._ 
```C++
static bool hex_nibble (
    char c,
    uint8_t * out
) 
```





**Parameters:**


* `c` Input character. 
* `out` [out] Decoded nibble (0..15). 



**Returns:**

true if c is a valid hexadecimal digit, false otherwise. 





        

<hr>



### function is\_nmea\_end 

_Check whether character terminates NMEA sentence payload._ 
```C++
static inline bool is_nmea_end (
    char c
) 
```





**Parameters:**


* `c` Character to inspect. 



**Returns:**

true if end-of-payload character. 





        

<hr>



### function nmea\_digits\_span\_valid 

_Verify that a character span is non-empty and consists only of ASCII decimal digits._ 
```C++
static bool nmea_digits_span_valid (
    const char * s,
    size_t len
) 
```





**Parameters:**


* `s` Pointer to character span. 
* `len` Number of characters to inspect. 



**Returns:**

true if len &gt; 0 and all characters are digits, false otherwise. 





        

<hr>



### function nmea\_find\_valid\_star 

_Locate first '\*' delimiter and verify no embedded CR/LF/'$' precedes it._ 
```C++
static const char * nmea_find_valid_star (
    const char * sentence
) 
```





**Parameters:**


* `sentence` Non-NULL NMEA sentence starting with '$'. 



**Returns:**

Pointer to '\*' if prefix framing is valid, NULL otherwise. 





        

<hr>



### function nmea\_gga\_parse\_metrics 

_Parse GGA quality, satellite count, HDOP, and altitude fields._ 
```C++
static void nmea_gga_parse_metrics (
    const char * sentence,
    SYN_NMEA_GGA * gga
) 
```





**Parameters:**


* `sentence` Validated GGA sentence string. 
* `gga` Destination GGA structure. 




        

<hr>



### function nmea\_is\_digit 

_Check whether a character is an ASCII decimal digit ('0'..'9')._ 
```C++
static inline bool nmea_is_digit (
    char c
) 
```





**Parameters:**


* `c` Character to inspect. 



**Returns:**

true if ASCII digit, false otherwise. 





        

<hr>



### function nmea\_parse\_coord\_parts 

_Validate and split an NMEA coordinate string (ddmm.mmmm / dddmm.mmmm) into components._ 
```C++
static bool nmea_parse_coord_parts (
    const char * nmea_coord,
    int32_t * out_deg,
    int32_t * out_min_whole,
    const char ** out_frac
) 
```





**Parameters:**


* `nmea_coord` Input NMEA coordinate string. 
* `out_deg` [out] Parsed degrees (0..180). 
* `out_min_whole` [out] Parsed whole minutes (0..59). 
* `out_frac` [out] Pointer to validated fractional minute digits after '.'. 



**Returns:**

true if valid NMEA coordinate syntax and range, false otherwise. 





        

<hr>



### function nmea\_parse\_u32\_bounded 

_Parse a non-empty unsigned decimal integer string within [min\_val, max\_val]._ 
```C++
static bool nmea_parse_u32_bounded (
    const char * s,
    uint32_t min_val,
    uint32_t max_val,
    uint32_t * out
) 
```





**Parameters:**


* `s` Non-NULL input NMEA field string. 
* `min_val` Minimum accepted value. 
* `max_val` Maximum accepted value (must be &lt;= 99999). 
* `out` [out] Parsed value on success. 



**Returns:**

true if s consists solely of ASCII digits and falls within bounds, false otherwise. 





        

<hr>



### function parse\_rmc\_date 

_Parse NMEA 6-character date field (ddmmyy) into bounded day, month, and full year._ 
```C++
static void parse_rmc_date (
    const char * f,
    SYN_NMEA_RMC * rmc
) 
```





**Parameters:**


* `f` Date field string. 
* `rmc` Destination RMC structure. 




        

<hr>



### function parse\_time 

_Parse NMEA UTC time field (hhmmss[.sss]) into bounded components._ 
```C++
static void parse_time (
    const char * field,
    uint8_t * h,
    uint8_t * m,
    uint8_t * s,
    uint16_t * ms
) 
```





**Parameters:**


* `field` NMEA time field string. 
* `h` Pointer to receive hours (0-23). 
* `m` Pointer to receive minutes (0-59). 
* `s` Pointer to receive seconds (0-60). 
* `ms` Pointer to receive milliseconds (0-999). 




        

<hr>



### function syn\_atof 

_Lightweight ASCII string to double conversion for non-empty NMEA numerical fields._ 
```C++
static double syn_atof (
    const char * s
) 
```





**Parameters:**


* `s` Non-NULL NMEA field string. 



**Returns:**

Converted double value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_nmea.c`

