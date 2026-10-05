

# Struct SYN\_DShot\_Packet



[**ClassList**](annotated.md) **>** [**SYN\_DShot\_Packet**](structSYN__DShot__Packet.md)



[More...](#detailed-description)

* `#include <syn_dshot.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**crc**](#variable-crc)  <br> |
|  uint16\_t | [**raw\_frame**](#variable-raw_frame)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**telemetry**](#variable-telemetry)  <br> |
|  uint16\_t | [**throttle**](#variable-throttle)  <br> |












































## Detailed Description


DShot Encoded Packet Structure. 


    
## Public Attributes Documentation




### variable crc 

```C++
uint8_t SYN_DShot_Packet::crc;
```



4-bit calculated CRC. 


        

<hr>



### variable raw\_frame 

```C++
uint16_t SYN_DShot_Packet::raw_frame;
```



16-bit packed raw frame ready for DMA transmission. 


        

<hr>



### variable telemetry 

```C++
bool SYN_DShot_Packet::telemetry;
```



Telemetry request flag. 


        

<hr>



### variable throttle 

```C++
uint16_t SYN_DShot_Packet::throttle;
```



11-bit throttle value (0..2047). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_dshot.h`

