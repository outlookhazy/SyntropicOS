

# File syn\_can.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_can.h**](syn__can_8h.md)

[Go to the source code of this file](syn__can_8h_source.md)

_CAN bus driver abstraction._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_can.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CAN**](structSYN__CAN.md) <br>_CAN bus instance — port, bitrate, callbacks, stats._  |
| struct | [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) <br>_CAN bus frame — standard or extended._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_CAN\_Callback**](#typedef-syn_can_callback)  <br>_CAN receive callback._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_can\_init**](#function-syn_can_init) ([**SYN\_CAN**](structSYN__CAN.md) \* can, uint8\_t port, uint32\_t bitrate) <br>_Initialize CAN driver._  |
|  void | [**syn\_can\_on\_receive**](#function-syn_can_on_receive) ([**SYN\_CAN**](structSYN__CAN.md) \* can, [**SYN\_CAN\_Callback**](syn__can_8h.md#typedef-syn_can_callback) cb, void \* ctx) <br>_Register receive callback._  |
|  void | [**syn\_can\_poll**](#function-syn_can_poll) ([**SYN\_CAN**](structSYN__CAN.md) \* can) <br>_Poll for received frames and dispatch callback._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_can\_send**](#function-syn_can_send) ([**SYN\_CAN**](structSYN__CAN.md) \* can, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Send a CAN frame._  |
|  void | [**syn\_can\_set\_filter**](#function-syn_can_set_filter) (const [**SYN\_CAN**](structSYN__CAN.md) \* can, uint32\_t id, uint32\_t mask) <br>_Set hardware acceptance filter._  |




























## Detailed Description


Port-based CAN driver with frame send/receive and callback dispatch.


Usage: 
```C++
static SYN_CAN can;
syn_can_init(&can, 0, 500000);  // CAN0 at 500kbps
syn_can_on_receive(&can, my_rx_handler, NULL);

SYN_CAN_Frame tx = { .id = 0x100, .dlc = 2 };
tx.data[0] = 0x42; tx.data[1] = 0x00;
syn_can_send(&can, &tx);

// In main loop:
syn_can_poll(&can);  // dispatches received frames
```
 


    
## Public Types Documentation




### typedef SYN\_CAN\_Callback 

_CAN receive callback._ 
```C++
typedef void(* SYN_CAN_Callback) (const SYN_CAN_Frame *frame, void *ctx);
```





**Parameters:**


* `frame` Received frame. 
* `ctx` User context. 




        

<hr>
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
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_can.h`

