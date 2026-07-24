

# File syn\_cobs.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cobs.c**](syn__cobs_8c.md)

[Go to the source code of this file](syn__cobs_8c_source.md)

_COBS packet framing implementation._ 

* `#include "syn_cobs.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_cobs\_decode**](#function-syn_cobs_decode) (const void \* src, size\_t src\_len, void \* dst) <br>_Decode a COBS-encoded packet._  |
|  void | [**syn\_cobs\_decoder\_feed**](#function-syn_cobs_decoder_feed) ([**SYN\_COBS\_Decoder**](structSYN__COBS__Decoder.md) \* dec, uint8\_t byte) <br>_Feed a byte to the streaming decoder._  |
|  void | [**syn\_cobs\_decoder\_init**](#function-syn_cobs_decoder_init) ([**SYN\_COBS\_Decoder**](structSYN__COBS__Decoder.md) \* dec, uint8\_t \* buf, size\_t buf\_size, [**SYN\_COBS\_PacketCallback**](syn__cobs_8h.md#typedef-syn_cobs_packetcallback) callback, void \* ctx) <br>_Initialize streaming COBS decoder._  |
|  void | [**syn\_cobs\_decoder\_reset**](#function-syn_cobs_decoder_reset) ([**SYN\_COBS\_Decoder**](structSYN__COBS__Decoder.md) \* dec) <br>_Reset the streaming decoder state._  |
|  size\_t | [**syn\_cobs\_encode**](#function-syn_cobs_encode) (const void \* src, size\_t src\_len, void \* dst) <br>_Encode data using COBS._  |




























## Public Functions Documentation




### function syn\_cobs\_decode 

_Decode a COBS-encoded packet._ 
```C++
size_t syn_cobs_decode (
    const void * src,
    size_t src_len,
    void * dst
) 
```





**Parameters:**


* `src` COBS-encoded data (without the 0x00 delimiter). 
* `src_len` Encoded length. 
* `dst` Output buffer (must be at least src\_len). 



**Returns:**

Number of decoded bytes, or 0 on error. 





        

<hr>



### function syn\_cobs\_decoder\_feed 

_Feed a byte to the streaming decoder._ 
```C++
void syn_cobs_decoder_feed (
    SYN_COBS_Decoder * dec,
    uint8_t byte
) 
```



When a 0x00 delimiter is received, the accumulated bytes are decoded and the callback is invoked with the decoded packet.




**Parameters:**


* `dec` Decoder. 
* `byte` Received byte. 




        

<hr>



### function syn\_cobs\_decoder\_init 

_Initialize streaming COBS decoder._ 
```C++
void syn_cobs_decoder_init (
    SYN_COBS_Decoder * dec,
    uint8_t * buf,
    size_t buf_size,
    SYN_COBS_PacketCallback callback,
    void * ctx
) 
```





**Parameters:**


* `dec` Decoder instance. 
* `buf` Receive buffer for accumulating encoded bytes. 
* `buf_size` Buffer capacity. 
* `callback` Called when a complete packet is decoded. 
* `ctx` User context for callback. 




        

<hr>



### function syn\_cobs\_decoder\_reset 

_Reset the streaming decoder state._ 
```C++
void syn_cobs_decoder_reset (
    SYN_COBS_Decoder * dec
) 
```





**Parameters:**


* `dec` Decoder. 




        

<hr>



### function syn\_cobs\_encode 

_Encode data using COBS._ 
```C++
size_t syn_cobs_encode (
    const void * src,
    size_t src_len,
    void * dst
) 
```





**Parameters:**


* `src` Input data. 
* `src_len` Input length. 
* `dst` Output buffer (must be at least src\_len + src\_len/254 + 1). 



**Returns:**

Number of bytes written to dst (excluding the trailing 0x00 delimiter). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cobs.c`

