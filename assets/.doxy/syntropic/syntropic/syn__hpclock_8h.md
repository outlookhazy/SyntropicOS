

# File syn\_hpclock.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_hpclock.h**](syn__hpclock_8h.md)

[Go to the source code of this file](syn__hpclock_8h_source.md)

_High-precision clock — 64-bit system-clock-precision timestamps._ [More...](#detailed-description)

* `#include "../common/syn_compiler.h"`
* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_hpclock.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) <br>_Raw high-precision timestamp — three-word snapshot._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint64\_t | [**syn\_hpclock\_elapsed**](#function-syn_hpclock_elapsed) (const [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) \* start, const [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) \* end) <br>_Compute elapsed ticks between two timestamps._  |
|  uint64\_t | [**syn\_hpclock\_resolve**](#function-syn_hpclock_resolve) (const [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) \* ts) <br>_Resolve a raw timestamp into a 64-bit tick count._  |
|  uint64\_t | [**syn\_hpclock\_ticks\_to\_ns**](#function-syn_hpclock_ticks_to_ns) (uint64\_t ticks) <br>_Convert a tick count to nanoseconds._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_hpclock\_is\_zero**](#function-syn_hpclock_is_zero) (const [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) \* ts) <br>_Check if a timestamp is zero (uninitialized)._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_HPCLOCK\_CAPTURE**](syn__hpclock_8h.md#define-syn_hpclock_capture) (ts) `/* multi line expression */`<br>_Snapshot the high-precision clock into a timestamp struct._  |
| define  | [**SYN\_HPTIMESTAMP\_INIT**](syn__hpclock_8h.md#define-syn_hptimestamp_init)  `{0, 0, 0}`<br>_Static initializer for_ [_**SYN\_HPTimestamp**_](structSYN__HPTimestamp.md) _(all zeros)._ |

## Detailed Description


## Public Functions Documentation




### function syn\_hpclock\_elapsed 

_Compute elapsed ticks between two timestamps._ 
```C++
uint64_t syn_hpclock_elapsed (
    const SYN_HPTimestamp * start,
    const SYN_HPTimestamp * end
) 
```



Resolves both timestamps and returns the difference. Assumes `end` was captured after `start`.




**Parameters:**


* `start` Earlier timestamp. 
* `end` Later timestamp. 



**Returns:**

Elapsed ticks (end - start). 





        

<hr>



### function syn\_hpclock\_resolve 

_Resolve a raw timestamp into a 64-bit tick count._ 
```C++
uint64_t syn_hpclock_resolve (
    const SYN_HPTimestamp * ts
) 
```



If `msb_1 == msb_2`, no overflow occurred and either MSB is correct. If they differ, the overflow happened during the capture window and the LSB value determines which side of the wrap it was captured on: a small LSB (&lt; 0x80000000) means post-wrap (use msb\_2), a large LSB means pre-wrap (use msb\_1).


This is a pure function — no side effects, no hardware access.




**Parameters:**


* `ts` Pointer to a captured timestamp. 



**Returns:**

64-bit tick count at system clock precision. 





        

<hr>



### function syn\_hpclock\_ticks\_to\_ns 

_Convert a tick count to nanoseconds._ 
```C++
uint64_t syn_hpclock_ticks_to_ns (
    uint64_t ticks
) 
```



Uses integer-only arithmetic: ns = ticks \* 1000000000 / freq\_hz. The division is performed with 64-bit precision to avoid overflow.




**Parameters:**


* `ticks` Tick count from [**syn\_hpclock\_resolve()**](syn__hpclock_8h.md#function-syn_hpclock_resolve). 



**Returns:**

Equivalent time in nanoseconds. 





        

<hr>
## Public Static Functions Documentation




### function syn\_hpclock\_is\_zero 

_Check if a timestamp is zero (uninitialized)._ 
```C++
static inline bool syn_hpclock_is_zero (
    const SYN_HPTimestamp * ts
) 
```





**Parameters:**


* `ts` Timestamp to check. 



**Returns:**

true if all fields are zero. 





        

<hr>
## Macro Definition Documentation





### define SYN\_HPCLOCK\_CAPTURE 

_Snapshot the high-precision clock into a timestamp struct._ 
```C++
#define SYN_HPCLOCK_CAPTURE (
    ts
) `/* multi line expression */`
```



Three volatile reads, no branching, no function call overhead. Safe to call from ISR context (provided the overflow ISR has a higher priority — see [**syn\_port\_hpclock.h**](syn__port__hpclock_8h.md)).


Compiler barriers between the MSB (SRAM) and LSB (peripheral bus) reads prevent the compiler from reordering the accesses.




**Parameters:**


* `ts` An [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) lvalue (not a pointer). 




        

<hr>



### define SYN\_HPTIMESTAMP\_INIT 

_Static initializer for_ [_**SYN\_HPTimestamp**_](structSYN__HPTimestamp.md) _(all zeros)._
```C++
#define SYN_HPTIMESTAMP_INIT `{0, 0, 0}`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_hpclock.h`

