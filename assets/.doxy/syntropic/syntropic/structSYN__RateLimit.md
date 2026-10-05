

# Struct SYN\_RateLimit



[**ClassList**](annotated.md) **>** [**SYN\_RateLimit**](structSYN__RateLimit.md)



_Token bucket rate limiter instance._ 

* `#include <syn_rate_limit.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**interval\_ms**](#variable-interval_ms)  <br> |
|  uint32\_t | [**last\_refill**](#variable-last_refill)  <br> |
|  uint32\_t | [**max\_tokens**](#variable-max_tokens)  <br> |
|  uint32\_t | [**tokens**](#variable-tokens)  <br> |












































## Public Attributes Documentation




### variable interval\_ms 

```C++
uint32_t SYN_RateLimit::interval_ms;
```



Refill interval (tokens refill to max) 
 


        

<hr>



### variable last\_refill 

```C++
uint32_t SYN_RateLimit::last_refill;
```



Tick of last refill 
 


        

<hr>



### variable max\_tokens 

```C++
uint32_t SYN_RateLimit::max_tokens;
```



Maximum (burst) capacity 
 


        

<hr>



### variable tokens 

```C++
uint32_t SYN_RateLimit::tokens;
```



Current token count 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_rate_limit.h`

