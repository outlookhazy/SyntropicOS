

# Struct SYN\_DLT645\_Decoder



[**ClassList**](annotated.md) **>** [**SYN\_DLT645\_Decoder**](structSYN__DLT645__Decoder.md)



_Streaming byte-at-a-time decoder state._ 

* `#include <syn_dlt645.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_DLT645\_FrameCallback**](syn__dlt645_8h.md#typedef-syn_dlt645_framecallback) | [**cb**](#variable-cb)  <br> |
|  void \* | [**cb\_ctx**](#variable-cb_ctx)  <br> |
|  uint8\_t | [**rx\_buf**](#variable-rx_buf)  <br> |
|  size\_t | [**rx\_len**](#variable-rx_len)  <br> |
|  [**SYN\_DLT645\_Ver**](syn__dlt645_8h.md#enum-syn_dlt645_ver) | [**version**](#variable-version)  <br> |












































## Public Attributes Documentation




### variable cb 

```C++
SYN_DLT645_FrameCallback SYN_DLT645_Decoder::cb;
```



Completion callback 
 


        

<hr>



### variable cb\_ctx 

```C++
void* SYN_DLT645_Decoder::cb_ctx;
```



Callback context 
 


        

<hr>



### variable rx\_buf 

```C++
uint8_t SYN_DLT645_Decoder::rx_buf[128];
```



Internal reception buffer 
 


        

<hr>



### variable rx\_len 

```C++
size_t SYN_DLT645_Decoder::rx_len;
```



Received byte count 
 


        

<hr>



### variable version 

```C++
SYN_DLT645_Ver SYN_DLT645_Decoder::version;
```



Expected version 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dlt645.h`

