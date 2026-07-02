

# File syn\_can.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_can.c**](syn__can_8c.md)

[Go to the source code of this file](syn__can_8c_source.md)

_CAN bus driver implementation._ 

* `#include "syn_can.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_can\_init**](#function-syn_can_init) ([**SYN\_CAN**](structSYN__CAN.md) \* can, uint8\_t port, uint32\_t bitrate) <br>_Initialize CAN driver._  |
|  void | [**syn\_can\_on\_receive**](#function-syn_can_on_receive) ([**SYN\_CAN**](structSYN__CAN.md) \* can, [**SYN\_CAN\_Callback**](syn__can_8h.md#typedef-syn_can_callback) cb, void \* ctx) <br>_Register receive callback._  |
|  void | [**syn\_can\_poll**](#function-syn_can_poll) ([**SYN\_CAN**](structSYN__CAN.md) \* can) <br>_Poll for received frames and dispatch callback._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_can\_send**](#function-syn_can_send) ([**SYN\_CAN**](structSYN__CAN.md) \* can, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Send a CAN frame._  |
|  void | [**syn\_can\_set\_filter**](#function-syn_can_set_filter) (const [**SYN\_CAN**](structSYN__CAN.md) \* can, uint32\_t id, uint32\_t mask) <br>_Set hardware acceptance filter._  |




























## Public Functions Documentation




### function syn\_can\_init 

_Initialize CAN driver._ 
```C++
SYN_Status syn_can_init (
    SYN_CAN * can,
    uint8_t port,
    uint32_t bitrate
) 
```





**Parameters:**


* `can` CAN instance. 
* `port` CAN peripheral port number. 
* `bitrate` Bitrate in bps (e.g. 500000). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_can\_on\_receive 

_Register receive callback._ 
```C++
void syn_can_on_receive (
    SYN_CAN * can,
    SYN_CAN_Callback cb,
    void * ctx
) 
```





**Parameters:**


* `can` CAN instance. 
* `cb` Callback function. 
* `ctx` User context. 




        

<hr>



### function syn\_can\_poll 

_Poll for received frames and dispatch callback._ 
```C++
void syn_can_poll (
    SYN_CAN * can
) 
```



Call from your main loop.




**Parameters:**


* `can` CAN instance. 




        

<hr>



### function syn\_can\_send 

_Send a CAN frame._ 
```C++
bool syn_can_send (
    SYN_CAN * can,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `can` CAN instance. 
* `frame` Frame to send. 



**Returns:**

true if frame was queued successfully. 





        

<hr>



### function syn\_can\_set\_filter 

_Set hardware acceptance filter._ 
```C++
void syn_can_set_filter (
    const SYN_CAN * can,
    uint32_t id,
    uint32_t mask
) 
```





**Parameters:**


* `can` CAN instance. 
* `id` Filter ID. 
* `mask` Filter mask. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_can.c`

