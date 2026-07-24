

# File syn\_rate\_limit.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_rate\_limit.h**](syn__rate__limit_8h.md)

[Go to the source code of this file](syn__rate__limit_8h_source.md)

_Token bucket rate limiter — header-only._ [More...](#detailed-description)

* `#include "../port/syn_port_system.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_RateLimit**](structSYN__RateLimit.md) <br>_Token bucket rate limiter instance._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_rate\_limit\_allow**](#function-syn_rate_limit_allow) ([**SYN\_RateLimit**](structSYN__RateLimit.md) \* rl) <br>_Try to consume one token. Returns true if allowed._  |
|  void | [**syn\_rate\_limit\_init**](#function-syn_rate_limit_init) ([**SYN\_RateLimit**](structSYN__RateLimit.md) \* rl, uint32\_t max\_tokens, uint32\_t interval\_ms) <br>_Initialize a rate limiter._  |
|  uint32\_t | [**syn\_rate\_limit\_remaining**](#function-syn_rate_limit_remaining) (const [**SYN\_RateLimit**](structSYN__RateLimit.md) \* rl) <br>_Check remaining tokens without consuming._  |
|  void | [**syn\_rate\_limit\_reset**](#function-syn_rate_limit_reset) ([**SYN\_RateLimit**](structSYN__RateLimit.md) \* rl) <br>_Reset the rate limiter to full capacity._  |


























## Detailed Description


Throttle operations to N-per-second. Classic use cases: limit log output, throttle retransmissions, cap sensor poll rate.




**
**


```C++
SYN_RateLimit rl;
syn_rate_limit_init(&rl, 10, 1000);  // 10 events per 1000ms

if (syn_rate_limit_allow(&rl)) {
    send_packet();
}
```
 





    
## Public Static Functions Documentation




### function syn\_rate\_limit\_allow 

_Try to consume one token. Returns true if allowed._ 
```C++
static inline bool syn_rate_limit_allow (
    SYN_RateLimit * rl
) 
```





**Parameters:**


* `rl` Rate limiter. 



**Returns:**

true if a token was consumed, false if exhausted. 





        

<hr>



### function syn\_rate\_limit\_init 

_Initialize a rate limiter._ 
```C++
static inline void syn_rate_limit_init (
    SYN_RateLimit * rl,
    uint32_t max_tokens,
    uint32_t interval_ms
) 
```





**Parameters:**


* `rl` Rate limiter instance. 
* `max_tokens` Maximum tokens (burst capacity). 
* `interval_ms` Interval over which tokens refill to max. 




        

<hr>



### function syn\_rate\_limit\_remaining 

_Check remaining tokens without consuming._ 
```C++
static inline uint32_t syn_rate_limit_remaining (
    const SYN_RateLimit * rl
) 
```





**Parameters:**


* `rl` Rate limiter. 



**Returns:**

Remaining tokens. 





        

<hr>



### function syn\_rate\_limit\_reset 

_Reset the rate limiter to full capacity._ 
```C++
static inline void syn_rate_limit_reset (
    SYN_RateLimit * rl
) 
```





**Parameters:**


* `rl` Rate limiter. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_rate_limit.h`

