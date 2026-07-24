

# Struct SYN\_NMEA\_GSA



[**ClassList**](annotated.md) **>** [**SYN\_NMEA\_GSA**](structSYN__NMEA__GSA.md)



_Parsed NMEA GSA (DOP & Active Satellites) Structure._ 

* `#include <syn_nmea.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**fix\_type**](#variable-fix_type)  <br> |
|  float | [**hdop**](#variable-hdop)  <br> |
|  char | [**mode**](#variable-mode)  <br> |
|  float | [**pdop**](#variable-pdop)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid**](#variable-valid)  <br> |
|  float | [**vdop**](#variable-vdop)  <br> |












































## Public Attributes Documentation




### variable fix\_type 

```C++
uint8_t SYN_NMEA_GSA::fix_type;
```



1 = No fix, 2 = 2D fix, 3 = 3D fix 


        

<hr>



### variable hdop 

```C++
float SYN_NMEA_GSA::hdop;
```




<hr>



### variable mode 

```C++
char SYN_NMEA_GSA::mode;
```



'M' = Manual, 'A' = Automatic 


        

<hr>



### variable pdop 

```C++
float SYN_NMEA_GSA::pdop;
```




<hr>



### variable valid 

```C++
bool SYN_NMEA_GSA::valid;
```




<hr>



### variable vdop 

```C++
float SYN_NMEA_GSA::vdop;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_nmea.h`

