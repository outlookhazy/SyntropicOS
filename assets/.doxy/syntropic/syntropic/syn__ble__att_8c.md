

# File syn\_ble\_att.c



[**FileList**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_att.c**](syn__ble__att_8c.md)

[Go to the source code of this file](syn__ble__att_8c_source.md)

_Zero-Heap Attribute Protocol (ATT) Implementation._ 

* `#include "syn_ble_att.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_att\_encode\_error\_rsp**](#function-syn_ble_att_encode_error_rsp) (uint8\_t req\_opcode, uint16\_t handle, uint8\_t err\_code, uint8\_t \* buf, uint16\_t \* len) <br>_Encode ATT Error Response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_att\_encode\_mtu\_rsp**](#function-syn_ble_att_encode_mtu_rsp) (uint16\_t server\_mtu, uint8\_t \* buf, uint16\_t \* len) <br>_Encode ATT Exchange MTU Response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_att\_encode\_notification**](#function-syn_ble_att_encode_notification) (uint16\_t handle, const uint8\_t \* val, uint16\_t val\_len, uint8\_t \* buf, uint16\_t \* len) <br>_Encode ATT Handle Value Notification._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_att\_encode\_read\_rsp**](#function-syn_ble_att_encode_read_rsp) (const uint8\_t \* val, uint16\_t val\_len, uint8\_t \* buf, uint16\_t \* len) <br>_Encode ATT Read Response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_att\_encode\_write\_rsp**](#function-syn_ble_att_encode_write_rsp) (uint8\_t \* buf, uint16\_t \* len) <br>_Encode ATT Write Response._  |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_att.c`

