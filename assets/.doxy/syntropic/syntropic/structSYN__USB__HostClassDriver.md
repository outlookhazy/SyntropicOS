

# Struct SYN\_USB\_HostClassDriver



[**ClassList**](annotated.md) **>** [**SYN\_USB\_HostClassDriver**](structSYN__USB__HostClassDriver.md)



[More...](#detailed-description)

* `#include <syn_usb_host.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**class\_code**](#variable-class_code)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  void(\* | [**disconnected**](#variable-disconnected)  <br>_Called when the device is detached._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**matched**](#variable-matched)  <br> |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**probe**](#variable-probe)  <br>_Called when a matching interface is found during enumeration._  |
|  void(\* | [**process**](#variable-process)  <br>_Called each tick while in READY state for class-specific polling._  |
|  uint8\_t | [**protocol\_code**](#variable-protocol_code)  <br> |
|  uint8\_t | [**subclass\_code**](#variable-subclass_code)  <br> |












































## Detailed Description


Host class driver registration entry 


    
## Public Attributes Documentation




### variable class\_code 

```C++
uint8_t SYN_USB_HostClassDriver::class_code;
```



bInterfaceClass to match (0xFF = any) 


        

<hr>



### variable ctx 

```C++
void* SYN_USB_HostClassDriver::ctx;
```



Driver context pointer 


        

<hr>



### variable disconnected 

_Called when the device is detached._ 
```C++
void(* SYN_USB_HostClassDriver::disconnected) (void *ctx);
```





**Parameters:**


* `ctx` Driver context. 




        

<hr>



### variable matched 

```C++
bool SYN_USB_HostClassDriver::matched;
```



True if this driver matched during probe 


        

<hr>



### variable probe 

_Called when a matching interface is found during enumeration._ 
```C++
SYN_Status(* SYN_USB_HostClassDriver::probe) (void *ctx, uint8_t dev_addr, const uint8_t *iface_desc, uint16_t len);
```





**Parameters:**


* `ctx` Driver context. 
* `dev_addr` Device bus address. 
* `iface_desc` Pointer to interface descriptor bytes. 
* `len` Length of interface descriptor + subordinate descriptors. 



**Returns:**

SYN\_OK to claim the interface. 





        

<hr>



### variable process 

_Called each tick while in READY state for class-specific polling._ 
```C++
void(* SYN_USB_HostClassDriver::process) (void *ctx);
```





**Parameters:**


* `ctx` Driver context. 




        

<hr>



### variable protocol\_code 

```C++
uint8_t SYN_USB_HostClassDriver::protocol_code;
```



bInterfaceProtocol to match (0xFF = any) 


        

<hr>



### variable subclass\_code 

```C++
uint8_t SYN_USB_HostClassDriver::subclass_code;
```



bInterfaceSubClass to match (0xFF = any) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_host.h`

