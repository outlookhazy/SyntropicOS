

# Struct SYN\_NMEA\_RMC



[**ClassList**](annotated.md) **>** [**SYN\_NMEA\_RMC**](structSYN__NMEA__RMC.md)



_Parsed NMEA RMC (Recommended Minimum Data) Structure._ 

* `#include <syn_nmea.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**course\_deg**](#variable-course_deg)  <br> |
|  uint8\_t | [**day**](#variable-day)  <br> |
|  uint8\_t | [**hours**](#variable-hours)  <br> |
|  double | [**latitude**](#variable-latitude)  <br> |
|  double | [**longitude**](#variable-longitude)  <br> |
|  uint16\_t | [**milliseconds**](#variable-milliseconds)  <br> |
|  uint8\_t | [**minutes**](#variable-minutes)  <br> |
|  uint8\_t | [**month**](#variable-month)  <br> |
|  uint8\_t | [**seconds**](#variable-seconds)  <br> |
|  float | [**speed\_knots**](#variable-speed_knots)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**status\_valid**](#variable-status_valid)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid**](#variable-valid)  <br> |
|  uint16\_t | [**year**](#variable-year)  <br> |












































## Public Attributes Documentation




### variable course\_deg 

```C++
float SYN_NMEA_RMC::course_deg;
```



Course over ground in true degrees 


        

<hr>



### variable day 

```C++
uint8_t SYN_NMEA_RMC::day;
```




<hr>



### variable hours 

```C++
uint8_t SYN_NMEA_RMC::hours;
```




<hr>



### variable latitude 

```C++
double SYN_NMEA_RMC::latitude;
```



Decimal degrees (+N, -S) 


        

<hr>



### variable longitude 

```C++
double SYN_NMEA_RMC::longitude;
```



Decimal degrees (+E, -W) 


        

<hr>



### variable milliseconds 

```C++
uint16_t SYN_NMEA_RMC::milliseconds;
```




<hr>



### variable minutes 

```C++
uint8_t SYN_NMEA_RMC::minutes;
```




<hr>



### variable month 

```C++
uint8_t SYN_NMEA_RMC::month;
```




<hr>



### variable seconds 

```C++
uint8_t SYN_NMEA_RMC::seconds;
```




<hr>



### variable speed\_knots 

```C++
float SYN_NMEA_RMC::speed_knots;
```



Speed over ground in knots 


        

<hr>



### variable status\_valid 

```C++
bool SYN_NMEA_RMC::status_valid;
```



'A' = valid, 'V' = receiver warning 


        

<hr>



### variable valid 

```C++
bool SYN_NMEA_RMC::valid;
```




<hr>



### variable year 

```C++
uint16_t SYN_NMEA_RMC::year;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_nmea.h`

