

# Struct SYN\_USB\_CDC



[**ClassList**](annotated.md) **>** [**SYN\_USB\_CDC**](structSYN__USB__CDC.md)



[More...](#detailed-description)

* `#include <syn_usb_cdc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**configured**](#variable-configured)  <br> |
|  uint8\_t | [**dev\_address**](#variable-dev_address)  <br> |
|  uint8\_t | [**ep\_cmd**](#variable-ep_cmd)  <br> |
|  uint8\_t | [**ep\_in**](#variable-ep_in)  <br> |
|  uint8\_t | [**ep\_out**](#variable-ep_out)  <br> |
|  [**SYN\_USB\_CDC\_LineCoding**](structSYN__USB__CDC__LineCoding.md) | [**line\_coding**](#variable-line_coding)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  uint16\_t | [**rx\_len**](#variable-rx_len)  <br> |
|  uint8\_t | [**tx\_buf**](#variable-tx_buf)  <br> |
|  uint16\_t | [**tx\_len**](#variable-tx_len)  <br> |












































## Detailed Description


USB CDC Instance. 


    
## Public Attributes Documentation




### variable configured 

```C++
bool SYN_USB_CDC::configured;
```



True if USB device is in Configured state 


        

<hr>



### variable dev\_address 

```C++
uint8_t SYN_USB_CDC::dev_address;
```



Assigned USB device bus address 


        

<hr>



### variable ep\_cmd 

```C++
uint8_t SYN_USB_CDC::ep_cmd;
```



Interrupt Command Endpoint address 


        

<hr>



### variable ep\_in 

```C++
uint8_t SYN_USB_CDC::ep_in;
```



Bulk IN Endpoint address 


        

<hr>



### variable ep\_out 

```C++
uint8_t SYN_USB_CDC::ep_out;
```



Bulk OUT Endpoint address 


        

<hr>



### variable line\_coding 

```C++
SYN_USB_CDC_LineCoding SYN_USB_CDC::line_coding;
```



Active CDC line coding configuration 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_USB_CDC::rx_buf[128];
```



Internal RX ring/linear payload buffer 


        

<hr>



### variable rx\_len 

```C++
uint16_t SYN_USB_CDC::rx_len;
```



Length of unread data in rx\_buf 


        

<hr>



### variable tx\_buf 

```C++
uint8_t SYN_USB_CDC::tx_buf[128];
```



Internal TX ring/linear payload buffer 


        

<hr>



### variable tx\_len 

```C++
uint16_t SYN_USB_CDC::tx_len;
```



Length of pending data in tx\_buf 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_cdc.h`

