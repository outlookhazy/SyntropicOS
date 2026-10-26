

# File syn\_transport\_usb\_cdc.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_transport\_usb\_cdc.c**](syn__transport__usb__cdc_8c.md)

[Go to the source code of this file](syn__transport__usb__cdc_8c_source.md)

_USB CDC Transport Binding Implementation._ 

* `#include "syntropic/drivers/syn_transport_usb_cdc.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_transport\_from\_usb\_cdc**](#function-syn_transport_from_usb_cdc) ([**SYN\_Transport**](structSYN__Transport.md) \* t, [**SYN\_USB\_CDC**](structSYN__USB__CDC.md) \* cdc) <br>_Bind_ [_**SYN\_USB\_CDC**_](structSYN__USB__CDC.md) _instance to a_[_**SYN\_Transport**_](structSYN__Transport.md) _interface._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**cdc\_transport\_recv**](#function-cdc_transport_recv) (uint8\_t \* data, size\_t max\_len, size\_t \* out\_len, void \* ctx) <br>_Receive packet via USB CDC transport._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**cdc\_transport\_send**](#function-cdc_transport_send) (const uint8\_t \* data, size\_t len, void \* ctx) <br>_Transmit packet via USB CDC transport._  |


























## Public Functions Documentation




### function syn\_transport\_from\_usb\_cdc 

_Bind_ [_**SYN\_USB\_CDC**_](structSYN__USB__CDC.md) _instance to a_[_**SYN\_Transport**_](structSYN__Transport.md) _interface._
```C++
void syn_transport_from_usb_cdc (
    SYN_Transport * t,
    SYN_USB_CDC * cdc
) 
```





**Parameters:**


* `t` Pointer to [**SYN\_Transport**](structSYN__Transport.md) struct to initialize. 
* `cdc` Pointer to [**SYN\_USB\_CDC**](structSYN__USB__CDC.md) instance. 




        

<hr>
## Public Static Functions Documentation




### function cdc\_transport\_recv 

_Receive packet via USB CDC transport._ 
```C++
static bool cdc_transport_recv (
    uint8_t * data,
    size_t max_len,
    size_t * out_len,
    void * ctx
) 
```





**Parameters:**


* `data` Buffer to receive payload. 
* `max_len` Capacity. 
* `out_len` Pointer to receive read byte count. 
* `ctx` Pointer to [**SYN\_USB\_CDC**](structSYN__USB__CDC.md) instance context. 



**Returns:**

true if packet read successfully. 





        

<hr>



### function cdc\_transport\_send 

_Transmit packet via USB CDC transport._ 
```C++
static bool cdc_transport_send (
    const uint8_t * data,
    size_t len,
    void * ctx
) 
```





**Parameters:**


* `data` Packet payload buffer. 
* `len` Packet length. 
* `ctx` Pointer to [**SYN\_USB\_CDC**](structSYN__USB__CDC.md) instance context. 



**Returns:**

true on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_transport_usb_cdc.c`

