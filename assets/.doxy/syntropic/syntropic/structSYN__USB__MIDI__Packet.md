

# Struct SYN\_USB\_MIDI\_Packet



[**ClassList**](annotated.md) **>** [**SYN\_USB\_MIDI\_Packet**](structSYN__USB__MIDI__Packet.md)



[More...](#detailed-description)

* `#include <syn_usb_midi.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**header**](#variable-header)  <br> |
|  uint8\_t | [**midi0**](#variable-midi0)  <br> |
|  uint8\_t | [**midi1**](#variable-midi1)  <br> |
|  uint8\_t | [**midi2**](#variable-midi2)  <br> |












































## Detailed Description


4-byte USB MIDI Event Packet 


    
## Public Attributes Documentation




### variable header 

```C++
uint8_t SYN_USB_MIDI_Packet::header;
```



Cable Number (4 bits) \| Code Index Number CIN (4 bits) 


        

<hr>



### variable midi0 

```C++
uint8_t SYN_USB_MIDI_Packet::midi0;
```



MIDI Status byte (e.g. 0x90 Note On) 
 


        

<hr>



### variable midi1 

```C++
uint8_t SYN_USB_MIDI_Packet::midi1;
```



MIDI Parameter 1 (e.g. Note Number) 
 


        

<hr>



### variable midi2 

```C++
uint8_t SYN_USB_MIDI_Packet::midi2;
```



MIDI Parameter 2 (e.g. Velocity) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_midi.h`

