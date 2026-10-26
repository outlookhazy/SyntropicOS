

# File syn\_ble\_l2cap.c



[**FileList**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_l2cap.c**](syn__ble__l2cap_8c.md)

[Go to the source code of this file](syn__ble__l2cap_8c_source.md)

_Zero-Heap BLE L2CAP Protocol Implementation._ 

* `#include "syn_ble_l2cap.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_l2cap\_connect**](#function-syn_ble_l2cap_connect) (SYN\_BLE\_L2CAP \* l2cap, uint16\_t conn\_handle) <br>_Register connection handle._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_l2cap\_disconnect**](#function-syn_ble_l2cap_disconnect) (SYN\_BLE\_L2CAP \* l2cap, uint16\_t conn\_handle) <br>_Unregister connection handle._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_l2cap\_encode\_pdu**](#function-syn_ble_l2cap_encode_pdu) (uint16\_t conn\_handle, uint16\_t cid, const uint8\_t \* payload, uint16\_t len, uint8\_t \* tx\_buf, uint16\_t \* tx\_len) <br>_Encode L2CAP PDU for transmission over HCI ACL._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_l2cap\_init**](#function-syn_ble_l2cap_init) (SYN\_BLE\_L2CAP \* l2cap, [**SYN\_BLE\_L2CAP\_RxCb**](syn__ble__l2cap_8h.md#typedef-syn_ble_l2cap_rxcb) rx\_cb, void \* user\_data) <br>_Initialize L2CAP engine context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_l2cap\_process\_acl**](#function-syn_ble_l2cap_process_acl) (SYN\_BLE\_L2CAP \* l2cap, uint16\_t conn\_handle, uint8\_t pb\_bc\_flags, const uint8\_t \* data, uint16\_t len) <br>_Process incoming ACL packet from HCI layer._  |




























## Public Functions Documentation




### function syn\_ble\_l2cap\_connect 

_Register connection handle._ 
```C++
SYN_Status syn_ble_l2cap_connect (
    SYN_BLE_L2CAP * l2cap,
    uint16_t conn_handle
) 
```





**Parameters:**


* `l2cap` Context handle pointer. 
* `conn_handle` Connection handle assigned by HCI. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_l2cap\_disconnect 

_Unregister connection handle._ 
```C++
SYN_Status syn_ble_l2cap_disconnect (
    SYN_BLE_L2CAP * l2cap,
    uint16_t conn_handle
) 
```





**Parameters:**


* `l2cap` Context handle pointer. 
* `conn_handle` Connection handle assigned by HCI. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_l2cap\_encode\_pdu 

_Encode L2CAP PDU for transmission over HCI ACL._ 
```C++
SYN_Status syn_ble_l2cap_encode_pdu (
    uint16_t conn_handle,
    uint16_t cid,
    const uint8_t * payload,
    uint16_t len,
    uint8_t * tx_buf,
    uint16_t * tx_len
) 
```





**Parameters:**


* `conn_handle` Connection handle. 
* `cid` Channel ID (e.g. SYN\_BLE\_L2CAP\_CID\_ATT). 
* `payload` PDU payload buffer. 
* `len` Payload length. 
* `tx_buf` Output buffer for HCI ACL frame. 
* `tx_len` Output size pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_l2cap\_init 

_Initialize L2CAP engine context._ 
```C++
SYN_Status syn_ble_l2cap_init (
    SYN_BLE_L2CAP * l2cap,
    SYN_BLE_L2CAP_RxCb rx_cb,
    void * user_data
) 
```





**Parameters:**


* `l2cap` Context handle pointer. 
* `rx_cb` PDU receive callback. 
* `user_data` User context pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_l2cap\_process\_acl 

_Process incoming ACL packet from HCI layer._ 
```C++
SYN_Status syn_ble_l2cap_process_acl (
    SYN_BLE_L2CAP * l2cap,
    uint16_t conn_handle,
    uint8_t pb_bc_flags,
    const uint8_t * data,
    uint16_t len
) 
```





**Parameters:**


* `l2cap` Context handle pointer. 
* `conn_handle` Connection handle. 
* `pb_bc_flags` Packet boundary/broadcast flags. 
* `data` ACL payload. 
* `len` Payload length. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_l2cap.c`

