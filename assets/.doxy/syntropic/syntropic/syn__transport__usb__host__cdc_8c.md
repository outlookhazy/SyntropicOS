

# File syn\_transport\_usb\_host\_cdc.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_transport\_usb\_host\_cdc.c**](syn__transport__usb__host__cdc_8c.md)

[Go to the source code of this file](syn__transport__usb__host__cdc_8c_source.md)

_USB Host CDC Transport Binding Implementation._ 

* `#include "syntropic/drivers/syn_transport_usb_host_cdc.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_transport\_from\_usb\_host\_cdc**](#function-syn_transport_from_usb_host_cdc) ([**SYN\_Transport**](structSYN__Transport.md) \* t, [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc) <br>_Bind_ [_**SYN\_USB\_HostCDC**_](structSYN__USB__HostCDC.md) _instance to a_[_**SYN\_Transport**_](structSYN__Transport.md) _interface._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**host\_cdc\_transport\_recv**](#function-host_cdc_transport_recv) (uint8\_t \* data, size\_t max\_len, size\_t \* out\_len, void \* ctx) <br>_Internal transport receive wrapper for USB Host CDC._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**host\_cdc\_transport\_send**](#function-host_cdc_transport_send) (const uint8\_t \* data, size\_t len, void \* ctx) <br>_Internal transport send wrapper for USB Host CDC._  |


























## Public Functions Documentation




### function syn\_transport\_from\_usb\_host\_cdc 

_Bind_ [_**SYN\_USB\_HostCDC**_](structSYN__USB__HostCDC.md) _instance to a_[_**SYN\_Transport**_](structSYN__Transport.md) _interface._
```C++
void syn_transport_from_usb_host_cdc (
    SYN_Transport * t,
    SYN_USB_HostCDC * hcdc
) 
```





**Parameters:**


* `t` Pointer to [**SYN\_Transport**](structSYN__Transport.md) struct to initialize. 
* `hcdc` Pointer to [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) instance. 




        

<hr>
## Public Static Functions Documentation




### function host\_cdc\_transport\_recv 

_Internal transport receive wrapper for USB Host CDC._ 
```C++
static bool host_cdc_transport_recv (
    uint8_t * data,
    size_t max_len,
    size_t * out_len,
    void * ctx
) 
```





**Parameters:**


* `data` Output data buffer. 
* `max_len` Capacity. 
* `out_len` Pointer to receive read byte count. 
* `ctx` Context pointer ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md)). 



**Returns:**

true on success. 





        

<hr>



### function host\_cdc\_transport\_send 

_Internal transport send wrapper for USB Host CDC._ 
```C++
static bool host_cdc_transport_send (
    const uint8_t * data,
    size_t len,
    void * ctx
) 
```





**Parameters:**


* `data` Packet data buffer. 
* `len` Byte length. 
* `ctx` Context pointer ([**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md)). 



**Returns:**

true on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_transport_usb_host_cdc.c`

