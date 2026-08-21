

# File syn\_ble\_hci.h



[**FileList**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_hci.h**](syn__ble__hci_8h.md)

[Go to the source code of this file](syn__ble__hci_8h_source.md)

_Zero-Heap Host Controller Interface (HCI) Driver (H:4 UART Framing)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_BLE\_HCI\_Config**](structSYN__BLE__HCI__Config.md) <br> |
| struct | [**SYN\_BLE\_HCI\_s**](structSYN__BLE__HCI__s.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_BLE\_HCI\_AclCb**](#typedef-syn_ble_hci_aclcb)  <br> |
| typedef void(\* | [**SYN\_BLE\_HCI\_EventCb**](#typedef-syn_ble_hci_eventcb)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_hci\_encode\_acl**](#function-syn_ble_hci_encode_acl) (uint16\_t conn\_handle, uint8\_t pb\_flags, uint8\_t bc\_flags, const uint8\_t \* data, uint16\_t len, uint8\_t \* tx\_buf, uint16\_t \* tx\_len) <br>_Encode HCI ACL Data Packet._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_hci\_encode\_command**](#function-syn_ble_hci_encode_command) (uint16\_t opcode, const uint8\_t \* params, uint8\_t param\_len, uint8\_t \* tx\_buf, uint16\_t \* tx\_len) <br>_Send HCI Command Packet._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_hci\_init**](#function-syn_ble_hci_init) (SYN\_BLE\_HCI \* hci, const [**SYN\_BLE\_HCI\_Config**](structSYN__BLE__HCI__Config.md) \* cfg) <br>_Initialize HCI Driver Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_hci\_rx\_buf**](#function-syn_ble_hci_rx_buf) (SYN\_BLE\_HCI \* hci, const uint8\_t \* buf, uint16\_t len) <br>_Feed raw buffer into HCI receiver state machine._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_hci\_rx\_byte**](#function-syn_ble_hci_rx_byte) (SYN\_BLE\_HCI \* hci, uint8\_t byte) <br>_Feed raw bytes from UART/SPI into HCI receiver state machine._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_BLE\_HCI\_EVT\_CMD\_COMPLETE**](syn__ble__hci_8h.md#define-syn_ble_hci_evt_cmd_complete)  `0x0EU`<br> |
| define  | [**SYN\_BLE\_HCI\_EVT\_CMD\_STATUS**](syn__ble__hci_8h.md#define-syn_ble_hci_evt_cmd_status)  `0x0FU`<br> |
| define  | [**SYN\_BLE\_HCI\_EVT\_DISCONN\_COMPLETE**](syn__ble__hci_8h.md#define-syn_ble_hci_evt_disconn_complete)  `0x05U`<br> |
| define  | [**SYN\_BLE\_HCI\_EVT\_LE\_META**](syn__ble__hci_8h.md#define-syn_ble_hci_evt_le_meta)  `0x3EU`<br> |
| define  | [**SYN\_BLE\_HCI\_LE\_SUBEVT\_ADV\_REPORT**](syn__ble__hci_8h.md#define-syn_ble_hci_le_subevt_adv_report)  `0x02U`<br> |
| define  | [**SYN\_BLE\_HCI\_LE\_SUBEVT\_CONN\_COMPLETE**](syn__ble__hci_8h.md#define-syn_ble_hci_le_subevt_conn_complete)  `0x01U`<br> |
| define  | [**SYN\_BLE\_HCI\_LE\_SUBEVT\_CONN\_UPDATE\_COMPLETE**](syn__ble__hci_8h.md#define-syn_ble_hci_le_subevt_conn_update_complete)  `0x03U`<br> |
| define  | [**SYN\_BLE\_HCI\_OGF\_INFO\_PARAM**](syn__ble__hci_8h.md#define-syn_ble_hci_ogf_info_param)  `0x04U`<br> |
| define  | [**SYN\_BLE\_HCI\_OGF\_LE\_CONTROLLER**](syn__ble__hci_8h.md#define-syn_ble_hci_ogf_le_controller)  `0x08U`<br> |
| define  | [**SYN\_BLE\_HCI\_OGF\_LINK\_CONTROL**](syn__ble__hci_8h.md#define-syn_ble_hci_ogf_link_control)  `0x01U`<br> |
| define  | [**SYN\_BLE\_HCI\_OGF\_STATUS\_PARAM**](syn__ble__hci_8h.md#define-syn_ble_hci_ogf_status_param)  `0x05U`<br> |
| define  | [**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode) (ogf, ocf) `((uint16\_t)(((ogf) &lt;&lt; 10) \| ((ocf) & 0x03FFU)))`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_DISCONNECT**](syn__ble__hci_8h.md#define-syn_ble_hci_op_disconnect)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x01U, 0x0006U)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_CREATE\_CONN**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_create_conn)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x000DU)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_CREATE\_CONN\_CANCEL**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_create_conn_cancel)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x000EU)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_READ\_BUFFER\_SIZE**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_read_buffer_size)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x0002U)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_SET\_ADV\_DATA**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_set_adv_data)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x0008U)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_SET\_ADV\_ENABLE**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_set_adv_enable)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x000AU)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_SET\_ADV\_PARAM**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_set_adv_param)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x0006U)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_SET\_EVENT\_MASK**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_set_event_mask)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x0001U)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_SET\_RANDOM\_ADDR**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_set_random_addr)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x0005U)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_SET\_SCAN\_ENABLE**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_set_scan_enable)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x000CU)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_SET\_SCAN\_PARAM**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_set_scan_param)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x000BU)`<br> |
| define  | [**SYN\_BLE\_HCI\_OP\_LE\_SET\_SCAN\_RESP\_DATA**](syn__ble__hci_8h.md#define-syn_ble_hci_op_le_set_scan_resp_data)  `[**SYN\_BLE\_HCI\_OPCODE**](syn__ble__hci_8h.md#define-syn_ble_hci_opcode)(0x08U, 0x0009U)`<br> |
| define  | [**SYN\_BLE\_HCI\_PKT\_ACL**](syn__ble__hci_8h.md#define-syn_ble_hci_pkt_acl)  `0x02U`<br> |
| define  | [**SYN\_BLE\_HCI\_PKT\_CMD**](syn__ble__hci_8h.md#define-syn_ble_hci_pkt_cmd)  `0x01U`<br> |
| define  | [**SYN\_BLE\_HCI\_PKT\_EVT**](syn__ble__hci_8h.md#define-syn_ble_hci_pkt_evt)  `0x04U`<br> |
| define  | [**SYN\_BLE\_HCI\_PKT\_ISO**](syn__ble__hci_8h.md#define-syn_ble_hci_pkt_iso)  `0x05U`<br> |
| define  | [**SYN\_BLE\_HCI\_PKT\_SCO**](syn__ble__hci_8h.md#define-syn_ble_hci_pkt_sco)  `0x03U`<br> |
| define  | [**SYN\_BLE\_HCI\_RX\_BUF\_SIZE**](syn__ble__hci_8h.md#define-syn_ble_hci_rx_buf_size)  `256U`<br> |
| define  | [**SYN\_BLE\_HCI\_TX\_BUF\_SIZE**](syn__ble__hci_8h.md#define-syn_ble_hci_tx_buf_size)  `256U`<br> |

## Detailed Description


Implements Bluetooth Core Specification Vol 4, Part A (H:4 Transport Layer). Encapsulates HCI Commands, HCI Events, and ACL Data packets over UART/SPI. 


    
## Public Types Documentation




### typedef SYN\_BLE\_HCI\_AclCb 

```C++
typedef void(* SYN_BLE_HCI_AclCb) (SYN_BLE_HCI *hci, uint16_t conn_handle, uint8_t pb_bc_flags, const uint8_t *data, uint16_t len, void *user_data);
```



Callback function pointer for received ACL Data 


        

<hr>



### typedef SYN\_BLE\_HCI\_EventCb 

```C++
typedef void(* SYN_BLE_HCI_EventCb) (SYN_BLE_HCI *hci, uint8_t evt_code, const uint8_t *payload, uint8_t len, void *user_data);
```



Callback function pointer for received HCI Events 


        

<hr>
## Public Functions Documentation




### function syn\_ble\_hci\_encode\_acl 

_Encode HCI ACL Data Packet._ 
```C++
SYN_Status syn_ble_hci_encode_acl (
    uint16_t conn_handle,
    uint8_t pb_flags,
    uint8_t bc_flags,
    const uint8_t * data,
    uint16_t len,
    uint8_t * tx_buf,
    uint16_t * tx_len
) 
```





**Parameters:**


* `conn_handle` 12-bit Connection Handle. 
* `pb_flags` Packet Boundary flags (2 bits). 
* `bc_flags` Broadcast flags (2 bits). 
* `data` Data payload bytes. 
* `len` Data payload length in bytes. 
* `tx_buf` Buffer to store framed ACL packet (minimum 5 + len bytes). 
* `tx_len` Pointer to store resulting framed size. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_hci\_encode\_command 

_Send HCI Command Packet._ 
```C++
SYN_Status syn_ble_hci_encode_command (
    uint16_t opcode,
    const uint8_t * params,
    uint8_t param_len,
    uint8_t * tx_buf,
    uint16_t * tx_len
) 
```





**Parameters:**


* `opcode` 16-bit HCI Command Opcode. 
* `params` Pointer to parameter payload (or NULL if len is 0). 
* `param_len` Parameter payload length in bytes. 
* `tx_buf` Buffer to store framed HCI packet (minimum 4 + param\_len bytes). 
* `tx_len` Pointer to store resulting framed size. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_hci\_init 

_Initialize HCI Driver Context._ 
```C++
SYN_Status syn_ble_hci_init (
    SYN_BLE_HCI * hci,
    const SYN_BLE_HCI_Config * cfg
) 
```





**Parameters:**


* `hci` Driver handle pointer. 
* `cfg` Driver configuration struct. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if null. 





        

<hr>



### function syn\_ble\_hci\_rx\_buf 

_Feed raw buffer into HCI receiver state machine._ 
```C++
SYN_Status syn_ble_hci_rx_buf (
    SYN_BLE_HCI * hci,
    const uint8_t * buf,
    uint16_t len
) 
```





**Parameters:**


* `hci` Driver handle pointer. 
* `buf` Data buffer pointer. 
* `len` Data buffer length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_hci\_rx\_byte 

_Feed raw bytes from UART/SPI into HCI receiver state machine._ 
```C++
SYN_Status syn_ble_hci_rx_byte (
    SYN_BLE_HCI * hci,
    uint8_t byte
) 
```





**Parameters:**


* `hci` Driver handle pointer. 
* `byte` Single received byte from H:4 transport. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_BLE\_HCI\_EVT\_CMD\_COMPLETE 

```C++
#define SYN_BLE_HCI_EVT_CMD_COMPLETE `0x0EU`
```



Command Complete Event 


        

<hr>



### define SYN\_BLE\_HCI\_EVT\_CMD\_STATUS 

```C++
#define SYN_BLE_HCI_EVT_CMD_STATUS `0x0FU`
```



Command Status Event 


        

<hr>



### define SYN\_BLE\_HCI\_EVT\_DISCONN\_COMPLETE 

```C++
#define SYN_BLE_HCI_EVT_DISCONN_COMPLETE `0x05U`
```



Disconnection Complete Event 


        

<hr>



### define SYN\_BLE\_HCI\_EVT\_LE\_META 

```C++
#define SYN_BLE_HCI_EVT_LE_META `0x3EU`
```



LE Meta Event 


        

<hr>



### define SYN\_BLE\_HCI\_LE\_SUBEVT\_ADV\_REPORT 

```C++
#define SYN_BLE_HCI_LE_SUBEVT_ADV_REPORT `0x02U`
```



LE Advertising Report Subevent 


        

<hr>



### define SYN\_BLE\_HCI\_LE\_SUBEVT\_CONN\_COMPLETE 

```C++
#define SYN_BLE_HCI_LE_SUBEVT_CONN_COMPLETE `0x01U`
```



LE Connection Complete Subevent 


        

<hr>



### define SYN\_BLE\_HCI\_LE\_SUBEVT\_CONN\_UPDATE\_COMPLETE 

```C++
#define SYN_BLE_HCI_LE_SUBEVT_CONN_UPDATE_COMPLETE `0x03U`
```



LE Connection Update Complete Subevent 


        

<hr>



### define SYN\_BLE\_HCI\_OGF\_INFO\_PARAM 

```C++
#define SYN_BLE_HCI_OGF_INFO_PARAM `0x04U`
```



Informational Parameters OGF 


        

<hr>



### define SYN\_BLE\_HCI\_OGF\_LE\_CONTROLLER 

```C++
#define SYN_BLE_HCI_OGF_LE_CONTROLLER `0x08U`
```



LE Controller Commands OGF 


        

<hr>



### define SYN\_BLE\_HCI\_OGF\_LINK\_CONTROL 

```C++
#define SYN_BLE_HCI_OGF_LINK_CONTROL `0x01U`
```



Link Control OGF 


        

<hr>



### define SYN\_BLE\_HCI\_OGF\_STATUS\_PARAM 

```C++
#define SYN_BLE_HCI_OGF_STATUS_PARAM `0x05U`
```



Status Parameters OGF 


        

<hr>



### define SYN\_BLE\_HCI\_OPCODE 

```C++
#define SYN_BLE_HCI_OPCODE (
    ogf,
    ocf
) `((uint16_t)(((ogf) << 10) | ((ocf) & 0x03FFU)))`
```



Construct 16-bit HCI Opcode from OGF and OCF 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_DISCONNECT 

```C++
#define SYN_BLE_HCI_OP_DISCONNECT `SYN_BLE_HCI_OPCODE (0x01U, 0x0006U)`
```



Disconnect Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_CREATE\_CONN 

```C++
#define SYN_BLE_HCI_OP_LE_CREATE_CONN `SYN_BLE_HCI_OPCODE (0x08U, 0x000DU)`
```



LE Create Connection Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_CREATE\_CONN\_CANCEL 

```C++
#define SYN_BLE_HCI_OP_LE_CREATE_CONN_CANCEL `SYN_BLE_HCI_OPCODE (0x08U, 0x000EU)`
```



LE Create Connection Cancel Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_READ\_BUFFER\_SIZE 

```C++
#define SYN_BLE_HCI_OP_LE_READ_BUFFER_SIZE `SYN_BLE_HCI_OPCODE (0x08U, 0x0002U)`
```



LE Read Buffer Size Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_SET\_ADV\_DATA 

```C++
#define SYN_BLE_HCI_OP_LE_SET_ADV_DATA `SYN_BLE_HCI_OPCODE (0x08U, 0x0008U)`
```



LE Set Advertising Data Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_SET\_ADV\_ENABLE 

```C++
#define SYN_BLE_HCI_OP_LE_SET_ADV_ENABLE `SYN_BLE_HCI_OPCODE (0x08U, 0x000AU)`
```



LE Set Advertising Enable Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_SET\_ADV\_PARAM 

```C++
#define SYN_BLE_HCI_OP_LE_SET_ADV_PARAM `SYN_BLE_HCI_OPCODE (0x08U, 0x0006U)`
```



LE Set Advertising Parameters Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_SET\_EVENT\_MASK 

```C++
#define SYN_BLE_HCI_OP_LE_SET_EVENT_MASK `SYN_BLE_HCI_OPCODE (0x08U, 0x0001U)`
```



LE Set Event Mask Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_SET\_RANDOM\_ADDR 

```C++
#define SYN_BLE_HCI_OP_LE_SET_RANDOM_ADDR `SYN_BLE_HCI_OPCODE (0x08U, 0x0005U)`
```



LE Set Random Address Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_SET\_SCAN\_ENABLE 

```C++
#define SYN_BLE_HCI_OP_LE_SET_SCAN_ENABLE `SYN_BLE_HCI_OPCODE (0x08U, 0x000CU)`
```



LE Set Scan Enable Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_SET\_SCAN\_PARAM 

```C++
#define SYN_BLE_HCI_OP_LE_SET_SCAN_PARAM `SYN_BLE_HCI_OPCODE (0x08U, 0x000BU)`
```



LE Set Scan Parameters Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_OP\_LE\_SET\_SCAN\_RESP\_DATA 

```C++
#define SYN_BLE_HCI_OP_LE_SET_SCAN_RESP_DATA `SYN_BLE_HCI_OPCODE (0x08U, 0x0009U)`
```



LE Set Scan Response Data Opcode 


        

<hr>



### define SYN\_BLE\_HCI\_PKT\_ACL 

```C++
#define SYN_BLE_HCI_PKT_ACL `0x02U`
```



HCI ACL Data Packet 


        

<hr>



### define SYN\_BLE\_HCI\_PKT\_CMD 

```C++
#define SYN_BLE_HCI_PKT_CMD `0x01U`
```



HCI Command Packet 


        

<hr>



### define SYN\_BLE\_HCI\_PKT\_EVT 

```C++
#define SYN_BLE_HCI_PKT_EVT `0x04U`
```



HCI Event Packet 


        

<hr>



### define SYN\_BLE\_HCI\_PKT\_ISO 

```C++
#define SYN_BLE_HCI_PKT_ISO `0x05U`
```



HCI ISO Data Packet 


        

<hr>



### define SYN\_BLE\_HCI\_PKT\_SCO 

```C++
#define SYN_BLE_HCI_PKT_SCO `0x03U`
```



HCI Synchronous Data Packet 


        

<hr>



### define SYN\_BLE\_HCI\_RX\_BUF\_SIZE 

```C++
#define SYN_BLE_HCI_RX_BUF_SIZE `256U`
```



HCI RX packet reassembly capacity 


        

<hr>



### define SYN\_BLE\_HCI\_TX\_BUF\_SIZE 

```C++
#define SYN_BLE_HCI_TX_BUF_SIZE `256U`
```



HCI TX packet capacity 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_hci.h`

