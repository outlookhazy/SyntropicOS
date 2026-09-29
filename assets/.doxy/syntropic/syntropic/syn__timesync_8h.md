

# File syn\_timesync.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_timesync.h**](syn__timesync_8h.md)

[Go to the source code of this file](syn__timesync_8h_source.md)

_High-precision time discipline and clock synchronization service._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "syn_hpclock.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_TimeSync**](structSYN__TimeSync.md) <br>_Time sync discipline state context (caller-allocated)._  |
| struct | [**SYN\_UTCTimestamp**](structSYN__UTCTimestamp.md) <br>_Universal UTC timestamp with error bound and quality tier._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_TimeSource**](#enum-syn_timesource)  <br>_Quality tier of a resolved UTC timestamp._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_timesync\_bind\_pps**](#function-syn_timesync_bind_pps) ([**SYN\_TimeSync**](structSYN__TimeSync.md) \* tsync, const [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) \* pps\_ts, uint64\_t utc\_sec) <br>_Bind a captured PPS timestamp to its corresponding UTC second._  |
|  void | [**syn\_timesync\_init**](#function-syn_timesync_init) ([**SYN\_TimeSync**](structSYN__TimeSync.md) \* tsync) <br>_Initialize a time discipline context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_timesync\_is\_pps\_locked**](#function-syn_timesync_is_pps_locked) (const [**SYN\_TimeSync**](structSYN__TimeSync.md) \* tsync) <br>_Check if active PPS lock is present._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_timesync\_resolve\_utc**](#function-syn_timesync_resolve_utc) (const [**SYN\_TimeSync**](structSYN__TimeSync.md) \* tsync, const [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) \* event\_ts, [**SYN\_UTCTimestamp**](structSYN__UTCTimestamp.md) \* out\_utc) <br>_Resolve an arbitrary high-precision timestamp into UTC._  |
|  uint64\_t | [**syn\_timesync\_to\_epoch\_ns**](#function-syn_timesync_to_epoch_ns) (const [**SYN\_TimeSync**](structSYN__TimeSync.md) \* tsync, const [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) \* event\_ts) <br>_Helper: resolve event timestamp directly to total 64-bit UTC nanoseconds._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_TIMESYNC\_DEFAULT\_HOLDOVER\_S**](syn__timesync_8h.md#define-syn_timesync_default_holdover_s)  `60U`<br> |
| define  | [**SYN\_TIMESYNC\_DEFAULT\_JITTER\_NS**](syn__timesync_8h.md#define-syn_timesync_default_jitter_ns)  `50U`<br> |
| define  | [**SYN\_TIMESYNC\_HAS\_RTC**](syn__timesync_8h.md#define-syn_timesync_has_rtc)  `0`<br> |

## Detailed Description


`syn_timesync` converts raw high-precision hardware timestamps (`SYN_HPTimestamp` from `syn_hpclock`) into universal UTC wall-clock time with quantified uncertainty bounds (± nanoseconds).


It combines an asynchronous 1 Hz pulse reference (such as a GPS PPS signal) with an epoch date/time source (such as NMEA UTC sentences), estimates hardware crystal oscillator drift in Parts-Per-Million (PPM), and provides a 4-tier quality fallback hierarchy:



* SYN\_TIME\_SOURCE\_GPS\_PPS Active PPS lock (sub-microsecond UTC)
* SYN\_TIME\_SOURCE\_GPS\_HOLDOVER PPS pulse lost; extrapolating with drift PPM
* SYN\_TIME\_SOURCE\_RTC\_SYNCED Long outage; using hardware RTC (if enabled)
* SYN\_TIME\_SOURCE\_UNSYNCED No time reference; raw monotonic uptime




### Usage Example




```C++
static SYN_TimeSync tsync;
syn_timesync_init(&tsync);

// In PPS ISR:
SYN_HPCLOCK_CAPTURE(pps_ts);

// In GPS UART task (after NMEA UTC second parsed):
syn_timesync_bind_pps(&tsync, &pps_ts, utc_epoch_seconds);

// Resolving an event timestamp:
SYN_UTCTimestamp utc;
syn_timesync_resolve_utc(&tsync, &event_ts, &utc);
if (utc.source == SYN_TIME_SOURCE_GPS_PPS && utc.uncertainty_ns < 1000) {
    // High-precision event processing
}
```
 



    
## Public Types Documentation




### enum SYN\_TimeSource 

_Quality tier of a resolved UTC timestamp._ 
```C++
enum SYN_TimeSource {
    SYN_TIME_SOURCE_UNSYNCED = 0,
    SYN_TIME_SOURCE_RTC_SYNCED = 1,
    SYN_TIME_SOURCE_GPS_HOLDOVER = 2,
    SYN_TIME_SOURCE_GPS_PPS = 3
};
```




<hr>
## Public Functions Documentation




### function syn\_timesync\_bind\_pps 

_Bind a captured PPS timestamp to its corresponding UTC second._ 
```C++
SYN_Status syn_timesync_bind_pps (
    SYN_TimeSync * tsync,
    const SYN_HPTimestamp * pps_ts,
    uint64_t utc_sec
) 
```



Call this when an asynchronous epoch time sentence (e.g. NMEA $GPZDA) is received, pairing the integer UTC second with the PPS timestamp captured at the top of that second.


Measures crystal drift PPM against nominal [**syn\_port\_hpclock\_freq\_hz()**](syn__port__hpclock_8h.md#function-syn_port_hpclock_freq_hz).




**Parameters:**


* `tsync` TimeSync context. Must not be NULL. 
* `pps_ts` Timestamp snapshot captured by ISR at PPS rising edge. 
* `utc_sec` Universal UTC epoch second corresponding to this PPS pulse. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if pps\_ts is NULL. 





        

<hr>



### function syn\_timesync\_init 

_Initialize a time discipline context._ 
```C++
void syn_timesync_init (
    SYN_TimeSync * tsync
) 
```



Sets default jitter (50 ns) and max holdover (60 s). If hardware RTC is enabled (`SYN_USE_RTC`), checks if the RTC is valid.




**Parameters:**


* `tsync` Context to initialize. Must not be NULL. 




        

<hr>



### function syn\_timesync\_is\_pps\_locked 

_Check if active PPS lock is present._ 
```C++
bool syn_timesync_is_pps_locked (
    const SYN_TimeSync * tsync
) 
```





**Parameters:**


* `tsync` TimeSync context. 



**Returns:**

true if PPS lock is active (received within last 1.1s). 





        

<hr>



### function syn\_timesync\_resolve\_utc 

_Resolve an arbitrary high-precision timestamp into UTC._ 
```C++
SYN_Status syn_timesync_resolve_utc (
    const SYN_TimeSync * tsync,
    const SYN_HPTimestamp * event_ts,
    SYN_UTCTimestamp * out_utc
) 
```



Converts a raw [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) (captured in an ISR or main thread) into absolute UTC time, calculating the sub-second offset, active quality tier, and linear error accumulation bound (uncertainty\_ns).


Handles event timestamps occurring before or after the reference PPS pulse.




**Parameters:**


* `tsync` TimeSync context. Must not be NULL. 
* `event_ts` Event timestamp to resolve. Must not be NULL. 
* `out_utc` Output UTC timestamp. Must not be NULL. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if args are NULL. 





        

<hr>



### function syn\_timesync\_to\_epoch\_ns 

_Helper: resolve event timestamp directly to total 64-bit UTC nanoseconds._ 
```C++
uint64_t syn_timesync_to_epoch_ns (
    const SYN_TimeSync * tsync,
    const SYN_HPTimestamp * event_ts
) 
```





**Parameters:**


* `tsync` TimeSync context. 
* `event_ts` Event timestamp to resolve. 



**Returns:**

Total nanoseconds since Unix epoch (1970-01-01 00:00:00 UTC). 





        

<hr>
## Macro Definition Documentation





### define SYN\_TIMESYNC\_DEFAULT\_HOLDOVER\_S 

```C++
#define SYN_TIMESYNC_DEFAULT_HOLDOVER_S `60U`
```



Default max holdover duration before degrading to RTC (60 seconds). 


        

<hr>



### define SYN\_TIMESYNC\_DEFAULT\_JITTER\_NS 

```C++
#define SYN_TIMESYNC_DEFAULT_JITTER_NS `50U`
```



Default assumed PPS reference jitter (50 nanoseconds). 


        

<hr>



### define SYN\_TIMESYNC\_HAS\_RTC 

```C++
#define SYN_TIMESYNC_HAS_RTC `0`
```



RTC driver presence flag (0) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_timesync.h`

