

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



Antenna altitude above mean sea level in meters 


        

<hr>



### variable fix\_quality 

```C++
SYN_NMEA_FixQuality SYN_NMEA_GGA::fix_quality;
```



Fix quality indicator 


        

<hr>



### variable hdop 

```C++
float SYN_NMEA_GGA::hdop;
```



Horizontal Dilution of Precision 


        

<hr>



### variable hours 

```C++
uint8_t SYN_NMEA_GGA::hours;
```



UTC hours (0..23) 


        

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



UTC milliseconds (0..999) 


        

<hr>



### variable minutes 

```C++
uint8_t SYN_NMEA_GGA::minutes;
```



UTC minutes (0..59) 


        

<hr>



### variable num\_satellites 

```C++
uint8_t SYN_NMEA_GGA::num_satellites;
```



Number of satellites in view/use 


        

<hr>



### variable seconds 

```C++
uint8_t SYN_NMEA_GGA::seconds;
```



UTC seconds (0..59) 


        

<hr>



### variable valid 

```C++
bool SYN_NMEA_GGA::valid;
```



True if frame parsed successfully 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_nmea.h`

