

# File syn\_usb\_msc.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_msc.c**](syn__usb__msc_8c.md)

[Go to the source code of this file](syn__usb__msc_8c_source.md)

_Zero-Heap USB 2.0 Mass Storage Class (MSC) Device Driver Implementation._ 

* `#include "syn_usb_msc.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_init**](#function-syn_usb_msc_init) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Initialize USB MSC Instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_process\_cbw**](#function-syn_usb_msc_process_cbw) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, const uint8\_t cbw\_raw) <br>_Process incoming BOT Command Block Wrapper (CBW)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_register**](#function-syn_usb_msc_register) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, [**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc) <br>_Register USB MSC class driver with USB core._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_msc\_set\_media**](#function-syn_usb_msc_set_media) ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) \* msc, const [**SYN\_USB\_MSC\_Media**](structSYN__USB__MSC__Media.md) \* media) <br>_Bind block storage media callbacks._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_msc\_configured\_handler**](#function-usb_msc_configured_handler) (void \* ctx, uint8\_t config) <br>_Configured callback for USB MSC class interface activation._  |
|  void | [**usb\_msc\_data\_in\_handler**](#function-usb_msc_data_in_handler) (void \* ctx, uint8\_t ep) <br>_Data IN handler callback for USB MSC IN endpoint completion._  |
|  void | [**usb\_msc\_data\_out\_handler**](#function-usb_msc_data_out_handler) (void \* ctx, uint8\_t ep, const uint8\_t \* data, uint16\_t len) <br>_Data OUT handler callback for USB MSC OUT endpoint data._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_msc\_setup\_handler**](#function-usb_msc_setup_handler) (void \* ctx, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t \* rlen) <br>_Setup handler callback for USB MSC class setup requests._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_USB\_MSC\_CBW\_SIGNATURE**](syn__usb__msc_8c.md#define-syn_usb_msc_cbw_signature)  `0x43425355U`<br> |
| define  | [**SYN\_USB\_MSC\_CSW\_SIGNATURE**](syn__usb__msc_8c.md#define-syn_usb_msc_csw_signature)  `0x53425355U`<br> |

## Public Functions Documentation




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

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL or corrupt. 





        

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
    uint16_t * rlen
) 
```





**Parameters:**


* `ctx` Class driver context ([**SYN\_USB\_MSC**](structSYN__USB__MSC.md) pointer). 
* `pkt` Setup packet pointer. 
* `resp` Response payload buffer. 
* `rlen` Output response payload byte length pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_msc.c`

