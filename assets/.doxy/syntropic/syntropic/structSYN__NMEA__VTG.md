

# Struct SYN\_NMEA\_VTG



[**ClassList**](annotated.md) **>** [**SYN\_NMEA\_VTG**](structSYN__NMEA__VTG.md)



_Parsed NMEA VTG (Velocity & Course) Structure._ 

* `#include <syn_nmea.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**course\_true\_deg**](#variable-course_true_deg)  <br> |
|  float | [**speed\_knots**](#variable-speed_knots)  <br> |
|  float | [**speed\_kph**](#variable-speed_kph)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid**](#variable-valid)  <br> |












































## Public Attributes Documentation




### variable course\_true\_deg 

```C++
float SYN_NMEA_VTG::course_true_deg;
```



True track course in degrees 


        

<hr>



### variable speed\_knots 

```C++
float SYN_NMEA_VTG::speed_knots;
```



Speed over ground in knots 


        

<hr>



### variable speed\_kph 

```C++
float SYN_NMEA_VTG::speed_kph;
```



Speed over ground in km/h 


        

<hr>



### variable valid 

```C++
bool SYN_NMEA_VTG::valid;
```



True if frame parsed successfully 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_nmea.h`

