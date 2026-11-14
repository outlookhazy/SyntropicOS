

# File syn\_opcua.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_opcua.c**](syn__opcua_8c.md)

[Go to the source code of this file](syn__opcua_8c_source.md)

_Micro OPC UA (IEC 62541) Binary Protocol Engine & Embedded Server Implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "../sched/syn_sched.h"`
* `#include "syn_opcua.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**OPCUA\_Reader**](structOPCUA__Reader.md) <br>_Binary deserializer context for OPC UA messages._  |
| struct | [**OPCUA\_WriteItem**](structOPCUA__WriteItem.md) <br>_Decoded single WriteValue entry from a WriteRequest._  |
| struct | [**OPCUA\_Writer**](structOPCUA__Writer.md) <br>_Binary serializer context for OPC UA messages._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_OPCUA\_Node**](structSYN__OPCUA__Node.md) \* | [**syn\_opcua\_server\_find\_node\_num**](#function-syn_opcua_server_find_node_num) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, uint16\_t ns, uint32\_t num\_id) <br>_Find a node in the address space by numeric NodeId._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_init**](#function-syn_opcua_server_init) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, const [**SYN\_OPCUA\_Config**](structSYN__OPCUA__Config.md) \* cfg) <br>_Initialize Micro OPC UA Server._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_process\_message**](#function-syn_opcua_server_process_message) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, const uint8\_t \* rx, size\_t rx\_len, uint8\_t \* tx, size\_t max\_tx, size\_t \* out\_len) <br>_Process an incoming raw OPC UA frame and generate a response frame._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_opcua\_server\_pt**](#function-syn_opcua_server_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Protothread continuation function for_ `Scheduling` _._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_read\_variable**](#function-syn_opcua_server_read_variable) (const [**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, uint16\_t ns, uint32\_t num\_id, [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) \* out\_val) <br>_Read a variable node's value from the address space._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_register\_node**](#function-syn_opcua_server_register_node) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, const [**SYN\_OPCUA\_Node**](structSYN__OPCUA__Node.md) \* node) <br>_Register a Node in the server address space._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_step**](#function-syn_opcua_server_step) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, uint32\_t now\_ms) <br>_Advance OPC UA server state machine by one step / tick._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_write\_variable**](#function-syn_opcua_server_write_variable) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, uint16\_t ns, uint32\_t num\_id, const [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) \* val) <br>_Update a variable node's value in the address space._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**opcua\_apply\_write\_item**](#function-opcua_apply_write_item) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, const [**OPCUA\_WriteItem**](structOPCUA__WriteItem.md) \* item) <br>_Validate and apply a decoded WriteValue item to the server address space._  |
|  uint32\_t | [**opcua\_commit\_write\_item**](#function-opcua_commit_write_item) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, [**SYN\_OPCUA\_Node**](structSYN__OPCUA__Node.md) \* node, const [**OPCUA\_WriteItem**](structOPCUA__WriteItem.md) \* item) <br>_Commit a validated WriteValue item to a Variable node and invoke on\_write._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_decode\_write\_datavalue**](#function-opcua_decode_write_datavalue) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, [**OPCUA\_WriteItem**](structOPCUA__WriteItem.md) \* item) <br>_Decode DataValue trailing fields for a WriteValue item._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_decode\_write\_item**](#function-opcua_decode_write_item) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, [**OPCUA\_WriteItem**](structOPCUA__WriteItem.md) \* item) <br>_Decode one WriteValue item from a WriteRequest stream._  |
|  void | [**opcua\_dispatch\_service**](#function-opcua_dispatch_service) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, uint32\_t req\_type\_id, uint32\_t req\_id, [**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, [**OPCUA\_Writer**](structOPCUA__Writer.md) \* w) <br>_Dispatch session-level OPC UA service after SecureChannel validation._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_drain\_one\_rx\_frame**](#function-opcua_drain_one_rx_frame) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv) <br>_Dispatch at most one complete OPC UA frame from srv-&gt;cfg.rx\_buf and retain partial tail._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**opcua\_handle\_hel**](#function-opcua_handle_hel) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, [**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, size\_t \* out\_len) <br>_Handle UACP Hello (HELF) message and encode ACKF response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**opcua\_handle\_msg**](#function-opcua_handle_msg) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, [**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, [**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, size\_t \* out\_len) <br>_Handle UASC Service Message (MSGF) and encode matching service response._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**opcua\_handle\_opn**](#function-opcua_handle_opn) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, [**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, [**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, size\_t \* out\_len) <br>_Handle UASC OpenSecureChannel (OPNF) request and encode OPNF response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_is\_root\_node**](#function-opcua_is_root_node) (const [**SYN\_OPCUA\_NodeId**](structSYN__OPCUA__NodeId.md) \* id) <br>_Check whether a NodeId identifies the standard RootFolder node._  |
|  void | [**opcua\_patch\_u32\_len**](#function-opcua_patch_u32_len) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, size\_t len\_pos) <br>_Patch 32-bit little-endian total message size into UASC frame header._  |
|  void | [**opcua\_populate\_standard\_nodes**](#function-opcua_populate_standard_nodes) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv) <br>_Populate default standard OPC UA address space folders and server status node._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_pt\_more\_work**](#function-opcua_pt_more_work) (const [**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, [**bool**](syn__defs_8h.md#enum-bool) progressed) <br>_Check whether another cooperative OPC UA server slice should run immediately._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_nodeid\_num**](#function-opcua_read_nodeid_num) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, uint16\_t \* out\_ns, uint32\_t \* out\_num) <br>_Read numeric OPC UA NodeId._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_string\_buf**](#function-opcua_read_string_buf) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, char buf, [**bool**](syn__defs_8h.md#enum-bool) \* out\_is\_null) <br>_Read OPC UA String payload into bounded scratch buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_u16**](#function-opcua_read_u16) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, uint16\_t \* out) <br>_Read 16-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_u32**](#function-opcua_read_u32) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, uint32\_t \* out) <br>_Read 32-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_u64**](#function-opcua_read_u64) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, uint64\_t \* out) <br>_Read 64-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_u8**](#function-opcua_read_u8) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, uint8\_t \* out) <br>_Read 8-bit unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_variant**](#function-opcua_read_variant) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, [**OPCUA\_WriteItem**](structOPCUA__WriteItem.md) \* item) <br>_Read typed OPC UA Variant from binary stream._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_variant\_int**](#function-opcua_read_variant_int) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, [**SYN\_OPCUA\_DataType**](syn__opcua_8h.md#enum-syn_opcua_datatype) type, [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) \* out, [**bool**](syn__defs_8h.md#enum-bool) \* out\_handled) <br>_Read integer or boolean OPC UA Variant payload._  |
|  void | [**opcua\_reader\_init**](#function-opcua_reader_init) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, const uint8\_t \* buf, size\_t size) <br>_Initialize OPC UA binary reader._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_recv\_rx\_bytes**](#function-opcua_recv_rx_bytes) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv) <br>_Read available transport bytes into the free tail of srv-&gt;cfg.rx\_buf._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**opcua\_server\_step\_pt\_once**](#function-opcua_server_step_pt_once) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, uint32\_t now\_ms, [**bool**](syn__defs_8h.md#enum-bool) \* out\_progressed) <br>_Execute one cooperative OPC UA server slice (at most one complete RX frame)._  |
|  void | [**opcua\_service\_browse**](#function-opcua_service_browse) (const [**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, [**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint32\_t req\_id) <br>_Encode BrowseResponse listing registered non-root nodes._  |
|  void | [**opcua\_service\_read**](#function-opcua_service_read) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, [**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, [**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint32\_t req\_id) <br>_Encode ReadResponse payload for an inbound ReadRequest._  |
|  void | [**opcua\_service\_write**](#function-opcua_service_write) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, [**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, [**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint32\_t req\_id) <br>_Encode WriteResponse payload for an inbound WriteRequest._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_skip**](#function-opcua_skip) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, size\_t count) <br>_Advance reader by count bytes with bounds checking._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_bytes**](#function-opcua_write_bytes) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, const uint8\_t \* buf, size\_t len) <br>_Write raw byte buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_datavalue**](#function-opcua_write_datavalue) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, const [**SYN\_OPCUA\_DataValue**](structSYN__OPCUA__DataValue.md) \* dv) <br>_Write OPC UA DataValue structure._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_nodeid**](#function-opcua_write_nodeid) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, const [**SYN\_OPCUA\_NodeId**](structSYN__OPCUA__NodeId.md) \* id) <br>_Write numeric or string OPC UA NodeId._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_nodeid\_num**](#function-opcua_write_nodeid_num) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint16\_t ns, uint32\_t num) <br>_Write numeric OPC UA NodeId._  |
|  void | [**opcua\_write\_service\_fault**](#function-opcua_write_service_fault) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint32\_t req\_id, uint32\_t status\_code) <br>_Encode OPC UA ServiceFault response body._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_string**](#function-opcua_write_string) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, const char \* str) <br>_Write length-prefixed OPC UA string._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_u16**](#function-opcua_write_u16) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint16\_t v) <br>_Write 16-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_u32**](#function-opcua_write_u32) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint32\_t v) <br>_Write 32-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_u64**](#function-opcua_write_u64) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint64\_t v) <br>_Write 64-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_u8**](#function-opcua_write_u8) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint8\_t v) <br>_Write 8-bit unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_variant**](#function-opcua_write_variant) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, const [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) \* var) <br>_Write OPC UA variant value._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_variant\_int**](#function-opcua_write_variant_int) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, const [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) \* var, [**bool**](syn__defs_8h.md#enum-bool) \* out\_wrote) <br>_Write integer or boolean OPC UA variant payload._  |
|  void | [**opcua\_writer\_init**](#function-opcua_writer_init) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint8\_t \* buf, size\_t size) <br>_Initialize OPC UA binary writer._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_OPCUA\_REQ\_ACTIVATE\_SESSION**](syn__opcua_8c.md#define-syn_opcua_req_activate_session)  `467U`<br>_OPC UA ActivateSessionRequest NodeId._  |
| define  | [**SYN\_OPCUA\_REQ\_BROWSE**](syn__opcua_8c.md#define-syn_opcua_req_browse)  `527U`<br>_OPC UA BrowseRequest NodeId._  |
| define  | [**SYN\_OPCUA\_REQ\_CLOSE\_SESSION**](syn__opcua_8c.md#define-syn_opcua_req_close_session)  `473U`<br>_OPC UA CloseSessionRequest NodeId._  |
| define  | [**SYN\_OPCUA\_REQ\_CREATE\_SESSION**](syn__opcua_8c.md#define-syn_opcua_req_create_session)  `461U`<br>_OPC UA CreateSessionRequest NodeId._  |
| define  | [**SYN\_OPCUA\_REQ\_READ**](syn__opcua_8c.md#define-syn_opcua_req_read)  `631U`<br>_OPC UA ReadRequest NodeId._  |
| define  | [**SYN\_OPCUA\_REQ\_WRITE**](syn__opcua_8c.md#define-syn_opcua_req_write)  `673U`<br>_OPC UA WriteRequest NodeId._  |
| define  | [**SYN\_OPCUA\_RESP\_ACTIVATE\_SESSION**](syn__opcua_8c.md#define-syn_opcua_resp_activate_session)  `470U`<br>_OPC UA ActivateSessionResponse NodeId._  |
| define  | [**SYN\_OPCUA\_RESP\_BROWSE**](syn__opcua_8c.md#define-syn_opcua_resp_browse)  `530U`<br>_OPC UA BrowseResponse NodeId._  |
| define  | [**SYN\_OPCUA\_RESP\_CLOSE\_SESSION**](syn__opcua_8c.md#define-syn_opcua_resp_close_session)  `476U`<br>_OPC UA CloseSessionResponse NodeId._  |
| define  | [**SYN\_OPCUA\_RESP\_CREATE\_SESSION**](syn__opcua_8c.md#define-syn_opcua_resp_create_session)  `464U`<br>_OPC UA CreateSessionResponse NodeId._  |
| define  | [**SYN\_OPCUA\_RESP\_READ**](syn__opcua_8c.md#define-syn_opcua_resp_read)  `634U`<br>_OPC UA ReadResponse NodeId._  |
| define  | [**SYN\_OPCUA\_RESP\_SERVICE\_FAULT**](syn__opcua_8c.md#define-syn_opcua_resp_service_fault)  `397U`<br>_OPC UA ServiceFault NodeId._  |
| define  | [**SYN\_OPCUA\_RESP\_WRITE**](syn__opcua_8c.md#define-syn_opcua_resp_write)  `676U`<br>_OPC UA WriteResponse NodeId._  |

## Public Functions Documentation




### function syn\_opcua\_server\_find\_node\_num 

_Find a node in the address space by numeric NodeId._ 
```C++
SYN_OPCUA_Node * syn_opcua_server_find_node_num (
    SYN_OPCUA_Server * srv,
    uint16_t ns,
    uint32_t num_id
) 
```





**Parameters:**


* `srv` Server instance. 
* `ns` Namespace index. 
* `num_id` Numeric identifier. 



**Returns:**

Pointer to node in address space or NULL if not found. 





        

<hr>



### function syn\_opcua\_server\_init 

_Initialize Micro OPC UA Server._ 
```C++
SYN_Status syn_opcua_server_init (
    SYN_OPCUA_Server * srv,
    const SYN_OPCUA_Config * cfg
) 
```





**Parameters:**


* `srv` Server instance to initialize. 
* `cfg` Server configuration. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid argument. 





        

<hr>



### function syn\_opcua\_server\_process\_message 

_Process an incoming raw OPC UA frame and generate a response frame._ 
```C++
SYN_Status syn_opcua_server_process_message (
    SYN_OPCUA_Server * srv,
    const uint8_t * rx,
    size_t rx_len,
    uint8_t * tx,
    size_t max_tx,
    size_t * out_len
) 
```





**Parameters:**


* `srv` Server instance. 
* `rx` Inbound raw message frame. 
* `rx_len` Inbound length in bytes. 
* `tx` Outbound response frame buffer. 
* `max_tx` Outbound buffer capacity. 
* `out_len` Pointer receiving generated response length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_opcua\_server\_pt 

_Protothread continuation function for_ `Scheduling` _._
```C++
SYN_PT_Status syn_opcua_server_pt (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread continuation structure. 
* `task` Task handle containing [**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) in user\_data. 



**Returns:**

[**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) continuation status. 





        

<hr>



### function syn\_opcua\_server\_read\_variable 

_Read a variable node's value from the address space._ 
```C++
SYN_Status syn_opcua_server_read_variable (
    const SYN_OPCUA_Server * srv,
    uint16_t ns,
    uint32_t num_id,
    SYN_OPCUA_Variant * out_val
) 
```





**Parameters:**


* `srv` Server instance. 
* `ns` Namespace index. 
* `num_id` Numeric identifier. 
* `out_val` Output variant value pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if not found. 





        

<hr>



### function syn\_opcua\_server\_register\_node 

_Register a Node in the server address space._ 
```C++
SYN_Status syn_opcua_server_register_node (
    SYN_OPCUA_Server * srv,
    const SYN_OPCUA_Node * node
) 
```





**Parameters:**


* `srv` Server instance. 
* `node` Node descriptor to copy into address space. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if address space is full. 





        

<hr>



### function syn\_opcua\_server\_step 

_Advance OPC UA server state machine by one step / tick._ 
```C++
SYN_Status syn_opcua_server_step (
    SYN_OPCUA_Server * srv,
    uint32_t now_ms
) 
```





**Parameters:**


* `srv` Server instance. 
* `now_ms` Current system timestamp in milliseconds. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_opcua\_server\_write\_variable 

_Update a variable node's value in the address space._ 
```C++
SYN_Status syn_opcua_server_write_variable (
    SYN_OPCUA_Server * srv,
    uint16_t ns,
    uint32_t num_id,
    const SYN_OPCUA_Variant * val
) 
```





**Parameters:**


* `srv` Server instance. 
* `ns` Namespace index. 
* `num_id` Numeric identifier. 
* `val` New variant value. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if not found. 





        

<hr>
## Public Static Functions Documentation




### function opcua\_apply\_write\_item 

_Validate and apply a decoded WriteValue item to the server address space._ 
```C++
static uint32_t opcua_apply_write_item (
    SYN_OPCUA_Server * srv,
    const OPCUA_WriteItem * item
) 
```





**Parameters:**


* `srv` OPC UA server instance. 
* `item` Decoded WriteValue item. 



**Returns:**

OPC UA StatusCode result for this write operation. 





        

<hr>



### function opcua\_commit\_write\_item 

_Commit a validated WriteValue item to a Variable node and invoke on\_write._ 
```C++
static uint32_t opcua_commit_write_item (
    SYN_OPCUA_Server * srv,
    SYN_OPCUA_Node * node,
    const OPCUA_WriteItem * item
) 
```





**Parameters:**


* `srv` OPC UA server instance. 
* `node` Target Variable node in server address space. 
* `item` Validated WriteValue item to commit. 



**Returns:**

OPC UA StatusCode (SYN\_OPCUA\_STATUS\_GOOD or error code). 





        

<hr>



### function opcua\_decode\_write\_datavalue 

_Decode DataValue trailing fields for a WriteValue item._ 
```C++
static bool opcua_decode_write_datavalue (
    OPCUA_Reader * r,
    OPCUA_WriteItem * item
) 
```





**Parameters:**


* `r` Binary stream reader. 
* `item` Output WriteValue item receiving DataValue fields. 



**Returns:**

true on valid DataValue decode, false on stream error. 





        

<hr>



### function opcua\_decode\_write\_item 

_Decode one WriteValue item from a WriteRequest stream._ 
```C++
static bool opcua_decode_write_item (
    OPCUA_Reader * r,
    OPCUA_WriteItem * item
) 
```





**Parameters:**


* `r` Binary stream reader. 
* `item` Output decoded WriteValue item. 



**Returns:**

true on valid decode, false on malformed or truncated stream. 





        

<hr>



### function opcua\_dispatch\_service 

_Dispatch session-level OPC UA service after SecureChannel validation._ 
```C++
static void opcua_dispatch_service (
    SYN_OPCUA_Server * srv,
    uint32_t req_type_id,
    uint32_t req_id,
    OPCUA_Reader * r,
    OPCUA_Writer * w
) 
```





**Parameters:**


* `srv` OPC UA server instance. 
* `req_type_id` Numeric NodeId of the requested service type. 
* `req_id` Client request identifier. 
* `r` Request payload reader positioned after RequestTypeId. 
* `w` Binary response writer. 




        

<hr>



### function opcua\_drain\_one\_rx\_frame 

_Dispatch at most one complete OPC UA frame from srv-&gt;cfg.rx\_buf and retain partial tail._ 
```C++
static bool opcua_drain_one_rx_frame (
    SYN_OPCUA_Server * srv
) 
```





**Parameters:**


* `srv` OPC UA server instance. 



**Returns:**

true if one complete frame was processed, false otherwise. 





        

<hr>



### function opcua\_handle\_hel 

_Handle UACP Hello (HELF) message and encode ACKF response._ 
```C++
static SYN_Status opcua_handle_hel (
    SYN_OPCUA_Server * srv,
    OPCUA_Writer * w,
    size_t * out_len
) 
```





**Parameters:**


* `srv` Server instance. 
* `w` Response writer. 
* `out_len` Output response length. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on buffer overflow. 





        

<hr>



### function opcua\_handle\_msg 

_Handle UASC Service Message (MSGF) and encode matching service response._ 
```C++
static SYN_Status opcua_handle_msg (
    SYN_OPCUA_Server * srv,
    OPCUA_Reader * r,
    OPCUA_Writer * w,
    size_t * out_len
) 
```





**Parameters:**


* `srv` Server instance. 
* `r` Request reader. 
* `w` Response writer. 
* `out_len` Output response length. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on invalid channel/token or buffer overflow. 





        

<hr>



### function opcua\_handle\_opn 

_Handle UASC OpenSecureChannel (OPNF) request and encode OPNF response._ 
```C++
static SYN_Status opcua_handle_opn (
    SYN_OPCUA_Server * srv,
    OPCUA_Reader * r,
    OPCUA_Writer * w,
    size_t * out_len
) 
```





**Parameters:**


* `srv` Server instance. 
* `r` Request reader. 
* `w` Response writer. 
* `out_len` Output response length. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on buffer overflow. 





        

<hr>



### function opcua\_is\_root\_node 

_Check whether a NodeId identifies the standard RootFolder node._ 
```C++
static bool opcua_is_root_node (
    const SYN_OPCUA_NodeId * id
) 
```





**Parameters:**


* `id` NodeId descriptor to inspect. 



**Returns:**

true if numeric RootFolder NodeId, false otherwise. 





        

<hr>



### function opcua\_patch\_u32\_len 

_Patch 32-bit little-endian total message size into UASC frame header._ 
```C++
static void opcua_patch_u32_len (
    OPCUA_Writer * w,
    size_t len_pos
) 
```





**Parameters:**


* `w` Writer context. 
* `len_pos` Offset of 4-byte length field inside w-&gt;data. 




        

<hr>



### function opcua\_populate\_standard\_nodes 

_Populate default standard OPC UA address space folders and server status node._ 
```C++
static void opcua_populate_standard_nodes (
    SYN_OPCUA_Server * srv
) 
```





**Parameters:**


* `srv` Server instance pointer. 




        

<hr>



### function opcua\_pt\_more\_work 

_Check whether another cooperative OPC UA server slice should run immediately._ 
```C++
static bool opcua_pt_more_work (
    const SYN_OPCUA_Server * srv,
    bool progressed
) 
```





**Parameters:**


* `srv` Non-NULL OPC UA server instance with valid transport. 
* `progressed` Whether the current slice read bytes or processed a frame. 



**Returns:**

true when progressed and buffered/transport data remains. 





        

<hr>



### function opcua\_read\_nodeid\_num 

_Read numeric OPC UA NodeId._ 
```C++
static bool opcua_read_nodeid_num (
    OPCUA_Reader * r,
    uint16_t * out_ns,
    uint32_t * out_num
) 
```





**Parameters:**


* `r` Reader context. 
* `out_ns` Pointer to receive namespace index. 
* `out_num` Pointer to receive numeric identifier. 



**Returns:**

True on success, false on invalid format or underflow. 





        

<hr>



### function opcua\_read\_string\_buf 

_Read OPC UA String payload into bounded scratch buffer._ 
```C++
static bool opcua_read_string_buf (
    OPCUA_Reader * r,
    char buf,
    bool * out_is_null
) 
```





**Parameters:**


* `r` Binary stream reader. 
* `buf` Destination NUL-terminated scratch character buffer. 
* `out_is_null` Set true if wire string length was -1 (null string). 



**Returns:**

true on success, false on stream underflow or length overflow. 





        

<hr>



### function opcua\_read\_u16 

_Read 16-bit little-endian unsigned integer._ 
```C++
static bool opcua_read_u16 (
    OPCUA_Reader * r,
    uint16_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Pointer to receive value. 



**Returns:**

True on success, false on underflow. 





        

<hr>



### function opcua\_read\_u32 

_Read 32-bit little-endian unsigned integer._ 
```C++
static bool opcua_read_u32 (
    OPCUA_Reader * r,
    uint32_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Pointer to receive value. 



**Returns:**

True on success, false on underflow. 





        

<hr>



### function opcua\_read\_u64 

_Read 64-bit little-endian unsigned integer._ 
```C++
static bool opcua_read_u64 (
    OPCUA_Reader * r,
    uint64_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Pointer to receive value. 



**Returns:**

True on success, false on underflow. 





        

<hr>



### function opcua\_read\_u8 

_Read 8-bit unsigned integer._ 
```C++
static bool opcua_read_u8 (
    OPCUA_Reader * r,
    uint8_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Pointer to receive value. 



**Returns:**

True on success, false on underflow. 





        

<hr>



### function opcua\_read\_variant 

_Read typed OPC UA Variant from binary stream._ 
```C++
static bool opcua_read_variant (
    OPCUA_Reader * r,
    OPCUA_WriteItem * item
) 
```





**Parameters:**


* `r` Binary stream reader. 
* `item` Output decoded WriteValue item receiving the Variant value. 



**Returns:**

true on valid scalar Variant decode, false otherwise. 





        

<hr>



### function opcua\_read\_variant\_int 

_Read integer or boolean OPC UA Variant payload._ 
```C++
static bool opcua_read_variant_int (
    OPCUA_Reader * r,
    SYN_OPCUA_DataType type,
    SYN_OPCUA_Variant * out,
    bool * out_handled
) 
```





**Parameters:**


* `r` Binary stream reader. 
* `type` OPC UA built-in data type identifier. 
* `out` Output Variant structure. 
* `out_handled` Set true if type belongs to integer/boolean family, false otherwise. 



**Returns:**

true on successful decode, false on stream underflow or unhandled type. 





        

<hr>



### function opcua\_reader\_init 

_Initialize OPC UA binary reader._ 
```C++
static void opcua_reader_init (
    OPCUA_Reader * r,
    const uint8_t * buf,
    size_t size
) 
```





**Parameters:**


* `r` Reader context. 
* `buf` Input buffer. 
* `size` Input buffer size. 




        

<hr>



### function opcua\_recv\_rx\_bytes 

_Read available transport bytes into the free tail of srv-&gt;cfg.rx\_buf._ 
```C++
static bool opcua_recv_rx_bytes (
    SYN_OPCUA_Server * srv
) 
```





**Parameters:**


* `srv` OPC UA server instance with non-NULL transport. 



**Returns:**

true if at least one byte was appended to srv-&gt;rx\_pending. 





        

<hr>



### function opcua\_server\_step\_pt\_once 

_Execute one cooperative OPC UA server slice (at most one complete RX frame)._ 
```C++
static SYN_Status opcua_server_step_pt_once (
    SYN_OPCUA_Server * srv,
    uint32_t now_ms,
    bool * out_progressed
) 
```





**Parameters:**


* `srv` Non-NULL OPC UA server instance. 
* `now_ms` Current system timestamp in milliseconds. 
* `out_progressed` [out] Set true when bytes were read or a buffered frame was processed. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM when srv-&gt;cfg.transport is NULL. 





        

<hr>



### function opcua\_service\_browse 

_Encode BrowseResponse listing registered non-root nodes._ 
```C++
static void opcua_service_browse (
    const SYN_OPCUA_Server * srv,
    OPCUA_Writer * w,
    uint32_t req_id
) 
```





**Parameters:**


* `srv` Server instance. 
* `w` Response writer. 
* `req_id` Request identifier. 




        

<hr>



### function opcua\_service\_read 

_Encode ReadResponse payload for an inbound ReadRequest._ 
```C++
static void opcua_service_read (
    SYN_OPCUA_Server * srv,
    OPCUA_Reader * r,
    OPCUA_Writer * w,
    uint32_t req_id
) 
```





**Parameters:**


* `srv` Server instance. 
* `r` Request reader positioned after RequestTypeId. 
* `w` Response writer. 
* `req_id` Request identifier. 




        

<hr>



### function opcua\_service\_write 

_Encode WriteResponse payload for an inbound WriteRequest._ 
```C++
static void opcua_service_write (
    SYN_OPCUA_Server * srv,
    OPCUA_Reader * r,
    OPCUA_Writer * w,
    uint32_t req_id
) 
```





**Parameters:**


* `srv` Server instance. 
* `r` Request reader positioned after RequestTypeId. 
* `w` Response writer. 
* `req_id` Request identifier. 




        

<hr>



### function opcua\_skip 

_Advance reader by count bytes with bounds checking._ 
```C++
static bool opcua_skip (
    OPCUA_Reader * r,
    size_t count
) 
```





**Parameters:**


* `r` Reader context. 
* `count` Number of bytes to skip. 



**Returns:**

True on success, false on underflow. 





        

<hr>



### function opcua\_write\_bytes 

_Write raw byte buffer._ 
```C++
static bool opcua_write_bytes (
    OPCUA_Writer * w,
    const uint8_t * buf,
    size_t len
) 
```





**Parameters:**


* `w` Writer context. 
* `buf` Source bytes. 
* `len` Number of bytes. 



**Returns:**

True on success, false on overflow. 





        

<hr>



### function opcua\_write\_datavalue 

_Write OPC UA DataValue structure._ 
```C++
static bool opcua_write_datavalue (
    OPCUA_Writer * w,
    const SYN_OPCUA_DataValue * dv
) 
```





**Parameters:**


* `w` Writer context. 
* `dv` DataValue structure pointer. 



**Returns:**

True on success, false on error. 





        

<hr>



### function opcua\_write\_nodeid 

_Write numeric or string OPC UA NodeId._ 
```C++
static bool opcua_write_nodeid (
    OPCUA_Writer * w,
    const SYN_OPCUA_NodeId * id
) 
```





**Parameters:**


* `w` Writer context. 
* `id` NodeId descriptor to encode. 



**Returns:**

True on success, false on overflow. 





        

<hr>



### function opcua\_write\_nodeid\_num 

_Write numeric OPC UA NodeId._ 
```C++
static bool opcua_write_nodeid_num (
    OPCUA_Writer * w,
    uint16_t ns,
    uint32_t num
) 
```





**Parameters:**


* `w` Writer context. 
* `ns` Namespace index. 
* `num` Numeric identifier. 



**Returns:**

True on success, false on overflow. 





        

<hr>



### function opcua\_write\_service\_fault 

_Encode OPC UA ServiceFault response body._ 
```C++
static void opcua_write_service_fault (
    OPCUA_Writer * w,
    uint32_t req_id,
    uint32_t status_code
) 
```





**Parameters:**


* `w` Binary response writer. 
* `req_id` Client request identifier. 
* `status_code` Fault OPC UA StatusCode. 




        

<hr>



### function opcua\_write\_string 

_Write length-prefixed OPC UA string._ 
```C++
static bool opcua_write_string (
    OPCUA_Writer * w,
    const char * str
) 
```





**Parameters:**


* `w` Writer context. 
* `str` Null-terminated string or NULL for null string. 



**Returns:**

True on success, false on overflow. 





        

<hr>



### function opcua\_write\_u16 

_Write 16-bit little-endian unsigned integer._ 
```C++
static bool opcua_write_u16 (
    OPCUA_Writer * w,
    uint16_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value to write. 



**Returns:**

True on success, false on overflow. 





        

<hr>



### function opcua\_write\_u32 

_Write 32-bit little-endian unsigned integer._ 
```C++
static bool opcua_write_u32 (
    OPCUA_Writer * w,
    uint32_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value to write. 



**Returns:**

True on success, false on overflow. 





        

<hr>



### function opcua\_write\_u64 

_Write 64-bit little-endian unsigned integer._ 
```C++
static bool opcua_write_u64 (
    OPCUA_Writer * w,
    uint64_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value to write. 



**Returns:**

True on success, false on overflow. 





        

<hr>



### function opcua\_write\_u8 

_Write 8-bit unsigned integer._ 
```C++
static bool opcua_write_u8 (
    OPCUA_Writer * w,
    uint8_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value to write. 



**Returns:**

True on success, false on overflow. 





        

<hr>



### function opcua\_write\_variant 

_Write OPC UA variant value._ 
```C++
static bool opcua_write_variant (
    OPCUA_Writer * w,
    const SYN_OPCUA_Variant * var
) 
```





**Parameters:**


* `w` Writer context. 
* `var` Variant structure pointer. 



**Returns:**

True on success, false on serialization error. 





        

<hr>



### function opcua\_write\_variant\_int 

_Write integer or boolean OPC UA variant payload._ 
```C++
static bool opcua_write_variant_int (
    OPCUA_Writer * w,
    const SYN_OPCUA_Variant * var,
    bool * out_wrote
) 
```





**Parameters:**


* `w` Writer context. 
* `var` Variant structure pointer. 
* `out_wrote` Set true if var-&gt;type was handled by this helper. 



**Returns:**

True on success, false on serialization overflow. 





        

<hr>



### function opcua\_writer\_init 

_Initialize OPC UA binary writer._ 
```C++
static void opcua_writer_init (
    OPCUA_Writer * w,
    uint8_t * buf,
    size_t size
) 
```





**Parameters:**


* `w` Writer context. 
* `buf` Output buffer. 
* `size` Output buffer size. 




        

<hr>
## Macro Definition Documentation





### define SYN\_OPCUA\_REQ\_ACTIVATE\_SESSION 

_OPC UA ActivateSessionRequest NodeId._ 
```C++
#define SYN_OPCUA_REQ_ACTIVATE_SESSION `467U`
```




<hr>



### define SYN\_OPCUA\_REQ\_BROWSE 

_OPC UA BrowseRequest NodeId._ 
```C++
#define SYN_OPCUA_REQ_BROWSE `527U`
```




<hr>



### define SYN\_OPCUA\_REQ\_CLOSE\_SESSION 

_OPC UA CloseSessionRequest NodeId._ 
```C++
#define SYN_OPCUA_REQ_CLOSE_SESSION `473U`
```




<hr>



### define SYN\_OPCUA\_REQ\_CREATE\_SESSION 

_OPC UA CreateSessionRequest NodeId._ 
```C++
#define SYN_OPCUA_REQ_CREATE_SESSION `461U`
```




<hr>



### define SYN\_OPCUA\_REQ\_READ 

_OPC UA ReadRequest NodeId._ 
```C++
#define SYN_OPCUA_REQ_READ `631U`
```




<hr>



### define SYN\_OPCUA\_REQ\_WRITE 

_OPC UA WriteRequest NodeId._ 
```C++
#define SYN_OPCUA_REQ_WRITE `673U`
```




<hr>



### define SYN\_OPCUA\_RESP\_ACTIVATE\_SESSION 

_OPC UA ActivateSessionResponse NodeId._ 
```C++
#define SYN_OPCUA_RESP_ACTIVATE_SESSION `470U`
```




<hr>



### define SYN\_OPCUA\_RESP\_BROWSE 

_OPC UA BrowseResponse NodeId._ 
```C++
#define SYN_OPCUA_RESP_BROWSE `530U`
```




<hr>



### define SYN\_OPCUA\_RESP\_CLOSE\_SESSION 

_OPC UA CloseSessionResponse NodeId._ 
```C++
#define SYN_OPCUA_RESP_CLOSE_SESSION `476U`
```




<hr>



### define SYN\_OPCUA\_RESP\_CREATE\_SESSION 

_OPC UA CreateSessionResponse NodeId._ 
```C++
#define SYN_OPCUA_RESP_CREATE_SESSION `464U`
```




<hr>



### define SYN\_OPCUA\_RESP\_READ 

_OPC UA ReadResponse NodeId._ 
```C++
#define SYN_OPCUA_RESP_READ `634U`
```




<hr>



### define SYN\_OPCUA\_RESP\_SERVICE\_FAULT 

_OPC UA ServiceFault NodeId._ 
```C++
#define SYN_OPCUA_RESP_SERVICE_FAULT `397U`
```




<hr>



### define SYN\_OPCUA\_RESP\_WRITE 

_OPC UA WriteResponse NodeId._ 
```C++
#define SYN_OPCUA_RESP_WRITE `676U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.c`

