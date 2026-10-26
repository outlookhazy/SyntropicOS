

# File syn\_mbus.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mbus.c**](syn__mbus_8c.md)

[Go to the source code of this file](syn__mbus_8c_source.md)

_M-Bus (Meter-Bus) master/slave protocol framing implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_mbus.h"`
* `#include <string.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**syn\_\_mbus\_8c\_1a61dadd085c1777f559549e05962b2c9e**](#enum-syn__mbus_8c_1a61dadd085c1777f559549e05962b2c9e)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_mbus\_calc\_checksum**](#function-syn_mbus_calc_checksum) (const uint8\_t \* data, size\_t len) <br>_Calculate M-Bus 8-bit checksum (modulo-256 sum over specified buffer)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mbus\_decode\_frame**](#function-syn_mbus_decode_frame) (const uint8\_t \* buf, size\_t len, [**SYN\_MBUS\_Frame**](structSYN__MBUS__Frame.md) \* frame) <br>_Decode a raw M-Bus byte buffer into a frame structure._  |
|  void | [**syn\_mbus\_decoder\_feed**](#function-syn_mbus_decoder_feed) ([**SYN\_MBUS\_Decoder**](structSYN__MBUS__Decoder.md) \* dec, uint8\_t byte) <br>_Feed a single byte to the streaming M-Bus decoder._  |
|  void | [**syn\_mbus\_decoder\_init**](#function-syn_mbus_decoder_init) ([**SYN\_MBUS\_Decoder**](structSYN__MBUS__Decoder.md) \* dec, [**SYN\_MBUS\_FrameCallback**](syn__mbus_8h.md#typedef-syn_mbus_framecallback) callback, void \* ctx) <br>_Initialize an M-Bus streaming state machine decoder._  |
|  void | [**syn\_mbus\_decoder\_reset**](#function-syn_mbus_decoder_reset) ([**SYN\_MBUS\_Decoder**](structSYN__MBUS__Decoder.md) \* dec) <br>_Reset the streaming decoder state._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mbus\_encode\_ack**](#function-syn_mbus_encode_ack) (uint8\_t \* buf, size\_t cap, size\_t \* out\_len) <br>_Encode a Single Character ACK frame (0xE5)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mbus\_encode\_control**](#function-syn_mbus_encode_control) (uint8\_t c\_field, uint8\_t a\_field, uint8\_t ci\_field, uint8\_t \* buf, size\_t cap, size\_t \* out\_len) <br>_Encode a Control Frame (9 bytes: 0x68 \| 0x03 \| 0x03 \| 0x68 \| C \| A \| CI \| Checksum \| 0x16)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mbus\_encode\_long**](#function-syn_mbus_encode_long) (uint8\_t c\_field, uint8\_t a\_field, uint8\_t ci\_field, const uint8\_t \* payload, uint8\_t payload\_len, uint8\_t \* buf, size\_t cap, size\_t \* out\_len) <br>_Encode a Long Frame (0x68 \| L \| L \| 0x68 \| C \| A \| CI \| Payload... \| Checksum \| 0x16)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_mbus\_encode\_short**](#function-syn_mbus_encode_short) (uint8\_t c\_field, uint8\_t a\_field, uint8\_t \* buf, size\_t cap, size\_t \* out\_len) <br>_Encode a Short Frame (5 bytes: 0x10 \| C \| A \| Checksum \| 0x16)._  |




























## Public Types Documentation




### enum syn\_\_mbus\_8c\_1a61dadd085c1777f559549e05962b2c9e 

```C++
enum syn__mbus_8c_1a61dadd085c1777f559549e05962b2c9e {
    STATE_IDLE = 0,
    STATE_SHORT_C,
    STATE_SHORT_A,
    STATE_SHORT_CHK,
    STATE_SHORT_STOP,
    STATE_LONG_L1,
    STATE_LONG_L2,
    STATE_LONG_SOF2,
    STATE_LONG_BODY
};
```




<hr>
## Public Functions Documentation




### function syn\_mbus\_calc\_checksum 

_Calculate M-Bus 8-bit checksum (modulo-256 sum over specified buffer)._ 
```C++
uint8_t syn_mbus_calc_checksum (
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `data` Pointer to buffer. 
* `len` Length of data in bytes. 



**Returns:**

Calculated checksum byte. 





        

<hr>



### function syn\_mbus\_decode\_frame 

_Decode a raw M-Bus byte buffer into a frame structure._ 
```C++
SYN_Status syn_mbus_decode_frame (
    const uint8_t * buf,
    size_t len,
    SYN_MBUS_Frame * frame
) 
```





**Parameters:**


* `buf` Input byte buffer containing a complete M-Bus frame. 
* `len` Buffer length in bytes. 
* `frame` Destination frame structure. 



**Returns:**

SYN\_OK on success, SYN\_ERR\_INVALID\_CHECKSUM on checksum failure, or error status. 





        

<hr>



### function syn\_mbus\_decoder\_feed 

_Feed a single byte to the streaming M-Bus decoder._ 
```C++
void syn_mbus_decoder_feed (
    SYN_MBUS_Decoder * dec,
    uint8_t byte
) 
```





**Parameters:**


* `dec` Decoder instance. 
* `byte` Byte received from UART/RS232/transceiver. 




        

<hr>



### function syn\_mbus\_decoder\_init 

_Initialize an M-Bus streaming state machine decoder._ 
```C++
void syn_mbus_decoder_init (
    SYN_MBUS_Decoder * dec,
    SYN_MBUS_FrameCallback callback,
    void * ctx
) 
```





**Parameters:**


* `dec` Decoder instance. 
* `callback` Callback called upon successful frame completion. 
* `ctx` User context passed to callback. 




        

<hr>



### function syn\_mbus\_decoder\_reset 

_Reset the streaming decoder state._ 
```C++
void syn_mbus_decoder_reset (
    SYN_MBUS_Decoder * dec
) 
```





**Parameters:**


* `dec` Decoder instance. 




        

<hr>



### function syn\_mbus\_encode\_ack 

_Encode a Single Character ACK frame (0xE5)._ 
```C++
SYN_Status syn_mbus_encode_ack (
    uint8_t * buf,
    size_t cap,
    size_t * out_len
) 
```





**Parameters:**


* `buf` Output buffer. 
* `cap` Buffer capacity (must be &gt;= 1). 
* `out_len` Receives total bytes written (1). 



**Returns:**

SYN\_OK on success, or error status. 





        

<hr>



### function syn\_mbus\_encode\_control 

_Encode a Control Frame (9 bytes: 0x68 \| 0x03 \| 0x03 \| 0x68 \| C \| A \| CI \| Checksum \| 0x16)._ 
```C++
SYN_Status syn_mbus_encode_control (
    uint8_t c_field,
    uint8_t a_field,
    uint8_t ci_field,
    uint8_t * buf,
    size_t cap,
    size_t * out_len
) 
```





**Parameters:**


* `c_field` Control field byte. 
* `a_field` Address field byte. 
* `ci_field` Control Information field byte. 
* `buf` Output buffer. 
* `cap` Buffer capacity (must be &gt;= 9). 
* `out_len` Receives total bytes written (9). 



**Returns:**

SYN\_OK on success, or error status. 





        

<hr>



### function syn\_mbus\_encode\_long 

_Encode a Long Frame (0x68 \| L \| L \| 0x68 \| C \| A \| CI \| Payload... \| Checksum \| 0x16)._ 
```C++
SYN_Status syn_mbus_encode_long (
    uint8_t c_field,
    uint8_t a_field,
    uint8_t ci_field,
    const uint8_t * payload,
    uint8_t payload_len,
    uint8_t * buf,
    size_t cap,
    size_t * out_len
) 
```





**Parameters:**


* `c_field` Control field byte. 
* `a_field` Address field byte. 
* `ci_field` Control Information field byte. 
* `payload` Pointer to payload data buffer. 
* `payload_len` Payload length (max 252 bytes). 
* `buf` Output buffer. 
* `cap` Buffer capacity (must be &gt;= payload\_len + 9). 
* `out_len` Receives total bytes written. 



**Returns:**

SYN\_OK on success, or error status. 





        

<hr>



### function syn\_mbus\_encode\_short 

_Encode a Short Frame (5 bytes: 0x10 \| C \| A \| Checksum \| 0x16)._ 
```C++
SYN_Status syn_mbus_encode_short (
    uint8_t c_field,
    uint8_t a_field,
    uint8_t * buf,
    size_t cap,
    size_t * out_len
) 
```





**Parameters:**


* `c_field` Control field byte. 
* `a_field` Address field byte. 
* `buf` Output buffer. 
* `cap` Buffer capacity (must be &gt;= 5). 
* `out_len` Receives total bytes written (5). 



**Returns:**

SYN\_OK on success, or error status. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mbus.c`

