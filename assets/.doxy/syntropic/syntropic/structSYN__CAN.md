

# Struct SYN\_CAN



[**ClassList**](annotated.md) **>** [**SYN\_CAN**](structSYN__CAN.md)



_CAN bus instance — port, bitrate, callbacks, stats._ 

* `#include <syn_can.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**bitrate**](#variable-bitrate)  <br> |
|  uint32\_t | [**err\_count**](#variable-err_count)  <br> |
|  [**SYN\_CAN\_Callback**](syn__can_8h.md#typedef-syn_can_callback) | [**on\_rx**](#variable-on_rx)  <br> |
|  void \* | [**on\_rx\_ctx**](#variable-on_rx_ctx)  <br> |
|  uint8\_t | [**port**](#variable-port)  <br> |
|  uint32\_t | [**rx\_count**](#variable-rx_count)  <br> |
|  uint32\_t | [**tx\_count**](#variable-tx_count)  <br> |












































## Public Attributes Documentation




### variable bitrate 

```C++
uint32_t SYN_CAN::bitrate;
```



Configured bitrate 
 


        

<hr>



### variable err\_count 

```C++
uint32_t SYN_CAN::err_count;
```



Transmission errors 
 


        

<hr>



### variable on\_rx 

```C++
SYN_CAN_Callback SYN_CAN::on_rx;
```



Receive callback 
 


        

<hr>



### variable on\_rx\_ctx 

```C++
void* SYN_CAN::on_rx_ctx;
```



Receive callback context 
 


        

<hr>



### variable port 

```C++
uint8_t SYN_CAN::port;
```



CAN port number 
 


        

<hr>



### variable rx\_count 

```C++
uint32_t SYN_CAN::rx_count;
```



Frames received 
 


        

<hr>



### variable tx\_count 

```C++
uint32_t SYN_CAN::tx_count;
```



Frames transmitted 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_can.h`

