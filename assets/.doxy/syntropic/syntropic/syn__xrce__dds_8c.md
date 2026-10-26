

# File syn\_xrce\_dds.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_xrce\_dds.c**](syn__xrce__dds_8c.md)

[Go to the source code of this file](syn__xrce__dds_8c_source.md)

_Micro-ROS & eProsima Micro XRCE-DDS v1.2 Protocol Client Engine Implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "syn_xrce_dds.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_bool**](#function-syn_cdr_read_bool) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, [**bool**](syn__defs_8h.md#enum-bool) \* out) <br>_Read boolean._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_bytes**](#function-syn_cdr_read_bytes) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, uint8\_t \* out\_buf, size\_t len) <br>_Read raw byte buffer without alignment padding._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_double**](#function-syn_cdr_read_double) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, double \* out) <br>_Read 64-bit double (8-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_float**](#function-syn_cdr_read_float) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, float \* out) <br>_Read 32-bit float (4-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_i16**](#function-syn_cdr_read_i16) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, int16\_t \* out) <br>_Read signed 16-bit integer (2-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_i32**](#function-syn_cdr_read_i32) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, int32\_t \* out) <br>_Read signed 32-bit integer (4-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_i64**](#function-syn_cdr_read_i64) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, int64\_t \* out) <br>_Read signed 64-bit integer (8-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_i8**](#function-syn_cdr_read_i8) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, int8\_t \* out) <br>_Read signed 8-bit integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_string**](#function-syn_cdr_read_string) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, char \* out\_buf, size\_t max\_len) <br>_Read null-terminated string with length prefix._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_u16**](#function-syn_cdr_read_u16) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, uint16\_t \* out) <br>_Read unsigned 16-bit integer (2-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_u32**](#function-syn_cdr_read_u32) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, uint32\_t \* out) <br>_Read unsigned 32-bit integer (4-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_u64**](#function-syn_cdr_read_u64) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, uint64\_t \* out) <br>_Read unsigned 64-bit integer (8-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_read\_u8**](#function-syn_cdr_read_u8) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, uint8\_t \* out) <br>_Read unsigned 8-bit integer._  |
|  void | [**syn\_cdr\_reader\_init**](#function-syn_cdr_reader_init) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, const uint8\_t \* buf, size\_t size) <br>_Initialize CDR reader._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_bool**](#function-syn_cdr_write_bool) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, [**bool**](syn__defs_8h.md#enum-bool) v) <br>_Write boolean._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_bytes**](#function-syn_cdr_write_bytes) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, const uint8\_t \* bytes, size\_t len) <br>_Write raw byte buffer without alignment padding._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_double**](#function-syn_cdr_write_double) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, double v) <br>_Write 64-bit double (8-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_float**](#function-syn_cdr_write_float) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, float v) <br>_Write 32-bit float (4-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_i16**](#function-syn_cdr_write_i16) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, int16\_t v) <br>_Write signed 16-bit integer (2-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_i32**](#function-syn_cdr_write_i32) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, int32\_t v) <br>_Write signed 32-bit integer (4-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_i64**](#function-syn_cdr_write_i64) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, int64\_t v) <br>_Write signed 64-bit integer (8-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_i8**](#function-syn_cdr_write_i8) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, int8\_t v) <br>_Write signed 8-bit integer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_string**](#function-syn_cdr_write_string) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, const char \* str) <br>_Write null-terminated string with 32-bit length prefix and null terminator._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_u16**](#function-syn_cdr_write_u16) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, uint16\_t v) <br>_Write unsigned 16-bit integer (2-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_u32**](#function-syn_cdr_write_u32) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, uint32\_t v) <br>_Write unsigned 32-bit integer (4-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_u64**](#function-syn_cdr_write_u64) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, uint64\_t v) <br>_Write unsigned 64-bit integer (8-byte aligned)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cdr\_write\_u8**](#function-syn_cdr_write_u8) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, uint8\_t v) <br>_Write unsigned 8-bit integer._  |
|  void | [**syn\_cdr\_writer\_init**](#function-syn_cdr_writer_init) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, uint8\_t \* buf, size\_t size) <br>_Initialize CDR writer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_create\_datareader**](#function-syn_xrce_client_create_datareader) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, uint16\_t reader\_id, uint16\_t subscriber\_id, uint16\_t topic\_id) <br>_Create a DDS DataReader entity._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_create\_datawriter**](#function-syn_xrce_client_create_datawriter) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, uint16\_t writer\_id, uint16\_t publisher\_id, uint16\_t topic\_id) <br>_Create a DDS DataWriter entity._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_create\_participant**](#function-syn_xrce_client_create_participant) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, uint16\_t participant\_id) <br>_Create a DDS DomainParticipant entity._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_create\_publisher**](#function-syn_xrce_client_create_publisher) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, uint16\_t publisher\_id, uint16\_t participant\_id) <br>_Create a DDS Publisher entity._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_create\_session**](#function-syn_xrce_client_create_session) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client) <br>_Connect and establish session with Micro-ROS Agent._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_create\_subscriber**](#function-syn_xrce_client_create_subscriber) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, uint16\_t subscriber\_id, uint16\_t participant\_id) <br>_Create a DDS Subscriber entity._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_create\_topic**](#function-syn_xrce_client_create_topic) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, uint16\_t topic\_id, uint16\_t participant\_id, const char \* topic\_name, const char \* type\_name) <br>_Create a DDS Topic entity._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_init**](#function-syn_xrce_client_init) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, const [**SYN\_XRCE\_Config**](structSYN__XRCE__Config.md) \* cfg) <br>_Initialize Micro XRCE-DDS Client._  |
|  [**SYN\_PT\_Status**](syn__pt_8h.md#enum-syn_pt_status) | [**syn\_xrce\_client\_pt**](#function-syn_xrce_client_pt) ([**SYN\_PT**](structSYN__PT.md) \* pt, [**SYN\_Task**](structSYN__Task.md) \* task) <br>_Protothread continuation function for_ `syn_sched` _._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_read\_data**](#function-syn_xrce_client_read_data) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, uint16\_t reader\_id) <br>_Request incoming topic samples from an active DataReader._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_step**](#function-syn_xrce_client_step) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, uint32\_t now\_ms) <br>_Advance the XRCE-DDS client state machine by one step / tick._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_xrce\_client\_write\_data**](#function-syn_xrce_client_write_data) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, uint16\_t writer\_id, const uint8\_t \* data, size\_t len) <br>_Publish CDR serialized data payload to a DataWriter._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**cdr\_pad\_reader**](#function-cdr_pad_reader) ([**SYN\_CDR\_Reader**](structSYN__CDR__Reader.md) \* r, size\_t align) <br>_Align CDR reader position to specified byte boundary._  |
|  size\_t | [**cdr\_pad\_writer**](#function-cdr_pad_writer) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, size\_t align) <br>_Align CDR writer position to specified byte boundary with zero padding._  |
|  void | [**xrce\_finish\_submessage**](#function-xrce_finish_submessage) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, size\_t len\_pos) <br>_Finish XRCE submessage and patch calculated payload length._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**xrce\_register\_local\_object**](#function-xrce_register_local_object) ([**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) \* client, uint16\_t id, uint8\_t kind, uint16\_t parent) <br>_Register local XRCE object in client's internal entity table._  |
|  size\_t | [**xrce\_start\_submessage**](#function-xrce_start_submessage) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, uint8\_t submsg\_id, uint8\_t flags) <br>_Start XRCE submessage with 4-byte header and return length patch offset._  |
|  void | [**xrce\_write\_header**](#function-xrce_write_header) ([**SYN\_CDR\_Writer**](structSYN__CDR__Writer.md) \* w, uint8\_t session\_id, uint8\_t stream\_id, uint16\_t seq\_num, uint32\_t client\_key) <br>_Write XRCE message header to CDR stream._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_XRCE\_DATA\_FORMAT\_DATA**](syn__xrce__dds_8c.md#define-syn_xrce_data_format_data)  `0x00U`<br>_XRCE data submessage format: Raw Data payload._  |
| define  | [**SYN\_XRCE\_DEFAULT\_HEARTBEAT\_MS**](syn__xrce__dds_8c.md#define-syn_xrce_default_heartbeat_ms)  `5000U`<br>_Default client heartbeat period in milliseconds._  |
| define  | [**SYN\_XRCE\_FLAG\_LITTLE\_ENDIAN**](syn__xrce__dds_8c.md#define-syn_xrce_flag_little_endian)  `0x01U`<br>_XRCE submessage little-endian endianness flag._  |
| define  | [**SYN\_XRCE\_REPRESENTATION\_AS\_STRING**](syn__xrce__dds_8c.md#define-syn_xrce_representation_as_string)  `0x02U`<br>_XRCE object representation as string / XML._  |
| define  | [**SYN\_XRCE\_REPRESENTATION\_BY\_REF**](syn__xrce__dds_8c.md#define-syn_xrce_representation_by_ref)  `0x01U`<br>_XRCE object representation by reference._  |

## Public Functions Documentation




### function syn\_cdr\_read\_bool 

_Read boolean._ 
```C++
bool syn_cdr_read_bool (
    SYN_CDR_Reader * r,
    bool * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_bytes 

_Read raw byte buffer without alignment padding._ 
```C++
bool syn_cdr_read_bytes (
    SYN_CDR_Reader * r,
    uint8_t * out_buf,
    size_t len
) 
```





**Parameters:**


* `r` Reader context. 
* `out_buf` Output buffer. 
* `len` Number of bytes to read. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_double 

_Read 64-bit double (8-byte aligned)._ 
```C++
bool syn_cdr_read_double (
    SYN_CDR_Reader * r,
    double * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_float 

_Read 32-bit float (4-byte aligned)._ 
```C++
bool syn_cdr_read_float (
    SYN_CDR_Reader * r,
    float * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_i16 

_Read signed 16-bit integer (2-byte aligned)._ 
```C++
bool syn_cdr_read_i16 (
    SYN_CDR_Reader * r,
    int16_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_i32 

_Read signed 32-bit integer (4-byte aligned)._ 
```C++
bool syn_cdr_read_i32 (
    SYN_CDR_Reader * r,
    int32_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_i64 

_Read signed 64-bit integer (8-byte aligned)._ 
```C++
bool syn_cdr_read_i64 (
    SYN_CDR_Reader * r,
    int64_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_i8 

_Read signed 8-bit integer._ 
```C++
bool syn_cdr_read_i8 (
    SYN_CDR_Reader * r,
    int8_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_string 

_Read null-terminated string with length prefix._ 
```C++
bool syn_cdr_read_string (
    SYN_CDR_Reader * r,
    char * out_buf,
    size_t max_len
) 
```





**Parameters:**


* `r` Reader context. 
* `out_buf` Output string buffer. 
* `max_len` Capacity of output string buffer. 



**Returns:**

true on success, false on underflow/truncation. 





        

<hr>



### function syn\_cdr\_read\_u16 

_Read unsigned 16-bit integer (2-byte aligned)._ 
```C++
bool syn_cdr_read_u16 (
    SYN_CDR_Reader * r,
    uint16_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_u32 

_Read unsigned 32-bit integer (4-byte aligned)._ 
```C++
bool syn_cdr_read_u32 (
    SYN_CDR_Reader * r,
    uint32_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_u64 

_Read unsigned 64-bit integer (8-byte aligned)._ 
```C++
bool syn_cdr_read_u64 (
    SYN_CDR_Reader * r,
    uint64_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_read\_u8 

_Read unsigned 8-bit integer._ 
```C++
bool syn_cdr_read_u8 (
    SYN_CDR_Reader * r,
    uint8_t * out
) 
```





**Parameters:**


* `r` Reader context. 
* `out` Output pointer. 



**Returns:**

true on success, false on underflow. 





        

<hr>



### function syn\_cdr\_reader\_init 

_Initialize CDR reader._ 
```C++
void syn_cdr_reader_init (
    SYN_CDR_Reader * r,
    const uint8_t * buf,
    size_t size
) 
```





**Parameters:**


* `r` Reader context. 
* `buf` Input buffer. 
* `size` Buffer size. 




        

<hr>



### function syn\_cdr\_write\_bool 

_Write boolean._ 
```C++
bool syn_cdr_write_bool (
    SYN_CDR_Writer * w,
    bool v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_bytes 

_Write raw byte buffer without alignment padding._ 
```C++
bool syn_cdr_write_bytes (
    SYN_CDR_Writer * w,
    const uint8_t * bytes,
    size_t len
) 
```





**Parameters:**


* `w` Writer context. 
* `bytes` Data buffer. 
* `len` Number of bytes. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_double 

_Write 64-bit double (8-byte aligned)._ 
```C++
bool syn_cdr_write_double (
    SYN_CDR_Writer * w,
    double v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_float 

_Write 32-bit float (4-byte aligned)._ 
```C++
bool syn_cdr_write_float (
    SYN_CDR_Writer * w,
    float v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_i16 

_Write signed 16-bit integer (2-byte aligned)._ 
```C++
bool syn_cdr_write_i16 (
    SYN_CDR_Writer * w,
    int16_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_i32 

_Write signed 32-bit integer (4-byte aligned)._ 
```C++
bool syn_cdr_write_i32 (
    SYN_CDR_Writer * w,
    int32_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_i64 

_Write signed 64-bit integer (8-byte aligned)._ 
```C++
bool syn_cdr_write_i64 (
    SYN_CDR_Writer * w,
    int64_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_i8 

_Write signed 8-bit integer._ 
```C++
bool syn_cdr_write_i8 (
    SYN_CDR_Writer * w,
    int8_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_string 

_Write null-terminated string with 32-bit length prefix and null terminator._ 
```C++
bool syn_cdr_write_string (
    SYN_CDR_Writer * w,
    const char * str
) 
```





**Parameters:**


* `w` Writer context. 
* `str` String pointer. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_u16 

_Write unsigned 16-bit integer (2-byte aligned)._ 
```C++
bool syn_cdr_write_u16 (
    SYN_CDR_Writer * w,
    uint16_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_u32 

_Write unsigned 32-bit integer (4-byte aligned)._ 
```C++
bool syn_cdr_write_u32 (
    SYN_CDR_Writer * w,
    uint32_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_u64 

_Write unsigned 64-bit integer (8-byte aligned)._ 
```C++
bool syn_cdr_write_u64 (
    SYN_CDR_Writer * w,
    uint64_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_write\_u8 

_Write unsigned 8-bit integer._ 
```C++
bool syn_cdr_write_u8 (
    SYN_CDR_Writer * w,
    uint8_t v
) 
```





**Parameters:**


* `w` Writer context. 
* `v` Value. 



**Returns:**

true on success, false on overflow. 





        

<hr>



### function syn\_cdr\_writer\_init 

_Initialize CDR writer._ 
```C++
void syn_cdr_writer_init (
    SYN_CDR_Writer * w,
    uint8_t * buf,
    size_t size
) 
```





**Parameters:**


* `w` Writer context. 
* `buf` Target buffer. 
* `size` Buffer size. 




        

<hr>



### function syn\_xrce\_client\_create\_datareader 

_Create a DDS DataReader entity._ 
```C++
SYN_Status syn_xrce_client_create_datareader (
    SYN_XRCE_Client * client,
    uint16_t reader_id,
    uint16_t subscriber_id,
    uint16_t topic_id
) 
```





**Parameters:**


* `client` Client instance. 
* `reader_id` Object ID for the reader. 
* `subscriber_id` Parent subscriber ID. 
* `topic_id` Associated topic ID. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_xrce\_client\_create\_datawriter 

_Create a DDS DataWriter entity._ 
```C++
SYN_Status syn_xrce_client_create_datawriter (
    SYN_XRCE_Client * client,
    uint16_t writer_id,
    uint16_t publisher_id,
    uint16_t topic_id
) 
```





**Parameters:**


* `client` Client instance. 
* `writer_id` Object ID for the writer. 
* `publisher_id` Parent publisher ID. 
* `topic_id` Associated topic ID. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_xrce\_client\_create\_participant 

_Create a DDS DomainParticipant entity._ 
```C++
SYN_Status syn_xrce_client_create_participant (
    SYN_XRCE_Client * client,
    uint16_t participant_id
) 
```





**Parameters:**


* `client` Client instance. 
* `participant_id` Object ID for the participant. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_xrce\_client\_create\_publisher 

_Create a DDS Publisher entity._ 
```C++
SYN_Status syn_xrce_client_create_publisher (
    SYN_XRCE_Client * client,
    uint16_t publisher_id,
    uint16_t participant_id
) 
```





**Parameters:**


* `client` Client instance. 
* `publisher_id` Object ID for the publisher. 
* `participant_id` Parent participant ID. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_xrce\_client\_create\_session 

_Connect and establish session with Micro-ROS Agent._ 
```C++
SYN_Status syn_xrce_client_create_session (
    SYN_XRCE_Client * client
) 
```





**Parameters:**


* `client` Client instance. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on transport failure. 





        

<hr>



### function syn\_xrce\_client\_create\_subscriber 

_Create a DDS Subscriber entity._ 
```C++
SYN_Status syn_xrce_client_create_subscriber (
    SYN_XRCE_Client * client,
    uint16_t subscriber_id,
    uint16_t participant_id
) 
```





**Parameters:**


* `client` Client instance. 
* `subscriber_id` Object ID for the subscriber. 
* `participant_id` Parent participant ID. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_xrce\_client\_create\_topic 

_Create a DDS Topic entity._ 
```C++
SYN_Status syn_xrce_client_create_topic (
    SYN_XRCE_Client * client,
    uint16_t topic_id,
    uint16_t participant_id,
    const char * topic_name,
    const char * type_name
) 
```





**Parameters:**


* `client` Client instance. 
* `topic_id` Object ID for the topic. 
* `participant_id` Parent participant ID. 
* `topic_name` ROS 2 topic name (e.g. "sensors/imu"). 
* `type_name` ROS 2 type identifier (e.g. "sensor\_msgs::msg::dds\_::Imu\_"). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_xrce\_client\_init 

_Initialize Micro XRCE-DDS Client._ 
```C++
SYN_Status syn_xrce_client_init (
    SYN_XRCE_Client * client,
    const SYN_XRCE_Config * cfg
) 
```





**Parameters:**


* `client` Client context to initialize. 
* `cfg` Configuration options and buffers. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid argument. 





        

<hr>



### function syn\_xrce\_client\_pt 

_Protothread continuation function for_ `syn_sched` _._
```C++
SYN_PT_Status syn_xrce_client_pt (
    SYN_PT * pt,
    SYN_Task * task
) 
```





**Parameters:**


* `pt` Protothread continuation structure. 
* `task` Task handle containing [**SYN\_XRCE\_Client**](structSYN__XRCE__Client.md) in user\_data. 



**Returns:**

SYN\_PT\_Status continuation status. 





        

<hr>



### function syn\_xrce\_client\_read\_data 

_Request incoming topic samples from an active DataReader._ 
```C++
SYN_Status syn_xrce_client_read_data (
    SYN_XRCE_Client * client,
    uint16_t reader_id
) 
```





**Parameters:**


* `client` Client instance. 
* `reader_id` DataReader entity ID. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_xrce\_client\_step 

_Advance the XRCE-DDS client state machine by one step / tick._ 
```C++
SYN_Status syn_xrce_client_step (
    SYN_XRCE_Client * client,
    uint32_t now_ms
) 
```





**Parameters:**


* `client` Client instance. 
* `now_ms` Current timestamp in milliseconds. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_xrce\_client\_write\_data 

_Publish CDR serialized data payload to a DataWriter._ 
```C++
SYN_Status syn_xrce_client_write_data (
    SYN_XRCE_Client * client,
    uint16_t writer_id,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `client` Client instance. 
* `writer_id` DataWriter entity ID. 
* `data` CDR payload buffer. 
* `len` Payload length. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Public Static Functions Documentation




### function cdr\_pad\_reader 

_Align CDR reader position to specified byte boundary._ 
```C++
static size_t cdr_pad_reader (
    SYN_CDR_Reader * r,
    size_t align
) 
```





**Parameters:**


* `r` Reader context. 
* `align` Alignment boundary in bytes (2, 4, 8). 



**Returns:**

Updated buffer position. 





        

<hr>



### function cdr\_pad\_writer 

_Align CDR writer position to specified byte boundary with zero padding._ 
```C++
static size_t cdr_pad_writer (
    SYN_CDR_Writer * w,
    size_t align
) 
```





**Parameters:**


* `w` Writer context. 
* `align` Alignment boundary in bytes (2, 4, 8). 



**Returns:**

Updated buffer position. 





        

<hr>



### function xrce\_finish\_submessage 

_Finish XRCE submessage and patch calculated payload length._ 
```C++
static void xrce_finish_submessage (
    SYN_CDR_Writer * w,
    size_t len_pos
) 
```





**Parameters:**


* `w` Writer context. 
* `len_pos` Byte offset of submessage length field. 




        

<hr>



### function xrce\_register\_local\_object 

_Register local XRCE object in client's internal entity table._ 
```C++
static SYN_Status xrce_register_local_object (
    SYN_XRCE_Client * client,
    uint16_t id,
    uint8_t kind,
    uint16_t parent
) 
```





**Parameters:**


* `client` Client instance pointer. 
* `id` Object identifier. 
* `kind` Object kind (participant, topic, publisher, etc.). 
* `parent` Parent object identifier. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on table overflow. 





        

<hr>



### function xrce\_start\_submessage 

_Start XRCE submessage with 4-byte header and return length patch offset._ 
```C++
static size_t xrce_start_submessage (
    SYN_CDR_Writer * w,
    uint8_t submsg_id,
    uint8_t flags
) 
```





**Parameters:**


* `w` Writer context. 
* `submsg_id` Submessage identifier. 
* `flags` Submessage flags. 



**Returns:**

Byte offset of submessage length field for later patching. 





        

<hr>



### function xrce\_write\_header 

_Write XRCE message header to CDR stream._ 
```C++
static void xrce_write_header (
    SYN_CDR_Writer * w,
    uint8_t session_id,
    uint8_t stream_id,
    uint16_t seq_num,
    uint32_t client_key
) 
```





**Parameters:**


* `w` Writer context. 
* `session_id` XRCE session identifier. 
* `stream_id` XRCE stream identifier. 
* `seq_num` Stream sequence number. 
* `client_key` 32-bit client key. 




        

<hr>
## Macro Definition Documentation





### define SYN\_XRCE\_DATA\_FORMAT\_DATA 

_XRCE data submessage format: Raw Data payload._ 
```C++
#define SYN_XRCE_DATA_FORMAT_DATA `0x00U`
```




<hr>



### define SYN\_XRCE\_DEFAULT\_HEARTBEAT\_MS 

_Default client heartbeat period in milliseconds._ 
```C++
#define SYN_XRCE_DEFAULT_HEARTBEAT_MS `5000U`
```




<hr>



### define SYN\_XRCE\_FLAG\_LITTLE\_ENDIAN 

_XRCE submessage little-endian endianness flag._ 
```C++
#define SYN_XRCE_FLAG_LITTLE_ENDIAN `0x01U`
```




<hr>



### define SYN\_XRCE\_REPRESENTATION\_AS\_STRING 

_XRCE object representation as string / XML._ 
```C++
#define SYN_XRCE_REPRESENTATION_AS_STRING `0x02U`
```




<hr>



### define SYN\_XRCE\_REPRESENTATION\_BY\_REF 

_XRCE object representation by reference._ 
```C++
#define SYN_XRCE_REPRESENTATION_BY_REF `0x01U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_xrce_dds.c`

