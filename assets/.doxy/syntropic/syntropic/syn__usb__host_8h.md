

# File syn\_usb\_host.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_host.h**](syn__usb__host_8h.md)

[Go to the source code of this file](syn__usb__host_8h_source.md)

_Zero-Heap USB 2.0 Host Core Protocol Engine._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/drivers/syn_usb.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "syntropic/pt/syn_pt.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_USB\_Host**](structSYN__USB__Host.md) <br> |
| struct | [**SYN\_USB\_HostClassDriver**](structSYN__USB__HostClassDriver.md) <br> |
| struct | [**SYN\_USB\_HostDevInfo**](structSYN__USB__HostDevInfo.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_USB\_HostDevInfo**](structSYN__USB__HostDevInfo.md) \* | [**syn\_usb\_host\_get\_dev\_info**](#function-syn_usb_host_get_dev_info) (const [**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Get cached device information from last enumeration._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_init**](#function-syn_usb_host_init) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Initialize USB Host Core._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_host\_is\_ready**](#function-syn_usb_host_is_ready) (const [**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Check if USB Host has a device enumerated and ready._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_process**](#function-syn_usb_host_process) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Process USB Host state machine (call each scheduler tick)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_register\_class**](#function-syn_usb_host_register_class) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host, const [**SYN\_USB\_HostClassDriver**](structSYN__USB__HostClassDriver.md) \* cls) <br>_Register a host class driver for interface matching._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_USB\_HOST\_WAIT\_READY**](syn__usb__host_8h.md#define-pt_usb_host_wait_ready) (pt, host) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_usb\_host\_is\_ready**](syn__usb__host_8c.md#function-syn_usb_host_is_ready)(host))`<br>_Block a protothread coroutine until a USB device is enumerated and ready._  |
| define  | [**SYN\_USB\_HOST\_ENUM\_BUF\_SIZE**](syn__usb__host_8h.md#define-syn_usb_host_enum_buf_size)  `256U`<br> |
| define  | [**SYN\_USB\_HOST\_ENUM\_CLASS\_PROBE**](syn__usb__host_8h.md#define-syn_usb_host_enum_class_probe)  `5U`<br> |
| define  | [**SYN\_USB\_HOST\_ENUM\_DONE**](syn__usb__host_8h.md#define-syn_usb_host_enum_done)  `6U`<br> |
| define  | [**SYN\_USB\_HOST\_ENUM\_GET\_CFG**](syn__usb__host_8h.md#define-syn_usb_host_enum_get_cfg)  `3U`<br> |
| define  | [**SYN\_USB\_HOST\_ENUM\_GET\_DEV8**](syn__usb__host_8h.md#define-syn_usb_host_enum_get_dev8)  `0U`<br> |
| define  | [**SYN\_USB\_HOST\_ENUM\_GET\_DEV\_FULL**](syn__usb__host_8h.md#define-syn_usb_host_enum_get_dev_full)  `2U`<br> |
| define  | [**SYN\_USB\_HOST\_ENUM\_SET\_ADDR**](syn__usb__host_8h.md#define-syn_usb_host_enum_set_addr)  `1U`<br> |
| define  | [**SYN\_USB\_HOST\_ENUM\_SET\_CFG**](syn__usb__host_8h.md#define-syn_usb_host_enum_set_cfg)  `4U`<br> |
| define  | [**SYN\_USB\_HOST\_MAX\_CLASSES**](syn__usb__host_8h.md#define-syn_usb_host_max_classes)  `4U`<br> |
| define  | [**SYN\_USB\_HOST\_MAX\_PIPES**](syn__usb__host_8h.md#define-syn_usb_host_max_pipes)  `4U`<br> |
| define  | [**SYN\_USB\_HOST\_STATE\_ATTACHED**](syn__usb__host_8h.md#define-syn_usb_host_state_attached)  `1U`<br> |
| define  | [**SYN\_USB\_HOST\_STATE\_DISCONNECTED**](syn__usb__host_8h.md#define-syn_usb_host_state_disconnected)  `0U`<br> |
| define  | [**SYN\_USB\_HOST\_STATE\_ENUMERATING**](syn__usb__host_8h.md#define-syn_usb_host_state_enumerating)  `3U`<br> |
| define  | [**SYN\_USB\_HOST\_STATE\_ERROR**](syn__usb__host_8h.md#define-syn_usb_host_state_error)  `5U`<br> |
| define  | [**SYN\_USB\_HOST\_STATE\_READY**](syn__usb__host_8h.md#define-syn_usb_host_state_ready)  `4U`<br> |
| define  | [**SYN\_USB\_HOST\_STATE\_RESET**](syn__usb__host_8h.md#define-syn_usb_host_state_reset)  `2U`<br> |

## Detailed Description


Manages device attach/detach detection, bus reset, standard enumeration (GET\_DESCRIPTOR, SET\_ADDRESS, SET\_CONFIGURATION), and dispatches matched interfaces to registered host class drivers. 


    
## Public Functions Documentation




### function syn\_usb\_host\_get\_dev\_info 

_Get cached device information from last enumeration._ 
```C++
const SYN_USB_HostDevInfo * syn_usb_host_get_dev_info (
    const SYN_USB_Host * host
) 
```





**Parameters:**


* `host` Pointer to USB Host instance. 



**Returns:**

Pointer to device info struct, or NULL if no device. 





        

<hr>



### function syn\_usb\_host\_init 

_Initialize USB Host Core._ 
```C++
SYN_Status syn_usb_host_init (
    SYN_USB_Host * host
) 
```





**Parameters:**


* `host` Pointer to USB Host instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_host\_is\_ready 

_Check if USB Host has a device enumerated and ready._ 
```C++
bool syn_usb_host_is_ready (
    const SYN_USB_Host * host
) 
```





**Parameters:**


* `host` Pointer to USB Host instance. 



**Returns:**

true if host state is READY. 





        

<hr>



### function syn\_usb\_host\_process 

_Process USB Host state machine (call each scheduler tick)._ 
```C++
SYN_Status syn_usb_host_process (
    SYN_USB_Host * host
) 
```



Drives attach detection, bus reset, enumeration, class probing, and detach handling. Non-blocking.




**Parameters:**


* `host` Pointer to USB Host instance. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if transfer pending. 





        

<hr>



### function syn\_usb\_host\_register\_class 

_Register a host class driver for interface matching._ 
```C++
SYN_Status syn_usb_host_register_class (
    SYN_USB_Host * host,
    const SYN_USB_HostClassDriver * cls
) 
```





**Parameters:**


* `host` Pointer to USB Host instance. 
* `cls` Host class driver vtable. 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define PT\_USB\_HOST\_WAIT\_READY 

_Block a protothread coroutine until a USB device is enumerated and ready._ 
```C++
#define PT_USB_HOST_WAIT_READY (
    pt,
    host
) `PT_WAIT_UNTIL (pt, syn_usb_host_is_ready (host))`
```





**Parameters:**


* `pt` Protothread context. 
* `host` Pointer to USB Host instance. 




        

<hr>



### define SYN\_USB\_HOST\_ENUM\_BUF\_SIZE 

```C++
#define SYN_USB_HOST_ENUM_BUF_SIZE `256U`
```



Enumeration descriptor scratch buffer 


        

<hr>



### define SYN\_USB\_HOST\_ENUM\_CLASS\_PROBE 

```C++
#define SYN_USB_HOST_ENUM_CLASS_PROBE `5U`
```



Match & probe class drivers 


        

<hr>



### define SYN\_USB\_HOST\_ENUM\_DONE 

```C++
#define SYN_USB_HOST_ENUM_DONE `6U`
```



Enumeration complete 


        

<hr>



### define SYN\_USB\_HOST\_ENUM\_GET\_CFG 

```C++
#define SYN_USB_HOST_ENUM_GET_CFG `3U`
```



Read Configuration Descriptor 


        

<hr>



### define SYN\_USB\_HOST\_ENUM\_GET\_DEV8 

```C++
#define SYN_USB_HOST_ENUM_GET_DEV8 `0U`
```



Enumeration sub-steps (internal to ENUMERATING state) Read first 8 bytes of Device Desc 


        

<hr>



### define SYN\_USB\_HOST\_ENUM\_GET\_DEV\_FULL 

```C++
#define SYN_USB_HOST_ENUM_GET_DEV_FULL `2U`
```



Read full 18-byte Device Desc 


        

<hr>



### define SYN\_USB\_HOST\_ENUM\_SET\_ADDR 

```C++
#define SYN_USB_HOST_ENUM_SET_ADDR `1U`
```



Assign bus address 


        

<hr>



### define SYN\_USB\_HOST\_ENUM\_SET\_CFG 

```C++
#define SYN_USB_HOST_ENUM_SET_CFG `4U`
```



Issue SET\_CONFIGURATION 


        

<hr>



### define SYN\_USB\_HOST\_MAX\_CLASSES 

```C++
#define SYN_USB_HOST_MAX_CLASSES `4U`
```



Maximum registered host class drivers 


        

<hr>



### define SYN\_USB\_HOST\_MAX\_PIPES 

```C++
#define SYN_USB_HOST_MAX_PIPES `4U`
```



Maximum host pipes (EP0 + data pipes) 


        

<hr>



### define SYN\_USB\_HOST\_STATE\_ATTACHED 

```C++
#define SYN_USB_HOST_STATE_ATTACHED `1U`
```



Device physically present 


        

<hr>



### define SYN\_USB\_HOST\_STATE\_DISCONNECTED 

```C++
#define SYN_USB_HOST_STATE_DISCONNECTED `0U`
```



USB Host States No device attached 


        

<hr>



### define SYN\_USB\_HOST\_STATE\_ENUMERATING 

```C++
#define SYN_USB_HOST_STATE_ENUMERATING `3U`
```



Reading descriptors & assigning address 


        

<hr>



### define SYN\_USB\_HOST\_STATE\_ERROR 

```C++
#define SYN_USB_HOST_STATE_ERROR `5U`
```



Enumeration or transfer error 


        

<hr>



### define SYN\_USB\_HOST\_STATE\_READY 

```C++
#define SYN_USB_HOST_STATE_READY `4U`
```



Device enumerated and class active 


        

<hr>



### define SYN\_USB\_HOST\_STATE\_RESET 

```C++
#define SYN_USB_HOST_STATE_RESET `2U`
```



Bus reset in progress 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_host.h`

