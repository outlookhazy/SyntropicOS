

# File syn\_usb\_msc.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_msc.h**](syn__usb__msc_8h.md)

[Go to the source code of this file](syn__usb__msc_8h_source.md)

_Zero-Heap USB 2.0 Mass Storage Class (MSC) Device Driver._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/drivers/syn_usb.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_USB\_MSC**](structSYN__USB__MSC.md) <br> |
| struct | [**SYN\_USB\_MSC\_CBW**](structSYN__USB__MSC__CBW.md) <br> |
| struct | [**SYN\_USB\_MSC\_CSW**](structSYN__USB__MSC__CSW.md) <br> |
| struct | [**SYN\_USB\_MSC\_Media**](structSYN__USB__MSC__Media.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_get\_config\_desc**](#function-syn_usb_msc_get_config_desc) (const [**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, uint8\_t desc\_out) <br>_Build the 23-byte MSC Interface + Bulk IN + Bulk OUT descriptor block._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_init**](#function-syn_usb_msc_init) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Initialize USB MSC Instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_process\_cbw**](#function-syn_usb_msc_process_cbw) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, const uint8\_t cbw\_raw) <br>_Process incoming BOT Command Block Wrapper (CBW)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_register**](#function-syn_usb_msc_register) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, [**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Register USB MSC class driver with USB core._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_set\_media**](#function-syn_usb_msc_set_media) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, const [**SYN\_USB\_MSC\_Media**](structSYN__USB__MSC__Media.md) \* media) <br>_Bind block storage media callbacks._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SCSI\_INQUIRY**](syn__usb__msc_8h.md#define-syn_scsi_inquiry)  `0x12U`<br> |
| define  | [**SYN\_SCSI\_MODE\_SENSE\_6**](syn__usb__msc_8h.md#define-syn_scsi_mode_sense_6)  `0x1AU`<br> |
| define  | [**SYN\_SCSI\_READ\_10**](syn__usb__msc_8h.md#define-syn_scsi_read_10)  `0x28U`<br> |
| define  | [**SYN\_SCSI\_READ\_CAPACITY\_10**](syn__usb__msc_8h.md#define-syn_scsi_read_capacity_10)  `0x25U`<br> |
| define  | [**SYN\_SCSI\_REQUEST\_SENSE**](syn__usb__msc_8h.md#define-syn_scsi_request_sense)  `0x03U`<br> |
| define  | [**SYN\_SCSI\_SENSE\_ILLEGAL\_REQUEST**](syn__usb__msc_8h.md#define-syn_scsi_sense_illegal_request)  `0x05U`<br> |
| define  | [**SYN\_SCSI\_SENSE\_MEDIUM\_ERROR**](syn__usb__msc_8h.md#define-syn_scsi_sense_medium_error)  `0x03U`<br> |
| define  | [**SYN\_SCSI\_SENSE\_NONE**](syn__usb__msc_8h.md#define-syn_scsi_sense_none)  `0x00U`<br> |
| define  | [**SYN\_SCSI\_TEST\_UNIT\_READY**](syn__usb__msc_8h.md#define-syn_scsi_test_unit_ready)  `0x00U`<br> |
| define  | [**SYN\_SCSI\_WRITE\_10**](syn__usb__msc_8h.md#define-syn_scsi_write_10)  `0x2AU`<br> |
| define  | [**SYN\_USB\_MSC\_BLOCK\_SIZE**](syn__usb__msc_8h.md#define-syn_usb_msc_block_size)  `512U`<br> |
| define  | [**SYN\_USB\_MSC\_BOT\_CSW**](syn__usb__msc_8h.md#define-syn_usb_msc_bot_csw)  `3U`<br> |
| define  | [**SYN\_USB\_MSC\_BOT\_DATA\_IN**](syn__usb__msc_8h.md#define-syn_usb_msc_bot_data_in)  `1U`<br> |
| define  | [**SYN\_USB\_MSC\_BOT\_DATA\_OUT**](syn__usb__msc_8h.md#define-syn_usb_msc_bot_data_out)  `2U`<br> |
| define  | [**SYN\_USB\_MSC\_BOT\_IDLE**](syn__usb__msc_8h.md#define-syn_usb_msc_bot_idle)  `0U`<br> |
| define  | [**SYN\_USB\_MSC\_DESC\_SIZE**](syn__usb__msc_8h.md#define-syn_usb_msc_desc_size)  `23U`<br> |
| define  | [**SYN\_USB\_MSC\_MAX\_PACKET\_SIZE**](syn__usb__msc_8h.md#define-syn_usb_msc_max_packet_size)  `64U`<br> |

## Detailed Description


Implements USB Mass Storage Class Bulk-Only Transport (BOT) v1.0 with SCSI Transparent Command Set to expose SD Card or Flash storage as a USB drive. 


    
## Public Functions Documentation




### function syn\_usb\_msc\_get\_config\_desc 

_Build the 23-byte MSC Interface + Bulk IN + Bulk OUT descriptor block._ 
```C++
SYN_Status syn_usb_msc_get_config_desc (
    const SYN_USB_MSC * msc,
    uint8_t desc_out
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 
* `desc_out` Output 23-byte buffer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_usb\_msc\_init 

_Initialize USB MSC Instance._ 
```C++
SYN_Status syn_usb_msc_init (
    SYN_USB_MSC * msc
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_usb\_msc\_process\_cbw 

_Process incoming BOT Command Block Wrapper (CBW)._ 
```C++
SYN_Status syn_usb_msc_process_cbw (
    SYN_USB_MSC * msc,
    const uint8_t cbw_raw
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 
* `cbw_raw` Pointer to 31-byte raw CBW buffer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL, SYN\_ERROR if signature invalid. 





        

<hr>



### function syn\_usb\_msc\_register 

_Register USB MSC class driver with USB core._ 
```C++
SYN_Status syn_usb_msc_register (
    SYN_USB_Device * dev,
    SYN_USB_MSC * msc
) 
```





**Parameters:**


* `dev` Pointer to USB device context. 
* `msc` Pointer to USB MSC instance. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_usb\_msc\_set\_media 

_Bind block storage media callbacks._ 
```C++
SYN_Status syn_usb_msc_set_media (
    SYN_USB_MSC * msc,
    const SYN_USB_MSC_Media * media
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 
* `media` Pointer to storage media interface. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>
## Macro Definition Documentation





### define SYN\_SCSI\_INQUIRY 

```C++
#define SYN_SCSI_INQUIRY `0x12U`
```



SCSI Inquiry opcode (0x12) 


        

<hr>



### define SYN\_SCSI\_MODE\_SENSE\_6 

```C++
#define SYN_SCSI_MODE_SENSE_6 `0x1AU`
```



SCSI Mode Sense (6) opcode (0x1A) 


        

<hr>



### define SYN\_SCSI\_READ\_10 

```C++
#define SYN_SCSI_READ_10 `0x28U`
```



SCSI Read (10) opcode (0x28) 


        

<hr>



### define SYN\_SCSI\_READ\_CAPACITY\_10 

```C++
#define SYN_SCSI_READ_CAPACITY_10 `0x25U`
```



SCSI Read Capacity (10) opcode (0x25) 


        

<hr>



### define SYN\_SCSI\_REQUEST\_SENSE 

```C++
#define SYN_SCSI_REQUEST_SENSE `0x03U`
```



SCSI Request Sense opcode (0x03) 


        

<hr>



### define SYN\_SCSI\_SENSE\_ILLEGAL\_REQUEST 

```C++
#define SYN_SCSI_SENSE_ILLEGAL_REQUEST `0x05U`
```



Illegal Request (0x05) 


        

<hr>



### define SYN\_SCSI\_SENSE\_MEDIUM\_ERROR 

```C++
#define SYN_SCSI_SENSE_MEDIUM_ERROR `0x03U`
```



Medium Error (0x03) 


        

<hr>



### define SYN\_SCSI\_SENSE\_NONE 

```C++
#define SYN_SCSI_SENSE_NONE `0x00U`
```



SCSI Sense Key Codes No Sense (0x00) 


        

<hr>



### define SYN\_SCSI\_TEST\_UNIT\_READY 

```C++
#define SYN_SCSI_TEST_UNIT_READY `0x00U`
```



SCSI Command Opcodes SCSI Test Unit Ready opcode (0x00) 


        

<hr>



### define SYN\_SCSI\_WRITE\_10 

```C++
#define SYN_SCSI_WRITE_10 `0x2AU`
```



SCSI Write (10) opcode (0x2A) 


        

<hr>



### define SYN\_USB\_MSC\_BLOCK\_SIZE 

```C++
#define SYN_USB_MSC_BLOCK_SIZE `512U`
```



Standard SCSI block sector size in bytes 


        

<hr>



### define SYN\_USB\_MSC\_BOT\_CSW 

```C++
#define SYN_USB_MSC_BOT_CSW `3U`
```



13-byte CSW staged on Bulk IN 


        

<hr>



### define SYN\_USB\_MSC\_BOT\_DATA\_IN 

```C++
#define SYN_USB_MSC_BOT_DATA_IN `1U`
```



Streaming IN payload to host 


        

<hr>



### define SYN\_USB\_MSC\_BOT\_DATA\_OUT 

```C++
#define SYN_USB_MSC_BOT_DATA_OUT `2U`
```



Receiving OUT payload from host 


        

<hr>



### define SYN\_USB\_MSC\_BOT\_IDLE 

```C++
#define SYN_USB_MSC_BOT_IDLE `0U`
```



Bulk-Only Transport (BOT) Protocol States Waiting for 31-byte CBW on Bulk OUT 


        

<hr>



### define SYN\_USB\_MSC\_DESC\_SIZE 

```C++
#define SYN_USB_MSC_DESC_SIZE `23U`
```



Interface (9B) + Bulk IN (7B) + Bulk OUT (7B) 


        

<hr>



### define SYN\_USB\_MSC\_MAX\_PACKET\_SIZE 

```C++
#define SYN_USB_MSC_MAX_PACKET_SIZE `64U`
```



Maximum USB endpoint packet size 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_msc.h`

