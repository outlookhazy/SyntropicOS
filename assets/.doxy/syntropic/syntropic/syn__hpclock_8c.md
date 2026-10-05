

# File syn\_hpclock.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_hpclock.c**](syn__hpclock_8c.md)

[Go to the source code of this file](syn__hpclock_8c_source.md)

_High-precision clock — resolution and conversion implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_hpclock.h"`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  volatile uint32\_t | [**syn\_hpclock\_msb**](#variable-syn_hpclock_msb)  <br>_Global overflow counter — incremented by the platform ISR._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint64\_t | [**syn\_hpclock\_elapsed**](#function-syn_hpclock_elapsed) (const [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) \* start, const [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) \* end) <br>_Compute elapsed ticks between two timestamps._  |
|  uint64\_t | [**syn\_hpclock\_resolve**](#function-syn_hpclock_resolve) (const [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md) \* ts) <br>_Resolve a raw timestamp into a 64-bit tick count._  |
|  uint64\_t | [**syn\_hpclock\_ticks\_to\_ns**](#function-syn_hpclock_ticks_to_ns) (uint64\_t ticks) <br>_Convert a tick count to nanoseconds._  |




























## Public Attributes Documentation




### variable syn\_hpclock\_msb 

_Global overflow counter — incremented by the platform ISR._ 
```C++
volatile uint32_t syn_hpclock_msb;
```



Declared here so both the port ISR and the driver can access it. Defined in [**syn\_hpclock.c**](syn__hpclock_8c.md). 


        

<hr>
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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_hpclock.c`

