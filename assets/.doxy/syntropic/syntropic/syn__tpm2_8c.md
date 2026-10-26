

# File syn\_tpm2.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_tpm2.c**](syn__tpm2_8c.md)

[Go to the source code of this file](syn__tpm2_8c_source.md)

_TCG TPM 2.0 Command Marshaller & Measured Boot Engine Implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "syn_tpm2.h"`
* `#include <string.h>`





































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


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**tpm2\_execute\_command**](#function-tpm2_execute_command) ([**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md) \* ctx, size\_t cmd\_len, size\_t \* out\_resp\_len) <br>_Execute TPM 2.0 command over transport and parse standard response header._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tpm2\_read\_bytes**](#function-tpm2_read_bytes) (const uint8\_t \* buf, size\_t \* pos, size\_t max\_len, uint8\_t \* out\_data, size\_t len) <br>_Read raw byte array from buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tpm2\_read\_u16**](#function-tpm2_read_u16) (const uint8\_t \* buf, size\_t \* pos, size\_t max\_len, uint16\_t \* out\_v) <br>_Read 16-bit unsigned integer from big-endian buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tpm2\_read\_u32**](#function-tpm2_read_u32) (const uint8\_t \* buf, size\_t \* pos, size\_t max\_len, uint32\_t \* out\_v) <br>_Read 32-bit unsigned integer from big-endian buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tpm2\_write\_bytes**](#function-tpm2_write_bytes) (uint8\_t \* buf, size\_t \* pos, size\_t max\_len, const uint8\_t \* data, size\_t len) <br>_Write raw byte array into buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tpm2\_write\_pw\_auth\_session**](#function-tpm2_write_pw_auth_session) (uint8\_t \* tx, size\_t \* pos, size\_t max\_len) <br>_Helper to write standard empty Password Authorization Session area._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tpm2\_write\_u16**](#function-tpm2_write_u16) (uint8\_t \* buf, size\_t \* pos, size\_t max\_len, uint16\_t v) <br>_Write 16-bit unsigned integer in big-endian network byte order._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tpm2\_write\_u32**](#function-tpm2_write_u32) (uint8\_t \* buf, size\_t \* pos, size\_t max\_len, uint32\_t v) <br>_Write 32-bit unsigned integer in big-endian network byte order._  |


























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
## Public Static Functions Documentation




### function tpm2\_execute\_command 

_Execute TPM 2.0 command over transport and parse standard response header._ 
```C++
static SYN_Status tpm2_execute_command (
    SYN_TPM2_Context * ctx,
    size_t cmd_len,
    size_t * out_resp_len
) 
```





**Parameters:**


* `ctx` Context instance. 
* `cmd_len` Total command buffer length. 
* `out_resp_len` Output response length. 



**Returns:**

SYN\_OK on TPM success, SYN\_ERROR on transport or TPM RC error. 





        

<hr>



### function tpm2\_read\_bytes 

_Read raw byte array from buffer._ 
```C++
static bool tpm2_read_bytes (
    const uint8_t * buf,
    size_t * pos,
    size_t max_len,
    uint8_t * out_data,
    size_t len
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Current offset pointer. 
* `max_len` Total available bytes. 
* `out_data` Output destination. 
* `len` Number of bytes to copy. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function tpm2\_read\_u16 

_Read 16-bit unsigned integer from big-endian buffer._ 
```C++
static bool tpm2_read_u16 (
    const uint8_t * buf,
    size_t * pos,
    size_t max_len,
    uint16_t * out_v
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Current offset pointer. 
* `max_len` Total available bytes. 
* `out_v` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function tpm2\_read\_u32 

_Read 32-bit unsigned integer from big-endian buffer._ 
```C++
static bool tpm2_read_u32 (
    const uint8_t * buf,
    size_t * pos,
    size_t max_len,
    uint32_t * out_v
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Current offset pointer. 
* `max_len` Total available bytes. 
* `out_v` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function tpm2\_write\_bytes 

_Write raw byte array into buffer._ 
```C++
static bool tpm2_write_bytes (
    uint8_t * buf,
    size_t * pos,
    size_t max_len,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Current offset pointer. 
* `max_len` Maximum buffer capacity. 
* `data` Source byte array. 
* `len` Number of bytes to copy. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function tpm2\_write\_pw\_auth\_session 

_Helper to write standard empty Password Authorization Session area._ 
```C++
static bool tpm2_write_pw_auth_session (
    uint8_t * tx,
    size_t * pos,
    size_t max_len
) 
```





**Parameters:**


* `tx` Command buffer. 
* `pos` Offset pointer. 
* `max_len` Maximum length. 



**Returns:**

true on success. 





        

<hr>



### function tpm2\_write\_u16 

_Write 16-bit unsigned integer in big-endian network byte order._ 
```C++
static bool tpm2_write_u16 (
    uint8_t * buf,
    size_t * pos,
    size_t max_len,
    uint16_t v
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Current offset pointer (updated on write). 
* `max_len` Maximum buffer capacity. 
* `v` Value to write. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function tpm2\_write\_u32 

_Write 32-bit unsigned integer in big-endian network byte order._ 
```C++
static bool tpm2_write_u32 (
    uint8_t * buf,
    size_t * pos,
    size_t max_len,
    uint32_t v
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Current offset pointer (updated on write). 
* `max_len` Maximum buffer capacity. 
* `v` Value to write. 



**Returns:**

true on success, false on overflow. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_tpm2.c`

