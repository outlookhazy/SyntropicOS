

# Struct SYN\_COBS\_Decoder



[**ClassList**](annotated.md) **>** [**SYN\_COBS\_Decoder**](structSYN__COBS__Decoder.md)



_Streaming COBS frame decoder._ 

* `#include <syn_cobs.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  size\_t | [**buf\_size**](#variable-buf_size)  <br> |
|  [**SYN\_COBS\_PacketCallback**](syn__cobs_8h.md#typedef-syn_cobs_packetcallback) | [**callback**](#variable-callback)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  size\_t | [**idx**](#variable-idx)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
uint8_t* SYN_COBS_Decoder::buf;
```



Receive buffer 
 


        

<hr>



### variable buf\_size 

```C++
size_t SYN_COBS_Decoder::buf_size;
```



Buffer capacity 
 


        

<hr>



### variable callback 

```C++
SYN_COBS_PacketCallback SYN_COBS_Decoder::callback;
```



Callback on complete packet 
 


        

<hr>



### variable ctx 

```C++
void* SYN_COBS_Decoder::ctx;
```



User context for callback 
 


        

<hr>



### variable idx 

```C++
size_t SYN_COBS_Decoder::idx;
```



Current write position 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cobs.h`

