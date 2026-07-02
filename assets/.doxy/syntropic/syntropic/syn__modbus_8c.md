

# File syn\_modbus.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_modbus.c**](syn__modbus_8c.md)

[Go to the source code of this file](syn__modbus_8c_source.md)

_Modbus RTU slave implementation._ 

* `#include "syn_modbus.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































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
|  void | [**handle\_read\_regs**](#function-handle_read_regs) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, const uint16\_t \* regs, uint16\_t reg\_count) <br>_Handle Modbus read holding/input register request._  |
|  void | [**handle\_write\_multiple**](#function-handle_write_multiple) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Modbus write multiple registers request._  |
|  void | [**handle\_write\_single**](#function-handle_write_single) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb) <br>_Handle Modbus write single register request._  |
|  uint16\_t | [**read\_u16**](#function-read_u16) (const uint8\_t \* buf) <br>_Read big-endian uint16 from a byte buffer._  |
|  void | [**send\_exception**](#function-send_exception) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, uint8\_t func, uint8\_t ex\_code) <br>_Send a Modbus exception response._  |
|  void | [**send\_response**](#function-send_response) ([**SYN\_Modbus**](structSYN__Modbus.md) \* mb, uint16\_t len) <br>_Append CRC and transmit a response frame._  |
|  void | [**write\_u16**](#function-write_u16) (uint8\_t \* buf, uint16\_t val) <br>_Write big-endian uint16 to a byte buffer._  |

























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



### function read\_u16 

_Read big-endian uint16 from a byte buffer._ 
```C++
static uint16_t read_u16 (
    const uint8_t * buf
) 
```





**Parameters:**


* `buf` Source buffer. 



**Returns:**

Decoded value. 





        

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



### function write\_u16 

_Write big-endian uint16 to a byte buffer._ 
```C++
static void write_u16 (
    uint8_t * buf,
    uint16_t val
) 
```





**Parameters:**


* `buf` Destination buffer. 
* `val` Value to encode. 




        

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

