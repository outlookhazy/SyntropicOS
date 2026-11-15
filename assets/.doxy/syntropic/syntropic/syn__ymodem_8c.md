

# File syn\_ymodem.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ymodem.c**](syn__ymodem_8c.md)

[Go to the source code of this file](syn__ymodem_8c_source.md)

_YMODEM / XMODEM-1K serial file transfer receiver implementation._ 

* `#include "../util/syn_crc.h"`
* `#include "syn_ymodem.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_YMODEM\_Status**](syn__ymodem_8h.md#enum-syn_ymodem_status) | [**syn\_ymodem\_receive**](#function-syn_ymodem_receive) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx) <br>_Execute a YMODEM receive session._  |
|  void | [**syn\_ymodem\_receiver\_init**](#function-syn_ymodem_receiver_init) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx, [**SYN\_YMODEM\_PutChar**](syn__ymodem_8h.md#typedef-syn_ymodem_putchar) putchar\_fn, [**SYN\_YMODEM\_GetChar**](syn__ymodem_8h.md#typedef-syn_ymodem_getchar) getchar\_fn, [**SYN\_YMODEM\_EventCallback**](syn__ymodem_8h.md#typedef-syn_ymodem_eventcallback) event\_fn, void \* ctx) <br>_Initialize a YMODEM receiver instance._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**ymodem\_abort**](#function-ymodem_abort) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx) <br>_Transmit a two-byte CAN abort sequence._  |
|  void | [**ymodem\_finish\_eot**](#function-ymodem_finish_eot) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx) <br>_Complete EOT handshake and emit FILE\_END callback._  |
|  [**SYN\_YMODEM\_Status**](syn__ymodem_8h.md#enum-syn_ymodem_status) | [**ymodem\_handshake\_block0**](#function-ymodem_handshake_block0) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx, [**bool**](syn__defs_8h.md#enum-bool) \* session\_done) <br>_Negotiate CRC mode and process YMODEM Block 0 header._  |
|  void | [**ymodem\_parse\_block0**](#function-ymodem_parse_block0) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx, const uint8\_t \* payload, size\_t payload\_len) <br>_Parse Block 0 payload extracting filename and bounded ASCII file size._  |
|  [**SYN\_YMODEM\_Status**](syn__ymodem_8h.md#enum-syn_ymodem_status) | [**ymodem\_process\_data\_block**](#function-ymodem_process_data_block) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx, size\_t data\_payload\_len) <br>_Deliver validated data block payload to application and advance sequence._  |
|  int | [**ymodem\_read\_packet**](#function-ymodem_read_packet) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx, uint8\_t header\_byte, size\_t \* out\_payload\_len) <br>_Read and validate a framed YMODEM packet._  |
|  [**SYN\_YMODEM\_Status**](syn__ymodem_8h.md#enum-syn_ymodem_status) | [**ymodem\_receive\_file\_data**](#function-ymodem_receive_file_data) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx) <br>_Receive all data packets for a single file transfer with bounded retries._  |
|  int | [**ymodem\_recv\_byte**](#function-ymodem_recv_byte) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx, uint32\_t timeout\_ms) <br>_Receive a single byte via user callback._  |
|  void | [**ymodem\_send\_byte**](#function-ymodem_send_byte) ([**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md) \* rx, uint8\_t b) <br>_Transmit a single byte via user callback._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_YMODEM\_RECV\_TIMEOUT\_MS**](syn__ymodem_8c.md#define-syn_ymodem_recv_timeout_ms)  `1000U`<br>_Single byte receive timeout in milliseconds._  |

## Public Functions Documentation




### function syn\_ymodem\_receive 

_Execute a YMODEM receive session._ 
```C++
SYN_YMODEM_Status syn_ymodem_receive (
    SYN_YMODEM_Receiver * rx
) 
```



Runs the YMODEM handshake, processes Block 0 header, accepts data packets, verifies CRCs, and invokes event callbacks until all files in the batch are transferred or an unrecoverable error occurs.




**Parameters:**


* `rx` Receiver context. 



**Returns:**

SYN\_YMODEM\_OK on successful session completion, or error code (&lt;0). 





        

<hr>



### function syn\_ymodem\_receiver\_init 

_Initialize a YMODEM receiver instance._ 
```C++
void syn_ymodem_receiver_init (
    SYN_YMODEM_Receiver * rx,
    SYN_YMODEM_PutChar putchar_fn,
    SYN_YMODEM_GetChar getchar_fn,
    SYN_YMODEM_EventCallback event_fn,
    void * ctx
) 
```





**Parameters:**


* `rx` Receiver context. 
* `putchar_fn` Serial TX function. 
* `getchar_fn` Serial RX function with timeout. 
* `event_fn` Event callback handler. 
* `ctx` User context pointer. 




        

<hr>
## Public Static Functions Documentation




### function ymodem\_abort 

_Transmit a two-byte CAN abort sequence._ 
```C++
static void ymodem_abort (
    SYN_YMODEM_Receiver * rx
) 
```





**Parameters:**


* `rx` Receiver context. 




        

<hr>



### function ymodem\_finish\_eot 

_Complete EOT handshake and emit FILE\_END callback._ 
```C++
static void ymodem_finish_eot (
    SYN_YMODEM_Receiver * rx
) 
```





**Parameters:**


* `rx` Receiver context. 




        

<hr>



### function ymodem\_handshake\_block0 

_Negotiate CRC mode and process YMODEM Block 0 header._ 
```C++
static SYN_YMODEM_Status ymodem_handshake_block0 (
    SYN_YMODEM_Receiver * rx,
    bool * session_done
) 
```





**Parameters:**


* `rx` Receiver context. 
* `session_done` [out] Set to true when empty Block 0 terminates session. 



**Returns:**

SYN\_YMODEM\_OK on success, or negative error status. 





        

<hr>



### function ymodem\_parse\_block0 

_Parse Block 0 payload extracting filename and bounded ASCII file size._ 
```C++
static void ymodem_parse_block0 (
    SYN_YMODEM_Receiver * rx,
    const uint8_t * payload,
    size_t payload_len
) 
```





**Parameters:**


* `rx` Receiver context. 
* `payload` Block 0 payload buffer. 
* `payload_len` Block payload length in bytes. 




        

<hr>



### function ymodem\_process\_data\_block 

_Deliver validated data block payload to application and advance sequence._ 
```C++
static SYN_YMODEM_Status ymodem_process_data_block (
    SYN_YMODEM_Receiver * rx,
    size_t data_payload_len
) 
```





**Parameters:**


* `rx` Receiver context. 
* `data_payload_len` Raw block payload size (128 or 1024). 



**Returns:**

SYN\_YMODEM\_OK on success, or SYN\_YMODEM\_ERR\_CANCEL if callback aborts. 





        

<hr>



### function ymodem\_read\_packet 

_Read and validate a framed YMODEM packet._ 
```C++
static int ymodem_read_packet (
    SYN_YMODEM_Receiver * rx,
    uint8_t header_byte,
    size_t * out_payload_len
) 
```





**Parameters:**


* `rx` Receiver context. 
* `header_byte` Packet header byte (SOH or STX). 
* `out_payload_len` Output pointer for payload byte count. 



**Returns:**

Sequence number (0-255) on success, or negative status error code. 





        

<hr>



### function ymodem\_receive\_file\_data 

_Receive all data packets for a single file transfer with bounded retries._ 
```C++
static SYN_YMODEM_Status ymodem_receive_file_data (
    SYN_YMODEM_Receiver * rx
) 
```





**Parameters:**


* `rx` Receiver context. 



**Returns:**

SYN\_YMODEM\_OK on EOT completion, or negative error status. 





        

<hr>



### function ymodem\_recv\_byte 

_Receive a single byte via user callback._ 
```C++
static int ymodem_recv_byte (
    SYN_YMODEM_Receiver * rx,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `rx` Receiver context. 
* `timeout_ms` Timeout in milliseconds. 



**Returns:**

Received byte value (0-255) or negative on timeout/error. 





        

<hr>



### function ymodem\_send\_byte 

_Transmit a single byte via user callback._ 
```C++
static void ymodem_send_byte (
    SYN_YMODEM_Receiver * rx,
    uint8_t b
) 
```





**Parameters:**


* `rx` Receiver context. 
* `b` Byte value to transmit. 




        

<hr>
## Macro Definition Documentation





### define SYN\_YMODEM\_RECV\_TIMEOUT\_MS 

_Single byte receive timeout in milliseconds._ 
```C++
#define SYN_YMODEM_RECV_TIMEOUT_MS `1000U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ymodem.c`

