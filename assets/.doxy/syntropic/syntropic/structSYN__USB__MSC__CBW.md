

# Struct SYN\_USB\_MSC\_CBW



[**ClassList**](annotated.md) **>** [**SYN\_USB\_MSC\_CBW**](structSYN__USB__MSC__CBW.md)



[More...](#detailed-description)

* `#include <syn_usb_msc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**CBWCB**](#variable-cbwcb)  <br> |
|  uint8\_t | [**bCBWCBLength**](#variable-bcbwcblength)  <br> |
|  uint8\_t | [**bCBWLUN**](#variable-bcbwlun)  <br> |
|  uint8\_t | [**bmCBWFlags**](#variable-bmcbwflags)  <br> |
|  uint32\_t | [**dCBWDataTransferLength**](#variable-dcbwdatatransferlength)  <br> |
|  uint32\_t | [**dCBWSignature**](#variable-dcbwsignature)  <br> |
|  uint32\_t | [**dCBWTag**](#variable-dcbwtag)  <br> |












































## Detailed Description


Command Block Wrapper (CBW) per USB BOT Spec §5.1 


    
## Public Attributes Documentation




### variable CBWCB 

```C++
uint8_t SYN_USB_MSC_CBW::CBWCB[16];
```



Raw SCSI Command block 


        

<hr>



### variable bCBWCBLength 

```C++
uint8_t SYN_USB_MSC_CBW::bCBWCBLength;
```



SCSI Command length 


        

<hr>



### variable bCBWLUN 

```C++
uint8_t SYN_USB_MSC_CBW::bCBWLUN;
```



Logical Unit Number 


        

<hr>



### variable bmCBWFlags 

```C++
uint8_t SYN_USB_MSC_CBW::bmCBWFlags;
```



Direction flag (0x80 IN, 0x00 OUT) 


        

<hr>



### variable dCBWDataTransferLength 

```C++
uint32_t SYN_USB_MSC_CBW::dCBWDataTransferLength;
```



Payload transfer length 


        

<hr>



### variable dCBWSignature 

```C++
uint32_t SYN_USB_MSC_CBW::dCBWSignature;
```



0x43425355 ("USBC") 


        

<hr>



### variable dCBWTag 

```C++
uint32_t SYN_USB_MSC_CBW::dCBWTag;
```



Command Block Tag 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_msc.h`

