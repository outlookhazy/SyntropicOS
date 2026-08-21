

# File syn\_ble\_gatt.h



[**FileList**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_gatt.h**](syn__ble__gatt_8h.md)

[Go to the source code of this file](syn__ble__gatt_8h_source.md)

_Zero-Heap Generic Attribute Profile (GATT) Server Engine._ 

* `#include "../common/syn_defs.h"`
* `#include "syn_ble_att.h"`
* `#include "syn_ble_l2cap.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_BLE\_GATT\_Attr**](structSYN__BLE__GATT__Attr.md) <br> |
| struct | [**SYN\_BLE\_GATT\_s**](structSYN__BLE__GATT__s.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**SYN\_BLE\_GATT\_ReadCb**](#typedef-syn_ble_gatt_readcb)  <br> |
| typedef [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**SYN\_BLE\_GATT\_WriteCb**](#typedef-syn_ble_gatt_writecb)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_gatt\_init**](#function-syn_ble_gatt_init) (SYN\_BLE\_GATT \* gatt, const [**SYN\_BLE\_GATT\_Attr**](structSYN__BLE__GATT__Attr.md) \* table, uint16\_t table\_len, void \* user\_data) <br>_Initialize GATT Server context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_gatt\_notify**](#function-syn_ble_gatt_notify) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, uint16\_t attr\_handle, const uint8\_t \* val, uint16\_t val\_len, uint8\_t \* tx\_buf, uint16\_t \* tx\_len) <br>_Encode Outbound GATT Notification for attribute._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_gatt\_process\_att\_pdu**](#function-syn_ble_gatt_process_att_pdu) (SYN\_BLE\_GATT \* gatt, uint16\_t conn\_handle, const uint8\_t \* att\_pdu, uint16\_t pdu\_len, uint8\_t \* resp\_buf, uint16\_t \* resp\_len) <br>_Process incoming ATT PDU received over L2CAP channel 0x0004._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_BLE\_MAX\_ATTRIBUTES**](syn__ble__gatt_8h.md#define-syn_ble_max_attributes)  `32U`<br> |
| define  | [**SYN\_BLE\_PROP\_INDICATE**](syn__ble__gatt_8h.md#define-syn_ble_prop_indicate)  `0x20U`<br> |
| define  | [**SYN\_BLE\_PROP\_NOTIFY**](syn__ble__gatt_8h.md#define-syn_ble_prop_notify)  `0x10U`<br> |
| define  | [**SYN\_BLE\_PROP\_READ**](syn__ble__gatt_8h.md#define-syn_ble_prop_read)  `0x02U`<br> |
| define  | [**SYN\_BLE\_PROP\_WRITE**](syn__ble__gatt_8h.md#define-syn_ble_prop_write)  `0x08U`<br> |
| define  | [**SYN\_BLE\_PROP\_WRITE\_CMD**](syn__ble__gatt_8h.md#define-syn_ble_prop_write_cmd)  `0x04U`<br> |
| define  | [**SYN\_BLE\_UUID\_CCCD**](syn__ble__gatt_8h.md#define-syn_ble_uuid_cccd)  `0x2902U`<br> |
| define  | [**SYN\_BLE\_UUID\_CHARACTERISTIC**](syn__ble__gatt_8h.md#define-syn_ble_uuid_characteristic)  `0x2803U`<br> |
| define  | [**SYN\_BLE\_UUID\_PRIMARY\_SERVICE**](syn__ble__gatt_8h.md#define-syn_ble_uuid_primary_service)  `0x2800U`<br> |
| define  | [**SYN\_BLE\_UUID\_SECONDARY\_SERVICE**](syn__ble__gatt_8h.md#define-syn_ble_uuid_secondary_service)  `0x2801U`<br> |

## Public Types Documentation




### typedef SYN\_BLE\_GATT\_ReadCb 

```C++
typedef SYN_Status(* SYN_BLE_GATT_ReadCb) (SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t attr_handle, uint8_t *val, uint16_t *val_len, void *user_data);
```



Attribute Read Callback 


        

<hr>



### typedef SYN\_BLE\_GATT\_WriteCb 

```C++
typedef SYN_Status(* SYN_BLE_GATT_WriteCb) (SYN_BLE_GATT *gatt, uint16_t conn_handle, uint16_t attr_handle, const uint8_t *val, uint16_t val_len, void *user_data);
```



Attribute Write Callback 


        

<hr>
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
## Macro Definition Documentation





### define SYN\_BLE\_MAX\_ATTRIBUTES 

```C++
#define SYN_BLE_MAX_ATTRIBUTES `32U`
```



Maximum GATT attribute table capacity 


        

<hr>



### define SYN\_BLE\_PROP\_INDICATE 

```C++
#define SYN_BLE_PROP_INDICATE `0x20U`
```



Indicate Property 


        

<hr>



### define SYN\_BLE\_PROP\_NOTIFY 

```C++
#define SYN_BLE_PROP_NOTIFY `0x10U`
```



Notify Property 


        

<hr>



### define SYN\_BLE\_PROP\_READ 

```C++
#define SYN_BLE_PROP_READ `0x02U`
```



Read Property 


        

<hr>



### define SYN\_BLE\_PROP\_WRITE 

```C++
#define SYN_BLE_PROP_WRITE `0x08U`
```



Write Request Property 


        

<hr>



### define SYN\_BLE\_PROP\_WRITE\_CMD 

```C++
#define SYN_BLE_PROP_WRITE_CMD `0x04U`
```



Write Command (Without Response) Property 


        

<hr>



### define SYN\_BLE\_UUID\_CCCD 

```C++
#define SYN_BLE_UUID_CCCD `0x2902U`
```



Client Characteristic Configuration Descriptor UUID 


        

<hr>



### define SYN\_BLE\_UUID\_CHARACTERISTIC 

```C++
#define SYN_BLE_UUID_CHARACTERISTIC `0x2803U`
```



Characteristic Declaration UUID 


        

<hr>



### define SYN\_BLE\_UUID\_PRIMARY\_SERVICE 

```C++
#define SYN_BLE_UUID_PRIMARY_SERVICE `0x2800U`
```



Primary Service Declaration UUID 


        

<hr>



### define SYN\_BLE\_UUID\_SECONDARY\_SERVICE 

```C++
#define SYN_BLE_UUID_SECONDARY_SERVICE `0x2801U`
```



Secondary Service Declaration UUID 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_gatt.h`

