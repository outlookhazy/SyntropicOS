

# File syn\_rtc.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_rtc.c**](syn__rtc_8c.md)

[Go to the source code of this file](syn__rtc_8c_source.md)

_RTC driver implementation — port delegation + epoch math._ 

* `#include "syn_rtc.h"`
* `#include "../util/syn_assert.h"`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t | [**s\_days\_in\_month**](#variable-s_days_in_month)   = `/* multi line expression */`<br>_Days per month (non-leap year)._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_rtc\_from\_epoch**](#function-syn_rtc_from_epoch) (uint32\_t epoch, [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Convert a Unix epoch timestamp to a date/time._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_rtc\_get**](#function-syn_rtc_get) ([**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Read the current date/time._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_rtc\_init**](#function-syn_rtc_init) (void) <br>_Initialize the RTC peripheral._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_rtc\_is\_valid**](#function-syn_rtc_is_valid) (const [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Check whether all fields of a_ [_**SYN\_RTC\_DateTime**_](structSYN__RTC__DateTime.md) _are in range._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_rtc\_set**](#function-syn_rtc_set) (const [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Set the RTC to a new date/time._  |
|  uint32\_t | [**syn\_rtc\_to\_epoch**](#function-syn_rtc_to_epoch) (const [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Convert a date/time to a Unix epoch timestamp._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**rtc\_days\_in\_month**](#function-rtc_days_in_month) (uint8\_t month, uint16\_t year) <br>_Get days in a given month._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**rtc\_is\_leap**](#function-rtc_is_leap) (uint16\_t year) <br>_Test if a year is a leap year._  |


























## Public Static Attributes Documentation




### variable s\_days\_in\_month 

_Days per month (non-leap year)._ 
```C++
const uint8_t s_days_in_month[12];
```




<hr>
## Public Functions Documentation




### function syn\_rtc\_from\_epoch 

_Convert a Unix epoch timestamp to a date/time._ 
```C++
void syn_rtc_from_epoch (
    uint32_t epoch,
    SYN_RTC_DateTime * dt
) 
```





**Parameters:**


* `epoch` Seconds since 1970-01-01 00:00:00 UTC. 
* `dt` Output. Must not be NULL. 




        

<hr>



### function syn\_rtc\_get 

_Read the current date/time._ 
```C++
SYN_Status syn_rtc_get (
    SYN_RTC_DateTime * dt
) 
```





**Parameters:**


* `dt` Output, must not be NULL. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on hardware failure. 





        

<hr>



### function syn\_rtc\_init 

_Initialize the RTC peripheral._ 
```C++
SYN_Status syn_rtc_init (
    void
) 
```





**Returns:**

SYN\_OK on success, SYN\_ERROR on failure. 





        

<hr>



### function syn\_rtc\_is\_valid 

_Check whether all fields of a_ [_**SYN\_RTC\_DateTime**_](structSYN__RTC__DateTime.md) _are in range._
```C++
bool syn_rtc_is_valid (
    const SYN_RTC_DateTime * dt
) 
```





**Parameters:**


* `dt` Date/time to validate. Must not be NULL. 



**Returns:**

true if valid, false if any field is out of range. 





        

<hr>



### function syn\_rtc\_set 

_Set the RTC to a new date/time._ 
```C++
SYN_Status syn_rtc_set (
    const SYN_RTC_DateTime * dt
) 
```



Validates all fields before writing to hardware.




**Parameters:**


* `dt` New time. Must not be NULL and must pass [**syn\_rtc\_is\_valid()**](syn__rtc_8h.md#function-syn_rtc_is_valid). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if fields are out of range, SYN\_ERROR on hardware failure. 





        

<hr>



### function syn\_rtc\_to\_epoch 

_Convert a date/time to a Unix epoch timestamp._ 
```C++
uint32_t syn_rtc_to_epoch (
    const SYN_RTC_DateTime * dt
) 
```



Epoch = seconds since 1970-01-01 00:00:00 UTC. Leap seconds are not accounted for (standard POSIX convention).




**Parameters:**


* `dt` Date/time to convert. Must be valid. 



**Returns:**

Seconds since Unix epoch. 





        

<hr>
## Public Static Functions Documentation




### function rtc\_days\_in\_month 

_Get days in a given month._ 
```C++
static uint8_t rtc_days_in_month (
    uint8_t month,
    uint16_t year
) 
```





**Parameters:**


* `month` Month (1–12). 
* `year` Gregorian year. 



**Returns:**

Day count. 





        

<hr>



### function rtc\_is\_leap 

_Test if a year is a leap year._ 
```C++
static bool rtc_is_leap (
    uint16_t year
) 
```





**Parameters:**


* `year` Gregorian year. 



**Returns:**

true if leap year. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_rtc.c`

