

# File syn\_usb\_host.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_host.c**](syn__usb__host_8c.md)

[Go to the source code of this file](syn__usb__host_8c_source.md)

_Zero-Heap USB 2.0 Host Core Protocol Engine Implementation._ [More...](#detailed-description)

* `#include "syntropic/drivers/syn_usb_host.h"`
* `#include "port/syn_port_usb.h"`
* `#include "port/syn_port_usb_host.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_USB\_HostDevInfo**](structSYN__USB__HostDevInfo.md) \* | [**syn\_usb\_host\_get\_dev\_info**](#function-syn_usb_host_get_dev_info) (const [**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Get cached device information from last enumeration._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_init**](#function-syn_usb_host_init) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Initialize USB Host Core._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_host\_is\_ready**](#function-syn_usb_host_is_ready) (const [**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Check if USB Host has a device enumerated and ready._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_process**](#function-syn_usb_host_process) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Process USB Host state machine (call each scheduler tick)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_host\_register\_class**](#function-syn_usb_host_register_class) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host, const [**SYN\_USB\_HostClassDriver**](structSYN__USB__HostClassDriver.md) \* cls) <br>_Register a host class driver for interface matching._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**build\_setup**](#function-build_setup) ([**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t bmRequestType, uint8\_t bRequest, uint16\_t wValue, uint16\_t wIndex, uint16\_t wLength) <br>_Build a standard USB control Setup Packet._  |
|  uint16\_t | [**calc\_iface\_span**](#function-calc_iface_span) (const uint8\_t \* buf, uint16\_t total, uint16\_t iface\_start) <br>_Compute byte span of an interface descriptor and its subordinate descriptors._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**enum\_poll\_xfer**](#function-enum_poll_xfer) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Poll an in-flight control pipe transfer during enumeration._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**enumerate\_step**](#function-enumerate_step) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Drive the enumeration sub-state machine. Non-blocking. Each call performs at most one HAL operation._  |
|  void | [**host\_process\_ready**](#function-host_process_ready) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Handle READY state detach detection and matched class ticking._  |
|  void | [**match\_iface\_drivers**](#function-match_iface_drivers) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host, const uint8\_t \* iface\_desc, uint16\_t iface\_len) <br>_Match a single interface descriptor slice against registered host class drivers._  |
|  void | [**probe\_class\_drivers**](#function-probe_class_drivers) ([**SYN\_USB\_Host**](structSYN__USB__Host.md) \* host) <br>_Walk configuration descriptor and probe registered class drivers against each interface descriptor's class/subclass/protocol triple._  |


























## Detailed Description


Tick-driven state machine handling device attach detection, bus reset, standard USB enumeration (Device/Config descriptor reads, SET\_ADDRESS, SET\_CONFIGURATION), and host class driver interface matching. 


    
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
## Public Static Functions Documentation




### function build\_setup 

_Build a standard USB control Setup Packet._ 
```C++
static void build_setup (
    SYN_USB_SetupPacket * pkt,
    uint8_t bmRequestType,
    uint8_t bRequest,
    uint16_t wValue,
    uint16_t wIndex,
    uint16_t wLength
) 
```





**Parameters:**


* `pkt` Target Setup Packet pointer. 
* `bmRequestType` Request characteristics bitfield. 
* `bRequest` Specific request code. 
* `wValue` Word-sized request parameter. 
* `wIndex` Word-sized index or offset. 
* `wLength` Transfer byte length. 




        

<hr>



### function calc\_iface\_span 

_Compute byte span of an interface descriptor and its subordinate descriptors._ 
```C++
static uint16_t calc_iface_span (
    const uint8_t * buf,
    uint16_t total,
    uint16_t iface_start
) 
```



For CDC Communication (`0x02`) interfaces, also includes an immediately following subordinate CDC Data (`0x0A`) interface and its endpoint descriptors.




**Parameters:**


* `buf` Configuration descriptor buffer. 
* `total` Total valid bytes in `buf`. 
* `iface_start` Offset of the interface descriptor in `buf`. 



**Returns:**

Total byte length of interface + subordinate descriptors. 





        

<hr>



### function enum\_poll\_xfer 

_Poll an in-flight control pipe transfer during enumeration._ 
```C++
static SYN_Status enum_poll_xfer (
    SYN_USB_Host * host
) 
```





**Parameters:**


* `host` Pointer to USB Host instance. 



**Returns:**

SYN\_OK if no transfer pending or transfer succeeded, SYN\_BUSY if in-flight, SYN\_ERROR on transfer failure. 





        

<hr>



### function enumerate\_step 

_Drive the enumeration sub-state machine. Non-blocking. Each call performs at most one HAL operation._ 
```C++
static SYN_Status enumerate_step (
    SYN_USB_Host * host
) 
```





**Parameters:**


* `host` Pointer to USB Host instance. 



**Returns:**

SYN\_OK when enumeration complete, SYN\_BUSY while in progress, SYN\_ERROR on failure. 





        

<hr>



### function host\_process\_ready 

_Handle READY state detach detection and matched class ticking._ 
```C++
static void host_process_ready (
    SYN_USB_Host * host
) 
```





**Parameters:**


* `host` Pointer to USB Host instance. 




        

<hr>



### function match\_iface\_drivers 

_Match a single interface descriptor slice against registered host class drivers._ 
```C++
static void match_iface_drivers (
    SYN_USB_Host * host,
    const uint8_t * iface_desc,
    uint16_t iface_len
) 
```





**Parameters:**


* `host` Pointer to USB Host instance. 
* `iface_desc` Pointer to interface descriptor slice. 
* `iface_len` Byte length of interface + subordinate descriptors. 




        

<hr>



### function probe\_class\_drivers 

_Walk configuration descriptor and probe registered class drivers against each interface descriptor's class/subclass/protocol triple._ 
```C++
static void probe_class_drivers (
    SYN_USB_Host * host
) 
```





**Parameters:**


* `host` Pointer to USB Host instance. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_host.c`

