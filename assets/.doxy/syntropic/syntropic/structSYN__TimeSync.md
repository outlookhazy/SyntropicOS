

# Struct SYN\_TimeSync



[**ClassList**](annotated.md) **>** [**SYN\_TimeSync**](structSYN__TimeSync.md)



_Time sync discipline state context (caller-allocated)._ 

* `#include <syn_timesync.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**base\_jitter\_ns**](#variable-base_jitter_ns)  <br> |
|  int32\_t | [**drift\_ppm**](#variable-drift_ppm)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_pps\_lock**](#variable-has_pps_lock)  <br> |
|  uint64\_t | [**last\_pps\_ticks**](#variable-last_pps_ticks)  <br> |
|  [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) | [**last\_pps\_ts**](#variable-last_pps_ts)  <br> |
|  uint64\_t | [**last\_utc\_sec**](#variable-last_utc_sec)  <br> |
|  uint32\_t | [**max\_holdover\_s**](#variable-max_holdover_s)  <br> |
|  uint32\_t | [**pps\_count**](#variable-pps_count)  <br> |
|  uint64\_t | [**prev\_pps\_ticks**](#variable-prev_pps_ticks)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**rtc\_synced**](#variable-rtc_synced)  <br> |












































## Public Attributes Documentation




### variable base\_jitter\_ns 

```C++
uint32_t SYN_TimeSync::base_jitter_ns;
```



Base reference jitter (default: 50 ns) 
 


        

<hr>



### variable drift\_ppm 

```C++
int32_t SYN_TimeSync::drift_ppm;
```



Measured crystal drift in Parts-Per-Million 


        

<hr>



### variable has\_pps\_lock 

```C++
bool SYN_TimeSync::has_pps_lock;
```



true if at least one PPS is bound 
 


        

<hr>



### variable last\_pps\_ticks 

```C++
uint64_t SYN_TimeSync::last_pps_ticks;
```



Resolved 64-bit ticks at last PPS 
 


        

<hr>



### variable last\_pps\_ts 

```C++
SYN_HPTimestamp SYN_TimeSync::last_pps_ts;
```



Raw timestamp at last PPS pulse 
 


        

<hr>



### variable last\_utc\_sec 

```C++
uint64_t SYN_TimeSync::last_utc_sec;
```



Universal UTC second at last PPS 
 


        

<hr>



### variable max\_holdover\_s 

```C++
uint32_t SYN_TimeSync::max_holdover_s;
```



Max holdover before degrading to RTC 
 


        

<hr>



### variable pps\_count 

```C++
uint32_t SYN_TimeSync::pps_count;
```



Total valid PPS updates received 
 


        

<hr>



### variable prev\_pps\_ticks 

```C++
uint64_t SYN_TimeSync::prev_pps_ticks;
```



Ticks at previous PPS for PPM calculation 


        

<hr>



### variable rtc\_synced 

```C++
bool SYN_TimeSync::rtc_synced;
```



true if RTC was previously set by GPS 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_timesync.h`

