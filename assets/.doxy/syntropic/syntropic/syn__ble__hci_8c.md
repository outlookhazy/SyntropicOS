

# File syn\_ble\_hci.c



[**FileList**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_hci.c**](syn__ble__hci_8c.md)

[Go to the source code of this file](syn__ble__hci_8c_source.md)

_Zero-Heap Host Controller Interface (HCI) Driver implementation._ 

* `#include "syn_ble_hci.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_hci\_encode\_acl**](#function-syn_ble_hci_encode_acl) (uint16\_t conn\_handle, uint8\_t pb\_flags, uint8\_t bc\_flags, const uint8\_t \* data, uint16\_t len, uint8\_t \* tx\_buf, uint16\_t \* tx\_len) <br>_Encode HCI ACL Data Packet._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_hci\_encode\_command**](#function-syn_ble_hci_encode_command) (uint16\_t opcode, const uint8\_t \* params, uint8\_t param\_len, uint8\_t \* tx\_buf, uint16\_t \* tx\_len) <br>_Send HCI Command Packet._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_hci\_init**](#function-syn_ble_hci_init) (SYN\_BLE\_HCI \* hci, const [**SYN\_BLE\_HCI\_Config**](structSYN__BLE__HCI__Config.md) \* cfg) <br>_Initialize HCI Driver Context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_hci\_rx\_buf**](#function-syn_ble_hci_rx_buf) (SYN\_BLE\_HCI \* hci, const uint8\_t \* buf, uint16\_t len) <br>_Feed raw buffer into HCI receiver state machine._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_hci\_rx\_byte**](#function-syn_ble_hci_rx_byte) (SYN\_BLE\_HCI \* hci, uint8\_t byte) <br>_Feed raw bytes from UART/SPI into HCI receiver state machine._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**parse\_and\_dispatch**](#function-parse_and_dispatch) (SYN\_BLE\_HCI \* hci) <br>_Internal parse and dispatch helper._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_BLE\_HCI\_RX\_STATE\_HDR**](syn__ble__hci_8c.md#define-syn_ble_hci_rx_state_hdr)  `1U`<br> |
| define  | [**SYN\_BLE\_HCI\_RX\_STATE\_PAYLOAD**](syn__ble__hci_8c.md#define-syn_ble_hci_rx_state_payload)  `2U`<br> |
| define  | [**SYN\_BLE\_HCI\_RX\_STATE\_TYPE**](syn__ble__hci_8c.md#define-syn_ble_hci_rx_state_type)  `0U`<br> |

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
## Public Static Functions Documentation




### function parse\_and\_dispatch 

_Internal parse and dispatch helper._ 
```C++
static void parse_and_dispatch (
    SYN_BLE_HCI * hci
) 
```





**Parameters:**


* `hci` HCI driver handle. 




        

<hr>
## Macro Definition Documentation





### define SYN\_BLE\_HCI\_RX\_STATE\_HDR 

```C++
#define SYN_BLE_HCI_RX_STATE_HDR `1U`
```



RX State: Header 


        

<hr>



### define SYN\_BLE\_HCI\_RX\_STATE\_PAYLOAD 

```C++
#define SYN_BLE_HCI_RX_STATE_PAYLOAD `2U`
```



RX State: Payload 


        

<hr>



### define SYN\_BLE\_HCI\_RX\_STATE\_TYPE 

```C++
#define SYN_BLE_HCI_RX_STATE_TYPE `0U`
```



RX State: Packet Type 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_hci.c`

