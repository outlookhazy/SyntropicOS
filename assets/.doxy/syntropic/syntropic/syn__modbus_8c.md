

# File syn\_modbus.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_modbus.c**](syn__modbus_8c.md)

[Go to the source code of this file](syn__modbus_8c_source.md)

_Modbus RTU slave implementation._ 

* `#include "syn_modbus.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`
* `#include "../util/syn_pack.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_modbus\_feed**](#function-syn_modbus_feed) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, uint8\_t byte) <br>_Feed a byte to the Modbus receiver._  |
|  void | [**syn\_modbus\_init**](#function-syn_modbus_init) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, const [**SYN\_Modbus\_Config**](structSYN__Modbus__Config.md) \* cfg, uint8\_t \* buf, uint16\_t buf\_size) <br>_Initialize the Modbus RTU slave._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_modbus\_poll**](#function-syn_modbus_poll) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Poll for incoming Modbus requests and process them._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_modbus\_process**](#function-syn_modbus_process) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Process a complete frame (call after feed + silence detection)._  |
|  void | [**syn\_modbus\_reset**](#function-syn_modbus_reset) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Reset the receiver state._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**append\_crc**](#function-append_crc) (uint8\_t \* buf, uint16\_t len) <br>_Append CRC-16 Modbus to a frame buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**check\_crc**](#function-check_crc) (const uint8\_t \* buf, uint16\_t len) <br>_Verify the CRC-16 Modbus trailer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**get\_bit**](#function-get_bit) (const uint8\_t \* bits, uint16\_t idx) <br>_Helper to get bit from array._  |
|  void | [**handle\_diagnostics**](#function-handle_diagnostics) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Diagnostics (FC 0x08)._  |
|  void | [**handle\_get\_comm\_event\_cnt**](#function-handle_get_comm_event_cnt) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Get Comm Event Counter (FC 0x0B)._  |
|  void | [**handle\_get\_comm\_event\_log**](#function-handle_get_comm_event_log) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Get Comm Event Log (FC 0x0C)._  |
|  void | [**handle\_mask\_write\_register**](#function-handle_mask_write_register) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Mask Write Register (FC 0x16)._  |
|  void | [**handle\_read\_bits**](#function-handle_read_bits) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, const uint8\_t \* bits, uint16\_t total\_bits) <br>_Handle Read Coils (FC 0x01) and Read Discrete Inputs (FC 0x02)._  |
|  void | [**handle\_read\_device\_info**](#function-handle_read_device_info) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Modbus read device identification (FC 0x2B / MEI 0x0E)._  |
|  void | [**handle\_read\_exception\_status**](#function-handle_read_exception_status) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Modbus read exception status (FC 0x07)._  |
|  void | [**handle\_read\_fifo\_queue**](#function-handle_read_fifo_queue) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Read FIFO Queue (FC 0x18)._  |
|  void | [**handle\_read\_file\_record**](#function-handle_read_file_record) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Modbus read file record (FC 0x14)._  |
|  void | [**handle\_read\_regs**](#function-handle_read_regs) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, const uint16\_t \* regs, uint16\_t reg\_count) <br>_Handle Modbus read holding/input register request._  |
|  void | [**handle\_read\_write\_multiple**](#function-handle_read_write_multiple) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Modbus read/write multiple registers (FC 0x17)._  |
|  void | [**handle\_report\_server\_id**](#function-handle_report_server_id) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Report Server ID (FC 0x11)._  |
|  void | [**handle\_write\_file\_record**](#function-handle_write_file_record) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Modbus write file record (FC 0x15)._  |
|  void | [**handle\_write\_multiple**](#function-handle_write_multiple) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Modbus write multiple registers request._  |
|  void | [**handle\_write\_multiple\_coils**](#function-handle_write_multiple_coils) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Write Multiple Coils (FC 0x0F)._  |
|  void | [**handle\_write\_single**](#function-handle_write_single) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Modbus write single register request._  |
|  void | [**handle\_write\_single\_coil**](#function-handle_write_single_coil) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Write Single Coil (FC 0x05)._  |
|  uint16\_t | [**read\_u16**](#function-read_u16) (const uint8\_t \* buf) <br> |
|  void | [**send\_exception**](#function-send_exception) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, uint8\_t func, uint8\_t ex\_code) <br>_Send a Modbus exception response._  |
|  void | [**send\_response**](#function-send_response) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, uint16\_t len) <br>_Append CRC and transmit a response frame._  |
|  void | [**set\_bit**](#function-set_bit) (uint8\_t \* bits, uint16\_t idx, [**bool**](syn__defs_8h.md#enum-bool) val) <br>_Helper to set bit in array._  |
|  void | [**write\_u16**](#function-write_u16) (uint8\_t \* buf, uint16\_t val) <br> |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**MB\_MAX\_PDU\_DATA**](syn__modbus_8c.md#define-mb_max_pdu_data)  `252`<br> |
| define  | [**MB\_MIN\_FRAME\_LEN**](syn__modbus_8c.md#define-mb_min_frame_len)  `4`<br> |
| define  | [**MB\_SILENCE\_MS**](syn__modbus_8c.md#define-mb_silence_ms)  `5`<br> |

## Public Functions Documentation




### function syn\_modbus\_feed 

_Feed a byte to the Modbus receiver._ 
```C++
void syn_modbus_feed (
    SYN_Modbus * mb,
    uint8_t byte
) 
```



Alternative to poll() — call from UART ISR if you want interrupt-driven reception.




**Parameters:**


* `mb` Modbus instance. 
* `byte` Received byte. 




        

<hr>



### function syn\_modbus\_init 

_Initialize the Modbus RTU slave._ 
```C++
void syn_modbus_init (
    SYN_Modbus * mb,
    const SYN_Modbus_Config * cfg,
    uint8_t * buf,
    uint16_t buf_size
) 
```





**Parameters:**


* `mb` Modbus instance. 
* `cfg` Configuration. 
* `buf` Frame buffer (must be at least 256 bytes). 
* `buf_size` Buffer capacity. 




        

<hr>



### function syn\_modbus\_poll 

_Poll for incoming Modbus requests and process them._ 
```C++
bool syn_modbus_poll (
    SYN_Modbus * mb
) 
```



Call from your main loop. This reads bytes from UART, detects frame boundaries (3.5 character times silence), validates CRC, and processes the request.




**Parameters:**


* `mb` Modbus instance. 



**Returns:**

true if a request was processed. 





        

<hr>



### function syn\_modbus\_process 

_Process a complete frame (call after feed + silence detection)._ 
```C++
bool syn_modbus_process (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 



**Returns:**

true if a response was sent. 





        

<hr>



### function syn\_modbus\_reset 

_Reset the receiver state._ 
```C++
void syn_modbus_reset (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 




        

<hr>
## Public Static Functions Documentation




### function append\_crc 

_Append CRC-16 Modbus to a frame buffer._ 
```C++
static void append_crc (
    uint8_t * buf,
    uint16_t len
) 
```





**Parameters:**


* `buf` Frame buffer. 
* `len` Frame length (excluding CRC). 




        

<hr>



### function check\_crc 

_Verify the CRC-16 Modbus trailer._ 
```C++
static bool check_crc (
    const uint8_t * buf,
    uint16_t len
) 
```





**Parameters:**


* `buf` Frame buffer. 
* `len` Total frame length (including CRC). 



**Returns:**

true if CRC matches. 





        

<hr>



### function get\_bit 

_Helper to get bit from array._ 
```C++
static bool get_bit (
    const uint8_t * bits,
    uint16_t idx
) 
```




<hr>



### function handle\_diagnostics 

_Handle Diagnostics (FC 0x08)._ 
```C++
static void handle_diagnostics (
    SYN_Modbus * mb
) 
```




<hr>



### function handle\_get\_comm\_event\_cnt 

_Handle Get Comm Event Counter (FC 0x0B)._ 
```C++
static void handle_get_comm_event_cnt (
    SYN_Modbus * mb
) 
```




<hr>



### function handle\_get\_comm\_event\_log 

_Handle Get Comm Event Log (FC 0x0C)._ 
```C++
static void handle_get_comm_event_log (
    SYN_Modbus * mb
) 
```




<hr>



### function handle\_mask\_write\_register 

_Handle Mask Write Register (FC 0x16)._ 
```C++
static void handle_mask_write_register (
    SYN_Modbus * mb
) 
```




<hr>



### function handle\_read\_bits 

_Handle Read Coils (FC 0x01) and Read Discrete Inputs (FC 0x02)._ 
```C++
static void handle_read_bits (
    SYN_Modbus * mb,
    const uint8_t * bits,
    uint16_t total_bits
) 
```




<hr>



### function handle\_read\_device\_info 

_Handle Modbus read device identification (FC 0x2B / MEI 0x0E)._ 
```C++
static void handle_read_device_info (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 




        

<hr>



### function handle\_read\_exception\_status 

_Handle Modbus read exception status (FC 0x07)._ 
```C++
static void handle_read_exception_status (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 




        

<hr>



### function handle\_read\_fifo\_queue 

_Handle Read FIFO Queue (FC 0x18)._ 
```C++
static void handle_read_fifo_queue (
    SYN_Modbus * mb
) 
```




<hr>



### function handle\_read\_file\_record 

_Handle Modbus read file record (FC 0x14)._ 
```C++
static void handle_read_file_record (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 




        

<hr>



### function handle\_read\_regs 

_Handle Modbus read holding/input register request._ 
```C++
static void handle_read_regs (
    SYN_Modbus * mb,
    const uint16_t * regs,
    uint16_t reg_count
) 
```





**Parameters:**


* `mb` Modbus instance. 
* `regs` Register array. 
* `reg_count` Total register count. 




        

<hr>



### function handle\_read\_write\_multiple 

_Handle Modbus read/write multiple registers (FC 0x17)._ 
```C++
static void handle_read_write_multiple (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 




        

<hr>



### function handle\_report\_server\_id 

_Handle Report Server ID (FC 0x11)._ 
```C++
static void handle_report_server_id (
    SYN_Modbus * mb
) 
```




<hr>



### function handle\_write\_file\_record 

_Handle Modbus write file record (FC 0x15)._ 
```C++
static void handle_write_file_record (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 




        

<hr>



### function handle\_write\_multiple 

_Handle Modbus write multiple registers request._ 
```C++
static void handle_write_multiple (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 




        

<hr>



### function handle\_write\_multiple\_coils 

_Handle Write Multiple Coils (FC 0x0F)._ 
```C++
static void handle_write_multiple_coils (
    SYN_Modbus * mb
) 
```




<hr>



### function handle\_write\_single 

_Handle Modbus write single register request._ 
```C++
static void handle_write_single (
    SYN_Modbus * mb
) 
```





**Parameters:**


* `mb` Modbus instance. 




        

<hr>



### function handle\_write\_single\_coil 

_Handle Write Single Coil (FC 0x05)._ 
```C++
static void handle_write_single_coil (
    SYN_Modbus * mb
) 
```




<hr>



### function read\_u16 

```C++
static inline uint16_t read_u16 (
    const uint8_t * buf
) 
```




<hr>



### function send\_exception 

_Send a Modbus exception response._ 
```C++
static void send_exception (
    SYN_Modbus * mb,
    uint8_t func,
    uint8_t ex_code
) 
```





**Parameters:**


* `mb` Modbus instance. 
* `func` Function code that caused the exception. 
* `ex_code` Exception code. 




        

<hr>



### function send\_response 

_Append CRC and transmit a response frame._ 
```C++
static void send_response (
    SYN_Modbus * mb,
    uint16_t len
) 
```





**Parameters:**


* `mb` Modbus instance. 
* `len` PDU length (excluding CRC). 




        

<hr>



### function set\_bit 

_Helper to set bit in array._ 
```C++
static void set_bit (
    uint8_t * bits,
    uint16_t idx,
    bool val
) 
```




<hr>



### function write\_u16 

```C++
static inline void write_u16 (
    uint8_t * buf,
    uint16_t val
) 
```




<hr>
## Macro Definition Documentation





### define MB\_MAX\_PDU\_DATA 

```C++
#define MB_MAX_PDU_DATA `252`
```



Max PDU data bytes. 
 


        

<hr>



### define MB\_MIN\_FRAME\_LEN 

```C++
#define MB_MIN_FRAME_LEN `4`
```



Minimum: addr + func + CRC16. 
 


        

<hr>



### define MB\_SILENCE\_MS 

```C++
#define MB_SILENCE_MS `5`
```



Inter-frame silence (ms). 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus.c`

