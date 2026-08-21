

# File syn\_opcua.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_opcua.h**](syn__opcua_8h.md)

[Go to the source code of this file](syn__opcua_8h_source.md)

_Micro OPC UA (IEC 62541) Binary Protocol Engine & Embedded Server._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../net/syn_transport.h"`
* `#include "../pt/syn_pt.h"`
* `#include "../sched/syn_sched.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_OPCUA\_Config**](structSYN__OPCUA__Config.md) <br>_Configuration parameters for Micro OPC UA Server._  |
| struct | [**SYN\_OPCUA\_DataValue**](structSYN__OPCUA__DataValue.md) <br>_DataValue with value, status code, and source timestamp._  |
| struct | [**SYN\_OPCUA\_Node**](structSYN__OPCUA__Node.md) <br>_In-memory OPC UA Node representation._  |
| struct | [**SYN\_OPCUA\_NodeId**](structSYN__OPCUA__NodeId.md) <br>_OPC UA NodeId._  |
| struct | [**SYN\_OPCUA\_Server**](structSYN__OPCUA__Server.md) <br>_Micro OPC UA Server Instance Context._  |
| struct | [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) <br>_OPC UA Variant containing dynamic value._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_OPCUA\_DataType**](#enum-syn_opcua_datatype)  <br>_Built-in DataType enum (per Part 6)._  |
| enum  | [**SYN\_OPCUA\_NodeClass**](#enum-syn_opcua_nodeclass)  <br>_NodeClass enum._  |
| enum  | [**SYN\_OPCUA\_NodeIdType**](#enum-syn_opcua_nodeidtype)  <br>_NodeId identifier type._  |
| typedef [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**SYN\_OPCUA\_ReadCb**](#typedef-syn_opcua_readcb)  <br>_Dynamic callback invoked when a variable node is read._  |
| enum  | [**SYN\_OPCUA\_ServerState**](#enum-syn_opcua_serverstate)  <br>_Server connection and session state._  |
| typedef [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**SYN\_OPCUA\_WriteCb**](#typedef-syn_opcua_writecb)  <br>_Dynamic callback invoked when a variable node is written._  |




















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_OPCUA\_ATTR\_ACCESS\_LEVEL**](syn__opcua_8h.md#define-syn_opcua_attr_access_level)  `17U`<br> |
| define  | [**SYN\_OPCUA\_ATTR\_BROWSE\_NAME**](syn__opcua_8h.md#define-syn_opcua_attr_browse_name)  `3U`<br> |
| define  | [**SYN\_OPCUA\_ATTR\_DATA\_TYPE**](syn__opcua_8h.md#define-syn_opcua_attr_data_type)  `14U`<br> |
| define  | [**SYN\_OPCUA\_ATTR\_DISPLAY\_NAME**](syn__opcua_8h.md#define-syn_opcua_attr_display_name)  `4U`<br> |
| define  | [**SYN\_OPCUA\_ATTR\_NODE\_CLASS**](syn__opcua_8h.md#define-syn_opcua_attr_node_class)  `2U`<br> |
| define  | [**SYN\_OPCUA\_ATTR\_NODE\_ID**](syn__opcua_8h.md#define-syn_opcua_attr_node_id)  `1U`<br> |
| define  | [**SYN\_OPCUA\_ATTR\_VALUE**](syn__opcua_8h.md#define-syn_opcua_attr_value)  `13U`<br> |
| define  | [**SYN\_OPCUA\_ATTR\_VALUE\_RANK**](syn__opcua_8h.md#define-syn_opcua_attr_value_rank)  `15U`<br> |
| define  | [**SYN\_OPCUA\_DEFAULT\_BUFFER\_SIZE**](syn__opcua_8h.md#define-syn_opcua_default_buffer_size)  `1024U`<br> |
| define  | [**SYN\_OPCUA\_DEFAULT\_PORT**](syn__opcua_8h.md#define-syn_opcua_default_port)  `4840U`<br> |
| define  | [**SYN\_OPCUA\_MAX\_NODES**](syn__opcua_8h.md#define-syn_opcua_max_nodes)  `32U`<br> |
| define  | [**SYN\_OPCUA\_MAX\_REFERENCES**](syn__opcua_8h.md#define-syn_opcua_max_references)  `8U`<br> |
| define  | [**SYN\_OPCUA\_MAX\_STRING\_LEN**](syn__opcua_8h.md#define-syn_opcua_max_string_len)  `64U`<br> |
| define  | [**SYN\_OPCUA\_NODEID\_OBJECTS\_FOLDER**](syn__opcua_8h.md#define-syn_opcua_nodeid_objects_folder)  `85U`<br> |
| define  | [**SYN\_OPCUA\_NODEID\_ROOT**](syn__opcua_8h.md#define-syn_opcua_nodeid_root)  `84U`<br> |
| define  | [**SYN\_OPCUA\_NODEID\_SERVER**](syn__opcua_8h.md#define-syn_opcua_nodeid_server)  `2253U`<br> |
| define  | [**SYN\_OPCUA\_NODEID\_SERVER\_SERVERSTATUS**](syn__opcua_8h.md#define-syn_opcua_nodeid_server_serverstatus)  `2256U`<br> |
| define  | [**SYN\_OPCUA\_NS\_STANDARD**](syn__opcua_8h.md#define-syn_opcua_ns_standard)  `0U`<br> |
| define  | [**SYN\_OPCUA\_NS\_VENDOR**](syn__opcua_8h.md#define-syn_opcua_ns_vendor)  `1U`<br> |
| define  | [**SYN\_OPCUA\_PROTOCOL\_VERSION**](syn__opcua_8h.md#define-syn_opcua_protocol_version)  `0U`<br> |
| define  | [**SYN\_OPCUA\_STATUS\_BAD\_ATTRIBUTE\_ID\_INVALID**](syn__opcua_8h.md#define-syn_opcua_status_bad_attribute_id_invalid)  `0x80350000U`<br> |
| define  | [**SYN\_OPCUA\_STATUS\_BAD\_INTERNAL\_ERROR**](syn__opcua_8h.md#define-syn_opcua_status_bad_internal_error)  `0x80020000U`<br> |
| define  | [**SYN\_OPCUA\_STATUS\_BAD\_NODE\_ID\_UNKNOWN**](syn__opcua_8h.md#define-syn_opcua_status_bad_node_id_unknown)  `0x80340000U`<br> |
| define  | [**SYN\_OPCUA\_STATUS\_BAD\_NOT\_READABLE**](syn__opcua_8h.md#define-syn_opcua_status_bad_not_readable)  `0x803A0000U`<br> |
| define  | [**SYN\_OPCUA\_STATUS\_BAD\_NOT\_WRITABLE**](syn__opcua_8h.md#define-syn_opcua_status_bad_not_writable)  `0x803B0000U`<br> |
| define  | [**SYN\_OPCUA\_STATUS\_BAD\_SECURITY\_CHECKS\_FAILED**](syn__opcua_8h.md#define-syn_opcua_status_bad_security_checks_failed)  `0x80130000U`<br> |
| define  | [**SYN\_OPCUA\_STATUS\_BAD\_SERVICE\_UNSUPPORTED**](syn__opcua_8h.md#define-syn_opcua_status_bad_service_unsupported)  `0x80430000U`<br> |
| define  | [**SYN\_OPCUA\_STATUS\_BAD\_UNEXPECTED**](syn__opcua_8h.md#define-syn_opcua_status_bad_unexpected)  `0x80010000U`<br> |
| define  | [**SYN\_OPCUA\_STATUS\_GOOD**](syn__opcua_8h.md#define-syn_opcua_status_good)  `0x00000000U`<br> |
| define  | [**SYN\_OPCUA\_STATUS\_UNCERTAIN**](syn__opcua_8h.md#define-syn_opcua_status_uncertain)  `0x40000000U`<br> |

## Detailed Description


Implements a zero-heap, deterministic OPC UA Binary protocol engine:
* UACP (OPC UA Connection Protocol) framing (`HEL`, `ACK`, `ERR`).
* UASC (OPC UA Secure Conversation) framing (`OPN`, `CLO`, `MSG`) with Security Policy None & Basic256Sha256.
* Static In-Memory Address Space: Object, Variable, and Method nodes with numeric & string NodeIds.
* Core Services: Read, Write, Browse, CreateSession, ActivateSession, CreateSubscription, Publish.
* Non-blocking poll step API and cooperative protothread continuation for `syn_sched`. 




    
## Public Types Documentation




### enum SYN\_OPCUA\_DataType 

_Built-in DataType enum (per Part 6)._ 
```C++
enum SYN_OPCUA_DataType {
    SYN_OPCUA_TYPE_NULL = 0,
    SYN_OPCUA_TYPE_BOOLEAN = 1,
    SYN_OPCUA_TYPE_SBYTE = 2,
    SYN_OPCUA_TYPE_BYTE = 3,
    SYN_OPCUA_TYPE_INT16 = 4,
    SYN_OPCUA_TYPE_UINT16 = 5,
    SYN_OPCUA_TYPE_INT32 = 6,
    SYN_OPCUA_TYPE_UINT32 = 7,
    SYN_OPCUA_TYPE_INT64 = 8,
    SYN_OPCUA_TYPE_UINT64 = 9,
    SYN_OPCUA_TYPE_FLOAT = 10,
    SYN_OPCUA_TYPE_DOUBLE = 11,
    SYN_OPCUA_TYPE_STRING = 12,
    SYN_OPCUA_TYPE_DATETIME = 13
};
```




<hr>



### enum SYN\_OPCUA\_NodeClass 

_NodeClass enum._ 
```C++
enum SYN_OPCUA_NodeClass {
    SYN_OPCUA_NODECLASS_UNSPECIFIED = 0,
    SYN_OPCUA_NODECLASS_OBJECT = 1,
    SYN_OPCUA_NODECLASS_VARIABLE = 2,
    SYN_OPCUA_NODECLASS_METHOD = 4
};
```




<hr>



### enum SYN\_OPCUA\_NodeIdType 

_NodeId identifier type._ 
```C++
enum SYN_OPCUA_NodeIdType {
    SYN_OPCUA_NODEID_NUMERIC = 0,
    SYN_OPCUA_NODEID_STRING = 1
};
```




<hr>



### typedef SYN\_OPCUA\_ReadCb 

_Dynamic callback invoked when a variable node is read._ 
```C++
typedef SYN_Status(* SYN_OPCUA_ReadCb) (struct SYN_OPCUA_Server *srv, const struct SYN_OPCUA_Node *node, SYN_OPCUA_DataValue *out_val, void *user_data);
```




<hr>



### enum SYN\_OPCUA\_ServerState 

_Server connection and session state._ 
```C++
enum SYN_OPCUA_ServerState {
    SYN_OPCUA_STATE_CLOSED = 0,
    SYN_OPCUA_STATE_HELLO_RECEIVED,
    SYN_OPCUA_STATE_SECURE_CHANNEL,
    SYN_OPCUA_STATE_SESSION_ACTIVE
};
```




<hr>



### typedef SYN\_OPCUA\_WriteCb 

_Dynamic callback invoked when a variable node is written._ 
```C++
typedef SYN_Status(* SYN_OPCUA_WriteCb) (struct SYN_OPCUA_Server *srv, const struct SYN_OPCUA_Node *node, const SYN_OPCUA_DataValue *in_val, void *user_data);
```




<hr>
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
## Macro Definition Documentation





### define SYN\_OPCUA\_ATTR\_ACCESS\_LEVEL 

```C++
#define SYN_OPCUA_ATTR_ACCESS_LEVEL `17U`
```



AccessLevel attribute 


        

<hr>



### define SYN\_OPCUA\_ATTR\_BROWSE\_NAME 

```C++
#define SYN_OPCUA_ATTR_BROWSE_NAME `3U`
```



BrowseName attribute 


        

<hr>



### define SYN\_OPCUA\_ATTR\_DATA\_TYPE 

```C++
#define SYN_OPCUA_ATTR_DATA_TYPE `14U`
```



DataType attribute 


        

<hr>



### define SYN\_OPCUA\_ATTR\_DISPLAY\_NAME 

```C++
#define SYN_OPCUA_ATTR_DISPLAY_NAME `4U`
```



DisplayName attribute 


        

<hr>



### define SYN\_OPCUA\_ATTR\_NODE\_CLASS 

```C++
#define SYN_OPCUA_ATTR_NODE_CLASS `2U`
```



NodeClass attribute 


        

<hr>



### define SYN\_OPCUA\_ATTR\_NODE\_ID 

```C++
#define SYN_OPCUA_ATTR_NODE_ID `1U`
```



NodeId attribute 


        

<hr>



### define SYN\_OPCUA\_ATTR\_VALUE 

```C++
#define SYN_OPCUA_ATTR_VALUE `13U`
```



Value attribute 


        

<hr>



### define SYN\_OPCUA\_ATTR\_VALUE\_RANK 

```C++
#define SYN_OPCUA_ATTR_VALUE_RANK `15U`
```



ValueRank attribute 


        

<hr>



### define SYN\_OPCUA\_DEFAULT\_BUFFER\_SIZE 

```C++
#define SYN_OPCUA_DEFAULT_BUFFER_SIZE `1024U`
```



Default Chunk Buffer Size 


        

<hr>



### define SYN\_OPCUA\_DEFAULT\_PORT 

```C++
#define SYN_OPCUA_DEFAULT_PORT `4840U`
```



Standard OPC UA TCP Port 


        

<hr>



### define SYN\_OPCUA\_MAX\_NODES 

```C++
#define SYN_OPCUA_MAX_NODES `32U`
```



Static Node Table Capacity 


        

<hr>



### define SYN\_OPCUA\_MAX\_REFERENCES 

```C++
#define SYN_OPCUA_MAX_REFERENCES `8U`
```



Max References per Node 


        

<hr>



### define SYN\_OPCUA\_MAX\_STRING\_LEN 

```C++
#define SYN_OPCUA_MAX_STRING_LEN `64U`
```



Max Identifier & Name String Length 


        

<hr>



### define SYN\_OPCUA\_NODEID\_OBJECTS\_FOLDER 

```C++
#define SYN_OPCUA_NODEID_OBJECTS_FOLDER `85U`
```



ObjectsFolder NodeId 


        

<hr>



### define SYN\_OPCUA\_NODEID\_ROOT 

```C++
#define SYN_OPCUA_NODEID_ROOT `84U`
```



RootFolder NodeId 


        

<hr>



### define SYN\_OPCUA\_NODEID\_SERVER 

```C++
#define SYN_OPCUA_NODEID_SERVER `2253U`
```



Server Object NodeId 


        

<hr>



### define SYN\_OPCUA\_NODEID\_SERVER\_SERVERSTATUS 

```C++
#define SYN_OPCUA_NODEID_SERVER_SERVERSTATUS `2256U`
```



ServerStatus Variable NodeId 


        

<hr>



### define SYN\_OPCUA\_NS\_STANDARD 

```C++
#define SYN_OPCUA_NS_STANDARD `0U`
```



OPC UA standard namespace index 


        

<hr>



### define SYN\_OPCUA\_NS\_VENDOR 

```C++
#define SYN_OPCUA_NS_VENDOR `1U`
```



Vendor custom namespace index 


        

<hr>



### define SYN\_OPCUA\_PROTOCOL\_VERSION 

```C++
#define SYN_OPCUA_PROTOCOL_VERSION `0U`
```



OPC UA Binary Protocol Version 


        

<hr>



### define SYN\_OPCUA\_STATUS\_BAD\_ATTRIBUTE\_ID\_INVALID 

```C++
#define SYN_OPCUA_STATUS_BAD_ATTRIBUTE_ID_INVALID `0x80350000U`
```



AttributeId is invalid 


        

<hr>



### define SYN\_OPCUA\_STATUS\_BAD\_INTERNAL\_ERROR 

```C++
#define SYN_OPCUA_STATUS_BAD_INTERNAL_ERROR `0x80020000U`
```



Internal error occurred 


        

<hr>



### define SYN\_OPCUA\_STATUS\_BAD\_NODE\_ID\_UNKNOWN 

```C++
#define SYN_OPCUA_STATUS_BAD_NODE_ID_UNKNOWN `0x80340000U`
```



NodeId is unknown in address space \ 


        

<hr>



### define SYN\_OPCUA\_STATUS\_BAD\_NOT\_READABLE 

```C++
#define SYN_OPCUA_STATUS_BAD_NOT_READABLE `0x803A0000U`
```



Node attribute is not readable 


        

<hr>



### define SYN\_OPCUA\_STATUS\_BAD\_NOT\_WRITABLE 

```C++
#define SYN_OPCUA_STATUS_BAD_NOT_WRITABLE `0x803B0000U`
```



Node attribute is not writable 


        

<hr>



### define SYN\_OPCUA\_STATUS\_BAD\_SECURITY\_CHECKS\_FAILED 

```C++
#define SYN_OPCUA_STATUS_BAD_SECURITY_CHECKS_FAILED `0x80130000U`
```



Security verification failed \ 


        

<hr>



### define SYN\_OPCUA\_STATUS\_BAD\_SERVICE\_UNSUPPORTED 

```C++
#define SYN_OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED `0x80430000U`
```



Requested service is unsupported 


        

<hr>



### define SYN\_OPCUA\_STATUS\_BAD\_UNEXPECTED 

```C++
#define SYN_OPCUA_STATUS_BAD_UNEXPECTED `0x80010000U`
```



Unexpected error 


        

<hr>



### define SYN\_OPCUA\_STATUS\_GOOD 

```C++
#define SYN_OPCUA_STATUS_GOOD `0x00000000U`
```



Operation succeeded 


        

<hr>



### define SYN\_OPCUA\_STATUS\_UNCERTAIN 

```C++
#define SYN_OPCUA_STATUS_UNCERTAIN `0x40000000U`
```



Value is uncertain 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.h`

