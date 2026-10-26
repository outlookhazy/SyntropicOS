

# File syn\_ble\_att.h



[**FileList**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_att.h**](syn__ble__att_8h.md)

[Go to the source code of this file](syn__ble__att_8h_source.md)

_Zero-Heap Attribute Protocol (ATT) Encoding and Decoding._ 

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_att\_encode\_error\_rsp**](#function-syn_ble_att_encode_error_rsp) (uint8\_t req\_opcode, uint16\_t handle, uint8\_t err\_code, uint8\_t \* buf, uint16\_t \* len) <br>_Encode ATT Error Response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_att\_encode\_mtu\_rsp**](#function-syn_ble_att_encode_mtu_rsp) (uint16\_t server\_mtu, uint8\_t \* buf, uint16\_t \* len) <br>_Encode ATT Exchange MTU Response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_att\_encode\_notification**](#function-syn_ble_att_encode_notification) (uint16\_t handle, const uint8\_t \* val, uint16\_t val\_len, uint8\_t \* buf, uint16\_t \* len) <br>_Encode ATT Handle Value Notification._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_att\_encode\_read\_rsp**](#function-syn_ble_att_encode_read_rsp) (const uint8\_t \* val, uint16\_t val\_len, uint8\_t \* buf, uint16\_t \* len) <br>_Encode ATT Read Response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_att\_encode\_write\_rsp**](#function-syn_ble_att_encode_write_rsp) (uint8\_t \* buf, uint16\_t \* len) <br>_Encode ATT Write Response._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_BLE\_ATT\_ERR\_ATTR\_NOT\_FOUND**](syn__ble__att_8h.md#define-syn_ble_att_err_attr_not_found)  `0x0AU`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_ATTR\_NOT\_LONG**](syn__ble__att_8h.md#define-syn_ble_att_err_attr_not_long)  `0x0BU`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_INSUFFICIENT\_AUTH**](syn__ble__att_8h.md#define-syn_ble_att_err_insufficient_auth)  `0x05U`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_INSUFFICIENT\_AUTHOR**](syn__ble__att_8h.md#define-syn_ble_att_err_insufficient_author)  `0x08U`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_INSUFFICIENT\_ENCRYPT**](syn__ble__att_8h.md#define-syn_ble_att_err_insufficient_encrypt)  `0x0FU`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_INSUFFICIENT\_KEY\_SIZE**](syn__ble__att_8h.md#define-syn_ble_att_err_insufficient_key_size)  `0x0CU`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_INSUFFICIENT\_RESOURCES**](syn__ble__att_8h.md#define-syn_ble_att_err_insufficient_resources)  `0x11U`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_INVALID\_ATTR\_LEN**](syn__ble__att_8h.md#define-syn_ble_att_err_invalid_attr_len)  `0x0DU`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_INVALID\_HANDLE**](syn__ble__att_8h.md#define-syn_ble_att_err_invalid_handle)  `0x01U`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_INVALID\_OFFSET**](syn__ble__att_8h.md#define-syn_ble_att_err_invalid_offset)  `0x07U`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_INVALID\_PDU**](syn__ble__att_8h.md#define-syn_ble_att_err_invalid_pdu)  `0x04U`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_PREPARE\_QUEUE\_FULL**](syn__ble__att_8h.md#define-syn_ble_att_err_prepare_queue_full)  `0x09U`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_READ\_NOT\_PERMITTED**](syn__ble__att_8h.md#define-syn_ble_att_err_read_not_permitted)  `0x02U`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_REQ\_NOT\_SUPPORTED**](syn__ble__att_8h.md#define-syn_ble_att_err_req_not_supported)  `0x06U`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_UNLIKELY\_ERROR**](syn__ble__att_8h.md#define-syn_ble_att_err_unlikely_error)  `0x0EU`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_UNSUPPORTED\_GROUP\_TYPE**](syn__ble__att_8h.md#define-syn_ble_att_err_unsupported_group_type)  `0x10U`<br> |
| define  | [**SYN\_BLE\_ATT\_ERR\_WRITE\_NOT\_PERMITTED**](syn__ble__att_8h.md#define-syn_ble_att_err_write_not_permitted)  `0x03U`<br> |
| define  | [**SYN\_BLE\_ATT\_MTU\_DEFAULT**](syn__ble__att_8h.md#define-syn_ble_att_mtu_default)  `23U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_ERROR\_RSP**](syn__ble__att_8h.md#define-syn_ble_att_op_error_rsp)  `0x01U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_EXCHANGE\_MTU\_REQ**](syn__ble__att_8h.md#define-syn_ble_att_op_exchange_mtu_req)  `0x02U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_EXCHANGE\_MTU\_RSP**](syn__ble__att_8h.md#define-syn_ble_att_op_exchange_mtu_rsp)  `0x03U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_FIND\_BY\_TYPE\_VALUE\_REQ**](syn__ble__att_8h.md#define-syn_ble_att_op_find_by_type_value_req)  `0x06U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_FIND\_BY\_TYPE\_VALUE\_RSP**](syn__ble__att_8h.md#define-syn_ble_att_op_find_by_type_value_rsp)  `0x07U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_FIND\_INFO\_REQ**](syn__ble__att_8h.md#define-syn_ble_att_op_find_info_req)  `0x04U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_FIND\_INFO\_RSP**](syn__ble__att_8h.md#define-syn_ble_att_op_find_info_rsp)  `0x05U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_HANDLE\_VAL\_CONF**](syn__ble__att_8h.md#define-syn_ble_att_op_handle_val_conf)  `0x1EU`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_HANDLE\_VAL\_IND**](syn__ble__att_8h.md#define-syn_ble_att_op_handle_val_ind)  `0x1DU`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_HANDLE\_VAL\_NOTIF**](syn__ble__att_8h.md#define-syn_ble_att_op_handle_val_notif)  `0x1BU`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_READ\_BLOB\_REQ**](syn__ble__att_8h.md#define-syn_ble_att_op_read_blob_req)  `0x0CU`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_READ\_BLOB\_RSP**](syn__ble__att_8h.md#define-syn_ble_att_op_read_blob_rsp)  `0x0DU`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_READ\_BY\_GROUP\_TYPE\_REQ**](syn__ble__att_8h.md#define-syn_ble_att_op_read_by_group_type_req)  `0x10U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_READ\_BY\_GROUP\_TYPE\_RSP**](syn__ble__att_8h.md#define-syn_ble_att_op_read_by_group_type_rsp)  `0x11U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_READ\_BY\_TYPE\_REQ**](syn__ble__att_8h.md#define-syn_ble_att_op_read_by_type_req)  `0x08U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_READ\_BY\_TYPE\_RSP**](syn__ble__att_8h.md#define-syn_ble_att_op_read_by_type_rsp)  `0x09U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_READ\_REQ**](syn__ble__att_8h.md#define-syn_ble_att_op_read_req)  `0x0AU`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_READ\_RSP**](syn__ble__att_8h.md#define-syn_ble_att_op_read_rsp)  `0x0BU`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_WRITE\_CMD**](syn__ble__att_8h.md#define-syn_ble_att_op_write_cmd)  `0x52U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_WRITE\_REQ**](syn__ble__att_8h.md#define-syn_ble_att_op_write_req)  `0x12U`<br> |
| define  | [**SYN\_BLE\_ATT\_OP\_WRITE\_RSP**](syn__ble__att_8h.md#define-syn_ble_att_op_write_rsp)  `0x13U`<br> |

## Public Functions Documentation




### function syn\_ble\_att\_encode\_error\_rsp 

_Encode ATT Error Response._ 
```C++
SYN_Status syn_ble_att_encode_error_rsp (
    uint8_t req_opcode,
    uint16_t handle,
    uint8_t err_code,
    uint8_t * buf,
    uint16_t * len
) 
```





**Parameters:**


* `req_opcode` Request Opcode. 
* `handle` Attribute Handle. 
* `err_code` Error Code. 
* `buf` Output Buffer. 
* `len` Output Length Pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_att\_encode\_mtu\_rsp 

_Encode ATT Exchange MTU Response._ 
```C++
SYN_Status syn_ble_att_encode_mtu_rsp (
    uint16_t server_mtu,
    uint8_t * buf,
    uint16_t * len
) 
```





**Parameters:**


* `server_mtu` Server MTU. 
* `buf` Output Buffer. 
* `len` Output Length Pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_att\_encode\_notification 

_Encode ATT Handle Value Notification._ 
```C++
SYN_Status syn_ble_att_encode_notification (
    uint16_t handle,
    const uint8_t * val,
    uint16_t val_len,
    uint8_t * buf,
    uint16_t * len
) 
```





**Parameters:**


* `handle` Attribute Handle. 
* `val` Value bytes. 
* `val_len` Value length. 
* `buf` Output Buffer. 
* `len` Output Length Pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_att\_encode\_read\_rsp 

_Encode ATT Read Response._ 
```C++
SYN_Status syn_ble_att_encode_read_rsp (
    const uint8_t * val,
    uint16_t val_len,
    uint8_t * buf,
    uint16_t * len
) 
```





**Parameters:**


* `val` Value bytes. 
* `val_len` Value length. 
* `buf` Output Buffer. 
* `len` Output Length Pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_att\_encode\_write\_rsp 

_Encode ATT Write Response._ 
```C++
SYN_Status syn_ble_att_encode_write_rsp (
    uint8_t * buf,
    uint16_t * len
) 
```





**Parameters:**


* `buf` Output Buffer. 
* `len` Output Length Pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_BLE\_ATT\_ERR\_ATTR\_NOT\_FOUND 

```C++
#define SYN_BLE_ATT_ERR_ATTR_NOT_FOUND `0x0AU`
```



Attribute Not Found 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_ATTR\_NOT\_LONG 

```C++
#define SYN_BLE_ATT_ERR_ATTR_NOT_LONG `0x0BU`
```



Attribute Not Long 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_INSUFFICIENT\_AUTH 

```C++
#define SYN_BLE_ATT_ERR_INSUFFICIENT_AUTH `0x05U`
```



Insufficient Authentication 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_INSUFFICIENT\_AUTHOR 

```C++
#define SYN_BLE_ATT_ERR_INSUFFICIENT_AUTHOR `0x08U`
```



Insufficient Authorization 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_INSUFFICIENT\_ENCRYPT 

```C++
#define SYN_BLE_ATT_ERR_INSUFFICIENT_ENCRYPT `0x0FU`
```



Insufficient Encryption 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_INSUFFICIENT\_KEY\_SIZE 

```C++
#define SYN_BLE_ATT_ERR_INSUFFICIENT_KEY_SIZE `0x0CU`
```



Insufficient Key Size 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_INSUFFICIENT\_RESOURCES 

```C++
#define SYN_BLE_ATT_ERR_INSUFFICIENT_RESOURCES `0x11U`
```



Insufficient Resources 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_INVALID\_ATTR\_LEN 

```C++
#define SYN_BLE_ATT_ERR_INVALID_ATTR_LEN `0x0DU`
```



Invalid Attribute Value Length 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_INVALID\_HANDLE 

```C++
#define SYN_BLE_ATT_ERR_INVALID_HANDLE `0x01U`
```



Invalid Handle 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_INVALID\_OFFSET 

```C++
#define SYN_BLE_ATT_ERR_INVALID_OFFSET `0x07U`
```



Invalid Offset 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_INVALID\_PDU 

```C++
#define SYN_BLE_ATT_ERR_INVALID_PDU `0x04U`
```



Invalid PDU 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_PREPARE\_QUEUE\_FULL 

```C++
#define SYN_BLE_ATT_ERR_PREPARE_QUEUE_FULL `0x09U`
```



Prepare Queue Full 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_READ\_NOT\_PERMITTED 

```C++
#define SYN_BLE_ATT_ERR_READ_NOT_PERMITTED `0x02U`
```



Read Not Permitted 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_REQ\_NOT\_SUPPORTED 

```C++
#define SYN_BLE_ATT_ERR_REQ_NOT_SUPPORTED `0x06U`
```



Request Not Supported 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_UNLIKELY\_ERROR 

```C++
#define SYN_BLE_ATT_ERR_UNLIKELY_ERROR `0x0EU`
```



Unlikely Error 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_UNSUPPORTED\_GROUP\_TYPE 

```C++
#define SYN_BLE_ATT_ERR_UNSUPPORTED_GROUP_TYPE `0x10U`
```



Unsupported Group Type 


        

<hr>



### define SYN\_BLE\_ATT\_ERR\_WRITE\_NOT\_PERMITTED 

```C++
#define SYN_BLE_ATT_ERR_WRITE_NOT_PERMITTED `0x03U`
```



Write Not Permitted 


        

<hr>



### define SYN\_BLE\_ATT\_MTU\_DEFAULT 

```C++
#define SYN_BLE_ATT_MTU_DEFAULT `23U`
```



Default minimum ATT MTU size 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_ERROR\_RSP 

```C++
#define SYN_BLE_ATT_OP_ERROR_RSP `0x01U`
```



Error Response 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_EXCHANGE\_MTU\_REQ 

```C++
#define SYN_BLE_ATT_OP_EXCHANGE_MTU_REQ `0x02U`
```



Exchange MTU Request 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_EXCHANGE\_MTU\_RSP 

```C++
#define SYN_BLE_ATT_OP_EXCHANGE_MTU_RSP `0x03U`
```



Exchange MTU Response 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_FIND\_BY\_TYPE\_VALUE\_REQ 

```C++
#define SYN_BLE_ATT_OP_FIND_BY_TYPE_VALUE_REQ `0x06U`
```



Find By Type Value Request 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_FIND\_BY\_TYPE\_VALUE\_RSP 

```C++
#define SYN_BLE_ATT_OP_FIND_BY_TYPE_VALUE_RSP `0x07U`
```



Find By Type Value Response 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_FIND\_INFO\_REQ 

```C++
#define SYN_BLE_ATT_OP_FIND_INFO_REQ `0x04U`
```



Find Information Request 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_FIND\_INFO\_RSP 

```C++
#define SYN_BLE_ATT_OP_FIND_INFO_RSP `0x05U`
```



Find Information Response 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_HANDLE\_VAL\_CONF 

```C++
#define SYN_BLE_ATT_OP_HANDLE_VAL_CONF `0x1EU`
```



Handle Value Confirmation 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_HANDLE\_VAL\_IND 

```C++
#define SYN_BLE_ATT_OP_HANDLE_VAL_IND `0x1DU`
```



Handle Value Indication 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_HANDLE\_VAL\_NOTIF 

```C++
#define SYN_BLE_ATT_OP_HANDLE_VAL_NOTIF `0x1BU`
```



Handle Value Notification 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_READ\_BLOB\_REQ 

```C++
#define SYN_BLE_ATT_OP_READ_BLOB_REQ `0x0CU`
```



Read Blob Request 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_READ\_BLOB\_RSP 

```C++
#define SYN_BLE_ATT_OP_READ_BLOB_RSP `0x0DU`
```



Read Blob Response 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_READ\_BY\_GROUP\_TYPE\_REQ 

```C++
#define SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_REQ `0x10U`
```



Read By Group Type Request 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_READ\_BY\_GROUP\_TYPE\_RSP 

```C++
#define SYN_BLE_ATT_OP_READ_BY_GROUP_TYPE_RSP `0x11U`
```



Read By Group Type Response 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_READ\_BY\_TYPE\_REQ 

```C++
#define SYN_BLE_ATT_OP_READ_BY_TYPE_REQ `0x08U`
```



Read By Type Request 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_READ\_BY\_TYPE\_RSP 

```C++
#define SYN_BLE_ATT_OP_READ_BY_TYPE_RSP `0x09U`
```



Read By Type Response 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_READ\_REQ 

```C++
#define SYN_BLE_ATT_OP_READ_REQ `0x0AU`
```



Read Request 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_READ\_RSP 

```C++
#define SYN_BLE_ATT_OP_READ_RSP `0x0BU`
```



Read Response 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_WRITE\_CMD 

```C++
#define SYN_BLE_ATT_OP_WRITE_CMD `0x52U`
```



Write Command 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_WRITE\_REQ 

```C++
#define SYN_BLE_ATT_OP_WRITE_REQ `0x12U`
```



Write Request 


        

<hr>



### define SYN\_BLE\_ATT\_OP\_WRITE\_RSP 

```C++
#define SYN_BLE_ATT_OP_WRITE_RSP `0x13U`
```



Write Response 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_att.h`

