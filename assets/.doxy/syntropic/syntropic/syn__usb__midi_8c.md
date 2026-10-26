

# File syn\_usb\_midi.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_midi.c**](syn__usb__midi_8c.md)

[Go to the source code of this file](syn__usb__midi_8c_source.md)

_Zero-Heap USB 2.0 MIDI Class Device Driver Implementation._ 

* `#include "syn_usb_midi.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_midi\_init**](#function-syn_usb_midi_init) ([**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) \* midi) <br>_Initialize USB MIDI Class Instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_midi\_parse\_packet**](#function-syn_usb_midi_parse_packet) (const uint8\_t raw\_bytes, [**SYN\_USB\_MIDI\_Packet**](structSYN__USB__MIDI__Packet.md) \* packet) <br>_Parse a 4-byte raw USB MIDI packet into structured event packet._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_midi\_register**](#function-syn_usb_midi_register) ([**SYN\_USB\_Device**](structSYN__USB__Device.md) \* dev, [**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) \* midi) <br>_Register USB MIDI class driver with USB device core._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_midi\_send\_cc**](#function-syn_usb_midi_send_cc) ([**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) \* midi, uint8\_t channel, uint8\_t controller, uint8\_t value) <br>_Encode and queue a Control Change (CC) MIDI message._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_midi\_send\_note\_off**](#function-syn_usb_midi_send_note_off) ([**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) \* midi, uint8\_t channel, uint8\_t note, uint8\_t velocity) <br>_Encode and queue a Note Off MIDI message._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_midi\_send\_note\_on**](#function-syn_usb_midi_send_note_on) ([**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) \* midi, uint8\_t channel, uint8\_t note, uint8\_t velocity) <br>_Encode and queue a Note On MIDI message._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_midi\_send\_pitch\_bend**](#function-syn_usb_midi_send_pitch_bend) ([**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) \* midi, uint8\_t channel, int16\_t value) <br>_Encode and queue a Pitch Bend MIDI message._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**queue\_midi\_packet**](#function-queue_midi_packet) ([**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) \* midi, uint8\_t cin, uint8\_t status, uint8\_t param1, uint8\_t param2) <br>_Queue a 4-byte USB MIDI Event Packet into TX buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_midi\_configured\_handler**](#function-usb_midi_configured_handler) (void \* ctx, uint8\_t config) <br>_Configured callback for USB MIDI class interface activation._  |
|  void | [**usb\_midi\_data\_in\_handler**](#function-usb_midi_data_in_handler) (void \* ctx, uint8\_t ep) <br>_Data IN handler callback for USB MIDI IN endpoint completion._  |
|  void | [**usb\_midi\_data\_out\_handler**](#function-usb_midi_data_out_handler) (void \* ctx, uint8\_t ep, const uint8\_t \* data, uint16\_t len) <br>_Data OUT handler callback for USB MIDI OUT endpoint data._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**usb\_midi\_setup\_handler**](#function-usb_midi_setup_handler) (void \* ctx, const [**SYN\_USB\_SetupPacket**](structSYN__USB__SetupPacket.md) \* pkt, uint8\_t \* resp, uint16\_t \* rlen) <br>_Setup handler callback for USB MIDI class setup requests._  |


























## Public Functions Documentation




### function syn\_usb\_midi\_init 

_Initialize USB MIDI Class Instance._ 
```C++
SYN_Status syn_usb_midi_init (
    SYN_USB_MIDI * midi
) 
```





**Parameters:**


* `midi` Pointer to USB MIDI instance. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_usb\_midi\_parse\_packet 

_Parse a 4-byte raw USB MIDI packet into structured event packet._ 
```C++
SYN_Status syn_usb_midi_parse_packet (
    const uint8_t raw_bytes,
    SYN_USB_MIDI_Packet * packet
) 
```





**Parameters:**


* `raw_bytes` 4-byte raw buffer. 
* `packet` Output structured packet pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_usb\_midi\_register 

_Register USB MIDI class driver with USB device core._ 
```C++
SYN_Status syn_usb_midi_register (
    SYN_USB_Device * dev,
    SYN_USB_MIDI * midi
) 
```





**Parameters:**


* `dev` Pointer to USB device core context. 
* `midi` Pointer to USB MIDI instance. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_usb\_midi\_send\_cc 

_Encode and queue a Control Change (CC) MIDI message._ 
```C++
SYN_Status syn_usb_midi_send_cc (
    SYN_USB_MIDI * midi,
    uint8_t channel,
    uint8_t controller,
    uint8_t value
) 
```





**Parameters:**


* `midi` Pointer to USB MIDI instance. 
* `channel` MIDI Channel (0..15). 
* `controller` Controller Number (0..127). 
* `value` Controller Value (0..127). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if invalid. 





        

<hr>



### function syn\_usb\_midi\_send\_note\_off 

_Encode and queue a Note Off MIDI message._ 
```C++
SYN_Status syn_usb_midi_send_note_off (
    SYN_USB_MIDI * midi,
    uint8_t channel,
    uint8_t note,
    uint8_t velocity
) 
```





**Parameters:**


* `midi` Pointer to USB MIDI instance. 
* `channel` MIDI Channel (0..15). 
* `note` MIDI Note Number (0..127). 
* `velocity` Release Velocity (0..127). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if invalid. 





        

<hr>



### function syn\_usb\_midi\_send\_note\_on 

_Encode and queue a Note On MIDI message._ 
```C++
SYN_Status syn_usb_midi_send_note_on (
    SYN_USB_MIDI * midi,
    uint8_t channel,
    uint8_t note,
    uint8_t velocity
) 
```





**Parameters:**


* `midi` Pointer to USB MIDI instance. 
* `channel` MIDI Channel (0..15). 
* `note` MIDI Note Number (0..127). 
* `velocity` Note Velocity (0..127). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if invalid. 





        

<hr>



### function syn\_usb\_midi\_send\_pitch\_bend 

_Encode and queue a Pitch Bend MIDI message._ 
```C++
SYN_Status syn_usb_midi_send_pitch_bend (
    SYN_USB_MIDI * midi,
    uint8_t channel,
    int16_t value
) 
```





**Parameters:**


* `midi` Pointer to USB MIDI instance. 
* `channel` MIDI Channel (0..15). 
* `value` Pitch Bend value (-8192..+8191, center 0). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if invalid. 





        

<hr>
## Public Static Functions Documentation




### function queue\_midi\_packet 

_Queue a 4-byte USB MIDI Event Packet into TX buffer._ 
```C++
static SYN_Status queue_midi_packet (
    SYN_USB_MIDI * midi,
    uint8_t cin,
    uint8_t status,
    uint8_t param1,
    uint8_t param2
) 
```





**Parameters:**


* `midi` Pointer to USB MIDI instance. 
* `cin` Code Index Number (4 bits). 
* `status` MIDI status byte. 
* `param1` MIDI parameter 1 byte. 
* `param2` MIDI parameter 2 byte. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on overflow. 





        

<hr>



### function usb\_midi\_configured\_handler 

_Configured callback for USB MIDI class interface activation._ 
```C++
static SYN_Status usb_midi_configured_handler (
    void * ctx,
    uint8_t config
) 
```





**Parameters:**


* `ctx` Class driver context ([**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) pointer). 
* `config` Configuration index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function usb\_midi\_data\_in\_handler 

_Data IN handler callback for USB MIDI IN endpoint completion._ 
```C++
static void usb_midi_data_in_handler (
    void * ctx,
    uint8_t ep
) 
```





**Parameters:**


* `ctx` Class driver context ([**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) pointer). 
* `ep` Endpoint address. 




        

<hr>



### function usb\_midi\_data\_out\_handler 

_Data OUT handler callback for USB MIDI OUT endpoint data._ 
```C++
static void usb_midi_data_out_handler (
    void * ctx,
    uint8_t ep,
    const uint8_t * data,
    uint16_t len
) 
```





**Parameters:**


* `ctx` Class driver context ([**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) pointer). 
* `ep` Endpoint address. 
* `data` Received payload bytes. 
* `len` Byte length of received payload. 




        

<hr>



### function usb\_midi\_setup\_handler 

_Setup handler callback for USB MIDI class setup requests._ 
```C++
static SYN_Status usb_midi_setup_handler (
    void * ctx,
    const SYN_USB_SetupPacket * pkt,
    uint8_t * resp,
    uint16_t * rlen
) 
```





**Parameters:**


* `ctx` Class driver context ([**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) pointer). 
* `pkt` Setup packet pointer. 
* `resp` Response payload buffer. 
* `rlen` Output response payload byte length pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_midi.c`

