

# Struct SYN\_NMEA\_GGA



[**ClassList**](annotated.md) **>** [**SYN\_NMEA\_GGA**](structSYN__NMEA__GGA.md)



_Parsed NMEA GGA (Fix Data) Structure._ 

* `#include <syn_nmea.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**altitude\_m**](#variable-altitude_m)  <br> |
|  [**SYN\_NMEA\_FixQuality**](syn__nmea_8h.md#enum-syn_nmea_fixquality) | [**fix\_quality**](#variable-fix_quality)  <br> |
|  float | [**hdop**](#variable-hdop)  <br> |
|  uint8\_t | [**hours**](#variable-hours)  <br> |
|  double | [**latitude**](#variable-latitude)  <br> |
|  double | [**longitude**](#variable-longitude)  <br> |
|  uint16\_t | [**milliseconds**](#variable-milliseconds)  <br> |
|  uint8\_t | [**minutes**](#variable-minutes)  <br> |
|  uint8\_t | [**num\_satellites**](#variable-num_satellites)  <br> |
|  uint8\_t | [**seconds**](#variable-seconds)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid**](#variable-valid)  <br> |












































## Public Attributes Documentation




### variable altitude\_m 

```C++
float SYN_NMEA_GGA::altitude_m;
```




<hr>



### variable fix\_quality 

```C++
SYN_NMEA_FixQuality SYN_NMEA_GGA::fix_quality;
```




<hr>



### variable hdop 

```C++
float SYN_NMEA_GGA::hdop;
```




<hr>



### variable hours 

```C++
uint8_t SYN_NMEA_GGA::hours;
```




<hr>



### variable latitude 

```C++
double SYN_NMEA_GGA::latitude;
```



Decimal degrees (+N, -S) 


        

<hr>



### variable longitude 

```C++
double SYN_NMEA_GGA::longitude;
```



Decimal degrees (+E, -W) 


        

<hr>



### variable milliseconds 

```C++
uint16_t SYN_NMEA_GGA::milliseconds;
```




<hr>



### variable minutes 

```C++
uint8_t SYN_NMEA_GGA::minutes;
```




<hr>



### variable num\_satellites 

```C++
uint8_t SYN_NMEA_GGA::num_satellites;
```




<hr>



### variable seconds 

```C++
uint8_t SYN_NMEA_GGA::seconds;
```




<hr>



### variable valid 

```C++
bool SYN_NMEA_GGA::valid;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_nmea.h`

