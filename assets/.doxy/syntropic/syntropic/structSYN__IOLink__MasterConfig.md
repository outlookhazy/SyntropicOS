

# Struct SYN\_IOLink\_MasterConfig



[**ClassList**](annotated.md) **>** [**SYN\_IOLink\_MasterConfig**](structSYN__IOLink__MasterConfig.md)



_Master configuration descriptor._ 

* `#include <syn_iolink.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**cycle\_time\_ms**](#variable-cycle_time_ms)  <br> |
|  uint8\_t \* | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_buf\_size**](#variable-rx_buf_size)  <br> |
|  [**SYN\_IOLink\_Baud**](syn__iolink_8h.md#enum-syn_iolink_baud) | [**target\_baud**](#variable-target_baud)  <br> |
|  [**SYN\_Transport**](structSYN__Transport.md) \* | [**transport**](#variable-transport)  <br> |
|  uint8\_t \* | [**tx\_buf**](#variable-tx_buf)  <br> |
|  size\_t | [**tx\_buf\_size**](#variable-tx_buf_size)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable cycle\_time\_ms 

```C++
uint32_t SYN_IOLink_MasterConfig::cycle_time_ms;
```



Master cycle period in milliseconds 


        

<hr>



### variable rx\_buf 

```C++
uint8_t* SYN_IOLink_MasterConfig::rx_buf;
```



Scratch RX buffer 


        

<hr>



### variable rx\_buf\_size 

```C++
size_t SYN_IOLink_MasterConfig::rx_buf_size;
```



RX buffer size (&gt;= 64 bytes) 


        

<hr>



### variable target\_baud 

```C++
SYN_IOLink_Baud SYN_IOLink_MasterConfig::target_baud;
```



Target communication baud rate 


        

<hr>



### variable transport 

```C++
SYN_Transport* SYN_IOLink_MasterConfig::transport;
```



Serial / UART transport interface 


        

<hr>



### variable tx\_buf 

```C++
uint8_t* SYN_IOLink_MasterConfig::tx_buf;
```



Scratch TX buffer 


        

<hr>



### variable tx\_buf\_size 

```C++
size_t SYN_IOLink_MasterConfig::tx_buf_size;
```



TX buffer size (&gt;= 64 bytes) 


        

<hr>



### variable user\_data 

```C++
void* SYN_IOLink_MasterConfig::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_iolink.h`

