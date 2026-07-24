

# File syn\_rtc.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_rtc.h**](syn__rtc_8h.md)

[Go to the source code of this file](syn__rtc_8h_source.md)

_RTC (Real-Time Clock) driver._ [More...](#detailed-description)

* `#include "../port/syn_port_rtc.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_rtc\_from\_epoch**](#function-syn_rtc_from_epoch) (uint32\_t epoch, [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Convert a Unix epoch timestamp to a date/time._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_rtc\_get**](#function-syn_rtc_get) ([**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Read the current date/time._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_rtc\_init**](#function-syn_rtc_init) (void) <br>_Initialize the RTC peripheral._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_rtc\_is\_valid**](#function-syn_rtc_is_valid) (const [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Check whether all fields of a_ [_**SYN\_RTC\_DateTime**_](structSYN__RTC__DateTime.md) _are in range._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_rtc\_set**](#function-syn_rtc_set) (const [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Set the RTC to a new date/time._  |
|  void | [**syn\_rtc\_set\_drift\_ppm**](#function-syn_rtc_set_drift_ppm) (int32\_t drift\_ppm) <br>_Set the RTC clock drift compensation in Parts-Per-Million (PPM)._  |
|  uint32\_t | [**syn\_rtc\_to\_epoch**](#function-syn_rtc_to_epoch) (const [**SYN\_RTC\_DateTime**](structSYN__RTC__DateTime.md) \* dt) <br>_Convert a date/time to a Unix epoch timestamp._  |




























## Detailed Description


Thin wrapper over [**syn\_port\_rtc.h**](syn__port__rtc_8h.md) that adds input validation and Unix-epoch conversion utilities. No heap allocation; no state struct is required — calls delegate directly to the port layer.


Epoch base: 1970-01-01 00:00:00 UTC (standard Unix time). Valid date range: 1970-01-01 through 2105-12-31.


Usage: 
```C++
SYN_RTC_DateTime dt = { .year=2025, .month=6, .day=1,
                         .hour=12, .minute=0, .second=0 };
syn_rtc_init();
syn_rtc_set(&dt);
// ... later ...
syn_rtc_get(&dt);
uint32_t t = syn_rtc_to_epoch(&dt);
```
 


    
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



### function syn\_rtc\_set\_drift\_ppm 

_Set the RTC clock drift compensation in Parts-Per-Million (PPM)._ 
```C++
void syn_rtc_set_drift_ppm (
    int32_t drift_ppm
) 
```





**Parameters:**


* `drift_ppm` Drift in Parts-Per-Million (+ = fast, - = slow). 




        

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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_rtc.h`

