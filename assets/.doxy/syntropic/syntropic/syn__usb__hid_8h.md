

# File syn\_usb\_hid.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_hid.h**](syn__usb__hid_8h.md)

[Go to the source code of this file](syn__usb__hid_8h_source.md)

_Zero-Heap USB 2.0 Human Interface Device (HID) Class Driver._ 

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/drivers/syn_usb.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include "syntropic/pt/syn_pt.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_USB\_HID**](structSYN__USB__HID.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_init**](#function-syn_usb_hid_init) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid) <br>_Initialize USB HID Class Instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_read\_report**](#function-syn_usb_hid_read_report) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, void \* buf, size\_t max\_len, size\_t \* out\_len) <br>_Read received OUT report from host._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_register**](#function-syn_usb_hid_register) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, [**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, const uint8\_t \* report\_desc, uint16\_t report\_desc\_len) <br>_Register USB HID class driver with USB device core._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_hid\_report\_available**](#function-syn_usb_hid_report_available) (const [**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid) <br>_Check if OUT report is available._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_send\_report**](#function-syn_usb_hid_send_report) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, const void \* report, size\_t len) <br>_Queue an IN report for transmission to host._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_usb\_hid\_tx\_ready**](#function-syn_usb_hid_tx_ready) (const [**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid) <br>_Check if IN transmit buffer is ready._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**PT\_USB\_HID\_WAIT\_RX**](syn__usb__hid_8h.md#define-pt_usb_hid_wait_rx) (pt, hid) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_usb\_hid\_report\_available**](syn__usb__hid_8c.md#function-syn_usb_hid_report_available)(hid))`<br>_Block a protothread coroutine until an OUT report is available._  |
| define  | [**PT\_USB\_HID\_WAIT\_TX\_READY**](syn__usb__hid_8h.md#define-pt_usb_hid_wait_tx_ready) (pt, hid) `[**PT\_WAIT\_UNTIL**](syn__pt_8h.md#define-pt_wait_until)(pt, [**syn\_usb\_hid\_tx\_ready**](syn__usb__hid_8c.md#function-syn_usb_hid_tx_ready)(hid))`<br>_Block a protothread coroutine until the IN report transmit buffer is ready._  |
| define  | [**SYN\_USB\_HID\_MAX\_REPORT\_SIZE**](syn__usb__hid_8h.md#define-syn_usb_hid_max_report_size)  `64U`<br> |
| define  | [**SYN\_USB\_HID\_REQ\_GET\_IDLE**](syn__usb__hid_8h.md#define-syn_usb_hid_req_get_idle)  `0x02U`<br> |
| define  | [**SYN\_USB\_HID\_REQ\_GET\_PROTOCOL**](syn__usb__hid_8h.md#define-syn_usb_hid_req_get_protocol)  `0x03U`<br> |
| define  | [**SYN\_USB\_HID\_REQ\_GET\_REPORT**](syn__usb__hid_8h.md#define-syn_usb_hid_req_get_report)  `0x01U`<br> |
| define  | [**SYN\_USB\_HID\_REQ\_SET\_IDLE**](syn__usb__hid_8h.md#define-syn_usb_hid_req_set_idle)  `0x0AU`<br> |
| define  | [**SYN\_USB\_HID\_REQ\_SET\_PROTOCOL**](syn__usb__hid_8h.md#define-syn_usb_hid_req_set_protocol)  `0x0BU`<br> |
| define  | [**SYN\_USB\_HID\_REQ\_SET\_REPORT**](syn__usb__hid_8h.md#define-syn_usb_hid_req_set_report)  `0x09U`<br> |

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

_Queue an IN report for transmission to host._ 
```C++
SYN_Status syn_usb_hid_send_report (
    SYN_USB_HID * hid,
    const void * report,
    size_t len
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 
* `report` Pointer to report data. 
* `len` Report byte length. 



**Returns:**

SYN\_OK on success. 





        

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
## Macro Definition Documentation





### define PT\_USB\_HID\_WAIT\_RX 

_Block a protothread coroutine until an OUT report is available._ 
```C++
#define PT_USB_HID_WAIT_RX (
    pt,
    hid
) `PT_WAIT_UNTIL (pt, syn_usb_hid_report_available (hid))`
```





**Parameters:**


* `pt` Protothread context. 
* `hid` Pointer to USB HID instance. 




        

<hr>



### define PT\_USB\_HID\_WAIT\_TX\_READY 

_Block a protothread coroutine until the IN report transmit buffer is ready._ 
```C++
#define PT_USB_HID_WAIT_TX_READY (
    pt,
    hid
) `PT_WAIT_UNTIL (pt, syn_usb_hid_tx_ready (hid))`
```





**Parameters:**


* `pt` Protothread context. 
* `hid` Pointer to USB HID instance. 




        

<hr>



### define SYN\_USB\_HID\_MAX\_REPORT\_SIZE 

```C++
#define SYN_USB_HID_MAX_REPORT_SIZE `64U`
```



Maximum HID report size 


        

<hr>



### define SYN\_USB\_HID\_REQ\_GET\_IDLE 

```C++
#define SYN_USB_HID_REQ_GET_IDLE `0x02U`
```



HID Request Get Idle (0x02) 


        

<hr>



### define SYN\_USB\_HID\_REQ\_GET\_PROTOCOL 

```C++
#define SYN_USB_HID_REQ_GET_PROTOCOL `0x03U`
```



HID Request Get Protocol (0x03) 


        

<hr>



### define SYN\_USB\_HID\_REQ\_GET\_REPORT 

```C++
#define SYN_USB_HID_REQ_GET_REPORT `0x01U`
```



HID Class Specific Requests per USB HID Spec 1.11 §7.2 HID Request Get Report (0x01) 


        

<hr>



### define SYN\_USB\_HID\_REQ\_SET\_IDLE 

```C++
#define SYN_USB_HID_REQ_SET_IDLE `0x0AU`
```



HID Request Set Idle (0x0A) 


        

<hr>



### define SYN\_USB\_HID\_REQ\_SET\_PROTOCOL 

```C++
#define SYN_USB_HID_REQ_SET_PROTOCOL `0x0BU`
```



HID Request Set Protocol (0x0B) 


        

<hr>



### define SYN\_USB\_HID\_REQ\_SET\_REPORT 

```C++
#define SYN_USB_HID_REQ_SET_REPORT `0x09U`
```



HID Request Set Report (0x09) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_hid.h`

