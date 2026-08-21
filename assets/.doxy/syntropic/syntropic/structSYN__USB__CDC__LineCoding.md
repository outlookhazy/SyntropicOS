

# Struct SYN\_USB\_CDC\_LineCoding



[**ClassList**](annotated.md) **>** [**SYN\_USB\_CDC\_LineCoding**](structSYN__USB__CDC__LineCoding.md)



[More...](#detailed-description)

* `#include <syn_usb_cdc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**baudrate**](#variable-baudrate)  <br> |
|  uint8\_t | [**data\_bits**](#variable-data_bits)  <br> |
|  uint8\_t | [**parity**](#variable-parity)  <br> |
|  uint8\_t | [**stop\_bits**](#variable-stop_bits)  <br> |












































## Detailed Description


CDC Line Coding Config (Baud rate, Stop bits, Parity, Data bits). 


    
## Public Attributes Documentation




### variable baudrate 

```C++
uint32_t SYN_USB_CDC_LineCoding::baudrate;
```



Transmission baud rate in bits per second 


        

<hr>



### variable data\_bits 

```C++
uint8_t SYN_USB_CDC_LineCoding::data_bits;
```



Data bits count (5, 6, 7, 8, 16) 


        

<hr>



### variable parity 

```C++
uint8_t SYN_USB_CDC_LineCoding::parity;
```



Parity setting (0=None, 1=Odd, 2=Even) 


        

<hr>



### variable stop\_bits 

```C++
uint8_t SYN_USB_CDC_LineCoding::stop_bits;
```



Stop bits setting (0=1, 1=1.5, 2=2) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_cdc.h`

