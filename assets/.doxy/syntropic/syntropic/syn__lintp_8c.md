

# File syn\_lintp.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lintp.c**](syn__lintp_8c.md)

[Go to the source code of this file](syn__lintp_8c_source.md)

_ISO 17987-2 (LIN Transport Layer & Network Layer Services / UDSonLIN)._ [More...](#detailed-description)

* `#include "syn_lintp.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lintp\_get\_tx\_frame**](#function-syn_lintp_get_tx_frame) ([**SYN\_LINTP\_Link**](structSYN__LINTP__Link.md) \* link, uint8\_t out\_frame) <br>_Retrieve next queued 8-byte LIN diagnostic payload to transmit on bus (0x3C / 0x3D)._  |
|  void | [**syn\_lintp\_init**](#function-syn_lintp_init) ([**SYN\_LINTP\_Link**](structSYN__LINTP__Link.md) \* link, uint8\_t nad, uint8\_t \* rx\_buf, size\_t rx\_size, uint8\_t \* tx\_buf, size\_t tx\_size) <br>_Initialize LIN TP Link context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lintp\_is\_tx\_idle**](#function-syn_lintp_is_tx_idle) (const [**SYN\_LINTP\_Link**](structSYN__LINTP__Link.md) \* link) <br>_Check if LIN TP TX engine is idle and ready for new message._  |
|  void | [**syn\_lintp\_process\_rx\_frame**](#function-syn_lintp_process_rx_frame) ([**SYN\_LINTP\_Link**](structSYN__LINTP__Link.md) \* link, const uint8\_t frame) <br>_Process an incoming 8-byte LIN diagnostic frame received from bus (0x3C / 0x3D)._  |
|  ssize\_t | [**syn\_lintp\_receive**](#function-syn_lintp_receive) ([**SYN\_LINTP\_Link**](structSYN__LINTP__Link.md) \* link, uint8\_t \* out\_buf, size\_t max\_len) <br>_Read a fully reassembled message payload from the LIN TP receiver._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lintp\_send**](#function-syn_lintp_send) ([**SYN\_LINTP\_Link**](structSYN__LINTP__Link.md) \* link, uint8\_t nad, const uint8\_t \* payload, size\_t len) <br>_Queue a message for multi-frame or single-frame LIN TP transmission._  |
|  void | [**syn\_lintp\_set\_padding**](#function-syn_lintp_set_padding) ([**SYN\_LINTP\_Link**](structSYN__LINTP__Link.md) \* link, uint8\_t pad\_byte) <br>_Configure padding byte value for unused frame bytes._  |
|  void | [**syn\_lintp\_set\_timeouts**](#function-syn_lintp_set_timeouts) ([**SYN\_LINTP\_Link**](structSYN__LINTP__Link.md) \* link, uint32\_t n\_as\_ms, uint32\_t n\_cr\_ms) <br>_Configure LIN TP timeouts in milliseconds._  |
|  void | [**syn\_lintp\_step**](#function-syn_lintp_step) ([**SYN\_LINTP\_Link**](structSYN__LINTP__Link.md) \* link, uint32\_t dt\_ms) <br>_Step LIN TP timers by dt\_ms._  |




























## Detailed Description


Implements non-blocking, zero-allocation ISO 17987-2 LIN TP segmentation and reassembly state machine. 


    
## Public Functions Documentation




### function syn\_lintp\_get\_tx\_frame 

_Retrieve next queued 8-byte LIN diagnostic payload to transmit on bus (0x3C / 0x3D)._ 
```C++
bool syn_lintp_get_tx_frame (
    SYN_LINTP_Link * link,
    uint8_t out_frame
) 
```





**Parameters:**


* `link` Pointer to LIN TP link context. 
* `out_frame` Array of 8 bytes to receive frame data. 



**Returns:**

true if a frame was ready and copied, false otherwise. 





        

<hr>



### function syn\_lintp\_init 

_Initialize LIN TP Link context._ 
```C++
void syn_lintp_init (
    SYN_LINTP_Link * link,
    uint8_t nad,
    uint8_t * rx_buf,
    size_t rx_size,
    uint8_t * tx_buf,
    size_t tx_size
) 
```





**Parameters:**


* `link` Pointer to LIN TP link structure. 
* `nad` Node Address (NAD) for this device (0x01..0x7D). 
* `rx_buf` Pointer to buffer for storing received payloads. 
* `rx_size` Size of RX buffer. 
* `tx_buf` Pointer to buffer reserved for TX (optional context pointer). 
* `tx_size` Size of TX buffer. 




        

<hr>



### function syn\_lintp\_is\_tx\_idle 

_Check if LIN TP TX engine is idle and ready for new message._ 
```C++
bool syn_lintp_is_tx_idle (
    const SYN_LINTP_Link * link
) 
```





**Parameters:**


* `link` Pointer to LIN TP link context. 



**Returns:**

true if idle, false if actively transmitting a message. 





        

<hr>



### function syn\_lintp\_process\_rx\_frame 

_Process an incoming 8-byte LIN diagnostic frame received from bus (0x3C / 0x3D)._ 
```C++
void syn_lintp_process_rx_frame (
    SYN_LINTP_Link * link,
    const uint8_t frame
) 
```





**Parameters:**


* `link` Pointer to LIN TP link context. 
* `frame` Pointer to 8-byte received LIN frame payload. 




        

<hr>



### function syn\_lintp\_receive 

_Read a fully reassembled message payload from the LIN TP receiver._ 
```C++
ssize_t syn_lintp_receive (
    SYN_LINTP_Link * link,
    uint8_t * out_buf,
    size_t max_len
) 
```





**Parameters:**


* `link` Pointer to LIN TP link context. 
* `out_buf` Buffer to receive assembled payload. 
* `max_len` Maximum bytes to copy. 



**Returns:**

Number of payload bytes read, 0 if no complete message ready, or negative error code. 





        

<hr>



### function syn\_lintp\_send 

_Queue a message for multi-frame or single-frame LIN TP transmission._ 
```C++
SYN_Status syn_lintp_send (
    SYN_LINTP_Link * link,
    uint8_t nad,
    const uint8_t * payload,
    size_t len
) 
```





**Parameters:**


* `link` Pointer to LIN TP link context. 
* `nad` Destination NAD. 
* `payload` Pointer to payload bytes to send. 
* `len` Payload length in bytes (1..4095). 



**Returns:**

SYN\_OK on success, or error status code. 





        

<hr>



### function syn\_lintp\_set\_padding 

_Configure padding byte value for unused frame bytes._ 
```C++
void syn_lintp_set_padding (
    SYN_LINTP_Link * link,
    uint8_t pad_byte
) 
```





**Parameters:**


* `link` Pointer to LIN TP link context. 
* `pad_byte` Fill byte value (default 0xFF). 




        

<hr>



### function syn\_lintp\_set\_timeouts 

_Configure LIN TP timeouts in milliseconds._ 
```C++
void syn_lintp_set_timeouts (
    SYN_LINTP_Link * link,
    uint32_t n_as_ms,
    uint32_t n_cr_ms
) 
```





**Parameters:**


* `link` Pointer to LIN TP link context. 
* `n_as_ms` Max frame TX time (default 1000ms). 
* `n_cr_ms` Max time between consecutive RX frames (default 1000ms). 




        

<hr>



### function syn\_lintp\_step 

_Step LIN TP timers by dt\_ms._ 
```C++
void syn_lintp_step (
    SYN_LINTP_Link * link,
    uint32_t dt_ms
) 
```





**Parameters:**


* `link` Pointer to LIN TP link context. 
* `dt_ms` Milliseconds elapsed since last step call. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lintp.c`

