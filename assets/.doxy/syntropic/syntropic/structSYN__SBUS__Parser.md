

# Struct SYN\_SBUS\_Parser



[**ClassList**](annotated.md) **>** [**SYN\_SBUS\_Parser**](structSYN__SBUS__Parser.md)



[More...](#detailed-description)

* `#include <syn_sbus.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buf**](#variable-buf)  <br> |
|  uint32\_t | [**failsafe\_count**](#variable-failsafe_count)  <br> |
|  uint32\_t | [**frame\_loss\_count**](#variable-frame_loss_count)  <br> |
|  uint32\_t | [**frames\_received**](#variable-frames_received)  <br> |
|  uint8\_t | [**idx**](#variable-idx)  <br> |
|  [**SYN\_SBUS\_Frame**](structSYN__SBUS__Frame.md) | [**last\_frame**](#variable-last_frame)  <br> |












































## Detailed Description


SBUS Streaming Decoder State Machine Instance. 


    
## Public Attributes Documentation




### variable buf 

```C++
uint8_t SYN_SBUS_Parser::buf[SYN_SBUS_FRAME_SIZE];
```



Frame assembly buffer 


        

<hr>



### variable failsafe\_count 

```C++
uint32_t SYN_SBUS_Parser::failsafe_count;
```



Count of frames with failsafe bit set 


        

<hr>



### variable frame\_loss\_count 

```C++
uint32_t SYN_SBUS_Parser::frame_loss_count;
```



Count of frames with frame loss bit set 


        

<hr>



### variable frames\_received 

```C++
uint32_t SYN_SBUS_Parser::frames_received;
```



Total valid SBUS frames decoded 


        

<hr>



### variable idx 

```C++
uint8_t SYN_SBUS_Parser::idx;
```



Current byte index in buffer 


        

<hr>



### variable last\_frame 

```C++
SYN_SBUS_Frame SYN_SBUS_Parser::last_frame;
```



Last successfully decoded SBUS frame 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_sbus.h`

