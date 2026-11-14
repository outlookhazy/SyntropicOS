

# Struct SYN\_USB\_MIDI



[**ClassList**](annotated.md) **>** [**SYN\_USB\_MIDI**](structSYN__USB__MIDI.md)



[More...](#detailed-description)

* `#include <syn_usb_midi.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**cable\_num**](#variable-cable_num)  <br> |
|  uint8\_t | [**ep\_in**](#variable-ep_in)  <br> |
|  uint8\_t | [**ep\_out**](#variable-ep_out)  <br> |
|  uint8\_t | [**iface\_num**](#variable-iface_num)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  uint16\_t | [**rx\_len**](#variable-rx_len)  <br> |
|  uint8\_t | [**tx\_buf**](#variable-tx_buf)  <br> |
|  uint16\_t | [**tx\_len**](#variable-tx_len)  <br> |












































## Detailed Description


USB MIDI Instance Context 


    
## Public Attributes Documentation




### variable cable\_num 

```C++
uint8_t SYN_USB_MIDI::cable_num;
```



Virtual Cable Number (0..15) 


        

<hr>



### variable ep\_in 

```C++
uint8_t SYN_USB_MIDI::ep_in;
```



Bulk IN Endpoint address 


        

<hr>



### variable ep\_out 

```C++
uint8_t SYN_USB_MIDI::ep_out;
```



Bulk OUT Endpoint address 


        

<hr>



### variable iface\_num 

```C++
uint8_t SYN_USB_MIDI::iface_num;
```



Assigned interface index 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_USB_MIDI::rx_buf[SYN_USB_MIDI_MAX_PACKET_SIZE];
```



OUT packet buffer 


        

<hr>



### variable rx\_len 

```C++
uint16_t SYN_USB_MIDI::rx_len;
```



Unread OUT byte length 


        

<hr>



### variable tx\_buf 

```C++
uint8_t SYN_USB_MIDI::tx_buf[SYN_USB_MIDI_MAX_PACKET_SIZE];
```



IN packet buffer 


        

<hr>



### variable tx\_len 

```C++
uint16_t SYN_USB_MIDI::tx_len;
```



Pending IN byte length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_midi.h`

