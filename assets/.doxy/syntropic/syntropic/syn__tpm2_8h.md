

# File syn\_tpm2.h



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_tpm2.h**](syn__tpm2_8h.md)

[Go to the source code of this file](syn__tpm2_8h_source.md)

_TCG TPM 2.0 Command Marshaller, Measured Boot, & Hardware Root-of-Trust Engine._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../net/syn_transport.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_TPM2\_Config**](structSYN__TPM2__Config.md) <br>_TPM 2.0 Context Configuration Descriptor._  |
| struct | [**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) <br>_TPM 2.0 Client Instance Context._  |
| struct | [**SYN\_TPM2\_QuoteResult**](structSYN__TPM2__QuoteResult.md) <br>_TPM 2.0 Attestation Quote Output Structure._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_tpm2\_get\_last\_rc**](#function-syn_tpm2_get_last_rc) (const [**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx) <br>_Get last TPM 2.0 response code returned by TPM hardware._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tpm2\_get\_random**](#function-syn_tpm2_get_random) ([**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx, uint16\_t num\_bytes, uint8\_t \* out\_random, uint16\_t \* out\_len) <br>_Generate cryptographic true random bytes from TPM TRNG (TPM2\_GetRandom)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tpm2\_init**](#function-syn_tpm2_init) ([**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx, const [**SYN\_TPM2\_Config**](structSYN__TPM2__Config.md) \* cfg) <br>_Initialize TPM 2.0 Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tpm2\_nv\_read**](#function-syn_tpm2_nv_read) ([**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx, uint32\_t auth\_handle, uint32\_t nv\_index, uint16\_t offset, uint16\_t size, uint8\_t \* out\_data, uint16\_t \* out\_len) <br>_Read data from secure Non-Volatile storage index (TPM2\_NV\_Read)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tpm2\_nv\_write**](#function-syn_tpm2_nv_write) ([**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx, uint32\_t auth\_handle, uint32\_t nv\_index, uint16\_t offset, const uint8\_t \* in\_data, uint16\_t size) <br>_Write data to secure Non-Volatile storage index (TPM2\_NV\_Write)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tpm2\_pcr\_extend**](#function-syn_tpm2_pcr_extend) ([**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx, uint32\_t pcr\_index, uint16\_t hash\_alg, const uint8\_t \* in\_digest, size\_t digest\_len) <br>_Extend Platform Configuration Register with measurement digest (TPM2\_PCR\_Extend)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tpm2\_pcr\_read**](#function-syn_tpm2_pcr_read) ([**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx, uint32\_t pcr\_index, uint16\_t hash\_alg, uint8\_t \* out\_digest, size\_t \* out\_digest\_len) <br>_Read single Platform Configuration Register (TPM2\_PCR\_Read)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tpm2\_quote**](#function-syn_tpm2_quote) ([**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx, uint32\_t key\_handle, const uint8\_t \* qualifying\_data, size\_t qual\_len, uint32\_t pcr\_mask, [**SYN\_TPM2\_QuoteResult**](structSYN__TPM2__QuoteResult.md) \* out\_quote) <br>_Generate Remote Attestation Quote over PCR values (TPM2\_Quote)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tpm2\_self\_test**](#function-syn_tpm2_self_test) ([**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx, [**bool**](syn__defs_8h.md#enum-bool) full\_test) <br>_Execute TPM2\_SelfTest command._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_tpm2\_startup**](#function-syn_tpm2_startup) ([**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx, uint16\_t startup\_type) <br>_Execute TPM2\_Startup command._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_TPM2\_ALG\_NULL**](syn__tpm2_8h.md#define-syn_tpm2_alg_null)  `0x0010U`<br> |
| define  | [**SYN\_TPM2\_ALG\_SHA256**](syn__tpm2_8h.md#define-syn_tpm2_alg_sha256)  `0x000BU`<br> |
| define  | [**SYN\_TPM2\_ALG\_SHA384**](syn__tpm2_8h.md#define-syn_tpm2_alg_sha384)  `0x000CU`<br> |
| define  | [**SYN\_TPM2\_CC\_GETRANDOM**](syn__tpm2_8h.md#define-syn_tpm2_cc_getrandom)  `0x0000017BU`<br> |
| define  | [**SYN\_TPM2\_CC\_NV\_READ**](syn__tpm2_8h.md#define-syn_tpm2_cc_nv_read)  `0x0000014EU`<br> |
| define  | [**SYN\_TPM2\_CC\_NV\_WRITE**](syn__tpm2_8h.md#define-syn_tpm2_cc_nv_write)  `0x00000137U`<br> |
| define  | [**SYN\_TPM2\_CC\_PCR\_EXTEND**](syn__tpm2_8h.md#define-syn_tpm2_cc_pcr_extend)  `0x00000182U`<br> |
| define  | [**SYN\_TPM2\_CC\_PCR\_READ**](syn__tpm2_8h.md#define-syn_tpm2_cc_pcr_read)  `0x0000017EU`<br> |
| define  | [**SYN\_TPM2\_CC\_QUOTE**](syn__tpm2_8h.md#define-syn_tpm2_cc_quote)  `0x00000158U`<br> |
| define  | [**SYN\_TPM2\_CC\_SELFTEST**](syn__tpm2_8h.md#define-syn_tpm2_cc_selftest)  `0x00000143U`<br> |
| define  | [**SYN\_TPM2\_CC\_STARTUP**](syn__tpm2_8h.md#define-syn_tpm2_cc_startup)  `0x00000144U`<br> |
| define  | [**SYN\_TPM2\_MAX\_DIGEST\_LEN**](syn__tpm2_8h.md#define-syn_tpm2_max_digest_len)  `48U`<br> |
| define  | [**SYN\_TPM2\_MAX\_QUOTE\_LEN**](syn__tpm2_8h.md#define-syn_tpm2_max_quote_len)  `256U`<br> |
| define  | [**SYN\_TPM2\_RC\_SUCCESS**](syn__tpm2_8h.md#define-syn_tpm2_rc_success)  `0x00000000U`<br> |
| define  | [**SYN\_TPM2\_RH\_OWNER**](syn__tpm2_8h.md#define-syn_tpm2_rh_owner)  `0x40000001U`<br> |
| define  | [**SYN\_TPM2\_RH\_PLATFORM**](syn__tpm2_8h.md#define-syn_tpm2_rh_platform)  `0x4000000CU`<br> |
| define  | [**SYN\_TPM2\_RS\_PW**](syn__tpm2_8h.md#define-syn_tpm2_rs_pw)  `0x40000009U`<br> |
| define  | [**SYN\_TPM2\_ST\_NO\_SESSIONS**](syn__tpm2_8h.md#define-syn_tpm2_st_no_sessions)  `0x8001U`<br> |
| define  | [**SYN\_TPM2\_ST\_SESSIONS**](syn__tpm2_8h.md#define-syn_tpm2_st_sessions)  `0x8002U`<br> |
| define  | [**SYN\_TPM2\_SU\_CLEAR**](syn__tpm2_8h.md#define-syn_tpm2_su_clear)  `0x0000U`<br> |
| define  | [**SYN\_TPM2\_SU\_STATE**](syn__tpm2_8h.md#define-syn_tpm2_su_state)  `0x0001U`<br> |

## Detailed Description


Implements a zero-heap, deterministic TCG TPM 2.0 command serializer and response parser:
* Direct interface over SPI / I2C / LPC TCG FIFO hardware interface (`SYN_Transport`).
* Measured Boot PCR Operations (`TPM2_PCR_Read`, `TPM2_PCR_Extend` for SHA-256 / SHA-384).
* Hardware Cryptographic Entropy (`TPM2_GetRandom`).
* Hardware Identity & Remote Attestation Quotes (`TPM2_Quote`).
* Tamper-Proof Non-Volatile Storage (`TPM2_NV_Read`, `TPM2_NV_Write`).
* Power & Self-Test Lifecycle (`TPM2_Startup`, `TPM2_SelfTest`). 




    
## Public Functions Documentation




### function syn\_tpm2\_get\_last\_rc 

_Get last TPM 2.0 response code returned by TPM hardware._ 
```C++
uint32_t syn_tpm2_get_last_rc (
    const SYN_TPM2_Context * ctx
) 
```





**Parameters:**


* `ctx` Context instance. 



**Returns:**

32-bit TPM response code (e.g. TPM\_RC\_SUCCESS = 0). 





        

<hr>



### function syn\_tpm2\_get\_random 

_Generate cryptographic true random bytes from TPM TRNG (TPM2\_GetRandom)._ 
```C++
SYN_Status syn_tpm2_get_random (
    SYN_TPM2_Context * ctx,
    uint16_t num_bytes,
    uint8_t * out_random,
    uint16_t * out_len
) 
```





**Parameters:**


* `ctx` Context instance. 
* `num_bytes` Number of random bytes requested. 
* `out_random` Buffer to receive random bytes. 
* `out_len` Pointer to receive actual number of bytes returned. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tpm2\_init 

_Initialize TPM 2.0 Context._ 
```C++
SYN_Status syn_tpm2_init (
    SYN_TPM2_Context * ctx,
    const SYN_TPM2_Config * cfg
) 
```





**Parameters:**


* `ctx` Context instance. 
* `cfg` Configuration descriptor. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid parameter. 





        

<hr>



### function syn\_tpm2\_nv\_read 

_Read data from secure Non-Volatile storage index (TPM2\_NV\_Read)._ 
```C++
SYN_Status syn_tpm2_nv_read (
    SYN_TPM2_Context * ctx,
    uint32_t auth_handle,
    uint32_t nv_index,
    uint16_t offset,
    uint16_t size,
    uint8_t * out_data,
    uint16_t * out_len
) 
```





**Parameters:**


* `ctx` Context instance. 
* `auth_handle` Authorization handle (e.g. SYN\_TPM2\_RH\_OWNER). 
* `nv_index` NVRAM index (e.g. 0x01500000). 
* `offset` Byte offset within NV area. 
* `size` Number of bytes to read. 
* `out_data` Output buffer. 
* `out_len` Pointer to receive bytes read. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tpm2\_nv\_write 

_Write data to secure Non-Volatile storage index (TPM2\_NV\_Write)._ 
```C++
SYN_Status syn_tpm2_nv_write (
    SYN_TPM2_Context * ctx,
    uint32_t auth_handle,
    uint32_t nv_index,
    uint16_t offset,
    const uint8_t * in_data,
    uint16_t size
) 
```





**Parameters:**


* `ctx` Context instance. 
* `auth_handle` Authorization handle (e.g. SYN\_TPM2\_RH\_OWNER). 
* `nv_index` NVRAM index. 
* `offset` Byte offset within NV area. 
* `in_data` Data buffer to write. 
* `size` Number of bytes to write. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tpm2\_pcr\_extend 

_Extend Platform Configuration Register with measurement digest (TPM2\_PCR\_Extend)._ 
```C++
SYN_Status syn_tpm2_pcr_extend (
    SYN_TPM2_Context * ctx,
    uint32_t pcr_index,
    uint16_t hash_alg,
    const uint8_t * in_digest,
    size_t digest_len
) 
```





**Parameters:**


* `ctx` Context instance. 
* `pcr_index` PCR register index (0..23). 
* `hash_alg` Hash algorithm (SYN\_TPM2\_ALG\_SHA256 or SYN\_TPM2\_ALG\_SHA384). 
* `in_digest` Digest bytes to extend PCR with. 
* `digest_len` Length of in\_digest (must match hash algorithm). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tpm2\_pcr\_read 

_Read single Platform Configuration Register (TPM2\_PCR\_Read)._ 
```C++
SYN_Status syn_tpm2_pcr_read (
    SYN_TPM2_Context * ctx,
    uint32_t pcr_index,
    uint16_t hash_alg,
    uint8_t * out_digest,
    size_t * out_digest_len
) 
```





**Parameters:**


* `ctx` Context instance. 
* `pcr_index` PCR register index (0..23). 
* `hash_alg` Hash algorithm bank (SYN\_TPM2\_ALG\_SHA256 or SYN\_TPM2\_ALG\_SHA384). 
* `out_digest` Output buffer to receive digest. 
* `out_digest_len` Pointer to receive digest length (32 for SHA-256, 48 for SHA-384). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tpm2\_quote 

_Generate Remote Attestation Quote over PCR values (TPM2\_Quote)._ 
```C++
SYN_Status syn_tpm2_quote (
    SYN_TPM2_Context * ctx,
    uint32_t key_handle,
    const uint8_t * qualifying_data,
    size_t qual_len,
    uint32_t pcr_mask,
    SYN_TPM2_QuoteResult * out_quote
) 
```





**Parameters:**


* `ctx` Context instance. 
* `key_handle` Attestation signing key handle (e.g. AK / EK). 
* `qualifying_data` Nonce / qualifying data to prevent replay. 
* `qual_len` Length of qualifying data. 
* `pcr_mask` 24-bit bitmask of PCRs to include in quote. 
* `out_quote` Pointer to receive quote structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tpm2\_self\_test 

_Execute TPM2\_SelfTest command._ 
```C++
SYN_Status syn_tpm2_self_test (
    SYN_TPM2_Context * ctx,
    bool full_test
) 
```





**Parameters:**


* `ctx` Context instance. 
* `full_test` If true, tests all algorithms; if false, tests incrementally. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_tpm2\_startup 

_Execute TPM2\_Startup command._ 
```C++
SYN_Status syn_tpm2_startup (
    SYN_TPM2_Context * ctx,
    uint16_t startup_type
) 
```





**Parameters:**


* `ctx` Context instance. 
* `startup_type` Startup mode (SYN\_TPM2\_SU\_CLEAR or SYN\_TPM2\_SU\_STATE). 



**Returns:**

SYN\_OK on success, SYN\_ERROR on TPM failure. 





        

<hr>
## Macro Definition Documentation





### define SYN\_TPM2\_ALG\_NULL 

```C++
#define SYN_TPM2_ALG_NULL `0x0010U`
```



Null algorithm 


        

<hr>



### define SYN\_TPM2\_ALG\_SHA256 

```C++
#define SYN_TPM2_ALG_SHA256 `0x000BU`
```



SHA-256 (32 bytes) 


        

<hr>



### define SYN\_TPM2\_ALG\_SHA384 

```C++
#define SYN_TPM2_ALG_SHA384 `0x000CU`
```



SHA-384 (48 bytes) 


        

<hr>



### define SYN\_TPM2\_CC\_GETRANDOM 

```C++
#define SYN_TPM2_CC_GETRANDOM `0x0000017BU`
```



TPM2\_GetRandom 


        

<hr>



### define SYN\_TPM2\_CC\_NV\_READ 

```C++
#define SYN_TPM2_CC_NV_READ `0x0000014EU`
```



TPM2\_NV\_Read 


        

<hr>



### define SYN\_TPM2\_CC\_NV\_WRITE 

```C++
#define SYN_TPM2_CC_NV_WRITE `0x00000137U`
```



TPM2\_NV\_Write 


        

<hr>



### define SYN\_TPM2\_CC\_PCR\_EXTEND 

```C++
#define SYN_TPM2_CC_PCR_EXTEND `0x00000182U`
```



TPM2\_PCR\_Extend 


        

<hr>



### define SYN\_TPM2\_CC\_PCR\_READ 

```C++
#define SYN_TPM2_CC_PCR_READ `0x0000017EU`
```



TPM2\_PCR\_Read 


        

<hr>



### define SYN\_TPM2\_CC\_QUOTE 

```C++
#define SYN_TPM2_CC_QUOTE `0x00000158U`
```



TPM2\_Quote 


        

<hr>



### define SYN\_TPM2\_CC\_SELFTEST 

```C++
#define SYN_TPM2_CC_SELFTEST `0x00000143U`
```



TPM2\_SelfTest 


        

<hr>



### define SYN\_TPM2\_CC\_STARTUP 

```C++
#define SYN_TPM2_CC_STARTUP `0x00000144U`
```



TPM2\_Startup 


        

<hr>



### define SYN\_TPM2\_MAX\_DIGEST\_LEN 

```C++
#define SYN_TPM2_MAX_DIGEST_LEN `48U`
```



Max hash digest length (SHA-384) 


        

<hr>



### define SYN\_TPM2\_MAX\_QUOTE\_LEN 

```C++
#define SYN_TPM2_MAX_QUOTE_LEN `256U`
```



Max attest quote signature length 


        

<hr>



### define SYN\_TPM2\_RC\_SUCCESS 

```C++
#define SYN_TPM2_RC_SUCCESS `0x00000000U`
```



TPM 2.0 Success Return Code 


        

<hr>



### define SYN\_TPM2\_RH\_OWNER 

```C++
#define SYN_TPM2_RH_OWNER `0x40000001U`
```



Owner hierarchy 


        

<hr>



### define SYN\_TPM2\_RH\_PLATFORM 

```C++
#define SYN_TPM2_RH_PLATFORM `0x4000000CU`
```



Platform hierarchy 


        

<hr>



### define SYN\_TPM2\_RS\_PW 

```C++
#define SYN_TPM2_RS_PW `0x40000009U`
```



Empty password authorization session 


        

<hr>



### define SYN\_TPM2\_ST\_NO\_SESSIONS 

```C++
#define SYN_TPM2_ST_NO_SESSIONS `0x8001U`
```



Command/Response Tag: No session authorization 


        

<hr>



### define SYN\_TPM2\_ST\_SESSIONS 

```C++
#define SYN_TPM2_ST_SESSIONS `0x8002U`
```



Command/Response Tag: With session authorization 


        

<hr>



### define SYN\_TPM2\_SU\_CLEAR 

```C++
#define SYN_TPM2_SU_CLEAR `0x0000U`
```



Startup clear (cold boot) 


        

<hr>



### define SYN\_TPM2\_SU\_STATE 

```C++
#define SYN_TPM2_SU_STATE `0x0001U`
```



Startup state (warm sleep resume) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_tpm2.h`

