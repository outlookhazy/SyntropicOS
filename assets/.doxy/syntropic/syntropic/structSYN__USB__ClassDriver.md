

# Struct SYN\_USB\_ClassDriver



[**ClassList**](annotated.md) **>** [**SYN\_USB\_ClassDriver**](structSYN__USB__ClassDriver.md)



[More...](#detailed-description)

* `#include <syn_usb.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**configured**](#variable-configured)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  void(\* | [**data\_in**](#variable-data_in)  <br> |
|  void(\* | [**data\_out**](#variable-data_out)  <br> |
|  uint8\_t | [**iface\_count**](#variable-iface_count)  <br> |
|  uint8\_t | [**iface\_start**](#variable-iface_start)  <br> |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**setup**](#variable-setup)  <br> |












































## Detailed Description


USB Class Driver Registration Entry 


    
## Public Attributes Documentation




### variable configured 

```C++
SYN_Status(* SYN_USB_ClassDriver::configured) (void *ctx, uint8_t config);
```



Configured state callback 


        

<hr>



### variable ctx 

```C++
void* SYN_USB_ClassDriver::ctx;
```



Driver context 


        

<hr>



### variable data\_in 

```C++
void(* SYN_USB_ClassDriver::data_in) (void *ctx, uint8_t ep);
```



Data IN handler 


        

<hr>



### variable data\_out 

```C++
void(* SYN_USB_ClassDriver::data_out) (void *ctx, uint8_t ep, const uint8_t *data, uint16_t len);
```



Data OUT handler 


        

<hr>



### variable iface\_count 

```C++
uint8_t SYN_USB_ClassDriver::iface_count;
```



Interface count 


        

<hr>



### variable iface\_start 

```C++
uint8_t SYN_USB_ClassDriver::iface_start;
```



First interface index 


        

<hr>



### variable setup 

```C++
SYN_Status(* SYN_USB_ClassDriver::setup) (void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp, uint16_t *rlen);
```



Class setup handler 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb.h`

