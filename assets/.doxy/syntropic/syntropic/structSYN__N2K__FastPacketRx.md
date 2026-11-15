

# Struct SYN\_N2K\_FastPacketRx



[**ClassList**](annotated.md) **>** [**SYN\_N2K\_FastPacketRx**](structSYN__N2K__FastPacketRx.md)



_NMEA 2000 Fast-Packet RX Re-assembly Context._ [More...](#detailed-description)

* `#include <syn_n2k.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint8\_t | [**data**](#variable-data)  <br> |
|  uint8\_t | [**expected\_frame**](#variable-expected_frame)  <br> |
|  uint32\_t | [**last\_rx\_ms**](#variable-last_rx_ms)  <br> |
|  uint32\_t | [**pgn**](#variable-pgn)  <br> |
|  uint8\_t | [**received\_bytes**](#variable-received_bytes)  <br> |
|  uint8\_t | [**sa**](#variable-sa)  <br> |
|  uint8\_t | [**seq\_id**](#variable-seq_id)  <br> |
|  uint8\_t | [**total\_bytes**](#variable-total_bytes)  <br> |












































## Detailed Description




**Note:**

Tracks one active `( pgn , sa )` Fast-Packet session at a time. Concurrent Frame 0 arrivals from another source address are rejected with SYN\_BUSY until the active session completes, aborts on a sequence break, is cleared via [**syn\_n2k\_fastpacket\_reset()**](syn__n2k_8c.md#function-syn_n2k_fastpacket_reset), or expires after SYN\_N2K\_FASTPACKET\_TIMEOUT\_MS. 





    
## Public Attributes Documentation




### variable active 

```C++
bool SYN_N2K_FastPacketRx::active;
```



Active Fast-Packet session flag 


        

<hr>



### variable data 

```C++
uint8_t SYN_N2K_FastPacketRx::data[223];
```



Re-assembled payload buffer 


        

<hr>



### variable expected\_frame 

```C++
uint8_t SYN_N2K_FastPacketRx::expected_frame;
```



Next expected frame index (0..31) 


        

<hr>



### variable last\_rx\_ms 

```C++
uint32_t SYN_N2K_FastPacketRx::last_rx_ms;
```



Tick timestamp of last accepted Fast-Packet frame 


        

<hr>



### variable pgn 

```C++
uint32_t SYN_N2K_FastPacketRx::pgn;
```



Target PGN 


        

<hr>



### variable received\_bytes 

```C++
uint8_t SYN_N2K_FastPacketRx::received_bytes;
```



Currently received byte count 


        

<hr>



### variable sa 

```C++
uint8_t SYN_N2K_FastPacketRx::sa;
```



Sender Source Address 


        

<hr>



### variable seq\_id 

```C++
uint8_t SYN_N2K_FastPacketRx::seq_id;
```



Fast-Packet Sequence Identifier (0-7) 


        

<hr>



### variable total\_bytes 

```C++
uint8_t SYN_N2K_FastPacketRx::total_bytes;
```



Total payload length in bytes (max 223) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_n2k.h`

