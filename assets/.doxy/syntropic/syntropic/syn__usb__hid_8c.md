

# File syn\_usb\_hid.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_hid.c**](syn__usb__hid_8c.md)

[Go to the source code of this file](syn__usb__hid_8c_source.md)

_Zero-Heap USB 2.0 Human Interface Device (HID) Class Driver Implementation._ 

* `#include "syntropic/drivers/syn_usb_hid.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t | [**HID\_INTERFACE\_DESC**](#variable-hid_interface_desc)   = `/* multi line expression */`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_init**](#function-syn_usb_hid_init) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid) <br>_Initialize USB HID Class Instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_read\_report**](#function-syn_usb_hid_read_report) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, void \* buf, size\_t max\_len, size\_t \* out\_len) <br>_Read received OUT report from host._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_register**](#function-syn_usb_hid_register) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, [**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, const uint8\_t \* report\_desc, uint16\_t report\_desc\_len) <br>_Register USB HID class driver with USB device core._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_hid\_report\_available**](#function-syn_usb_hid_report_available) (const [**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid) <br>_Check if OUT report is available._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_send\_report**](#function-syn_usb_hid_send_report) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, const void \* report, size\_t len) <br>_Stage an IN report in the HID Interrupt IN transmit buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_hid\_tx\_ready**](#function-syn_usb_hid_tx_ready) (const [**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid) <br>_Check if IN transmit buffer is ready._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**hid\_class\_configured**](#function-hid_class_configured) (void \* ctx, uint8\_t config\_val) <br>_Reset staged HID IN report state upon USB SET\_CONFIGURATION._  |
|  void | [**hid\_class\_data\_in**](#function-hid_class_data_in) (void \* ctx, uint8\_t ep) <br>_Data IN completion callback for HID Interrupt IN endpoint._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**hid\_class\_setup**](#function-hid_class_setup) (void \* ctx, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t resp\_cap, uint16\_t \* rlen) <br>_Class setup request dispatcher for HID class requests._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**hid\_handle\_get\_descriptor**](#function-hid_handle_get_descriptor) (const [**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t resp\_cap, uint16\_t \* rlen) <br>_Handle HID GET\_DESCRIPTOR setup requests (0x22 Report Descriptor)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**hid\_write\_resp**](#function-hid_write_resp) (uint8\_t \* resp, uint16\_t resp\_cap, const uint8\_t \* src, uint16\_t len, uint16\_t \* rlen) <br>_Copy a HID EP0 IN response into_ `resp` _after checking_`resp_cap` _._ |


























## Public Static Attributes Documentation




### variable HID\_INTERFACE\_DESC 

```C++
const uint8_t HID_INTERFACE_DESC[25];
```



Standard HID Interface Descriptor Template (1 interface, 1 IN EP, 25 bytes) 


        

<hr>
## Public Functions Documentation




### function syn\_usb\_hid\_init 

_Initialize USB HID Class Instance._ 
```C++
SYN_Status syn_usb_hid_init (
    SYN_USB_HID * hid
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_hid\_read\_report 

_Read received OUT report from host._ 
```C++
SYN_Status syn_usb_hid_read_report (
    SYN_USB_HID * hid,
    void * buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 
* `buf` Output buffer. 
* `max_len` Capacity. 
* `out_len` Pointer to receive read byte count. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_hid\_register 

_Register USB HID class driver with USB device core._ 
```C++
SYN_Status syn_usb_hid_register (
    SYN_USB_Device * dev,
    SYN_USB_HID * hid,
    const uint8_t * report_desc,
    uint16_t report_desc_len
) 
```





**Parameters:**


* `dev` Pointer to USB device core context. 
* `hid` Pointer to USB HID instance. 
* `report_desc` Pointer to static HID report descriptor. 
* `report_desc_len` Byte length of report descriptor. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_hid\_report\_available 

_Check if OUT report is available._ 
```C++
bool syn_usb_hid_report_available (
    const SYN_USB_HID * hid
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 



**Returns:**

true if unread report data exists. 





        

<hr>



### function syn\_usb\_hid\_send\_report 

_Stage an IN report in the HID Interrupt IN transmit buffer._ 
```C++
SYN_Status syn_usb_hid_send_report (
    SYN_USB_HID * hid,
    const void * report,
    size_t len
) 
```





**Note:**

Port-decoupled staging contract: copies up to `SYN_USB_HID_MAX_REPORT_SIZE` (64 B) into `hid->tx_buf` and sets `hid->tx_len`. The platform USB ISR transmits `hid->tx_buf` on `hid->ep_in` and invokes the class `data_in` callback upon completion to clear `hid->tx_len` (or advance a queued auto-release report).




**Parameters:**


* `hid` Pointer to USB HID instance. 
* `report` Pointer to report data. 
* `len` Report byte length. 



**Returns:**

SYN\_OK on success, SYN\_BUSY if a staged report is still pending. 





        

<hr>



### function syn\_usb\_hid\_tx\_ready 

_Check if IN transmit buffer is ready._ 
```C++
bool syn_usb_hid_tx_ready (
    const SYN_USB_HID * hid
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 



**Returns:**

true if transmit buffer is free. 





        

<hr>
## Public Static Functions Documentation




### function hid\_class\_configured 

_Reset staged HID IN report state upon USB SET\_CONFIGURATION._ 
```C++
static SYN_Status hid_class_configured (
    void * ctx,
    uint8_t config_val
) 
```





**Parameters:**


* `ctx` Pointer to [**SYN\_USB\_HID**](structSYN__USB__HID.md) instance. 
* `config_val` Active configuration value. 



**Returns:**

SYN\_OK. 





        

<hr>



### function hid\_class\_data\_in 

_Data IN completion callback for HID Interrupt IN endpoint._ 
```C++
static void hid_class_data_in (
    void * ctx,
    uint8_t ep
) 
```





**Parameters:**


* `ctx` Pointer to [**SYN\_USB\_HID**](structSYN__USB__HID.md) instance. 
* `ep` Endpoint address. 




        

<hr>



### function hid\_class\_setup 

_Class setup request dispatcher for HID class requests._ 
```C++
static SYN_Status hid_class_setup (
    void * ctx,
    const SYN_USB_SetupPacket * pkt,
    uint8_t * resp,
    uint16_t resp_cap,
    uint16_t * rlen
) 
```





**Parameters:**


* `ctx` Pointer to [**SYN\_USB\_HID**](structSYN__USB__HID.md) instance. 
* `pkt` Pointer to received Setup Packet. 
* `resp` Response buffer. 
* `resp_cap` Capacity of `resp` in bytes. 
* `rlen` Output pointer to receive response byte length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function hid\_handle\_get\_descriptor 

_Handle HID GET\_DESCRIPTOR setup requests (0x22 Report Descriptor)._ 
```C++
static SYN_Status hid_handle_get_descriptor (
    const SYN_USB_HID * hid,
    const SYN_USB_SetupPacket * pkt,
    uint8_t * resp,
    uint16_t resp_cap,
    uint16_t * rlen
) 
```





**Parameters:**


* `hid` Pointer to [**SYN\_USB\_HID**](structSYN__USB__HID.md) instance. 
* `pkt` Setup packet pointer. 
* `resp` Response buffer. 
* `resp_cap` Capacity of `resp` in bytes. 
* `rlen` Output response length pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if unsupported or exceeds `resp_cap`. 





        

<hr>



### function hid\_write\_resp 

_Copy a HID EP0 IN response into_ `resp` _after checking_`resp_cap` _._
```C++
static SYN_Status hid_write_resp (
    uint8_t * resp,
    uint16_t resp_cap,
    const uint8_t * src,
    uint16_t len,
    uint16_t * rlen
) 
```





**Parameters:**


* `resp` Destination response buffer. 
* `resp_cap` Capacity of `resp` in bytes. 
* `src` Source payload bytes. 
* `len` Payload byte length. 
* `rlen` Output response length pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if `len` exceeds `resp_cap`. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_hid.c`

