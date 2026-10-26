

# Struct SYN\_USB\_HID



[**ClassList**](annotated.md) **>** [**SYN\_USB\_HID**](structSYN__USB__HID.md)



[More...](#detailed-description)

* `#include <syn_usb_hid.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**active\_protocol**](#variable-active_protocol)  <br> |
|  uint8\_t | [**ep\_in**](#variable-ep_in)  <br> |
|  uint8\_t | [**ep\_out**](#variable-ep_out)  <br> |
|  uint8\_t | [**idle\_rate**](#variable-idle_rate)  <br> |
|  uint8\_t | [**iface\_num**](#variable-iface_num)  <br> |
|  uint8\_t | [**protocol**](#variable-protocol)  <br> |
|  const uint8\_t \* | [**report\_desc**](#variable-report_desc)  <br> |
|  uint16\_t | [**report\_desc\_len**](#variable-report_desc_len)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  uint16\_t | [**rx\_len**](#variable-rx_len)  <br> |
|  uint8\_t | [**tx\_buf**](#variable-tx_buf)  <br> |
|  uint16\_t | [**tx\_len**](#variable-tx_len)  <br> |












































## Detailed Description


USB HID Instance Context 


    
## Public Attributes Documentation




### variable active\_protocol 

```C++
uint8_t SYN_USB_HID::active_protocol;
```



Active HID protocol 


        

<hr>



### variable ep\_in 

```C++
uint8_t SYN_USB_HID::ep_in;
```



Interrupt IN Endpoint address 


        

<hr>



### variable ep\_out 

```C++
uint8_t SYN_USB_HID::ep_out;
```



Interrupt OUT Endpoint address 


        

<hr>



### variable idle\_rate 

```C++
uint8_t SYN_USB_HID::idle_rate;
```



Active HID idle rate 


        

<hr>



### variable iface\_num 

```C++
uint8_t SYN_USB_HID::iface_num;
```



Assigned interface index 


        

<hr>



### variable protocol 

```C++
uint8_t SYN_USB_HID::protocol;
```



Subclass protocol 


        

<hr>



### variable report\_desc 

```C++
const uint8_t* SYN_USB_HID::report_desc;
```



Pointer to HID report descriptor 


        

<hr>



### variable report\_desc\_len 

```C++
uint16_t SYN_USB_HID::report_desc_len;
```



Length of HID report descriptor 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_USB_HID::rx_buf[SYN_USB_HID_MAX_REPORT_SIZE];
```



OUT report payload buffer 


        

<hr>



### variable rx\_len 

```C++
uint16_t SYN_USB_HID::rx_len;
```



Unread OUT report byte length 


        

<hr>



### variable tx\_buf 

```C++
uint8_t SYN_USB_HID::tx_buf[SYN_USB_HID_MAX_REPORT_SIZE];
```



IN report payload buffer 


        

<hr>



### variable tx\_len 

```C++
uint16_t SYN_USB_HID::tx_len;
```



Pending IN report byte length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_hid.h`

