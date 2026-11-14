

# File syn\_ble\_gatt.c



[**FileList**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_gatt.c**](syn__ble__gatt_8c.md)

[Go to the source code of this file](syn__ble__gatt_8c_source.md)

_Zero-Heap Generic Attribute Profile (GATT) Implementation._ 

* `#include "syn_ble_gatt.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_gatt\_init**](#function-syn_ble_gatt_init) (SYN\_BLE\_GATT \* gatt, const [**SYN\_BLE\_GATT\_Attr**](structSYN__BLE__GATT__Attr.md) \* table, uint16\_t table\_len, void \* user\_data) <br>_Initialize GATT Server context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_gatt\_notify**](#function-syn_ble_gatt_notify) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, uint16\_t attr\_handle, const uint8\_t \* val, uint16\_t val\_len, uint8\_t \* tx\_buf, uint16\_t \* tx\_len) <br>_Encode Outbound GATT Notification for attribute._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_gatt\_process\_att\_pdu**](#function-syn_ble_gatt_process_att_pdu) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, const uint8\_t \* att\_pdu, uint16\_t pdu\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Process incoming ATT PDU received over L2CAP channel 0x0004._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_BLE\_GATT\_Attr**](structSYN__BLE__GATT__Attr.md) \* | [**find\_attr**](#function-find_attr) (const SYN\_BLE\_GATT \* gatt, uint16\_t handle) <br>_Internal attribute lookup helper._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**gatt\_attr\_in\_range**](#function-gatt_attr_in_range) (const [**SYN\_BLE\_GATT\_Attr**](structSYN__BLE__GATT__Attr.md) \* attr, uint16\_t start\_h, uint16\_t end\_h) <br>_Check whether attribute handle falls within non-zero [start\_h, end\_h]._  |
|  uint8\_t | [**gatt\_cccd\_slot**](#function-gatt_cccd_slot) (const SYN\_BLE\_GATT \* gatt, uint16\_t handle) <br>_Resolve per-CCCD slot index for a 0x2902 descriptor handle._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**gatt\_extract\_char\_decl**](#function-gatt_extract_char_decl) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, uint16\_t idx, uint8\_t decl\_val) <br>_Build 5-byte Characteristic Declaration payload for 0x2803 row idx._  |
|  uint16\_t | [**gatt\_get\_le16**](#function-gatt_get_le16) (const uint8\_t \* p) <br>_Decode little-endian 16-bit integer from buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**gatt\_handle\_find\_info**](#function-gatt_handle_find_info) (const SYN\_BLE\_GATT \* gatt, const uint8\_t \* att\_pdu, uint16\_t pdu\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle ATT Find Information Request (0x04) for handles and CCCD (0x2902)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**gatt\_handle\_read\_by\_group**](#function-gatt_handle_read_by_group) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, const uint8\_t \* att\_pdu, uint16\_t pdu\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle ATT Read By Group Type Request (0x10) for Primary/Secondary Services._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**gatt\_handle\_read\_by\_type**](#function-gatt_handle_read_by_type) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, const uint8\_t \* att\_pdu, uint16\_t pdu\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle ATT Read By Type Request (0x08)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**gatt\_handle\_read\_req**](#function-gatt_handle_read_req) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, const uint8\_t \* att\_pdu, uint16\_t pdu\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle ATT Read Request (0x0A)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**gatt\_handle\_write\_cmd**](#function-gatt_handle_write_cmd) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, const uint8\_t \* att\_pdu, uint16\_t pdu\_len, uint16\_t \* resp\_len) <br>_Handle ATT Write Command (0x52) without response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**gatt\_handle\_write\_req**](#function-gatt_handle_write_req) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, const uint8\_t \* att\_pdu, uint16\_t pdu\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle ATT Write Request (0x12)._  |
|  void | [**gatt\_put\_le16**](#function-gatt_put_le16) (uint8\_t \* p, uint16\_t v) <br>_Encode little-endian 16-bit integer into buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**gatt\_read\_attr\_val**](#function-gatt_read_attr_val) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, const [**SYN\_BLE\_GATT\_Attr**](structSYN__BLE__GATT__Attr.md) \* attr, uint8\_t \* val, uint16\_t max\_len, uint16\_t \* val\_len) <br>_Read raw attribute value via callback, static buffer, or CCCD state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**gatt\_read\_char\_decls**](#function-gatt_read_char_decls) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, uint16\_t start\_h, uint16\_t end\_h, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Handle Characteristic Declaration (0x2803) Read By Type discovery._  |
|  uint16\_t | [**gatt\_service\_end\_handle**](#function-gatt_service_end_handle) (const SYN\_BLE\_GATT \* gatt, uint16\_t svc\_idx) <br>_Compute end group handle for a service attribute at table index svc\_idx._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**gatt\_validate\_range**](#function-gatt_validate_range) (uint8\_t opcode, const uint8\_t \* att\_pdu, uint16\_t pdu\_len, uint16\_t expected\_len, [**bool**](syn__defs_8h.md#enum-bool) allow\_uuid128, uint16\_t \* start\_h, uint16\_t \* end\_h, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Validate common handle range header for ATT discovery requests._  |


























## Public Functions Documentation




### function syn\_ble\_gatt\_init 

_Initialize GATT Server context._ 
```C++
SYN_Status syn_ble_gatt_init (
    SYN_BLE_GATT * gatt,
    const SYN_BLE_GATT_Attr * table,
    uint16_t table_len,
    void * user_data
) 
```





**Parameters:**


* `gatt` Context handle pointer. 
* `table` Attribute table array pointer. 
* `table_len` Count of attribute entries. 
* `user_data` User context pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_gatt\_notify 

_Encode Outbound GATT Notification for attribute._ 
```C++
SYN_Status syn_ble_gatt_notify (
    SYN_BLE_GATT * gatt,
    uint16_t conn_handle,
    uint16_t attr_handle,
    const uint8_t * val,
    uint16_t val_len,
    uint8_t * tx_buf,
    uint16_t * tx_len
) 
```





**Parameters:**


* `gatt` Context handle pointer. 
* `conn_handle` Target connection handle. 
* `attr_handle` Target characteristic value attribute handle. 
* `val` Value buffer pointer. 
* `val_len` Value byte length. 
* `tx_buf` Output buffer for HCI ACL payload. 
* `tx_len` Output size pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_gatt\_process\_att\_pdu 

_Process incoming ATT PDU received over L2CAP channel 0x0004._ 
```C++
SYN_Status syn_ble_gatt_process_att_pdu (
    SYN_BLE_GATT * gatt,
    uint16_t conn_handle,
    const uint8_t * att_pdu,
    uint16_t pdu_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `gatt` Context handle pointer. 
* `conn_handle` Connection handle. 
* `att_pdu` Received ATT PDU buffer. 
* `pdu_len` PDU length. 
* `resp_buf` Buffer to store outbound ATT Response PDU. 
* `resp_len` Pointer to store response size. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function find\_attr 

_Internal attribute lookup helper._ 
```C++
static const SYN_BLE_GATT_Attr * find_attr (
    const SYN_BLE_GATT * gatt,
    uint16_t handle
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `handle` Attribute handle. 



**Returns:**

Attribute pointer or NULL. 





        

<hr>



### function gatt\_attr\_in\_range 

_Check whether attribute handle falls within non-zero [start\_h, end\_h]._ 
```C++
static bool gatt_attr_in_range (
    const SYN_BLE_GATT_Attr * attr,
    uint16_t start_h,
    uint16_t end_h
) 
```





**Parameters:**


* `attr` Attribute descriptor. 
* `start_h` Inclusive start handle. 
* `end_h` Inclusive end handle. 



**Returns:**

true if attribute handle is non-zero and within range, false otherwise. 





        

<hr>



### function gatt\_cccd\_slot 

_Resolve per-CCCD slot index for a 0x2902 descriptor handle._ 
```C++
static uint8_t gatt_cccd_slot (
    const SYN_BLE_GATT * gatt,
    uint16_t handle
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `handle` CCCD attribute handle. 



**Returns:**

CCCD slot index in [0, SYN\_BLE\_MAX\_CCCDS - 1]. 





        

<hr>



### function gatt\_extract\_char\_decl 

_Build 5-byte Characteristic Declaration payload for 0x2803 row idx._ 
```C++
static bool gatt_extract_char_decl (
    SYN_BLE_GATT * gatt,
    uint16_t conn_handle,
    uint16_t idx,
    uint8_t decl_val
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `conn_handle` Connection handle. 
* `idx` Attribute table row index. 
* `decl_val` Output 5-byte Characteristic Declaration value buffer. 



**Returns:**

true if row idx is a valid 0x2803 declaration with decl\_handle &lt; val\_handle. 





        

<hr>



### function gatt\_get\_le16 

_Decode little-endian 16-bit integer from buffer._ 
```C++
static uint16_t gatt_get_le16 (
    const uint8_t * p
) 
```





**Parameters:**


* `p` 2-byte pointer. 



**Returns:**

Decoded uint16\_t. 





        

<hr>



### function gatt\_handle\_find\_info 

_Handle ATT Find Information Request (0x04) for handles and CCCD (0x2902)._ 
```C++
static SYN_Status gatt_handle_find_info (
    const SYN_BLE_GATT * gatt,
    const uint8_t * att_pdu,
    uint16_t pdu_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `att_pdu` Incoming ATT request PDU. 
* `pdu_len` Length of att\_pdu in bytes. 
* `resp_buf` Output ATT response buffer. 
* `resp_len` Output ATT response byte length. 



**Returns:**

SYN\_OK on encoded response or error code on invalid parameters. 





        

<hr>



### function gatt\_handle\_read\_by\_group 

_Handle ATT Read By Group Type Request (0x10) for Primary/Secondary Services._ 
```C++
static SYN_Status gatt_handle_read_by_group (
    SYN_BLE_GATT * gatt,
    uint16_t conn_handle,
    const uint8_t * att_pdu,
    uint16_t pdu_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `conn_handle` Connection handle. 
* `att_pdu` Incoming ATT request PDU. 
* `pdu_len` Length of att\_pdu in bytes. 
* `resp_buf` Output ATT response buffer. 
* `resp_len` Output ATT response byte length. 



**Returns:**

SYN\_OK on encoded response or error code on invalid parameters. 





        

<hr>



### function gatt\_handle\_read\_by\_type 

_Handle ATT Read By Type Request (0x08)._ 
```C++
static SYN_Status gatt_handle_read_by_type (
    SYN_BLE_GATT * gatt,
    uint16_t conn_handle,
    const uint8_t * att_pdu,
    uint16_t pdu_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `conn_handle` Connection handle. 
* `att_pdu` Incoming ATT request PDU. 
* `pdu_len` Length of att\_pdu in bytes. 
* `resp_buf` Output ATT response buffer. 
* `resp_len` Output ATT response byte length. 



**Returns:**

SYN\_OK on encoded response or error code on invalid parameters. 





        

<hr>



### function gatt\_handle\_read\_req 

_Handle ATT Read Request (0x0A)._ 
```C++
static SYN_Status gatt_handle_read_req (
    SYN_BLE_GATT * gatt,
    uint16_t conn_handle,
    const uint8_t * att_pdu,
    uint16_t pdu_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `conn_handle` Connection handle. 
* `att_pdu` Incoming ATT request PDU. 
* `pdu_len` Length of att\_pdu in bytes. 
* `resp_buf` Output ATT response buffer. 
* `resp_len` Output ATT response byte length. 



**Returns:**

SYN\_OK on encoded response or error code on invalid parameters. 





        

<hr>



### function gatt\_handle\_write\_cmd 

_Handle ATT Write Command (0x52) without response._ 
```C++
static SYN_Status gatt_handle_write_cmd (
    SYN_BLE_GATT * gatt,
    uint16_t conn_handle,
    const uint8_t * att_pdu,
    uint16_t pdu_len,
    uint16_t * resp_len
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `conn_handle` Connection handle. 
* `att_pdu` Incoming ATT command PDU. 
* `pdu_len` Length of att\_pdu in bytes. 
* `resp_len` Output response byte length (set to 0). 



**Returns:**

SYN\_OK. 





        

<hr>



### function gatt\_handle\_write\_req 

_Handle ATT Write Request (0x12)._ 
```C++
static SYN_Status gatt_handle_write_req (
    SYN_BLE_GATT * gatt,
    uint16_t conn_handle,
    const uint8_t * att_pdu,
    uint16_t pdu_len,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `conn_handle` Connection handle. 
* `att_pdu` Incoming ATT request PDU. 
* `pdu_len` Length of att\_pdu in bytes. 
* `resp_buf` Output ATT response buffer. 
* `resp_len` Output ATT response byte length. 



**Returns:**

SYN\_OK on encoded response or error code on invalid parameters. 





        

<hr>



### function gatt\_put\_le16 

_Encode little-endian 16-bit integer into buffer._ 
```C++
static void gatt_put_le16 (
    uint8_t * p,
    uint16_t v
) 
```





**Parameters:**


* `p` 2-byte output pointer. 
* `v` Value to encode. 




        

<hr>



### function gatt\_read\_attr\_val 

_Read raw attribute value via callback, static buffer, or CCCD state._ 
```C++
static SYN_Status gatt_read_attr_val (
    SYN_BLE_GATT * gatt,
    uint16_t conn_handle,
    const SYN_BLE_GATT_Attr * attr,
    uint8_t * val,
    uint16_t max_len,
    uint16_t * val_len
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `conn_handle` Connection handle. 
* `attr` Attribute descriptor. 
* `val` Output value buffer (must be at least SYN\_BLE\_GATT\_MAX\_ATTR\_LEN bytes). 
* `max_len` Capacity of val buffer. 
* `val_len` Output value byte length. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if no value source, or callback error. 





        

<hr>



### function gatt\_read\_char\_decls 

_Handle Characteristic Declaration (0x2803) Read By Type discovery._ 
```C++
static SYN_Status gatt_read_char_decls (
    SYN_BLE_GATT * gatt,
    uint16_t conn_handle,
    uint16_t start_h,
    uint16_t end_h,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `conn_handle` Connection handle. 
* `start_h` Inclusive start handle. 
* `end_h` Inclusive end handle. 
* `resp_buf` Output ATT response buffer. 
* `resp_len` Output ATT response byte length. 



**Returns:**

SYN\_OK on encoded response or error code on invalid parameters. 





        

<hr>



### function gatt\_service\_end\_handle 

_Compute end group handle for a service attribute at table index svc\_idx._ 
```C++
static uint16_t gatt_service_end_handle (
    const SYN_BLE_GATT * gatt,
    uint16_t svc_idx
) 
```





**Parameters:**


* `gatt` GATT server handle. 
* `svc_idx` Service attribute index in gatt-&gt;table. 



**Returns:**

Last attribute handle belonging to the service group. 





        

<hr>



### function gatt\_validate\_range 

_Validate common handle range header for ATT discovery requests._ 
```C++
static bool gatt_validate_range (
    uint8_t opcode,
    const uint8_t * att_pdu,
    uint16_t pdu_len,
    uint16_t expected_len,
    bool allow_uuid128,
    uint16_t * start_h,
    uint16_t * end_h,
    uint8_t * resp_buf,
    uint16_t * resp_len
) 
```





**Parameters:**


* `opcode` ATT request opcode for error responses. 
* `att_pdu` Incoming ATT request PDU. 
* `pdu_len` Actual PDU byte length. 
* `expected_len` Expected 16-bit UUID PDU byte length. 
* `allow_uuid128` Allow 21-byte 128-bit UUID discovery requests. 
* `start_h` Output decoded start handle. 
* `end_h` Output decoded end handle. 
* `resp_buf` Output ATT error response buffer on validation failure. 
* `resp_len` Output ATT error response byte length on validation failure. 



**Returns:**

true if PDU length and handle range are valid, false otherwise. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_gatt.c`

