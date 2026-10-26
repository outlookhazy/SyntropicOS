

# File syn\_ota.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_ota.h**](syn__ota_8h.md)

[Go to the source code of this file](syn__ota_8h_source.md)

_Secure Streaming OTA Orchestrator — zero-heap, transport-agnostic._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "syn_fwboot.h"`
* `#include "syn_fwimage.h"`
* `#include "syn_fwupdate.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) <br>_Zero-heap Streaming OTA Manager context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_OTA\_CryptoMode**](#enum-syn_ota_cryptomode)  <br>_Cryptographic verification mode for the OTA image._  |
| enum  | [**SYN\_OTA\_ErrorCode**](#enum-syn_ota_errorcode)  <br>_OTA Error Codes._  |
| enum  | [**SYN\_OTA\_State**](#enum-syn_ota_state)  <br>_OTA update lifecycle states._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_ota\_abort**](#function-syn_ota_abort) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr, [**SYN\_OTA\_ErrorCode**](syn__ota_8h.md#enum-syn_ota_errorcode) err) <br>_Abort an active or failed OTA session._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ota\_apply**](#function-syn_ota_apply) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr) <br>_Mark the verified slot as ready for immediate boot on next system restart._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ota\_begin**](#function-syn_ota_begin) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr, uint32\_t expected\_total\_sz, uint32\_t target\_version, uint32\_t expected\_crc) <br>_Begin a new OTA firmware update session._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ota\_bind\_lwm2m**](#function-syn_ota_bind_lwm2m) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr, void \* lwm2m\_fw\_ctx) <br>_Bind an OMA LwM2M Object 5 (Firmware Update) context to the OTA manager._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ota\_finish**](#function-syn_ota_finish) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr, const uint8\_t \* expected\_sig\_or\_tag, size\_t sig\_len) <br>_Finalize and verify the downloaded firmware image._  |
|  [**SYN\_OTA\_ErrorCode**](syn__ota_8h.md#enum-syn_ota_errorcode) | [**syn\_ota\_get\_last\_error**](#function-syn_ota_get_last_error) (const [**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr) <br>_Get last recorded error code._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ota\_get\_progress**](#function-syn_ota_get_progress) (const [**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr, uint32\_t \* out\_written, uint32\_t \* out\_total, uint8\_t \* out\_percent) <br>_Query current OTA download and flash write progress._  |
|  [**SYN\_OTA\_State**](syn__ota_8h.md#enum-syn_ota_state) | [**syn\_ota\_get\_state**](#function-syn_ota_get_state) (const [**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr) <br>_Get current OTA lifecycle state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ota\_init**](#function-syn_ota_init) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr, [**SYN\_FwBootManager**](structSYN__FwBootManager.md) \* boot\_mgr, uint32\_t slot\_size, uint8\_t \* page\_buf, size\_t page\_buf\_sz) <br>_Initialize the OTA manager._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ota\_set\_aes\_gcm\_params**](#function-syn_ota_set_aes_gcm_params) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr, const uint8\_t \* key, size\_t key\_len, const uint8\_t \* iv, size\_t iv\_len) <br>_Configure AES-GCM decryption key and IV for encrypted updates._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ota\_set\_target\_slot**](#function-syn_ota_set_target_slot) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr, uint8\_t slot\_idx) <br>_Configure target flash slot (explicit slot index or automatic selection)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ota\_set\_verification\_key**](#function-syn_ota_set_verification_key) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr, [**SYN\_OTA\_CryptoMode**](syn__ota_8h.md#enum-syn_ota_cryptomode) mode, const uint8\_t \* key, size\_t key\_len) <br>_Configure cryptographic verification key._  |
|  void | [**syn\_ota\_sync\_lwm2m**](#function-syn_ota_sync_lwm2m) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr) <br>_Synchronize OTA state and results into bound LwM2M Object 5 context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ota\_write\_chunk**](#function-syn_ota_write_chunk) ([**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md) \* mgr, const uint8\_t \* chunk, size\_t chunk\_sz) <br>_Write a chunk of incoming firmware stream into the target flash slot._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_OTA\_SLOT\_AUTO**](syn__ota_8h.md#define-syn_ota_slot_auto)  `0xFFU`<br>_Automatic slot selection constant._  |

## Detailed Description


Coordinates streaming firmware updates across heterogeneous transports (CoAP Block2, HTTP, BLE, LwM2M Object 5, UART/YMODEM) directly into flash.


Integrates dual-bank flash slot selection (syn\_fwboot), streaming page flushing and flash sector management (syn\_fwupdate), cryptographic verification (CRC-32, HMAC-SHA256, Ed25519, AES-GCM), and LwM2M Object 5 lifecycle state sync.




**
**


```C++
static uint8_t page_buf[256];
SYN_FwBootManager boot_mgr;
SYN_OTA_Manager ota;

syn_fwboot_init(&boot_mgr, SLOT_A_ADDR, SLOT_B_ADDR);
syn_ota_init(&ota, &boot_mgr, SLOT_SIZE, page_buf, sizeof(page_buf));

// Optional: configure cryptographic verification (e.g. Ed25519)
syn_ota_set_verification_key(&ota, SYN_OTA_CRYPTO_ED25519, pubkey, 32U);

// Begin OTA session
syn_ota_begin(&ota, fw_total_len, new_version_code, expected_crc32);

// Stream incoming chunks from transport
while (receiving_chunks) {
    syn_ota_write_chunk(&ota, chunk_data, chunk_len);
}

// Finish and verify signature
if (syn_ota_finish(&ota, expected_signature, 64U) == SYN_OK) {
    syn_ota_apply(&ota); // Ready to reboot into new image
}
```
 





    
## Public Types Documentation




### enum SYN\_OTA\_CryptoMode 

_Cryptographic verification mode for the OTA image._ 
```C++
enum SYN_OTA_CryptoMode {
    SYN_OTA_CRYPTO_NONE = 0,
    SYN_OTA_CRYPTO_HMAC_SHA256,
    SYN_OTA_CRYPTO_ED25519,
    SYN_OTA_CRYPTO_AES_GCM
};
```




<hr>



### enum SYN\_OTA\_ErrorCode 

_OTA Error Codes._ 
```C++
enum SYN_OTA_ErrorCode {
    SYN_OTA_ERR_NONE = 0,
    SYN_OTA_ERR_INVALID_PARAM,
    SYN_OTA_ERR_NO_FLASH_SLOT,
    SYN_OTA_ERR_FLASH_ERASE,
    SYN_OTA_ERR_FLASH_WRITE,
    SYN_OTA_ERR_OUT_OF_SPACE,
    SYN_OTA_ERR_INTEGRITY_CHECK,
    SYN_OTA_ERR_UNSUPPORTED_CRYPTO,
    SYN_OTA_ERR_INVALID_STATE
};
```




<hr>



### enum SYN\_OTA\_State 

_OTA update lifecycle states._ 
```C++
enum SYN_OTA_State {
    SYN_OTA_STATE_IDLE = 0,
    SYN_OTA_STATE_DOWNLOADING,
    SYN_OTA_STATE_DOWNLOADED,
    SYN_OTA_STATE_VERIFYING,
    SYN_OTA_STATE_READY_TO_APPLY,
    SYN_OTA_STATE_APPLIED,
    SYN_OTA_STATE_ERROR
};
```




<hr>
## Public Functions Documentation




### function syn\_ota\_abort 

_Abort an active or failed OTA session._ 
```C++
void syn_ota_abort (
    SYN_OTA_Manager * mgr,
    SYN_OTA_ErrorCode err
) 
```



Invalidate the target flash slot header and resets state to SYN\_OTA\_STATE\_IDLE or ERROR.




**Parameters:**


* `mgr` OTA manager instance. 
* `err` Error code triggering the abort. 




        

<hr>



### function syn\_ota\_apply 

_Mark the verified slot as ready for immediate boot on next system restart._ 
```C++
SYN_Status syn_ota_apply (
    SYN_OTA_Manager * mgr
) 
```



Promotes OTA state to SYN\_OTA\_STATE\_APPLIED and synchronizes LwM2M Object 5.




**Parameters:**


* `mgr` OTA manager instance. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if not in READY\_TO\_APPLY state. 





        

<hr>



### function syn\_ota\_begin 

_Begin a new OTA firmware update session._ 
```C++
SYN_Status syn_ota_begin (
    SYN_OTA_Manager * mgr,
    uint32_t expected_total_sz,
    uint32_t target_version,
    uint32_t expected_crc
) 
```



Resolves target slot, erases initial flash sector, primes running digests, and enters SYN\_OTA\_STATE\_DOWNLOADING.




**Parameters:**


* `mgr` OTA manager instance. 
* `expected_total_sz` Total expected firmware binary size (excl header). 
* `target_version` Version code for the incoming firmware. 
* `expected_crc` Expected CRC-32 checksum (0 to compute dynamically). 



**Returns:**

SYN\_OK on success, error code on flash erase or configuration failure. 





        

<hr>



### function syn\_ota\_bind\_lwm2m 

_Bind an OMA LwM2M Object 5 (Firmware Update) context to the OTA manager._ 
```C++
SYN_Status syn_ota_bind_lwm2m (
    SYN_OTA_Manager * mgr,
    void * lwm2m_fw_ctx
) 
```



Allows automatic bidirectional synchronization between OTA progress/states and LwM2M.




**Parameters:**


* `mgr` OTA manager instance. 
* `lwm2m_fw_ctx` Pointer to [**SYN\_LwM2M\_FirmwareContext**](structSYN__LwM2M__FirmwareContext.md) instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ota\_finish 

_Finalize and verify the downloaded firmware image._ 
```C++
SYN_Status syn_ota_finish (
    SYN_OTA_Manager * mgr,
    const uint8_t * expected_sig_or_tag,
    size_t sig_len
) 
```



Flushes page buffers, verifies CRC-32 and cryptographic signature / HMAC / GCM tag, and writes the new image header to mark the slot as SYN\_FW\_STATE\_NEW.




**Parameters:**


* `mgr` OTA manager instance. 
* `expected_sig_or_tag` Expected signature, HMAC, or GCM tag (NULL if CRC-32 only). 
* `sig_len` Length of expected signature / tag buffer. 



**Returns:**

SYN\_OK on successful verification, SYN\_ERROR on verification failure. 





        

<hr>



### function syn\_ota\_get\_last\_error 

_Get last recorded error code._ 
```C++
SYN_OTA_ErrorCode syn_ota_get_last_error (
    const SYN_OTA_Manager * mgr
) 
```





**Parameters:**


* `mgr` OTA manager instance. 



**Returns:**

Last SYN\_OTA\_ErrorCode. 





        

<hr>



### function syn\_ota\_get\_progress 

_Query current OTA download and flash write progress._ 
```C++
SYN_Status syn_ota_get_progress (
    const SYN_OTA_Manager * mgr,
    uint32_t * out_written,
    uint32_t * out_total,
    uint8_t * out_percent
) 
```





**Parameters:**


* `mgr` OTA manager instance. 
* `out_written` [out] Bytes written so far (optional, may be NULL). 
* `out_total` [out] Total expected bytes (optional, may be NULL). 
* `out_percent` [out] Integer percentage 0..100 (optional, may be NULL). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL mgr. 





        

<hr>



### function syn\_ota\_get\_state 

_Get current OTA lifecycle state._ 
```C++
SYN_OTA_State syn_ota_get_state (
    const SYN_OTA_Manager * mgr
) 
```





**Parameters:**


* `mgr` OTA manager instance. 



**Returns:**

Current SYN\_OTA\_State. 





        

<hr>



### function syn\_ota\_init 

_Initialize the OTA manager._ 
```C++
SYN_Status syn_ota_init (
    SYN_OTA_Manager * mgr,
    SYN_FwBootManager * boot_mgr,
    uint32_t slot_size,
    uint8_t * page_buf,
    size_t page_buf_sz
) 
```





**Parameters:**


* `mgr` OTA manager instance to initialize. 
* `boot_mgr` Dual-bank boot manager instance. 
* `slot_size` Maximum firmware capacity per slot in bytes. 
* `page_buf` Caller-provided page-aligned flash write buffer. 
* `page_buf_sz` Size of page buffer (must match flash write granularity). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL or invalid inputs. 





        

<hr>



### function syn\_ota\_set\_aes\_gcm\_params 

_Configure AES-GCM decryption key and IV for encrypted updates._ 
```C++
SYN_Status syn_ota_set_aes_gcm_params (
    SYN_OTA_Manager * mgr,
    const uint8_t * key,
    size_t key_len,
    const uint8_t * iv,
    size_t iv_len
) 
```





**Parameters:**


* `mgr` OTA manager instance. 
* `key` AES key (16, 24, or 32 bytes). 
* `key_len` Key length in bytes. 
* `iv` 12-byte initialization vector / nonce. 
* `iv_len` Length of IV (must be 12). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR if AES-GCM disabled. 





        

<hr>



### function syn\_ota\_set\_target\_slot 

_Configure target flash slot (explicit slot index or automatic selection)._ 
```C++
SYN_Status syn_ota_set_target_slot (
    SYN_OTA_Manager * mgr,
    uint8_t slot_idx
) 
```



If set to SYN\_OTA\_SLOT\_AUTO, [**syn\_ota\_begin()**](syn__ota_8h.md#function-syn_ota_begin) will automatically pick the currently inactive slot based on boot\_mgr-&gt;active\_slot.




**Parameters:**


* `mgr` OTA manager instance. 
* `slot_idx` SYN\_FW\_SLOT\_A (0), SYN\_FW\_SLOT\_B (1), or SYN\_OTA\_SLOT\_AUTO (0xFF). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid slot index. 





        

<hr>



### function syn\_ota\_set\_verification\_key 

_Configure cryptographic verification key._ 
```C++
SYN_Status syn_ota_set_verification_key (
    SYN_OTA_Manager * mgr,
    SYN_OTA_CryptoMode mode,
    const uint8_t * key,
    size_t key_len
) 
```





**Parameters:**


* `mgr` OTA manager instance. 
* `mode` Cryptographic verification mode. 
* `key` Secret key or public key buffer. 
* `key_len` Length of key buffer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on unsupported mode. 





        

<hr>



### function syn\_ota\_sync\_lwm2m 

_Synchronize OTA state and results into bound LwM2M Object 5 context._ 
```C++
void syn_ota_sync_lwm2m (
    SYN_OTA_Manager * mgr
) 
```





**Parameters:**


* `mgr` OTA manager instance. 




        

<hr>



### function syn\_ota\_write\_chunk 

_Write a chunk of incoming firmware stream into the target flash slot._ 
```C++
SYN_Status syn_ota_write_chunk (
    SYN_OTA_Manager * mgr,
    const uint8_t * chunk,
    size_t chunk_sz
) 
```





**Parameters:**


* `mgr` OTA manager instance. 
* `chunk` Incoming data chunk buffer. 
* `chunk_sz` Length of chunk in bytes. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on flash write failure, overflow, or invalid state. 





        

<hr>
## Macro Definition Documentation





### define SYN\_OTA\_SLOT\_AUTO 

_Automatic slot selection constant._ 
```C++
#define SYN_OTA_SLOT_AUTO `0xFFU`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_ota.h`

