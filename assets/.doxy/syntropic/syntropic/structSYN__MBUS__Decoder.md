

# Struct SYN\_MBUS\_Decoder



[**ClassList**](annotated.md) **>** [**SYN\_MBUS\_Decoder**](structSYN__MBUS__Decoder.md)



_M-Bus streaming state machine decoder._ 

* `#include <syn_mbus.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_MBUS\_FrameCallback**](syn__mbus_8h.md#typedef-syn_mbus_framecallback) | [**callback**](#variable-callback)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  size\_t | [**expected\_len**](#variable-expected_len)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_idx**](#variable-rx_idx)  <br> |
|  uint8\_t | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable callback 

```C++
SYN_MBUS_FrameCallback SYN_MBUS_Decoder::callback;
```



Frame completion callback 


        

<hr>



### variable ctx 

```C++
void* SYN_MBUS_Decoder::ctx;
```



Callback user context 


        

<hr>



### variable expected\_len 

```C++
size_t SYN_MBUS_Decoder::expected_len;
```



Expected total frame length 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_MBUS_Decoder::rx_buf[SYN_MBUS_MAX_FRAME_LEN];
```



Internal frame receive buffer 


        

<hr>



### variable rx\_idx 

```C++
size_t SYN_MBUS_Decoder::rx_idx;
```



Buffer write index 


        

<hr>



### variable state 

```C++
uint8_t SYN_MBUS_Decoder::state;
```



Decoder state machine state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mbus.h`

