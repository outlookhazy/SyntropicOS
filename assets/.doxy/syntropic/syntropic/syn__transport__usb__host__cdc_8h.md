

# File syn\_transport\_usb\_host\_cdc.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_transport\_usb\_host\_cdc.h**](syn__transport__usb__host__cdc_8h.md)

[Go to the source code of this file](syn__transport__usb__host__cdc_8h_source.md)

_USB Host CDC Transport Binding Header._ 

* `#include "syntropic/drivers/syn_usb_host_cdc.h"`
* `#include "syntropic/net/syn_transport.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_transport\_from\_usb\_host\_cdc**](#function-syn_transport_from_usb_host_cdc) ([**SYN\_Transport**](structSYN__Transport.md) \* t, [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md) \* hcdc) <br>_Bind_ [_**SYN\_USB\_HostCDC**_](structSYN__USB__HostCDC.md) _instance to a_[_**SYN\_Transport**_](structSYN__Transport.md) _interface._ |




























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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_transport_usb_host_cdc.h`

