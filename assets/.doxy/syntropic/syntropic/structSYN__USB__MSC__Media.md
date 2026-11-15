

# Struct SYN\_USB\_MSC\_Media



[**ClassList**](annotated.md) **>** [**SYN\_USB\_MSC\_Media**](structSYN__USB__MSC__Media.md)



[More...](#detailed-description)

* `#include <syn_usb_msc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**block\_count**](#variable-block_count)  <br> |
|  uint16\_t | [**block\_size**](#variable-block_size)  <br> |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**read\_blocks**](#variable-read_blocks)  <br> |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status)(\* | [**write\_blocks**](#variable-write_blocks)  <br> |












































## Detailed Description


USB MSC Block Storage Callbacks 


    
## Public Attributes Documentation




### variable block\_count 

```C++
uint32_t SYN_USB_MSC_Media::block_count;
```



Total sectors 


        

<hr>



### variable block\_size 

```C++
uint16_t SYN_USB_MSC_Media::block_size;
```



Sector size in bytes (512) 


        

<hr>



### variable read\_blocks 

```C++
SYN_Status(* SYN_USB_MSC_Media::read_blocks) (uint32_t lba, uint8_t *buf, uint16_t count);
```



Block read callback 


        

<hr>



### variable write\_blocks 

```C++
SYN_Status(* SYN_USB_MSC_Media::write_blocks) (uint32_t lba, const uint8_t *buf, uint16_t count);
```



Block write callback 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_msc.h`

