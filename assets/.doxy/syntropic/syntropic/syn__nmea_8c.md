

# File syn\_nmea.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_nmea.c**](syn__nmea_8c.md)

[Go to the source code of this file](syn__nmea_8c_source.md)

_NMEA 0183 / GNSS sentence parser and encoder implementation._ 

* `#include "syn_nmea.h"`
* `#include "../util/syn_assert.h"`
* `#include <stdio.h>`
* `#include <stdlib.h>`
* `#include <string.h>`
* `#include <ctype.h>`





































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
|  [**bool**](syn__defs_8h.md#enum-bool) | [**get\_field**](#function-get_field) (const char \* sentence, uint8\_t index, char \* dest, size\_t max\_len) <br> |
|  uint8\_t | [**hex2dec**](#function-hex2dec) (char c) <br> |
|  void | [**parse\_time**](#function-parse_time) (const char \* field, uint8\_t \* h, uint8\_t \* m, uint8\_t \* s, uint16\_t \* ms) <br> |


























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

SYN\_NMEA\_SentenceType enum. 





        

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

```C++
static bool get_field (
    const char * sentence,
    uint8_t index,
    char * dest,
    size_t max_len
) 
```




<hr>



### function hex2dec 

```C++
static uint8_t hex2dec (
    char c
) 
```




<hr>



### function parse\_time 

```C++
static void parse_time (
    const char * field,
    uint8_t * h,
    uint8_t * m,
    uint8_t * s,
    uint16_t * ms
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_nmea.c`

