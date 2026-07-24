

# Struct SYN\_ISOTP\_Link



[**ClassList**](annotated.md) **>** [**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md)



_ISO 15765-2 Link Handle._ 

* `#include <syn_isotp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**n\_bs\_timeout\_us**](#variable-n_bs_timeout_us)  <br> |
|  uint32\_t | [**n\_cr\_timeout\_us**](#variable-n_cr_timeout_us)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  size\_t | [**rx\_expected**](#variable-rx_expected)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**rx\_fc\_pending**](#variable-rx_fc_pending)  <br> |
|  uint8\_t | [**rx\_fc\_status**](#variable-rx_fc_status)  <br> |
|  uint32\_t | [**rx\_id**](#variable-rx_id)  <br> |
|  size\_t | [**rx\_len**](#variable-rx_len)  <br> |
|  uint8\_t | [**rx\_seq**](#variable-rx_seq)  <br> |
|  [**SYN\_ISOTP\_RxState**](syn__isotp_8h.md#enum-syn_isotp_rxstate) | [**rx\_state**](#variable-rx_state)  <br> |
|  uint32\_t | [**rx\_timeout\_timer\_us**](#variable-rx_timeout_timer_us)  <br> |
|  uint8\_t | [**tx\_bs**](#variable-tx_bs)  <br> |
|  uint8\_t | [**tx\_bs\_count**](#variable-tx_bs_count)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  uint32\_t | [**tx\_id**](#variable-tx_id)  <br> |
|  size\_t | [**tx\_len**](#variable-tx_len)  <br> |
|  size\_t | [**tx\_offset**](#variable-tx_offset)  <br> |
|  uint8\_t | [**tx\_seq**](#variable-tx_seq)  <br> |
|  uint8\_t | [**tx\_st\_min**](#variable-tx_st_min)  <br> |
|  uint32\_t | [**tx\_st\_timer\_us**](#variable-tx_st_timer_us)  <br> |
|  [**SYN\_ISOTP\_TxState**](syn__isotp_8h.md#enum-syn_isotp_txstate) | [**tx\_state**](#variable-tx_state)  <br> |
|  uint32\_t | [**tx\_timeout\_timer\_us**](#variable-tx_timeout_timer_us)  <br> |












































## Public Attributes Documentation




### variable n\_bs\_timeout\_us 

```C++
uint32_t SYN_ISOTP_Link::n_bs_timeout_us;
```



N\_Bs timeout (microseconds) 


        

<hr>



### variable n\_cr\_timeout\_us 

```C++
uint32_t SYN_ISOTP_Link::n_cr_timeout_us;
```



N\_Cr timeout (microseconds) 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_ISOTP_Link::rx_buf;
```



Rx assembly buffer 
 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_ISOTP_Link::rx_buf_size;
```



Rx capacity 
 


        

<hr>



### variable rx\_expected 

```C++
size_t SYN_ISOTP_Link::rx_expected;
```



Total expected msg length 
 


        

<hr>



### variable rx\_fc\_pending 

```C++
bool SYN_ISOTP_Link::rx_fc_pending;
```



Pending Flow Control frame 


        

<hr>



### variable rx\_fc\_status 

```C++
uint8_t SYN_ISOTP_Link::rx_fc_status;
```



Flow Control status to tx 
 


        

<hr>



### variable rx\_id 

```C++
uint32_t SYN_ISOTP_Link::rx_id;
```



Expected CAN Rx ID 
 


        

<hr>



### variable rx\_len 

```C++
size_t SYN_ISOTP_Link::rx_len;
```



Assembled byte count 
 


        

<hr>



### variable rx\_seq 

```C++
uint8_t SYN_ISOTP_Link::rx_seq;
```



Next expected sequence 
 


        

<hr>



### variable rx\_state 

```C++
SYN_ISOTP_RxState SYN_ISOTP_Link::rx_state;
```



Receive state 
 


        

<hr>



### variable rx\_timeout\_timer\_us 

```C++
uint32_t SYN_ISOTP_Link::rx_timeout_timer_us;
```



Active N\_Cr timer us 
 


        

<hr>



### variable tx\_bs 

```C++
uint8_t SYN_ISOTP_Link::tx_bs;
```



Active block size limit 
 


        

<hr>



### variable tx\_bs\_count 

```C++
uint8_t SYN_ISOTP_Link::tx_bs_count;
```



Sent block frame counter 
 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_ISOTP_Link::tx_buf;
```



Tx payload buffer 
 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_ISOTP_Link::tx_buf_size;
```



Tx capacity 
 


        

<hr>



### variable tx\_id 

```C++
uint32_t SYN_ISOTP_Link::tx_id;
```



Transmit CAN Tx ID 
 


        

<hr>



### variable tx\_len 

```C++
size_t SYN_ISOTP_Link::tx_len;
```



Total payload length to tx 


        

<hr>



### variable tx\_offset 

```C++
size_t SYN_ISOTP_Link::tx_offset;
```



Sent byte counter 
 


        

<hr>



### variable tx\_seq 

```C++
uint8_t SYN_ISOTP_Link::tx_seq;
```



Sequence counter (0..15) 
 


        

<hr>



### variable tx\_st\_min 

```C++
uint8_t SYN_ISOTP_Link::tx_st_min;
```



STmin from receiver 
 


        

<hr>



### variable tx\_st\_timer\_us 

```C++
uint32_t SYN_ISOTP_Link::tx_st_timer_us;
```



STmin timer (microseconds) 


        

<hr>



### variable tx\_state 

```C++
SYN_ISOTP_TxState SYN_ISOTP_Link::tx_state;
```



Transmit state 
 


        

<hr>



### variable tx\_timeout\_timer\_us 

```C++
uint32_t SYN_ISOTP_Link::tx_timeout_timer_us;
```



Active N\_Bs timer us 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_isotp.h`

