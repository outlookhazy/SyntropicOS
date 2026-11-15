

# File syn\_devicenet.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_devicenet.c**](syn__devicenet_8c.md)

[Go to the source code of this file](syn__devicenet_8c_source.md)

_ODVA DeviceNet Protocol Stack Implementation._ 

* `#include "syn_devicenet.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_devicenet\_init**](#function-syn_devicenet_init) ([**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, uint8\_t mac\_id, [**SYN\_DeviceNet\_Baud**](syn__devicenet_8h.md#enum-syn_devicenet_baud) baud) <br>_Initialize DeviceNet node instance context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_devicenet\_on\_can\_rx**](#function-syn_devicenet_on_can_rx) ([**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, uint32\_t can\_id, const uint8\_t \* data, uint8\_t len, uint32\_t \* tx\_can\_id, uint8\_t \* tx\_data, uint8\_t \* tx\_len) <br>_Process incoming CAN message frame and generate optional response frame._  |
|  void | [**syn\_devicenet\_poll**](#function-syn_devicenet_poll) ([**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, uint32\_t dt\_ms) <br>_Periodic timer tick to service Duplicate MAC ID check & QuickConnect timers._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_devicenet\_set\_assembly**](#function-syn_devicenet_set_assembly) ([**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, uint8\_t \* in\_buf, uint8\_t in\_len, uint8\_t \* out\_buf, uint8\_t out\_len) <br>_Configure Assembly Object (Class 0x04) input/output I/O buffers._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_devicenet\_set\_quickconnect**](#function-syn_devicenet_set_quickconnect) ([**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable QuickConnect™ fast startup mode._  |
|  void | [**syn\_devicenet\_set\_serial\_number**](#function-syn_devicenet_set_serial_number) ([**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, uint32\_t serial\_number) <br>_Set CIP Identity serial number for DeviceNet node._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**dnet\_get\_attr\_single**](#function-dnet_get_attr_single) (const [**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, uint8\_t class\_id, uint8\_t attr\_id, uint8\_t \* tx\_data, uint8\_t \* out\_idx) <br>_Handle CIP Get\_Attribute\_Single (0x0E) service across supported classes._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnet\_get\_identity\_attr**](#function-dnet_get_identity_attr) (const [**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, uint8\_t attr\_id, uint8\_t \* tx\_data, uint8\_t \* out\_idx) <br>_Encode a CIP Identity Object (Class 0x01) attribute into the response buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**dnet\_handle\_polled\_io**](#function-dnet_handle_polled_io) ([**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, uint8\_t mac, const uint8\_t \* data, uint8\_t len, uint32\_t \* tx\_can\_id, uint8\_t \* tx\_data, uint8\_t \* tx\_len) <br>_Process a Group 2 Polled I/O command and populate input assembly response._  |
|  void | [**dnet\_set\_attr\_single**](#function-dnet_set_attr_single) ([**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, uint8\_t class\_id, uint8\_t attr\_id, const uint8\_t \* data, uint8\_t len, uint8\_t \* tx\_data, uint8\_t \* out\_idx) <br>_Handle CIP Set\_Attribute\_Single (0x10) service for writable attributes._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**handle\_explicit\_message**](#function-handle_explicit_message) ([**SYN\_DeviceNet\_Node**](structSYN__DeviceNet__Node.md) \* node, const uint8\_t \* data, uint8\_t len, uint8\_t \* tx\_data, uint8\_t \* tx\_len) <br>_Process an explicit DeviceNet message payload._  |


























## Public Functions Documentation




### function syn\_devicenet\_init 

_Initialize DeviceNet node instance context._ 
```C++
bool syn_devicenet_init (
    SYN_DeviceNet_Node * node,
    uint8_t mac_id,
    SYN_DeviceNet_Baud baud
) 
```





**Parameters:**


* `node` Pointer to DeviceNet node context. 
* `mac_id` Initial Node Address (0..63). 
* `baud` Network baud rate setting. 



**Returns:**

true on success, false if parameters invalid. 





        

<hr>



### function syn\_devicenet\_on\_can\_rx 

_Process incoming CAN message frame and generate optional response frame._ 
```C++
bool syn_devicenet_on_can_rx (
    SYN_DeviceNet_Node * node,
    uint32_t can_id,
    const uint8_t * data,
    uint8_t len,
    uint32_t * tx_can_id,
    uint8_t * tx_data,
    uint8_t * tx_len
) 
```





**Parameters:**


* `node` Pointer to DeviceNet node instance. 
* `can_id` Received 11-bit CAN Identifier. 
* `data` Received CAN payload bytes. 
* `len` Received CAN payload length (0..8). 
* `tx_can_id` Output pointer for response CAN ID. 
* `tx_data` Output buffer for response payload bytes (min 8 bytes capacity). 
* `tx_len` Output pointer for response payload length. 



**Returns:**

true if response CAN frame generated, false if no response required. 





        

<hr>



### function syn\_devicenet\_poll 

_Periodic timer tick to service Duplicate MAC ID check & QuickConnect timers._ 
```C++
void syn_devicenet_poll (
    SYN_DeviceNet_Node * node,
    uint32_t dt_ms
) 
```





**Parameters:**


* `node` Pointer to DeviceNet node instance. 
* `dt_ms` Time elapsed in milliseconds since last poll call. 




        

<hr>



### function syn\_devicenet\_set\_assembly 

_Configure Assembly Object (Class 0x04) input/output I/O buffers._ 
```C++
bool syn_devicenet_set_assembly (
    SYN_DeviceNet_Node * node,
    uint8_t * in_buf,
    uint8_t in_len,
    uint8_t * out_buf,
    uint8_t out_len
) 
```





**Parameters:**


* `node` Pointer to DeviceNet node instance. 
* `in_buf` Pointer to input data buffer (sensor/status data to Master). 
* `in_len` Length of input buffer (max 8 bytes). 
* `out_buf` Pointer to output data buffer (actuator/cmd data from Master). 
* `out_len` Length of output buffer (max 8 bytes). 



**Returns:**

true on success, false if parameters invalid. 





        

<hr>



### function syn\_devicenet\_set\_quickconnect 

_Enable or disable QuickConnect™ fast startup mode._ 
```C++
bool syn_devicenet_set_quickconnect (
    SYN_DeviceNet_Node * node,
    bool enable
) 
```





**Parameters:**


* `node` Pointer to DeviceNet node instance. 
* `enable` True to enable QuickConnect feature. 



**Returns:**

true on success, false if node is NULL. 





        

<hr>



### function syn\_devicenet\_set\_serial\_number 

_Set CIP Identity serial number for DeviceNet node._ 
```C++
void syn_devicenet_set_serial_number (
    SYN_DeviceNet_Node * node,
    uint32_t serial_number
) 
```





**Parameters:**


* `node` Pointer to DeviceNet node context. 
* `serial_number` 32-bit serial number. 




        

<hr>
## Public Static Functions Documentation




### function dnet\_get\_attr\_single 

_Handle CIP Get\_Attribute\_Single (0x0E) service across supported classes._ 
```C++
static void dnet_get_attr_single (
    const SYN_DeviceNet_Node * node,
    uint8_t class_id,
    uint8_t attr_id,
    uint8_t * tx_data,
    uint8_t * out_idx
) 
```





**Parameters:**


* `node` Target DeviceNet node instance. 
* `class_id` Requested CIP class identifier. 
* `attr_id` Requested CIP attribute identifier. 
* `tx_data` Response message payload output buffer. 
* `out_idx` In/out write offset within tx\_data. 




        

<hr>



### function dnet\_get\_identity\_attr 

_Encode a CIP Identity Object (Class 0x01) attribute into the response buffer._ 
```C++
static bool dnet_get_identity_attr (
    const SYN_DeviceNet_Node * node,
    uint8_t attr_id,
    uint8_t * tx_data,
    uint8_t * out_idx
) 
```





**Parameters:**


* `node` Target DeviceNet node instance. 
* `attr_id` Requested Identity attribute identifier. 
* `tx_data` Response message payload output buffer. 
* `out_idx` In/out write offset within tx\_data. 



**Returns:**

true if attribute exists and was encoded, false otherwise. 





        

<hr>



### function dnet\_handle\_polled\_io 

_Process a Group 2 Polled I/O command and populate input assembly response._ 
```C++
static bool dnet_handle_polled_io (
    SYN_DeviceNet_Node * node,
    uint8_t mac,
    const uint8_t * data,
    uint8_t len,
    uint32_t * tx_can_id,
    uint8_t * tx_data,
    uint8_t * tx_len
) 
```





**Parameters:**


* `node` Target DeviceNet node instance. 
* `mac` Node MAC identifier. 
* `data` Received output assembly payload. 
* `len` Byte length of received payload. 
* `tx_can_id` Output CAN identifier for polled I/O response. 
* `tx_data` Output response payload buffer. 
* `tx_len` Output byte length of response payload. 



**Returns:**

true when polled I/O response is ready to transmit. 





        

<hr>



### function dnet\_set\_attr\_single 

_Handle CIP Set\_Attribute\_Single (0x10) service for writable attributes._ 
```C++
static void dnet_set_attr_single (
    SYN_DeviceNet_Node * node,
    uint8_t class_id,
    uint8_t attr_id,
    const uint8_t * data,
    uint8_t len,
    uint8_t * tx_data,
    uint8_t * out_idx
) 
```





**Parameters:**


* `node` Target DeviceNet node instance. 
* `class_id` Requested CIP class identifier. 
* `attr_id` Requested CIP attribute identifier. 
* `data` Received explicit request payload. 
* `len` Byte length of received payload. 
* `tx_data` Response message payload output buffer. 
* `out_idx` In/out write offset within tx\_data. 




        

<hr>



### function handle\_explicit\_message 

_Process an explicit DeviceNet message payload._ 
```C++
static bool handle_explicit_message (
    SYN_DeviceNet_Node * node,
    const uint8_t * data,
    uint8_t len,
    uint8_t * tx_data,
    uint8_t * tx_len
) 
```





**Parameters:**


* `node` Target DeviceNet node instance. 
* `data` Received message payload buffer. 
* `len` Byte length of received payload. 
* `tx_data` Response message payload output buffer. 
* `tx_len` Pointer to output byte length variable. 



**Returns:**

true if response frame was constructed, false on error. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_devicenet.c`

