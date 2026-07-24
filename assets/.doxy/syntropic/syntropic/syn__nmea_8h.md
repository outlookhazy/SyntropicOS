

# File syn\_nmea.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_nmea.h**](syn__nmea_8h.md)

[Go to the source code of this file](syn__nmea_8h_source.md)

_NMEA 0183 / GNSS sentence parser and encoder._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_NMEA\_GGA**](structSYN__NMEA__GGA.md) <br>_Parsed NMEA GGA (Fix Data) Structure._  |
| struct | [**SYN\_NMEA\_GSA**](structSYN__NMEA__GSA.md) <br>_Parsed NMEA GSA (DOP & Active Satellites) Structure._  |
| struct | [**SYN\_NMEA\_Parser**](structSYN__NMEA__Parser.md) <br>_Streaming NMEA Parser State Machine._  |
| struct | [**SYN\_NMEA\_RMC**](structSYN__NMEA__RMC.md) <br>_Parsed NMEA RMC (Recommended Minimum Data) Structure._  |
| struct | [**SYN\_NMEA\_VTG**](structSYN__NMEA__VTG.md) <br>_Parsed NMEA VTG (Velocity & Course) Structure._  |
| struct | [**SYN\_NMEA\_ZDA**](structSYN__NMEA__ZDA.md) <br>_Parsed NMEA ZDA (UTC Date & Time) Structure._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_NMEA\_FixQuality**](#enum-syn_nmea_fixquality)  <br>_GPS Fix Quality Enum (from GGA)_  |
| enum  | [**SYN\_NMEA\_SentenceType**](#enum-syn_nmea_sentencetype)  <br>_NMEA Sentence Type Enum._  |




















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_NMEA\_MAX\_SENTENCE\_LEN**](syn__nmea_8h.md#define-syn_nmea_max_sentence_len)  `82`<br> |

## Detailed Description


Provides a zero-allocation, streaming byte-at-a-time NMEA 0183 parser, sentence checksum verification, and structured data decoders for standard GNSS sentences:
* GGA: Global Positioning System Fix Data
* RMC: Recommended Minimum Specific GNSS Data
* VTG: Course Over Ground & Ground Speed
* GSA: GNSS DOP & Active Satellites
* ZDA: UTC Time & Date 




    
## Public Types Documentation




### enum SYN\_NMEA\_FixQuality 

_GPS Fix Quality Enum (from GGA)_ 
```C++
enum SYN_NMEA_FixQuality {
    SYN_NMEA_FIX_INVALID = 0,
    SYN_NMEA_FIX_GPS = 1,
    SYN_NMEA_FIX_DGPS = 2,
    SYN_NMEA_FIX_PPS = 3,
    SYN_NMEA_FIX_RTK = 4,
    SYN_NMEA_FIX_FLOAT_RTK = 5,
    SYN_NMEA_FIX_ESTIMATED = 6
};
```




<hr>



### enum SYN\_NMEA\_SentenceType 

_NMEA Sentence Type Enum._ 
```C++
enum SYN_NMEA_SentenceType {
    SYN_NMEA_SENTENCE_UNKNOWN = 0,
    SYN_NMEA_SENTENCE_GGA,
    SYN_NMEA_SENTENCE_RMC,
    SYN_NMEA_SENTENCE_VTG,
    SYN_NMEA_SENTENCE_GSA,
    SYN_NMEA_SENTENCE_ZDA
};
```




<hr>
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
## Macro Definition Documentation





### define SYN\_NMEA\_MAX\_SENTENCE\_LEN 

```C++
#define SYN_NMEA_MAX_SENTENCE_LEN `82`
```



Max NMEA 0183 sentence length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_nmea.h`

