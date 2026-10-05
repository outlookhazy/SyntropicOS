

# Struct SYN\_NMEA\_ZDA



[**ClassList**](annotated.md) **>** [**SYN\_NMEA\_ZDA**](structSYN__NMEA__ZDA.md)



_Parsed NMEA ZDA (UTC Date & Time) Structure._ 

* `#include <syn_nmea.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**day**](#variable-day)  <br> |
|  uint8\_t | [**hours**](#variable-hours)  <br> |
|  uint16\_t | [**milliseconds**](#variable-milliseconds)  <br> |
|  uint8\_t | [**minutes**](#variable-minutes)  <br> |
|  uint8\_t | [**month**](#variable-month)  <br> |
|  uint8\_t | [**seconds**](#variable-seconds)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid**](#variable-valid)  <br> |
|  uint16\_t | [**year**](#variable-year)  <br> |












































## Public Attributes Documentation




### variable day 

```C++
uint8_t SYN_NMEA_ZDA::day;
```



Day of month (1..31) 


        

<hr>



### variable hours 

```C++
uint8_t SYN_NMEA_ZDA::hours;
```



UTC hours (0..23) 


        

<hr>



### variable milliseconds 

```C++
uint16_t SYN_NMEA_ZDA::milliseconds;
```



UTC milliseconds (0..999) 


        

<hr>



### variable minutes 

```C++
uint8_t SYN_NMEA_ZDA::minutes;
```



UTC minutes (0..59) 


        

<hr>



### variable month 

```C++
uint8_t SYN_NMEA_ZDA::month;
```



Month of year (1..12) 


        

<hr>



### variable seconds 

```C++
uint8_t SYN_NMEA_ZDA::seconds;
```



UTC seconds (0..59) 


        

<hr>



### variable valid 

```C++
bool SYN_NMEA_ZDA::valid;
```



True if frame parsed successfully 


        

<hr>



### variable year 

```C++
uint16_t SYN_NMEA_ZDA::year;
```



Full year (e.g. 2026) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_nmea.h`

