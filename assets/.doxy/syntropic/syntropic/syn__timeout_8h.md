

# File syn\_timeout.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_timeout.h**](syn__timeout_8h.md)

[Go to the source code of this file](syn__timeout_8h_source.md)

_Non-blocking timeout helper — tick wrap-around safe._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_system.h"`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Timeout**](structSYN__Timeout.md) <br>_Non-blocking timeout — tick wrap-around safe._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_timeout\_elapsed**](#function-syn_timeout_elapsed) (const [**SYN\_Timeout**](structSYN__Timeout.md) \* to) <br>_Get the elapsed time since the timeout was started._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_timeout\_expired**](#function-syn_timeout_expired) (const [**SYN\_Timeout**](structSYN__Timeout.md) \* to) <br>_Check if the timeout has expired._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_timeout\_periodic**](#function-syn_timeout_periodic) ([**SYN\_Timeout**](structSYN__Timeout.md) \* to) <br>_Check expiry and auto-restart if expired (periodic use)._  |
|  uint32\_t | [**syn\_timeout\_remaining**](#function-syn_timeout_remaining) (const [**SYN\_Timeout**](structSYN__Timeout.md) \* to) <br>_Get the remaining time until expiry._  |
|  void | [**syn\_timeout\_restart**](#function-syn_timeout_restart) ([**SYN\_Timeout**](structSYN__Timeout.md) \* to) <br>_Restart the timeout from_ _now_ _with the same duration._ |
|  void | [**syn\_timeout\_start**](#function-syn_timeout_start) ([**SYN\_Timeout**](structSYN__Timeout.md) \* to, uint32\_t duration) <br>_Start (or restart) a timeout._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_WAIT\_TIMEOUT**](syn__timeout_8h.md#define-pt_wait_timeout) (pt, to) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_timeout\_expired**](syn__timeout_8h.md#function-syn_timeout_expired)(to))`<br>_Block a protothread until the timeout expires._  |

## Detailed Description


The "has N ms elapsed?" pattern is used everywhere in embedded code and is easy to get wrong with tick counter wrap-around. This header provides a tiny inline API that handles it correctly.


Header-only — zero code size if unused.




**
**


```C++
SYN_Timeout to;
syn_timeout_start(&to, 1000);

while (!syn_timeout_expired(&to)) {
    // try something...
}

// In a protothread:
PT_WAIT_TIMEOUT(pt, &to);
```
 





    
## Public Static Functions Documentation




### function syn\_timeout\_elapsed 

_Get the elapsed time since the timeout was started._ 
```C++
static inline uint32_t syn_timeout_elapsed (
    const SYN_Timeout * to
) 
```





**Parameters:**


* `to` Timeout. 



**Returns:**

Elapsed milliseconds. 





        

<hr>



### function syn\_timeout\_expired 

_Check if the timeout has expired._ 
```C++
static inline bool syn_timeout_expired (
    const SYN_Timeout * to
) 
```



Uses unsigned subtraction, which is wrap-around safe for a single wrap of the 32-bit tick counter (~49 days at 1 kHz).




**Parameters:**


* `to` Timeout to check. 



**Returns:**

true if the timeout has expired. 





        

<hr>



### function syn\_timeout\_periodic 

_Check expiry and auto-restart if expired (periodic use)._ 
```C++
static inline bool syn_timeout_periodic (
    SYN_Timeout * to
) 
```



Useful in main loops: "every N ms, do X".




**Parameters:**


* `to` Timeout. 



**Returns:**

true if the timeout expired (and was restarted). 





        

<hr>



### function syn\_timeout\_remaining 

_Get the remaining time until expiry._ 
```C++
static inline uint32_t syn_timeout_remaining (
    const SYN_Timeout * to
) 
```





**Parameters:**


* `to` Timeout. 



**Returns:**

Remaining milliseconds, or 0 if already expired. 





        

<hr>



### function syn\_timeout\_restart 

_Restart the timeout from_ _now_ _with the same duration._
```C++
static inline void syn_timeout_restart (
    SYN_Timeout * to
) 
```





**Parameters:**


* `to` Timeout. 




        

<hr>



### function syn\_timeout\_start 

_Start (or restart) a timeout._ 
```C++
static inline void syn_timeout_start (
    SYN_Timeout * to,
    uint32_t duration
) 
```





**Parameters:**


* `to` Timeout to start. 
* `duration` Duration in milliseconds. 




        

<hr>
## Macro Definition Documentation





### define PT\_WAIT\_TIMEOUT 

_Block a protothread until the timeout expires._ 
```C++
#define PT_WAIT_TIMEOUT (
    pt,
    to
) `PT_WAIT_UNTIL (pt, syn_timeout_expired (to))`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_timeout.h`

