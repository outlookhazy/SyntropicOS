

# File syn\_dmx512.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dmx512.c**](syn__dmx512_8c.md)

[Go to the source code of this file](syn__dmx512_8c_source.md)

_USITT DMX512-A Stage Lighting Protocol implementation._ 

* `#include "syn_dmx512.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dmx512.c`

