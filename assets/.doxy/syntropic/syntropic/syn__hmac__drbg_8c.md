

# File syn\_hmac\_drbg.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_hmac\_drbg.c**](syn__hmac__drbg_8c.md)

[Go to the source code of this file](syn__hmac__drbg_8c_source.md)

_NIST SP 800-90A HMAC-DRBG (Deterministic Random Bit Generator) implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_hmac_drbg.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hmac\_drbg\_generate**](#function-syn_hmac_drbg_generate) ([**SYN\_HMAC\_DRBG**](structSYN__HMAC__DRBG.md) \* ctx, uint8\_t \* out, size\_t out\_len, const uint8\_t \* add\_input, size\_t add\_len) <br>_Generate pseudorandom bytes from the HMAC-DRBG._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hmac\_drbg\_generate\_pr**](#function-syn_hmac_drbg_generate_pr) ([**SYN\_HMAC\_DRBG**](structSYN__HMAC__DRBG.md) \* ctx, uint8\_t \* out, size\_t out\_len, const uint8\_t \* entropy, size\_t ent\_len, const uint8\_t \* add\_input, size\_t add\_len) <br>_Generate pseudorandom bytes with Prediction Resistance (PR)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hmac\_drbg\_init**](#function-syn_hmac_drbg_init) ([**SYN\_HMAC\_DRBG**](structSYN__HMAC__DRBG.md) \* ctx, const uint8\_t \* entropy, size\_t ent\_len, const uint8\_t \* nonce, size\_t nonce\_len, const uint8\_t \* pers\_str, size\_t pers\_len) <br>_Instantiate the HMAC-DRBG state with initial entropy, nonce, and optional personalization._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_hmac\_drbg\_reseed**](#function-syn_hmac_drbg_reseed) ([**SYN\_HMAC\_DRBG**](structSYN__HMAC__DRBG.md) \* ctx, const uint8\_t \* entropy, size\_t ent\_len, const uint8\_t \* add\_input, size\_t add\_len) <br>_Reseed the HMAC-DRBG with fresh entropy and optional additional input._  |
|  void | [**syn\_hmac\_drbg\_set\_reseed\_interval**](#function-syn_hmac_drbg_set_reseed_interval) ([**SYN\_HMAC\_DRBG**](structSYN__HMAC__DRBG.md) \* ctx, uint32\_t interval) <br>_Set the maximum reseed interval for the DRBG instance._  |
|  void | [**syn\_hmac\_drbg\_wipe**](#function-syn_hmac_drbg_wipe) ([**SYN\_HMAC\_DRBG**](structSYN__HMAC__DRBG.md) \* ctx) <br>_Securely wipe internal working key, state vector, and counters._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**drbg\_update**](#function-drbg_update) ([**SYN\_HMAC\_DRBG**](structSYN__HMAC__DRBG.md) \* ctx, const uint8\_t \* c1, size\_t l1, const uint8\_t \* c2, size\_t l2, const uint8\_t \* c3, size\_t l3) <br>_Internal HMAC-DRBG state update function per NIST SP 800-90A Rev 1 Section 10.1.2.2._  |


























## Public Functions Documentation




### function syn\_hmac\_drbg\_generate 

_Generate pseudorandom bytes from the HMAC-DRBG._ 
```C++
SYN_Status syn_hmac_drbg_generate (
    SYN_HMAC_DRBG * ctx,
    uint8_t * out,
    size_t out_len,
    const uint8_t * add_input,
    size_t add_len
) 
```



Implements NIST SP 800-90A Rev 1 Section 10.1.2.5 without prediction resistance.




**Parameters:**


* `ctx` Pointer to initialized DRBG context. 
* `out` [out] Destination buffer for generated random bytes. 
* `out_len` Number of random bytes to generate (&lt;= 65536). 
* `add_input` Optional additional input buffer (can be NULL). 
* `add_len` Length of additional input in bytes. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if reseed interval is exceeded (reseed required), SYN\_INVALID\_PARAM if parameters or state are invalid. 





        

<hr>



### function syn\_hmac\_drbg\_generate\_pr 

_Generate pseudorandom bytes with Prediction Resistance (PR)._ 
```C++
SYN_Status syn_hmac_drbg_generate_pr (
    SYN_HMAC_DRBG * ctx,
    uint8_t * out,
    size_t out_len,
    const uint8_t * entropy,
    size_t ent_len,
    const uint8_t * add_input,
    size_t add_len
) 
```



Reseeds the DRBG with fresh entropy before generating pseudorandom bytes.




**Parameters:**


* `ctx` Pointer to initialized DRBG context. 
* `out` [out] Destination buffer for generated random bytes. 
* `out_len` Number of random bytes to generate (&lt;= 65536). 
* `entropy` Fresh entropy for prediction resistance (must be &gt;= 32 bytes). 
* `ent_len` Length of entropy in bytes (must be &gt;= 32). 
* `add_input` Optional additional input buffer (can be NULL). 
* `add_len` Length of additional input in bytes. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_hmac\_drbg\_init 

_Instantiate the HMAC-DRBG state with initial entropy, nonce, and optional personalization._ 
```C++
SYN_Status syn_hmac_drbg_init (
    SYN_HMAC_DRBG * ctx,
    const uint8_t * entropy,
    size_t ent_len,
    const uint8_t * nonce,
    size_t nonce_len,
    const uint8_t * pers_str,
    size_t pers_len
) 
```



Implements NIST SP 800-90A Rev 1 Section 10.1.2.3.




**Parameters:**


* `ctx` Pointer to caller-allocated DRBG context. 
* `entropy` Initial entropy input buffer (must be at least 32 bytes). 
* `ent_len` Length of entropy in bytes (must be &gt;= 32). 
* `nonce` Nonce buffer (can be NULL if ent\_len includes extra entropy). 
* `nonce_len` Length of nonce in bytes. 
* `pers_str` Optional personalization string (can be NULL). 
* `pers_len` Length of personalization string in bytes. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if inputs are invalid. 





        

<hr>



### function syn\_hmac\_drbg\_reseed 

_Reseed the HMAC-DRBG with fresh entropy and optional additional input._ 
```C++
SYN_Status syn_hmac_drbg_reseed (
    SYN_HMAC_DRBG * ctx,
    const uint8_t * entropy,
    size_t ent_len,
    const uint8_t * add_input,
    size_t add_len
) 
```



Implements NIST SP 800-90A Rev 1 Section 10.1.2.4.




**Parameters:**


* `ctx` Pointer to initialized DRBG context. 
* `entropy` Fresh entropy input buffer (must be at least 32 bytes). 
* `ent_len` Length of entropy in bytes (must be &gt;= 32). 
* `add_input` Optional additional input buffer (can be NULL). 
* `add_len` Length of additional input in bytes. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if inputs or state are invalid. 





        

<hr>



### function syn\_hmac\_drbg\_set\_reseed\_interval 

_Set the maximum reseed interval for the DRBG instance._ 
```C++
void syn_hmac_drbg_set_reseed_interval (
    SYN_HMAC_DRBG * ctx,
    uint32_t interval
) 
```





**Parameters:**


* `ctx` Pointer to DRBG context. 
* `interval` Maximum number of generate calls before reseed is required (default: 10000). 




        

<hr>



### function syn\_hmac\_drbg\_wipe 

_Securely wipe internal working key, state vector, and counters._ 
```C++
void syn_hmac_drbg_wipe (
    SYN_HMAC_DRBG * ctx
) 
```





**Parameters:**


* `ctx` Pointer to DRBG context. 




        

<hr>
## Public Static Functions Documentation




### function drbg\_update 

_Internal HMAC-DRBG state update function per NIST SP 800-90A Rev 1 Section 10.1.2.2._ 
```C++
static void drbg_update (
    SYN_HMAC_DRBG * ctx,
    const uint8_t * c1,
    size_t l1,
    const uint8_t * c2,
    size_t l2,
    const uint8_t * c3,
    size_t l3
) 
```





**Parameters:**


* `ctx` Pointer to DRBG context. 
* `c1` First chunk of provided data (or NULL). 
* `l1` Length of first chunk. 
* `c2` Second chunk of provided data (or NULL). 
* `l2` Length of second chunk. 
* `c3` Third chunk of provided data (or NULL). 
* `l3` Length of third chunk. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_hmac_drbg.c`

