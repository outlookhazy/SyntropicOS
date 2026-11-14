

# Struct SYN\_USB\_MSC



[**ClassList**](annotated.md) **>** [**SYN\_USB\_MSC**](structSYN__USB__MSC.md)



[More...](#detailed-description)

* `#include <syn_usb_msc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**bot\_state**](#variable-bot_state)  <br> |
|  [**SYN\_USB\_MSC\_CBW**](structSYN__USB__MSC__CBW.md) | [**cbw**](#variable-cbw)  <br> |
|  [**SYN\_USB\_MSC\_CSW**](structSYN__USB__MSC__CSW.md) | [**csw**](#variable-csw)  <br> |
|  uint16\_t | [**data\_in\_len**](#variable-data_in_len)  <br> |
|  uint8\_t | [**ep\_in**](#variable-ep_in)  <br> |
|  uint8\_t | [**ep\_out**](#variable-ep_out)  <br> |
|  uint8\_t | [**iface\_num**](#variable-iface_num)  <br> |
|  [**SYN\_USB\_MSC\_Media**](structSYN__USB__MSC__Media.md) | [**media**](#variable-media)  <br> |
|  uint8\_t | [**sector\_buf**](#variable-sector_buf)  <br> |
|  uint16\_t | [**sector\_offset**](#variable-sector_offset)  <br> |
|  uint8\_t | [**sense\_asc**](#variable-sense_asc)  <br> |
|  uint8\_t | [**sense\_key**](#variable-sense_key)  <br> |
|  uint8\_t | [**tx\_buf**](#variable-tx_buf)  <br> |
|  uint16\_t | [**tx\_len**](#variable-tx_len)  <br> |
|  uint16\_t | [**xfer\_blocks\_rem**](#variable-xfer_blocks_rem)  <br> |
|  uint32\_t | [**xfer\_lba**](#variable-xfer_lba)  <br> |












































## Detailed Description


USB MSC Instance Context 


    
## Public Attributes Documentation




### variable bot\_state 

```C++
uint8_t SYN_USB_MSC::bot_state;
```



Active BOT state (SYN\_USB\_MSC\_BOT\_\*) 


        

<hr>



### variable cbw 

```C++
SYN_USB_MSC_CBW SYN_USB_MSC::cbw;
```



Active SCSI CBW command 


        

<hr>



### variable csw 

```C++
SYN_USB_MSC_CSW SYN_USB_MSC::csw;
```



Pending SCSI CSW status 


        

<hr>



### variable data\_in\_len 

```C++
uint16_t SYN_USB_MSC::data_in_len;
```



Staged byte count in sector\_buf 


        

<hr>



### variable ep\_in 

```C++
uint8_t SYN_USB_MSC::ep_in;
```



Bulk IN Endpoint address 


        

<hr>



### variable ep\_out 

```C++
uint8_t SYN_USB_MSC::ep_out;
```



Bulk OUT Endpoint address 


        

<hr>



### variable iface\_num 

```C++
uint8_t SYN_USB_MSC::iface_num;
```



Assigned interface index 


        

<hr>



### variable media 

```C++
SYN_USB_MSC_Media SYN_USB_MSC::media;
```



Bound storage media device 


        

<hr>



### variable sector\_buf 

```C++
uint8_t SYN_USB_MSC::sector_buf[SYN_USB_MSC_BLOCK_SIZE];
```



Single-sector BOT staging buffer 


        

<hr>



### variable sector\_offset 

```C++
uint16_t SYN_USB_MSC::sector_offset;
```



Byte offset within sector\_buf 


        

<hr>



### variable sense\_asc 

```C++
uint8_t SYN_USB_MSC::sense_asc;
```



Pending SCSI Additional Sense Code 


        

<hr>



### variable sense\_key 

```C++
uint8_t SYN_USB_MSC::sense_key;
```



Pending SCSI Sense Key 


        

<hr>



### variable tx\_buf 

```C++
uint8_t SYN_USB_MSC::tx_buf[SYN_USB_MSC_MAX_PACKET_SIZE];
```



IN packet buffer 


        

<hr>



### variable tx\_len 

```C++
uint16_t SYN_USB_MSC::tx_len;
```



Pending IN byte length 


        

<hr>



### variable xfer\_blocks\_rem 

```C++
uint16_t SYN_USB_MSC::xfer_blocks_rem;
```



Remaining sectors in READ\_10/WRITE\_10 


        

<hr>



### variable xfer\_lba 

```C++
uint32_t SYN_USB_MSC::xfer_lba;
```



Next LBA for active block transfer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_msc.h`

