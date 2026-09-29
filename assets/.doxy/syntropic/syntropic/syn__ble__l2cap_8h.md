

# File syn\_ble\_l2cap.h



[**FileList**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_l2cap.h**](syn__ble__l2cap_8h.md)

[Go to the source code of this file](syn__ble__l2cap_8h_source.md)

_Zero-Heap BLE Logical Link Control and Adaptation Protocol (L2CAP)._ 

* `#include "../common/syn_defs.h"`
* `#include "syn_ble_hci.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_BLE\_L2CAP\_Connection**](structSYN__BLE__L2CAP__Connection.md) <br> |
| struct | [**SYN\_BLE\_L2CAP\_s**](structSYN__BLE__L2CAP__s.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_BLE\_L2CAP\_RxCb**](#typedef-syn_ble_l2cap_rxcb)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_l2cap\_connect**](#function-syn_ble_l2cap_connect) (SYN\_BLE\_L2CAP \* l2cap, uint16\_t conn\_handle) <br>_Register connection handle._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_l2cap\_disconnect**](#function-syn_ble_l2cap_disconnect) (SYN\_BLE\_L2CAP \* l2cap, uint16\_t conn\_handle) <br>_Unregister connection handle._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_l2cap\_encode\_pdu**](#function-syn_ble_l2cap_encode_pdu) (uint16\_t conn\_handle, uint16\_t cid, const uint8\_t \* payload, uint16\_t len, uint8\_t \* tx\_buf, uint16\_t \* tx\_len) <br>_Encode L2CAP PDU for transmission over HCI ACL._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_l2cap\_init**](#function-syn_ble_l2cap_init) (SYN\_BLE\_L2CAP \* l2cap, [**SYN\_BLE\_L2CAP\_RxCb**](syn__ble__l2cap_8h.md#typedef-syn_ble_l2cap_rxcb) rx\_cb, void \* user\_data) <br>_Initialize L2CAP engine context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_l2cap\_process\_acl**](#function-syn_ble_l2cap_process_acl) (SYN\_BLE\_L2CAP \* l2cap, uint16\_t conn\_handle, uint8\_t pb\_bc\_flags, const uint8\_t \* data, uint16\_t len) <br>_Process incoming ACL packet from HCI layer._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_BLE\_L2CAP\_BUF\_SIZE**](syn__ble__l2cap_8h.md#define-syn_ble_l2cap_buf_size)  `256U`<br> |
| define  | [**SYN\_BLE\_L2CAP\_CID\_ATT**](syn__ble__l2cap_8h.md#define-syn_ble_l2cap_cid_att)  `0x0004U`<br> |
| define  | [**SYN\_BLE\_L2CAP\_CID\_SIGNALING**](syn__ble__l2cap_8h.md#define-syn_ble_l2cap_cid_signaling)  `0x0005U`<br> |
| define  | [**SYN\_BLE\_L2CAP\_CID\_SMP**](syn__ble__l2cap_8h.md#define-syn_ble_l2cap_cid_smp)  `0x0006U`<br> |
| define  | [**SYN\_BLE\_MAX\_CONNECTIONS**](syn__ble__l2cap_8h.md#define-syn_ble_max_connections)  `2U`<br> |

## Public Types Documentation




### typedef SYN\_BLE\_L2CAP\_RxCb 

```C++
typedef void(* SYN_BLE_L2CAP_RxCb) (SYN_BLE_L2CAP *l2cap, uint16_t conn_handle, uint16_t cid, const uint8_t *payload, uint16_t len, void *user_data);
```



L2CAP PDU Received Callback 


        

<hr>
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
## Macro Definition Documentation





### define SYN\_BLE\_L2CAP\_BUF\_SIZE 

```C++
#define SYN_BLE_L2CAP_BUF_SIZE `256U`
```



Maximum reassembled L2CAP PDU size 


        

<hr>



### define SYN\_BLE\_L2CAP\_CID\_ATT 

```C++
#define SYN_BLE_L2CAP_CID_ATT `0x0004U`
```



Attribute Protocol (ATT) Channel 


        

<hr>



### define SYN\_BLE\_L2CAP\_CID\_SIGNALING 

```C++
#define SYN_BLE_L2CAP_CID_SIGNALING `0x0005U`
```



LE Signaling Channel 


        

<hr>



### define SYN\_BLE\_L2CAP\_CID\_SMP 

```C++
#define SYN_BLE_L2CAP_CID_SMP `0x0006U`
```



Security Manager Protocol (SMP) Channel 


        

<hr>



### define SYN\_BLE\_MAX\_CONNECTIONS 

```C++
#define SYN_BLE_MAX_CONNECTIONS `2U`
```



Maximum active BLE connection handles 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_l2cap.h`

