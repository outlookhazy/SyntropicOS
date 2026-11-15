

# Struct SYN\_USB\_HostDevInfo



[**ClassList**](annotated.md) **>** [**SYN\_USB\_HostDevInfo**](structSYN__USB__HostDevInfo.md)



[More...](#detailed-description)

* `#include <syn_usb_host.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**dev\_addr**](#variable-dev_addr)  <br> |
|  uint8\_t | [**dev\_class**](#variable-dev_class)  <br> |
|  uint8\_t | [**dev\_protocol**](#variable-dev_protocol)  <br> |
|  uint8\_t | [**dev\_subclass**](#variable-dev_subclass)  <br> |
|  uint8\_t | [**max\_pkt\_ep0**](#variable-max_pkt_ep0)  <br> |
|  uint8\_t | [**num\_configs**](#variable-num_configs)  <br> |
|  uint16\_t | [**pid**](#variable-pid)  <br> |
|  uint16\_t | [**vid**](#variable-vid)  <br> |












































## Detailed Description


Cached device information from enumeration 


    
## Public Attributes Documentation




### variable dev\_addr 

```C++
uint8_t SYN_USB_HostDevInfo::dev_addr;
```



Assigned USB bus address 


        

<hr>



### variable dev\_class 

```C++
uint8_t SYN_USB_HostDevInfo::dev_class;
```



Device class code 


        

<hr>



### variable dev\_protocol 

```C++
uint8_t SYN_USB_HostDevInfo::dev_protocol;
```



Device protocol code 


        

<hr>



### variable dev\_subclass 

```C++
uint8_t SYN_USB_HostDevInfo::dev_subclass;
```



Device subclass code 


        

<hr>



### variable max\_pkt\_ep0 

```C++
uint8_t SYN_USB_HostDevInfo::max_pkt_ep0;
```



EP0 maximum packet size 


        

<hr>



### variable num\_configs 

```C++
uint8_t SYN_USB_HostDevInfo::num_configs;
```



Number of configurations 


        

<hr>



### variable pid 

```C++
uint16_t SYN_USB_HostDevInfo::pid;
```



Product ID 


        

<hr>



### variable vid 

```C++
uint16_t SYN_USB_HostDevInfo::vid;
```



Vendor ID 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_host.h`

