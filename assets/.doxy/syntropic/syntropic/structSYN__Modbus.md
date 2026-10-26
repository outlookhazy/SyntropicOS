

# Struct SYN\_Modbus



[**ClassList**](annotated.md) **>** [**SYN\_Modbus**](structSYN__Modbus.md)



_Modbus RTU slave instance — frame buffer, config, and statistics._ 

* `#include <syn_modbus.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  uint16\_t | [**buf\_size**](#variable-buf_size)  <br> |
|  uint16\_t | [**bus\_message\_count**](#variable-bus_message_count)  <br> |
|  [**SYN\_Modbus\_Config**](structSYN__Modbus__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  uint16\_t | [**comm\_event\_counter**](#variable-comm_event_counter)  <br> |
|  uint32\_t | [**errors**](#variable-errors)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**frame\_ready**](#variable-frame_ready)  <br> |
|  uint32\_t | [**frames\_rx**](#variable-frames_rx)  <br> |
|  uint32\_t | [**frames\_tx**](#variable-frames_tx)  <br> |
|  uint32\_t | [**last\_byte\_tick**](#variable-last_byte_tick)  <br> |
|  uint16\_t | [**rx\_len**](#variable-rx_len)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
uint8_t* SYN_Modbus::buf;
```



RX/TX buffer 
 


        

<hr>



### variable buf\_size 

```C++
uint16_t SYN_Modbus::buf_size;
```



Buffer capacity 
 


        

<hr>



### variable bus\_message\_count 

```C++
uint16_t SYN_Modbus::bus_message_count;
```



Bus message count for FC 0x0C 
 


        

<hr>



### variable cfg 

```C++
SYN_Modbus_Config SYN_Modbus::cfg;
```



Configuration snapshot 
 


        

<hr>



### variable comm\_event\_counter 

```C++
uint16_t SYN_Modbus::comm_event_counter;
```



Event counter for FC 0x0B/0x0C 


        

<hr>



### variable errors 

```C++
uint32_t SYN_Modbus::errors;
```



CRC/framing error count 
 


        

<hr>



### variable frame\_ready 

```C++
bool SYN_Modbus::frame_ready;
```



Complete frame available? 
 


        

<hr>



### variable frames\_rx 

```C++
uint32_t SYN_Modbus::frames_rx;
```



Total frames received 
 


        

<hr>



### variable frames\_tx 

```C++
uint32_t SYN_Modbus::frames_tx;
```



Total frames sent 
 


        

<hr>



### variable last\_byte\_tick 

```C++
uint32_t SYN_Modbus::last_byte_tick;
```



Tick of last received byte 
 


        

<hr>



### variable rx\_len 

```C++
uint16_t SYN_Modbus::rx_len;
```



Bytes received in current frame 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus.h`

