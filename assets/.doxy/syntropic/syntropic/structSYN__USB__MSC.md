

# Struct SYN\_USB\_MSC



[**ClassList**](annotated.md) **>** [**SYN\_USB\_MSC**](structSYN__USB__MSC.md)



[More...](#detailed-description)

* `#include <syn_usb_msc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_USB\_MSC\_CBW**](structSYN__USB__MSC__CBW.md) | [**cbw**](#variable-cbw)  <br> |
|  [**SYN\_USB\_MSC\_CSW**](structSYN__USB__MSC__CSW.md) | [**csw**](#variable-csw)  <br> |
|  uint8\_t | [**ep\_in**](#variable-ep_in)  <br> |
|  uint8\_t | [**ep\_out**](#variable-ep_out)  <br> |
|  uint8\_t | [**iface\_num**](#variable-iface_num)  <br> |
|  [**SYN\_USB\_MSC\_Media**](structSYN__USB__MSC__Media.md) | [**media**](#variable-media)  <br> |
|  uint8\_t | [**tx\_buf**](#variable-tx_buf)  <br> |
|  uint16\_t | [**tx\_len**](#variable-tx_len)  <br> |












































## Detailed Description


USB MSC Instance Context 


    
## Public Attributes Documentation




### variable cbw 

```C++
SYN_USB_MSC_CBW SYN_USB_MSC::cbw;
```



Active SCSI CBW command 


        

<hr>



### variable csw 

```C++
SYN_USB_MSC_CSW SYN_USB_MSC::csw;
```



Pending SCSI CSW status 


        

<hr>



### variable ep\_in 

```C++
uint8_t SYN_USB_MSC::ep_in;
```



Bulk IN Endpoint address 


        

<hr>



### variable ep\_out 

```C++
uint8_t SYN_USB_MSC::ep_out;
```



Bulk OUT Endpoint address 


        

<hr>



### variable iface\_num 

```C++
uint8_t SYN_USB_MSC::iface_num;
```



Assigned interface index 


        

<hr>



### variable media 

```C++
SYN_USB_MSC_Media SYN_USB_MSC::media;
```



Bound storage media device 


        

<hr>



### variable tx\_buf 

```C++
uint8_t SYN_USB_MSC::tx_buf[SYN_USB_MSC_MAX_PACKET_SIZE];
```



IN buffer 


        

<hr>



### variable tx\_len 

```C++
uint16_t SYN_USB_MSC::tx_len;
```



Pending IN length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_msc.h`

