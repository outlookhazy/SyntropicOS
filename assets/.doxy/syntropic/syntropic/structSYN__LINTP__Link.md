

# Struct SYN\_LINTP\_Link



[**ClassList**](annotated.md) **>** [**SYN\_LINTP\_Link**](structSYN__LINTP__Link.md)



_LIN TP Link Context structure._ 

* `#include <syn_lintp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**nad**](#variable-nad)  <br> |
|  uint8\_t | [**padding\_byte**](#variable-padding_byte)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**rx\_completed**](#variable-rx_completed)  <br> |
|  uint8\_t | [**rx\_nad**](#variable-rx_nad)  <br> |
|  size\_t | [**rx\_offset**](#variable-rx_offset)  <br> |
|  uint8\_t | [**rx\_sn**](#variable-rx_sn)  <br> |
|  [**SYN\_LINTP\_State**](syn__lintp_8h.md#enum-syn_lintp_state) | [**rx\_state**](#variable-rx_state)  <br> |
|  uint32\_t | [**rx\_timer\_ms**](#variable-rx_timer_ms)  <br> |
|  size\_t | [**rx\_total\_len**](#variable-rx_total_len)  <br> |
|  uint32\_t | [**timer\_n\_as\_ms**](#variable-timer_n_as_ms)  <br> |
|  uint32\_t | [**timer\_n\_cr\_ms**](#variable-timer_n_cr_ms)  <br> |
|  const uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  uint8\_t | [**tx\_frame**](#variable-tx_frame)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tx\_frame\_pending**](#variable-tx_frame_pending)  <br> |
|  uint8\_t | [**tx\_nad**](#variable-tx_nad)  <br> |
|  size\_t | [**tx\_offset**](#variable-tx_offset)  <br> |
|  uint8\_t | [**tx\_sn**](#variable-tx_sn)  <br> |
|  [**SYN\_LINTP\_State**](syn__lintp_8h.md#enum-syn_lintp_state) | [**tx\_state**](#variable-tx_state)  <br> |
|  uint32\_t | [**tx\_timer\_ms**](#variable-tx_timer_ms)  <br> |
|  size\_t | [**tx\_total\_len**](#variable-tx_total_len)  <br> |












































## Public Attributes Documentation




### variable nad 

```C++
uint8_t SYN_LINTP_Link::nad;
```



Configured target/local Node Address (1..0x7D) 


        

<hr>



### variable padding\_byte 

```C++
uint8_t SYN_LINTP_Link::padding_byte;
```



Unused byte fill value (default 0xFF) 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_LINTP_Link::rx_buf;
```



Pointer to user RX buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_LINTP_Link::rx_buf_size;
```



Capacity of user RX buffer 


        

<hr>



### variable rx\_completed 

```C++
bool SYN_LINTP_Link::rx_completed;
```



True when a complete message is ready to consume 


        

<hr>



### variable rx\_nad 

```C++
uint8_t SYN_LINTP_Link::rx_nad;
```



NAD of active received message 


        

<hr>



### variable rx\_offset 

```C++
size_t SYN_LINTP_Link::rx_offset;
```



Bytes received so far 


        

<hr>



### variable rx\_sn 

```C++
uint8_t SYN_LINTP_Link::rx_sn;
```



Next expected sequence number (1..15, mod 16) 


        

<hr>



### variable rx\_state 

```C++
SYN_LINTP_State SYN_LINTP_Link::rx_state;
```



RX state machine state 


        

<hr>



### variable rx\_timer\_ms 

```C++
uint32_t SYN_LINTP_Link::rx_timer_ms;
```



Elapsed time since last RX frame 


        

<hr>



### variable rx\_total\_len 

```C++
size_t SYN_LINTP_Link::rx_total_len;
```



Expected message payload length 


        

<hr>



### variable timer\_n\_as\_ms 

```C++
uint32_t SYN_LINTP_Link::timer_n_as_ms;
```



Maximum time for frame TX completion (default 1000ms) 


        

<hr>



### variable timer\_n\_cr\_ms 

```C++
uint32_t SYN_LINTP_Link::timer_n_cr_ms;
```



Maximum time between consecutive RX frames (default 1000ms) 


        

<hr>



### variable tx\_buf 

```C++
const uint8_t* SYN_LINTP_Link::tx_buf;
```



Pointer to active TX payload 


        

<hr>



### variable tx\_frame 

```C++
uint8_t SYN_LINTP_Link::tx_frame[8];
```



Queued 8-byte LIN frame payload 


        

<hr>



### variable tx\_frame\_pending 

```C++
bool SYN_LINTP_Link::tx_frame_pending;
```



True when a 8-byte frame is queued for TX 


        

<hr>



### variable tx\_nad 

```C++
uint8_t SYN_LINTP_Link::tx_nad;
```



Target NAD for active transmission 


        

<hr>



### variable tx\_offset 

```C++
size_t SYN_LINTP_Link::tx_offset;
```



Payload bytes transmitted so far 


        

<hr>



### variable tx\_sn 

```C++
uint8_t SYN_LINTP_Link::tx_sn;
```



Sequence number for next CF (1..15, mod 16) 


        

<hr>



### variable tx\_state 

```C++
SYN_LINTP_State SYN_LINTP_Link::tx_state;
```



TX state machine state 


        

<hr>



### variable tx\_timer\_ms 

```C++
uint32_t SYN_LINTP_Link::tx_timer_ms;
```



Elapsed time since last frame queued 


        

<hr>



### variable tx\_total\_len 

```C++
size_t SYN_LINTP_Link::tx_total_len;
```



Total payload size to transmit 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lintp.h`

