

# Struct SYN\_NMEA\_Parser



[**ClassList**](annotated.md) **>** [**SYN\_NMEA\_Parser**](structSYN__NMEA__Parser.md)



_Streaming NMEA Parser State Machine._ 

* `#include <syn_nmea.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  char | [**buf**](#variable-buf)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**in\_sentence**](#variable-in_sentence)  <br> |
|  uint8\_t | [**pos**](#variable-pos)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
char SYN_NMEA_Parser::buf[SYN_NMEA_MAX_SENTENCE_LEN+1];
```




<hr>



### variable in\_sentence 

```C++
bool SYN_NMEA_Parser::in_sentence;
```




<hr>



### variable pos 

```C++
uint8_t SYN_NMEA_Parser::pos;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_nmea.h`

