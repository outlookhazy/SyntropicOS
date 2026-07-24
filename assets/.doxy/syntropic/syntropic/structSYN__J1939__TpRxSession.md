

# Struct SYN\_J1939\_TpRxSession



[**ClassList**](annotated.md) **>** [**SYN\_J1939\_TpRxSession**](structSYN__J1939__TpRxSession.md)



_J1939 Transport Protocol Rx Re-assembly Buffer._ 

* `#include <syn_j1939.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  uint8\_t | [**da**](#variable-da)  <br> |
|  uint8\_t | [**data**](#variable-data)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_bam**](#variable-is_bam)  <br> |
|  uint8\_t | [**next\_seq**](#variable-next_seq)  <br> |
|  uint32\_t | [**pgn**](#variable-pgn)  <br> |
|  uint8\_t | [**received\_packets**](#variable-received_packets)  <br> |
|  uint8\_t | [**sa**](#variable-sa)  <br> |
|  uint16\_t | [**total\_bytes**](#variable-total_bytes)  <br> |
|  uint8\_t | [**total\_packets**](#variable-total_packets)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_J1939_TpRxSession::active;
```



Active reception session flag 


        

<hr>



### variable da 

```C++
uint8_t SYN_J1939_TpRxSession::da;
```



Destination Address 


        

<hr>



### variable data 

```C++
uint8_t SYN_J1939_TpRxSession::data[1785];
```



Maximum 1785-byte payload buffer 


        

<hr>



### variable is\_bam 

```C++
bool SYN_J1939_TpRxSession::is_bam;
```



Broadcast Announce Message session flag 


        

<hr>



### variable next\_seq 

```C++
uint8_t SYN_J1939_TpRxSession::next_seq;
```



Next expected sequence number (1..255) 


        

<hr>



### variable pgn 

```C++
uint32_t SYN_J1939_TpRxSession::pgn;
```



Reassembled Parameter Group Number 


        

<hr>



### variable received\_packets 

```C++
uint8_t SYN_J1939_TpRxSession::received_packets;
```



Currently received packet count 


        

<hr>



### variable sa 

```C++
uint8_t SYN_J1939_TpRxSession::sa;
```



Sender Source Address 


        

<hr>



### variable total\_bytes 

```C++
uint16_t SYN_J1939_TpRxSession::total_bytes;
```



Total payload length in bytes 


        

<hr>



### variable total\_packets 

```C++
uint8_t SYN_J1939_TpRxSession::total_packets;
```



Total packet count 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_j1939.h`

