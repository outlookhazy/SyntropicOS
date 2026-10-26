

# Struct SYN\_HMAC\_DRBG



[**ClassList**](annotated.md) **>** [**SYN\_HMAC\_DRBG**](structSYN__HMAC__DRBG.md)



_HMAC-DRBG state context (NIST SP 800-90A)._ 

* `#include <syn_hmac_drbg.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialized**](#variable-initialized)  <br> |
|  uint8\_t | [**k**](#variable-k)  <br> |
|  uint32\_t | [**reseed\_counter**](#variable-reseed_counter)  <br> |
|  uint32\_t | [**reseed\_interval**](#variable-reseed_interval)  <br> |
|  uint8\_t | [**v**](#variable-v)  <br> |












































## Public Attributes Documentation




### variable initialized 

```C++
bool SYN_HMAC_DRBG::initialized;
```



True if DRBG has been instantiated 


        

<hr>



### variable k 

```C++
uint8_t SYN_HMAC_DRBG::k[SYN_HMAC_SHA256_SIZE];
```



Internal working key K (32 bytes) 


        

<hr>



### variable reseed\_counter 

```C++
uint32_t SYN_HMAC_DRBG::reseed_counter;
```



Number of generate calls since init/reseed 


        

<hr>



### variable reseed\_interval 

```C++
uint32_t SYN_HMAC_DRBG::reseed_interval;
```



Maximum generate calls before reseed required 


        

<hr>



### variable v 

```C++
uint8_t SYN_HMAC_DRBG::v[SYN_HMAC_SHA256_SIZE];
```



Internal working state V (32 bytes) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_hmac_drbg.h`

