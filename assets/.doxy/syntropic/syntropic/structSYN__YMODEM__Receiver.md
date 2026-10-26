

# Struct SYN\_YMODEM\_Receiver



[**ClassList**](annotated.md) **>** [**SYN\_YMODEM\_Receiver**](structSYN__YMODEM__Receiver.md)



_YMODEM Receiver Context._ 

* `#include <syn_ymodem.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**bytes\_received**](#variable-bytes_received)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**SYN\_YMODEM\_EventCallback**](syn__ymodem_8h.md#typedef-syn_ymodem_eventcallback) | [**event\_fn**](#variable-event_fn)  <br> |
|  uint8\_t | [**expected\_seq**](#variable-expected_seq)  <br> |
|  char | [**filename**](#variable-filename)  <br> |
|  uint32\_t | [**filesize**](#variable-filesize)  <br> |
|  [**SYN\_YMODEM\_GetChar**](syn__ymodem_8h.md#typedef-syn_ymodem_getchar) | [**getchar\_fn**](#variable-getchar_fn)  <br> |
|  uint8\_t | [**pkt\_buf**](#variable-pkt_buf)  <br> |
|  [**SYN\_YMODEM\_PutChar**](syn__ymodem_8h.md#typedef-syn_ymodem_putchar) | [**putchar\_fn**](#variable-putchar_fn)  <br> |












































## Public Attributes Documentation




### variable bytes\_received 

```C++
uint32_t SYN_YMODEM_Receiver::bytes_received;
```



Accumulated payload bytes received 


        

<hr>



### variable ctx 

```C++
void* SYN_YMODEM_Receiver::ctx;
```



User context pointer 


        

<hr>



### variable event\_fn 

```C++
SYN_YMODEM_EventCallback SYN_YMODEM_Receiver::event_fn;
```



Event callback 


        

<hr>



### variable expected\_seq 

```C++
uint8_t SYN_YMODEM_Receiver::expected_seq;
```



Next expected block sequence number 


        

<hr>



### variable filename 

```C++
char SYN_YMODEM_Receiver::filename[SYN_YMODEM_MAX_FILENAME];
```



Parsed filename from Block 0 


        

<hr>



### variable filesize 

```C++
uint32_t SYN_YMODEM_Receiver::filesize;
```



Parsed file size from Block 0 


        

<hr>



### variable getchar\_fn 

```C++
SYN_YMODEM_GetChar SYN_YMODEM_Receiver::getchar_fn;
```



Serial RX function 


        

<hr>



### variable pkt\_buf 

```C++
uint8_t SYN_YMODEM_Receiver::pkt_buf[SYN_YMODEM_MAX_BLOCK_SIZE+5U];
```



Packet RX buffer 


        

<hr>



### variable putchar\_fn 

```C++
SYN_YMODEM_PutChar SYN_YMODEM_Receiver::putchar_fn;
```



Serial TX function 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ymodem.h`

