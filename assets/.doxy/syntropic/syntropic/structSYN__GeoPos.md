

# Struct SYN\_GeoPos



[**ClassList**](annotated.md) **>** [**SYN\_GeoPos**](structSYN__GeoPos.md)



_Structured Geographic Position with accuracy bound and fix quality._ 

* `#include <syn_geo.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**accuracy\_m**](#variable-accuracy_m)  <br> |
|  double | [**altitude\_m**](#variable-altitude_m)  <br> |
|  [**SYN\_NMEA\_FixQuality**](syn__nmea_8h.md#enum-syn_nmea_fixquality) | [**fix\_type**](#variable-fix_type)  <br> |
|  double | [**latitude**](#variable-latitude)  <br> |
|  double | [**longitude**](#variable-longitude)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid**](#variable-valid)  <br> |












































## Public Attributes Documentation




### variable accuracy\_m 

```C++
float SYN_GeoPos::accuracy_m;
```



Estimated 3D position error bound 
 


        

<hr>



### variable altitude\_m 

```C++
double SYN_GeoPos::altitude_m;
```



Altitude above MSL/ellipsoid (m) 
 


        

<hr>



### variable fix\_type 

```C++
SYN_NMEA_FixQuality SYN_GeoPos::fix_type;
```



GPS, DGPS, RTK\_FIXED, RTK\_FLOAT 
 


        

<hr>



### variable latitude 

```C++
double SYN_GeoPos::latitude;
```



Decimal degrees (+N, -S) 
 


        

<hr>



### variable longitude 

```C++
double SYN_GeoPos::longitude;
```



Decimal degrees (+E, -W) 
 


        

<hr>



### variable valid 

```C++
bool SYN_GeoPos::valid;
```



true if position data is valid 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_geo.h`

