

# File syn\_opcua.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_opcua.c**](syn__opcua_8c.md)

[Go to the source code of this file](syn__opcua_8c_source.md)

_Micro OPC UA (IEC 62541) Binary Protocol Engine & Embedded Server Implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "syn_opcua.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**OPCUA\_Reader**](structOPCUA__Reader.md) <br>_Binary deserializer context for OPC UA messages._  |
| struct | [**OPCUA\_Writer**](structOPCUA__Writer.md) <br>_Binary serializer context for OPC UA messages._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_OPCUA\_Node**](structSYN__OPCUA__Node.md) \* | [**syn\_opcua\_server\_find\_node\_num**](#function-syn_opcua_server_find_node_num) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, uint16\_t ns, uint32\_t num\_id) <br>_Find a node in the address space by numeric NodeId._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_init**](#function-syn_opcua_server_init) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, const [**SYN\_OPCUA\_Config**](structSYN__OPCUA__Config.md) \* cfg) <br>_Initialize Micro OPC UA Server._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_process\_message**](#function-syn_opcua_server_process_message) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, const uint8\_t \* rx, size\_t rx\_len, uint8\_t \* tx, size\_t max\_tx, size\_t \* out\_len) <br>_Process an incoming raw OPC UA frame and generate a response frame._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_opcua\_server\_pt**](#function-syn_opcua_server_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Protothread continuation function for_ `syn_sched` _._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_read\_variable**](#function-syn_opcua_server_read_variable) (const [**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, uint16\_t ns, uint32\_t num\_id, [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) \* out\_val) <br>_Read a variable node's value from the address space._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_register\_node**](#function-syn_opcua_server_register_node) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, const [**SYN\_OPCUA\_Node**](structSYN__OPCUA__Node.md) \* node) <br>_Register a Node in the server address space._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_step**](#function-syn_opcua_server_step) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, uint32\_t now\_ms) <br>_Advance OPC UA server state machine by one step / tick._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_opcua\_server\_write\_variable**](#function-syn_opcua_server_write_variable) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv, uint16\_t ns, uint32\_t num\_id, const [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) \* val) <br>_Update a variable node's value in the address space._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**opcua\_populate\_standard\_nodes**](#function-opcua_populate_standard_nodes) ([**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) \* srv) <br>_Populate default standard OPC UA address space folders and server status node._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_nodeid\_num**](#function-opcua_read_nodeid_num) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, uint16\_t \* out\_ns, uint32\_t \* out\_num) <br>_Read numeric OPC UA NodeId._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_u16**](#function-opcua_read_u16) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, uint16\_t \* out) <br>_Read 16-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_u32**](#function-opcua_read_u32) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, uint32\_t \* out) <br>_Read 32-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_read\_u8**](#function-opcua_read_u8) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, uint8\_t \* out) <br>_Read 8-bit unsigned integer._  |
|  void | [**opcua\_reader\_init**](#function-opcua_reader_init) ([**OPCUA\_Reader**](structOPCUA__Reader.md) \* r, const uint8\_t \* buf, size\_t size) <br>_Initialize OPC UA binary reader._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_bytes**](#function-opcua_write_bytes) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, const uint8\_t \* buf, size\_t len) <br>_Write raw byte buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_datavalue**](#function-opcua_write_datavalue) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, const [**SYN\_OPCUA\_DataValue**](structSYN__OPCUA__DataValue.md) \* dv) <br>_Write OPC UA DataValue structure._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_nodeid\_num**](#function-opcua_write_nodeid_num) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint16\_t ns, uint32\_t num) <br>_Write numeric OPC UA NodeId._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_string**](#function-opcua_write_string) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, const char \* str) <br>_Write length-prefixed OPC UA string._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_u16**](#function-opcua_write_u16) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint16\_t v) <br>_Write 16-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_u32**](#function-opcua_write_u32) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint32\_t v) <br>_Write 32-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_u64**](#function-opcua_write_u64) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint64\_t v) <br>_Write 64-bit little-endian unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_u8**](#function-opcua_write_u8) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, uint8\_t v) <br>_Write 8-bit unsigned integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**opcua\_write\_variant**](#function-opcua_write_variant) ([**OPCUA\_Writer**](structOPCUA__Writer.md) \* w, const [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) \* var) <br>_Write OPC UA variant value._  |
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

_Protothread continuation function for_ `syn_sched` _._
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

SYN\_PT\_Status continuation status. 





        

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



### define SYN\_OPCUA\_RESP\_WRITE 

_OPC UA WriteResponse NodeId._ 
```C++
#define SYN_OPCUA_RESP_WRITE `676U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.c`

