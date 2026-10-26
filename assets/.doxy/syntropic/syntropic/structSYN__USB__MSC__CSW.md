

# Struct SYN\_USB\_MSC\_CSW



[**ClassList**](annotated.md) **>** [**SYN\_USB\_MSC\_CSW**](structSYN__USB__MSC__CSW.md)



[More...](#detailed-description)

* `#include <syn_usb_msc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**bCSWStatus**](#variable-bcswstatus)  <br> |
|  uint32\_t | [**dCSWDataResidue**](#variable-dcswdataresidue)  <br> |
|  uint32\_t | [**dCSWSignature**](#variable-dcswsignature)  <br> |
|  uint32\_t | [**dCSWTag**](#variable-dcswtag)  <br> |












































## Detailed Description


Command Status Wrapper (CSW) per USB BOT Spec §5.2 


    
## Public Attributes Documentation




### variable bCSWStatus 

```C++
uint8_t SYN_USB_MSC_CSW::bCSWStatus;
```



0=Success, 1=Failed, 2=Phase Error 


        

<hr>



### variable dCSWDataResidue 

```C++
uint32_t SYN_USB_MSC_CSW::dCSWDataResidue;
```



Difference in expected/actual bytes 


        

<hr>



### variable dCSWSignature 

```C++
uint32_t SYN_USB_MSC_CSW::dCSWSignature;
```



0x53425355 ("USBS") 


        

<hr>



### variable dCSWTag 

```C++
uint32_t SYN_USB_MSC_CSW::dCSWTag;
```



Matches dCBWTag 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_msc.h`

