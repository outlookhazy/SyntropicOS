

# File syn\_usb\_msc.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_msc.c**](syn__usb__msc_8c.md)

[Go to the source code of this file](syn__usb__msc_8c_source.md)

_Zero-Heap USB 2.0 Mass Storage Class (MSC) Device Driver Implementation._ 

* `#include "syn_usb_msc.h"`
* `#include "port/syn_port_usb.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_get\_config\_desc**](#function-syn_usb_msc_get_config_desc) (const [**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, uint8\_t desc\_out) <br>_Build the 23-byte MSC Interface + Bulk IN + Bulk OUT descriptor block._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_init**](#function-syn_usb_msc_init) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Initialize USB MSC Instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_process\_cbw**](#function-syn_usb_msc_process_cbw) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, const uint8\_t cbw\_raw) <br>_Process incoming BOT Command Block Wrapper (CBW)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_register**](#function-syn_usb_msc_register) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, [**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Register USB MSC class driver with USB core._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_set\_media**](#function-syn_usb_msc_set_media) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, const [**SYN\_USB\_MSC\_Media**](structSYN__USB__MSC__Media.md) \* media) <br>_Bind block storage media callbacks._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**msc\_fail\_cbw**](#function-msc_fail_cbw) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, uint8\_t csw\_status, uint8\_t sense\_key, uint8\_t sense\_asc) <br>_Record SCSI sense codes, STALL pending data pipe if needed, and stage CSW._  |
|  uint32\_t | [**msc\_get\_be32**](#function-msc_get_be32) (const uint8\_t src) <br>_Read a 32-bit unsigned value in big-endian order._  |
|  void | [**msc\_handle\_data\_out\_stream**](#function-msc_handle_data_out_stream) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, const uint8\_t \* data, uint16\_t len) <br>_Stream Bulk OUT bytes into sector\_buf and commit 512B blocks via write\_blocks._  |
|  void | [**msc\_handle\_inquiry**](#function-msc_handle_inquiry) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Handle SCSI INQUIRY (0x12) command, rejecting EVPD/PageCode and clamping alloc\_len._  |
|  void | [**msc\_handle\_mode\_sense6**](#function-msc_handle_mode_sense6) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Handle SCSI MODE SENSE (6) (0x1A) command with CDB allocation length clamping._  |
|  void | [**msc\_handle\_read10**](#function-msc_handle_read10) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Handle SCSI READ (10) command._  |
|  void | [**msc\_handle\_read\_capacity10**](#function-msc_handle_read_capacity10) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Handle SCSI READ CAPACITY (10) command._  |
|  void | [**msc\_handle\_request\_sense**](#function-msc_handle_request_sense) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Handle SCSI REQUEST SENSE (0x03) command with CDB allocation length clamping._  |
|  void | [**msc\_handle\_write10**](#function-msc_handle_write10) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Handle SCSI WRITE (10) command._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**msc\_lba\_range\_valid**](#function-msc_lba_range_valid) (const [**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, uint32\_t lba, uint16\_t blocks) <br>_Validate LBA range for a SCSI READ\_10 or WRITE\_10 command._  |
|  void | [**msc\_put\_le32**](#function-msc_put_le32) (uint8\_t dst, uint32\_t val) <br>_Write a 32-bit unsigned value in little-endian order._  |
|  void | [**msc\_stage\_csw**](#function-msc_stage_csw) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, uint8\_t status, uint32\_t residue) <br>_Serialize and stage the 13-byte BOT Command Status Wrapper (CSW) on Bulk IN._  |
|  void | [**msc\_stage\_next\_in**](#function-msc_stage_next_in) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Stage the next Bulk IN packet (up to 64B) or transition to CSW when complete._  |
|  void | [**msc\_stall\_cbw\_pipe**](#function-msc_stall_cbw_pipe) (const [**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_STALL the active Bulk data endpoint when failing or short-ending a CBW with data phase._  |
|  void | [**msc\_start\_short\_in**](#function-msc_start_short_in) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, uint16\_t resp\_len) <br>_Begin a short non-block SCSI DATA\_IN response from msc-&gt;sector\_buf._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_msc\_configured\_handler**](#function-usb_msc_configured_handler) (void \* ctx, uint8\_t config) <br>_Configured callback for USB MSC class interface activation._  |
|  void | [**usb\_msc\_data\_in\_handler**](#function-usb_msc_data_in_handler) (void \* ctx, uint8\_t ep) <br>_Data IN handler callback for USB MSC IN endpoint completion._  |
|  void | [**usb\_msc\_data\_out\_handler**](#function-usb_msc_data_out_handler) (void \* ctx, uint8\_t ep, const uint8\_t \* data, uint16\_t len) <br>_Data OUT handler callback for USB MSC OUT endpoint data._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_msc\_setup\_handler**](#function-usb_msc_setup_handler) (void \* ctx, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t resp\_cap, uint16\_t \* rlen) <br>_Setup handler callback for USB MSC class setup requests._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_USB\_MSC\_CBW\_SIGNATURE**](syn__usb__msc_8c.md#define-syn_usb_msc_cbw_signature)  `0x43425355U`<br> |
| define  | [**SYN\_USB\_MSC\_CSW\_SIGNATURE**](syn__usb__msc_8c.md#define-syn_usb_msc_csw_signature)  `0x53425355U`<br> |
| define  | [**SYN\_USB\_MSC\_CSW\_SIZE**](syn__usb__msc_8c.md#define-syn_usb_msc_csw_size)  `13U`<br> |

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
## Public Static Functions Documentation




### function msc\_fail\_cbw 

_Record SCSI sense codes, STALL pending data pipe if needed, and stage CSW._ 
```C++
static void msc_fail_cbw (
    SYN_USB_MSC * msc,
    uint8_t csw_status,
    uint8_t sense_key,
    uint8_t sense_asc
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 
* `csw_status` CSW status byte (0=Passed, 1=Failed, 2=Phase Error). 
* `sense_key` SCSI sense key. 
* `sense_asc` SCSI additional sense code. 




        

<hr>



### function msc\_get\_be32 

_Read a 32-bit unsigned value in big-endian order._ 
```C++
static uint32_t msc_get_be32 (
    const uint8_t src
) 
```





**Parameters:**


* `src` 4-byte input pointer. 



**Returns:**

Decoded 32-bit integer. 





        

<hr>



### function msc\_handle\_data\_out\_stream 

_Stream Bulk OUT bytes into sector\_buf and commit 512B blocks via write\_blocks._ 
```C++
static void msc_handle_data_out_stream (
    SYN_USB_MSC * msc,
    const uint8_t * data,
    uint16_t len
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 
* `data` Received Bulk OUT payload. 
* `len` Byte length of received payload. 




        

<hr>



### function msc\_handle\_inquiry 

_Handle SCSI INQUIRY (0x12) command, rejecting EVPD/PageCode and clamping alloc\_len._ 
```C++
static void msc_handle_inquiry (
    SYN_USB_MSC * msc
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 




        

<hr>



### function msc\_handle\_mode\_sense6 

_Handle SCSI MODE SENSE (6) (0x1A) command with CDB allocation length clamping._ 
```C++
static void msc_handle_mode_sense6 (
    SYN_USB_MSC * msc
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 




        

<hr>



### function msc\_handle\_read10 

_Handle SCSI READ (10) command._ 
```C++
static void msc_handle_read10 (
    SYN_USB_MSC * msc
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 




        

<hr>



### function msc\_handle\_read\_capacity10 

_Handle SCSI READ CAPACITY (10) command._ 
```C++
static void msc_handle_read_capacity10 (
    SYN_USB_MSC * msc
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 




        

<hr>



### function msc\_handle\_request\_sense 

_Handle SCSI REQUEST SENSE (0x03) command with CDB allocation length clamping._ 
```C++
static void msc_handle_request_sense (
    SYN_USB_MSC * msc
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 




        

<hr>



### function msc\_handle\_write10 

_Handle SCSI WRITE (10) command._ 
```C++
static void msc_handle_write10 (
    SYN_USB_MSC * msc
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 




        

<hr>



### function msc\_lba\_range\_valid 

_Validate LBA range for a SCSI READ\_10 or WRITE\_10 command._ 
```C++
static bool msc_lba_range_valid (
    const SYN_USB_MSC * msc,
    uint32_t lba,
    uint16_t blocks
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 
* `lba` Starting logical block address. 
* `blocks` Number of blocks (&gt; 0). 



**Returns:**

true if range fits within msc-&gt;media.block\_count. 





        

<hr>



### function msc\_put\_le32 

_Write a 32-bit unsigned value in little-endian order._ 
```C++
static void msc_put_le32 (
    uint8_t dst,
    uint32_t val
) 
```





**Parameters:**


* `dst` 4-byte output pointer. 
* `val` Value to serialize. 




        

<hr>



### function msc\_stage\_csw 

_Serialize and stage the 13-byte BOT Command Status Wrapper (CSW) on Bulk IN._ 
```C++
static void msc_stage_csw (
    SYN_USB_MSC * msc,
    uint8_t status,
    uint32_t residue
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 
* `status` CSW status code (0 = Passed, 1 = Failed, 2 = Phase Error). 
* `residue` Unprocessed byte count from dCBWDataTransferLength. 




        

<hr>



### function msc\_stage\_next\_in 

_Stage the next Bulk IN packet (up to 64B) or transition to CSW when complete._ 
```C++
static void msc_stage_next_in (
    SYN_USB_MSC * msc
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 




        

<hr>



### function msc\_stall\_cbw\_pipe 

_STALL the active Bulk data endpoint when failing or short-ending a CBW with data phase._ 
```C++
static void msc_stall_cbw_pipe (
    const SYN_USB_MSC * msc
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 




        

<hr>



### function msc\_start\_short\_in 

_Begin a short non-block SCSI DATA\_IN response from msc-&gt;sector\_buf._ 
```C++
static void msc_start_short_in (
    SYN_USB_MSC * msc,
    uint16_t resp_len
) 
```





**Parameters:**


* `msc` Pointer to USB MSC instance. 
* `resp_len` Total prepared response bytes in msc-&gt;sector\_buf. 




        

<hr>



### function usb\_msc\_configured\_handler 

_Configured callback for USB MSC class interface activation._ 
```C++
static SYN_Status usb_msc_configured_handler (
    void * ctx,
    uint8_t config
) 
```





**Parameters:**


* `ctx` Class driver context ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) pointer). 
* `config` Configuration index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function usb\_msc\_data\_in\_handler 

_Data IN handler callback for USB MSC IN endpoint completion._ 
```C++
static void usb_msc_data_in_handler (
    void * ctx,
    uint8_t ep
) 
```





**Parameters:**


* `ctx` Class driver context ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) pointer). 
* `ep` Endpoint address. 




        

<hr>



### function usb\_msc\_data\_out\_handler 

_Data OUT handler callback for USB MSC OUT endpoint data._ 
```C++
static void usb_msc_data_out_handler (
    void * ctx,
    uint8_t ep,
    const uint8_t * data,
    uint16_t len
) 
```





**Parameters:**


* `ctx` Class driver context ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) pointer). 
* `ep` Endpoint address. 
* `data` Received payload bytes. 
* `len` Byte length of received payload. 




        

<hr>



### function usb\_msc\_setup\_handler 

_Setup handler callback for USB MSC class setup requests._ 
```C++
static SYN_Status usb_msc_setup_handler (
    void * ctx,
    const SYN_USB_SetupPacket * pkt,
    uint8_t * resp,
    uint16_t resp_cap,
    uint16_t * rlen
) 
```





**Parameters:**


* `ctx` Class driver context ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) pointer). 
* `pkt` Setup packet pointer. 
* `resp` Response payload buffer. 
* `resp_cap` Capacity of `resp` in bytes. 
* `rlen` Output response payload byte length pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL, SYN\_ERROR if response exceeds `resp_cap`. 





        

<hr>
## Macro Definition Documentation





### define SYN\_USB\_MSC\_CBW\_SIGNATURE 

```C++
#define SYN_USB_MSC_CBW_SIGNATURE `0x43425355U`
```



BOT CBW Magic Signature "USBC" 


        

<hr>



### define SYN\_USB\_MSC\_CSW\_SIGNATURE 

```C++
#define SYN_USB_MSC_CSW_SIGNATURE `0x53425355U`
```



BOT CSW Magic Signature "USBS" 


        

<hr>



### define SYN\_USB\_MSC\_CSW\_SIZE 

```C++
#define SYN_USB_MSC_CSW_SIZE `13U`
```



BOT CSW packet byte length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_msc.c`

