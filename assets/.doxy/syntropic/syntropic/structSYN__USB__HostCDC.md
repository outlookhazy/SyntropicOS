

# Struct SYN\_USB\_HostCDC



[**ClassList**](annotated.md) **>** [**SYN\_USB\_HostCDC**](structSYN__USB__HostCDC.md)



[More...](#detailed-description)

* `#include <syn_usb_host_cdc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**connected**](#variable-connected)  <br> |
|  uint8\_t | [**dev\_addr**](#variable-dev_addr)  <br> |
|  uint8\_t | [**ep\_bulk\_in**](#variable-ep_bulk_in)  <br> |
|  uint8\_t | [**ep\_bulk\_out**](#variable-ep_bulk_out)  <br> |
|  [**SYN\_USB\_CDC\_LineCoding**](structSYN__USB__CDC__LineCoding.md) | [**line\_coding**](#variable-line_coding)  <br> |
|  uint8\_t | [**pipe\_bulk\_in**](#variable-pipe_bulk_in)  <br> |
|  uint8\_t | [**pipe\_bulk\_out**](#variable-pipe_bulk_out)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  uint16\_t | [**rx\_len**](#variable-rx_len)  <br> |
|  uint8\_t | [**tx\_buf**](#variable-tx_buf)  <br> |
|  uint16\_t | [**tx\_len**](#variable-tx_len)  <br> |












































## Detailed Description


USB Host CDC Instance Context 


    
## Public Attributes Documentation




### variable connected 

```C++
bool SYN_USB_HostCDC::connected;
```



True if probe succeeded and device active 


        

<hr>



### variable dev\_addr 

```C++
uint8_t SYN_USB_HostCDC::dev_addr;
```



Connected device bus address 


        

<hr>



### variable ep\_bulk\_in 

```C++
uint8_t SYN_USB_HostCDC::ep_bulk_in;
```



Endpoint address for Bulk IN 


        

<hr>



### variable ep\_bulk\_out 

```C++
uint8_t SYN_USB_HostCDC::ep_bulk_out;
```



Endpoint address for Bulk OUT 


        

<hr>



### variable line\_coding 

```C++
SYN_USB_CDC_LineCoding SYN_USB_HostCDC::line_coding;
```



Active line coding config 


        

<hr>



### variable pipe\_bulk\_in 

```C++
uint8_t SYN_USB_HostCDC::pipe_bulk_in;
```



Pipe index for Bulk IN 


        

<hr>



### variable pipe\_bulk\_out 

```C++
uint8_t SYN_USB_HostCDC::pipe_bulk_out;
```



Pipe index for Bulk OUT 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_USB_HostCDC::rx_buf[SYN_USB_HOST_CDC_BUF_SIZE];
```



RX data buffer 


        

<hr>



### variable rx\_len 

```C++
uint16_t SYN_USB_HostCDC::rx_len;
```



Unread bytes in rx\_buf 


        

<hr>



### variable tx\_buf 

```C++
uint8_t SYN_USB_HostCDC::tx_buf[SYN_USB_HOST_CDC_BUF_SIZE];
```



TX data buffer 


        

<hr>



### variable tx\_len 

```C++
uint16_t SYN_USB_HostCDC::tx_len;
```



Pending bytes in tx\_buf 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_host_cdc.h`

