

# Struct SYN\_CRSF\_Parser



[**ClassList**](annotated.md) **>** [**SYN\_CRSF\_Parser**](structSYN__CRSF__Parser.md)



[More...](#detailed-description)

* `#include <syn_crsf.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buf**](#variable-buf)  <br> |
|  uint32\_t | [**crc\_errors**](#variable-crc_errors)  <br> |
|  uint8\_t | [**idx**](#variable-idx)  <br> |
|  [**SYN\_CRSF\_ChannelsFrame**](structSYN__CRSF__ChannelsFrame.md) | [**last\_channels**](#variable-last_channels)  <br> |
|  [**SYN\_CRSF\_LinkStats**](structSYN__CRSF__LinkStats.md) | [**last\_link\_stats**](#variable-last_link_stats)  <br> |
|  uint32\_t | [**packets\_received**](#variable-packets_received)  <br> |
|  uint8\_t | [**payload\_len**](#variable-payload_len)  <br> |












































## Detailed Description


CRSF Parser Instance. 


    
## Public Attributes Documentation




### variable buf 

```C++
uint8_t SYN_CRSF_Parser::buf[SYN_CRSF_MAX_PACKET_LEN];
```



Assembly buffer 


        

<hr>



### variable crc\_errors 

```C++
uint32_t SYN_CRSF_Parser::crc_errors;
```



Count of CRC errors 


        

<hr>



### variable idx 

```C++
uint8_t SYN_CRSF_Parser::idx;
```



Current write index 


        

<hr>



### variable last\_channels 

```C++
SYN_CRSF_ChannelsFrame SYN_CRSF_Parser::last_channels;
```



Last decoded RC channels 


        

<hr>



### variable last\_link\_stats 

```C++
SYN_CRSF_LinkStats SYN_CRSF_Parser::last_link_stats;
```



Last decoded link statistics 


        

<hr>



### variable packets\_received 

```C++
uint32_t SYN_CRSF_Parser::packets_received;
```



Total valid packets received 


        

<hr>



### variable payload\_len 

```C++
uint8_t SYN_CRSF_Parser::payload_len;
```



Expected payload length 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_crsf.h`

