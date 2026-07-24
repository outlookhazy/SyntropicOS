

# File syn\_backoff.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_backoff.c**](syn__backoff_8c.md)

[Go to the source code of this file](syn__backoff_8c_source.md)

_Exponential backoff implementation._ 

* `#include "syn_backoff.h"`
* `#include "syn_random.h"`
* `#include "syn_assert.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_backoff\_exhausted**](#function-syn_backoff_exhausted) (const [**SYN\_Backoff**](structSYN__Backoff.md) \* b) <br>_Check if the maximum number of attempts has been reached._  |
|  void | [**syn\_backoff\_init**](#function-syn_backoff_init) ([**SYN\_Backoff**](structSYN__Backoff.md) \* b, uint32\_t min\_ms, uint32\_t max\_ms, uint8\_t factor, uint8\_t max\_attempts) <br>_Initialize backoff state._  |
|  uint32\_t | [**syn\_backoff\_next\_ms**](#function-syn_backoff_next_ms) ([**SYN\_Backoff**](structSYN__Backoff.md) \* b) <br>_Get the next delay in milliseconds._  |
|  void | [**syn\_backoff\_reset**](#function-syn_backoff_reset) ([**SYN\_Backoff**](structSYN__Backoff.md) \* b) <br>_Reset the backoff state to the initial minimum delay._  |




























## Public Functions Documentation




### function syn\_backoff\_exhausted 

_Check if the maximum number of attempts has been reached._ 
```C++
bool syn_backoff_exhausted (
    const SYN_Backoff * b
) 
```





**Parameters:**


* `b` Backoff context. 



**Returns:**

true if attempts &gt;= max\_attempts. 





        

<hr>



### function syn\_backoff\_init 

_Initialize backoff state._ 
```C++
void syn_backoff_init (
    SYN_Backoff * b,
    uint32_t min_ms,
    uint32_t max_ms,
    uint8_t factor,
    uint8_t max_attempts
) 
```





**Parameters:**


* `b` Backoff context. 
* `min_ms` Initial delay (e.g., 1000). 
* `max_ms` Maximum delay (e.g., 60000). 
* `factor` Exponential factor (usually 2). 
* `max_attempts` Maximum number of attempts. 




        

<hr>



### function syn\_backoff\_next\_ms 

_Get the next delay in milliseconds._ 
```C++
uint32_t syn_backoff_next_ms (
    SYN_Backoff * b
) 
```



Each call increases the delay exponentially (up to max\_ms). Includes RFC 7252-style jitter (1.0x to 1.5x) to avoid synchronization.




**Parameters:**


* `b` Backoff context. 



**Returns:**

Next delay duration in milliseconds. 





        

<hr>



### function syn\_backoff\_reset 

_Reset the backoff state to the initial minimum delay._ 
```C++
void syn_backoff_reset (
    SYN_Backoff * b
) 
```





**Parameters:**


* `b` Backoff context. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_backoff.c`

