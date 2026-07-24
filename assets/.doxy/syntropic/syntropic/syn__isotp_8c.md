

# File syn\_isotp.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_isotp.c**](syn__isotp_8c.md)

[Go to the source code of this file](syn__isotp_8c_source.md)

_ISO 15765-2 (ISO-TP) Multi-Frame CAN Transport implementation._ 

* `#include "syn_isotp.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_isotp\_get\_tx\_frame**](#function-syn_isotp_get_tx_frame) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Retrieve next pending outgoing CAN frame from ISO-TP engine._  |
|  void | [**syn\_isotp\_init**](#function-syn_isotp_init) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, uint32\_t rx\_id, uint32\_t tx\_id, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize ISO-TP Link in Classic CAN mode (8-byte frames)._  |
|  void | [**syn\_isotp\_process\_rx\_frame**](#function-syn_isotp_process_rx_frame) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process an incoming CAN frame into the ISO-TP engine._  |
|  ssize\_t | [**syn\_isotp\_receive**](#function-syn_isotp_receive) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, uint8\_t \* out\_buf, size\_t max\_len) <br>_Read completed assembled message from ISO-TP receiver._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_isotp\_send**](#function-syn_isotp_send) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, const uint8\_t \* payload, size\_t payload\_len) <br>_Initiate transmission of a multi-byte payload._  |
|  void | [**syn\_isotp\_set\_timeouts**](#function-syn_isotp_set_timeouts) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, uint32\_t n\_bs\_ms, uint32\_t n\_cr\_ms) <br>_Configure custom ISO 15765-2 network layer timeouts._  |
|  void | [**syn\_isotp\_step**](#function-syn_isotp_step) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, uint32\_t dt\_ms) <br>_Step ISO-TP timers (milliseconds)._  |
|  void | [**syn\_isotp\_step\_us**](#function-syn_isotp_step_us) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, uint32\_t dt\_us) <br>_Step ISO-TP timers (microseconds for fine STmin pacing)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_isotp\_decode\_stmin\_us**](#function-syn_isotp_decode_stmin_us) (uint8\_t st\_min) <br> |


























## Public Functions Documentation




### function syn\_isotp\_get\_tx\_frame 

_Retrieve next pending outgoing CAN frame from ISO-TP engine._ 
```C++
bool syn_isotp_get_tx_frame (
    SYN_ISOTP_Link * link,
    SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `link` Link handle. 
* `frame` Output CAN frame buffer. 



**Returns:**

true if a frame was generated, false otherwise. 





        

<hr>



### function syn\_isotp\_init 

_Initialize ISO-TP Link in Classic CAN mode (8-byte frames)._ 
```C++
void syn_isotp_init (
    SYN_ISOTP_Link * link,
    uint32_t rx_id,
    uint32_t tx_id,
    uint8_t * rx_buf,
    size_t rx_buf_size,
    uint8_t * tx_buf,
    size_t tx_buf_size
) 
```





**Parameters:**


* `link` Link handle. 
* `rx_id` Incoming CAN identifier. 
* `tx_id` Outgoing CAN identifier. 
* `rx_buf` Receive assembly buffer. 
* `rx_buf_size` Receive buffer capacity. 
* `tx_buf` Transmit buffer. 
* `tx_buf_size` Transmit buffer capacity. 




        

<hr>



### function syn\_isotp\_process\_rx\_frame 

_Process an incoming CAN frame into the ISO-TP engine._ 
```C++
void syn_isotp_process_rx_frame (
    SYN_ISOTP_Link * link,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `link` Link handle. 
* `frame` Incoming CAN frame. 




        

<hr>



### function syn\_isotp\_receive 

_Read completed assembled message from ISO-TP receiver._ 
```C++
ssize_t syn_isotp_receive (
    SYN_ISOTP_Link * link,
    uint8_t * out_buf,
    size_t max_len
) 
```





**Parameters:**


* `link` Link handle. 
* `out_buf` Output buffer. 
* `max_len` Capacity of out\_buf. 



**Returns:**

Number of bytes copied, or -1 if no message ready. 





        

<hr>



### function syn\_isotp\_send 

_Initiate transmission of a multi-byte payload._ 
```C++
SYN_Status syn_isotp_send (
    SYN_ISOTP_Link * link,
    const uint8_t * payload,
    size_t payload_len
) 
```





**Parameters:**


* `link` Link handle. 
* `payload` Payload buffer. 
* `payload_len` Length of payload (1 to max buffer capacity). 



**Returns:**

SYN\_OK on success, error code on busy/invalid params. 





        

<hr>



### function syn\_isotp\_set\_timeouts 

_Configure custom ISO 15765-2 network layer timeouts._ 
```C++
void syn_isotp_set_timeouts (
    SYN_ISOTP_Link * link,
    uint32_t n_bs_ms,
    uint32_t n_cr_ms
) 
```





**Parameters:**


* `link` Link handle. 
* `n_bs_ms` N\_Bs max timeout in milliseconds (0 = use default 1000ms). 
* `n_cr_ms` N\_Cr max timeout in milliseconds (0 = use default 1000ms). 




        

<hr>



### function syn\_isotp\_step 

_Step ISO-TP timers (milliseconds)._ 
```C++
void syn_isotp_step (
    SYN_ISOTP_Link * link,
    uint32_t dt_ms
) 
```





**Parameters:**


* `link` Link handle. 
* `dt_ms` Elapsed time in milliseconds. 




        

<hr>



### function syn\_isotp\_step\_us 

_Step ISO-TP timers (microseconds for fine STmin pacing)._ 
```C++
void syn_isotp_step_us (
    SYN_ISOTP_Link * link,
    uint32_t dt_us
) 
```





**Parameters:**


* `link` Link handle. 
* `dt_us` Elapsed time in microseconds. 




        

<hr>
## Public Static Functions Documentation




### function syn\_isotp\_decode\_stmin\_us 

```C++
static uint32_t syn_isotp_decode_stmin_us (
    uint8_t st_min
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_isotp.c`

