

# Struct SYN\_LIN\_Frame



[**ClassList**](annotated.md) **>** [**SYN\_LIN\_Frame**](structSYN__LIN__Frame.md)



_LIN Frame representation._ 

* `#include <syn_lin.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**checksum**](#variable-checksum)  <br> |
|  uint8\_t | [**data**](#variable-data)  <br> |
|  uint8\_t | [**id**](#variable-id)  <br> |
|  uint8\_t | [**len**](#variable-len)  <br> |
|  uint8\_t | [**pid**](#variable-pid)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid\_checksum**](#variable-valid_checksum)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**valid\_pid**](#variable-valid_pid)  <br> |












































## Public Attributes Documentation




### variable checksum 

```C++
uint8_t SYN_LIN_Frame::checksum;
```



Received or computed checksum byte 


        

<hr>



### variable data 

```C++
uint8_t SYN_LIN_Frame::data[SYN_LIN_DATA_MAX];
```



Payload data bytes 


        

<hr>



### variable id 

```C++
uint8_t SYN_LIN_Frame::id;
```



6-bit Frame ID (0..63) 


        

<hr>



### variable len 

```C++
uint8_t SYN_LIN_Frame::len;
```



Data payload length (1..8 bytes) 


        

<hr>



### variable pid 

```C++
uint8_t SYN_LIN_Frame::pid;
```



Protected Identifier byte (ID + Parity P0/P1) 


        

<hr>



### variable valid\_checksum 

```C++
bool SYN_LIN_Frame::valid_checksum;
```



True if checksum matches frame payload 


        

<hr>



### variable valid\_pid 

```C++
bool SYN_LIN_Frame::valid_pid;
```



True if PID parity bits are valid 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lin.h`

