

# File syn\_dmx512.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dmx512.h**](syn__dmx512_8h.md)

[Go to the source code of this file](syn__dmx512_8h_source.md)

_USITT DMX512-A Stage & Architectural Lighting Protocol Stack._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DMX512\_Master**](structSYN__DMX512__Master.md) <br>_DMX512 Master Handle._  |
| struct | [**SYN\_DMX512\_Slave**](structSYN__DMX512__Slave.md) <br>_DMX512 Slave Handle._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_DMX512\_RxState**](#enum-syn_dmx512_rxstate)  <br>_DMX512 Receiver State Machine States._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_dmx512\_master\_build\_frame**](#function-syn_dmx512_master_build_frame) (const [**SYN\_DMX512\_Master**](structSYN__DMX512__Master.md) \* master, uint8\_t \* out\_buf, size\_t max\_len) <br>_Build DMX packet payload (Start Code + Channel Slots)._  |
|  uint8\_t | [**syn\_dmx512\_master\_get\_channel**](#function-syn_dmx512_master_get_channel) (const [**SYN\_DMX512\_Master**](structSYN__DMX512__Master.md) \* master, uint16\_t channel) <br>_Get channel value (1-indexed channel, 1..512)._  |
|  void | [**syn\_dmx512\_master\_init**](#function-syn_dmx512_master_init) ([**SYN\_DMX512\_Master**](structSYN__DMX512__Master.md) \* master, uint16\_t num\_channels) <br>_Initialize DMX512 Master engine._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dmx512\_master\_set\_channel**](#function-syn_dmx512_master_set_channel) ([**SYN\_DMX512\_Master**](structSYN__DMX512__Master.md) \* master, uint16\_t channel, uint8\_t value) <br>_Set channel value (1-indexed channel, 1..512)._  |
|  uint8\_t | [**syn\_dmx512\_slave\_get\_channel**](#function-syn_dmx512_slave_get_channel) (const [**SYN\_DMX512\_Slave**](structSYN__DMX512__Slave.md) \* slave, uint16\_t offset) <br>_Read channel value from local slave footprint (0-indexed offset)._  |
|  void | [**syn\_dmx512\_slave\_init**](#function-syn_dmx512_slave_init) ([**SYN\_DMX512\_Slave**](structSYN__DMX512__Slave.md) \* slave, uint16\_t start\_address, uint16\_t footprint) <br>_Initialize DMX512 Slave engine._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dmx512\_slave\_is\_updated**](#function-syn_dmx512_slave_is_updated) ([**SYN\_DMX512\_Slave**](structSYN__DMX512__Slave.md) \* slave) <br>_Check if a complete DMX packet was received since last query._  |
|  void | [**syn\_dmx512\_slave\_rx\_break**](#function-syn_dmx512_slave_rx_break) ([**SYN\_DMX512\_Slave**](structSYN__DMX512__Slave.md) \* slave) <br>_Signal line Break event to Slave engine (e.g. from UART framing error interrupt)._  |
|  void | [**syn\_dmx512\_slave\_rx\_byte**](#function-syn_dmx512_slave_rx_byte) ([**SYN\_DMX512\_Slave**](structSYN__DMX512__Slave.md) \* slave, uint8\_t byte) <br>_Ingest incoming byte stream into Slave engine._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_DMX512\_BAUD\_RATE**](syn__dmx512_8h.md#define-syn_dmx512_baud_rate)  `250000UL`<br> |
| define  | [**SYN\_DMX512\_MAX\_CHANNELS**](syn__dmx512_8h.md#define-syn_dmx512_max_channels)  `512U`<br>_DMX512 Protocol Constants._  |
| define  | [**SYN\_DMX512\_START\_CODE\_NULL**](syn__dmx512_8h.md#define-syn_dmx512_start_code_null)  `0x00U`<br> |

## Detailed Description


Provides non-blocking, zero-malloc DMX512 Master transmitter and Slave receiver engines supporting standard 512-channel universes, start code filtering, start address slot mapping, and UART framing breaking. 


    
## Public Types Documentation




### enum SYN\_DMX512\_RxState 

_DMX512 Receiver State Machine States._ 
```C++
enum SYN_DMX512_RxState {
    SYN_DMX512_STATE_WAIT_BREAK = 0,
    SYN_DMX512_STATE_WAIT_START,
    SYN_DMX512_STATE_READ_SLOTS
};
```




<hr>
## Public Functions Documentation




### function syn\_dmx512\_master\_build\_frame 

_Build DMX packet payload (Start Code + Channel Slots)._ 
```C++
size_t syn_dmx512_master_build_frame (
    const SYN_DMX512_Master * master,
    uint8_t * out_buf,
    size_t max_len
) 
```





**Parameters:**


* `master` Master handle. 
* `out_buf` Output buffer for frame payload. 
* `max_len` Maximum capacity of out\_buf. 



**Returns:**

Number of payload bytes written (1 + num\_channels). 





        

<hr>



### function syn\_dmx512\_master\_get\_channel 

_Get channel value (1-indexed channel, 1..512)._ 
```C++
uint8_t syn_dmx512_master_get_channel (
    const SYN_DMX512_Master * master,
    uint16_t channel
) 
```





**Parameters:**


* `master` Master handle. 
* `channel` Channel index (1..512). 



**Returns:**

8-bit slot value. 





        

<hr>



### function syn\_dmx512\_master\_init 

_Initialize DMX512 Master engine._ 
```C++
void syn_dmx512_master_init (
    SYN_DMX512_Master * master,
    uint16_t num_channels
) 
```





**Parameters:**


* `master` Master handle. 
* `num_channels` Active universe length (1..512, default 512). 




        

<hr>



### function syn\_dmx512\_master\_set\_channel 

_Set channel value (1-indexed channel, 1..512)._ 
```C++
SYN_Status syn_dmx512_master_set_channel (
    SYN_DMX512_Master * master,
    uint16_t channel,
    uint8_t value
) 
```





**Parameters:**


* `master` Master handle. 
* `channel` Channel index (1..512). 
* `value` 8-bit slot value (0..255). 



**Returns:**

SYN\_OK on success, error code on invalid channel. 





        

<hr>



### function syn\_dmx512\_slave\_get\_channel 

_Read channel value from local slave footprint (0-indexed offset)._ 
```C++
uint8_t syn_dmx512_slave_get_channel (
    const SYN_DMX512_Slave * slave,
    uint16_t offset
) 
```





**Parameters:**


* `slave` Slave handle. 
* `offset` Footprint channel offset (0 .. footprint - 1). 



**Returns:**

Channel value byte. 





        

<hr>



### function syn\_dmx512\_slave\_init 

_Initialize DMX512 Slave engine._ 
```C++
void syn_dmx512_slave_init (
    SYN_DMX512_Slave * slave,
    uint16_t start_address,
    uint16_t footprint
) 
```





**Parameters:**


* `slave` Slave handle. 
* `start_address` Start channel address (1..512). 
* `footprint` Channel footprint length (e.g. 4 for 4-channel fixture). 




        

<hr>



### function syn\_dmx512\_slave\_is\_updated 

_Check if a complete DMX packet was received since last query._ 
```C++
bool syn_dmx512_slave_is_updated (
    SYN_DMX512_Slave * slave
) 
```





**Parameters:**


* `slave` Slave handle. 



**Returns:**

true if updated, false otherwise. 





        

<hr>



### function syn\_dmx512\_slave\_rx\_break 

_Signal line Break event to Slave engine (e.g. from UART framing error interrupt)._ 
```C++
void syn_dmx512_slave_rx_break (
    SYN_DMX512_Slave * slave
) 
```





**Parameters:**


* `slave` Slave handle. 




        

<hr>



### function syn\_dmx512\_slave\_rx\_byte 

_Ingest incoming byte stream into Slave engine._ 
```C++
void syn_dmx512_slave_rx_byte (
    SYN_DMX512_Slave * slave,
    uint8_t byte
) 
```





**Parameters:**


* `slave` Slave handle. 
* `byte` Incoming byte from UART rx. 




        

<hr>
## Macro Definition Documentation





### define SYN\_DMX512\_BAUD\_RATE 

```C++
#define SYN_DMX512_BAUD_RATE `250000UL`
```




<hr>



### define SYN\_DMX512\_MAX\_CHANNELS 

_DMX512 Protocol Constants._ 
```C++
#define SYN_DMX512_MAX_CHANNELS `512U`
```




<hr>



### define SYN\_DMX512\_START\_CODE\_NULL 

```C++
#define SYN_DMX512_START_CODE_NULL `0x00U`
```



Standard dimmer start code 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dmx512.h`

