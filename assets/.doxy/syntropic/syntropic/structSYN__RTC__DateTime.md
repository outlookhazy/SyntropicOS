

# Struct SYN\_RTC\_DateTime



[**ClassList**](annotated.md) **>** [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md)



_Calendar date and time representation._ [More...](#detailed-description)

* `#include <syn_port_rtc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**day**](#variable-day)  <br> |
|  uint8\_t | [**hour**](#variable-hour)  <br> |
|  uint8\_t | [**minute**](#variable-minute)  <br> |
|  uint8\_t | [**month**](#variable-month)  <br> |
|  uint8\_t | [**second**](#variable-second)  <br> |
|  uint16\_t | [**year**](#variable-year)  <br> |












































## Detailed Description


All fields are in natural units (year = 4-digit, month 1-12, etc.). 


    
## Public Attributes Documentation




### variable day 

```C++
uint8_t SYN_RTC_DateTime::day;
```



Day of month: 1 – 31 
 


        

<hr>



### variable hour 

```C++
uint8_t SYN_RTC_DateTime::hour;
```



Hour: 0 – 23 
 


        

<hr>



### variable minute 

```C++
uint8_t SYN_RTC_DateTime::minute;
```



Minute: 0 – 59 
 


        

<hr>



### variable month 

```C++
uint8_t SYN_RTC_DateTime::month;
```



Month: 1 = January, 12 = December 


        

<hr>



### variable second 

```C++
uint8_t SYN_RTC_DateTime::second;
```



Second: 0 – 59 
 


        

<hr>



### variable year 

```C++
uint16_t SYN_RTC_DateTime::year;
```



Full year, e.g. 2025 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_rtc.h`

