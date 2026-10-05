

# File syn\_usb\_midi.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_midi.h**](syn__usb__midi_8h.md)

[Go to the source code of this file](syn__usb__midi_8h_source.md)

_Zero-Heap USB 2.0 MIDI Class Device Driver._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/drivers/syn_usb.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md) <br> |
| struct | [**SYN\_USB\_MIDI\_Packet**](structSYN__USB__MIDI__Packet.md) <br> |






















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_USB\_MIDI\_CIN\_CABLE\_EVENT**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_cable_event)  `0x1U`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_CHANNEL\_PRESSURE**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_channel_pressure)  `0xDU`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_CONTROL\_CHANGE**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_control_change)  `0xBU`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_MISC**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_misc)  `0x0U`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_NOTE\_OFF**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_note_off)  `0x8U`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_NOTE\_ON**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_note_on)  `0x9U`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_PITCH\_BEND**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_pitch_bend)  `0xEU`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_POLY\_KEYPRESS**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_poly_keypress)  `0xAU`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_PROGRAM\_CHANGE**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_program_change)  `0xCU`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_SINGLE\_BYTE**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_single_byte)  `0xFU`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_SYSEX\_ENDS\_1**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_sysex_ends_1)  `0x5U`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_SYSEX\_ENDS\_2**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_sysex_ends_2)  `0x6U`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_SYSEX\_ENDS\_3**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_sysex_ends_3)  `0x7U`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_SYSEX\_START**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_sysex_start)  `0x4U`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_SYSTEM\_COMMON\_2**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_system_common_2)  `0x2U`<br> |
| define  | [**SYN\_USB\_MIDI\_CIN\_SYSTEM\_COMMON\_3**](syn__usb__midi_8h.md#define-syn_usb_midi_cin_system_common_3)  `0x3U`<br> |
| define  | [**SYN\_USB\_MIDI\_MAX\_PACKET\_SIZE**](syn__usb__midi_8h.md#define-syn_usb_midi_max_packet_size)  `64U`<br> |

## Detailed Description


Implements USB Class Definition for MIDI Devices v1.0 (Audio Streaming subclass). Provides 4-byte USB MIDI Event Packet encoding and decoding for Note On/Off, Control Change, Pitch Bend, and Program Change messages. 


    
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
## Macro Definition Documentation





### define SYN\_USB\_MIDI\_CIN\_CABLE\_EVENT 

```C++
#define SYN_USB_MIDI_CIN_CABLE_EVENT `0x1U`
```



Cable events 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_CHANNEL\_PRESSURE 

```C++
#define SYN_USB_MIDI_CIN_CHANNEL_PRESSURE `0xDU`
```



Channel Pressure message (0xD0) 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_CONTROL\_CHANGE 

```C++
#define SYN_USB_MIDI_CIN_CONTROL_CHANGE `0xBU`
```



Control Change message (0xB0) 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_MISC 

```C++
#define SYN_USB_MIDI_CIN_MISC `0x0U`
```



USB MIDI Code Index Number (CIN) definitions per USB MIDI Spec Table 4-1 Miscellaneous function codes 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_NOTE\_OFF 

```C++
#define SYN_USB_MIDI_CIN_NOTE_OFF `0x8U`
```



Note Off message (0x80) 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_NOTE\_ON 

```C++
#define SYN_USB_MIDI_CIN_NOTE_ON `0x9U`
```



Note On message (0x90) 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_PITCH\_BEND 

```C++
#define SYN_USB_MIDI_CIN_PITCH_BEND `0xEU`
```



Pitch Bend message (0xE0) 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_POLY\_KEYPRESS 

```C++
#define SYN_USB_MIDI_CIN_POLY_KEYPRESS `0xAU`
```



Poly KeyPress message (0xA0) 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_PROGRAM\_CHANGE 

```C++
#define SYN_USB_MIDI_CIN_PROGRAM_CHANGE `0xCU`
```



Program Change message (0xC0) 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_SINGLE\_BYTE 

```C++
#define SYN_USB_MIDI_CIN_SINGLE_BYTE `0xFU`
```



Single Byte message 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_SYSEX\_ENDS\_1 

```C++
#define SYN_USB_MIDI_CIN_SYSEX_ENDS_1 `0x5U`
```



Single-byte SysEx ends 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_SYSEX\_ENDS\_2 

```C++
#define SYN_USB_MIDI_CIN_SYSEX_ENDS_2 `0x6U`
```



2-byte SysEx ends 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_SYSEX\_ENDS\_3 

```C++
#define SYN_USB_MIDI_CIN_SYSEX_ENDS_3 `0x7U`
```



3-byte SysEx ends 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_SYSEX\_START 

```C++
#define SYN_USB_MIDI_CIN_SYSEX_START `0x4U`
```



SysEx starts or continues 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_SYSTEM\_COMMON\_2 

```C++
#define SYN_USB_MIDI_CIN_SYSTEM_COMMON_2 `0x2U`
```



2-byte System Common messages 


        

<hr>



### define SYN\_USB\_MIDI\_CIN\_SYSTEM\_COMMON\_3 

```C++
#define SYN_USB_MIDI_CIN_SYSTEM_COMMON_3 `0x3U`
```



3-byte System Common messages 


        

<hr>



### define SYN\_USB\_MIDI\_MAX\_PACKET\_SIZE 

```C++
#define SYN_USB_MIDI_MAX_PACKET_SIZE `64U`
```



Maximum USB MIDI packet size in bytes 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_midi.h`

