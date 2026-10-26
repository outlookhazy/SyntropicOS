

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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_gatt.c`

