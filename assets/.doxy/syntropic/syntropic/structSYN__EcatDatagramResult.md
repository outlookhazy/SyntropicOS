

# Struct SYN\_EcatDatagramResult



[**ClassList**](annotated.md) **>** [**SYN\_EcatDatagramResult**](structSYN__EcatDatagramResult.md)



_Parsed datagram item from frame iteration._ 

* `#include <syn_ethercat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**addr**](#variable-addr)  <br> |
|  uint8\_t | [**cmd**](#variable-cmd)  <br> |
|  const uint8\_t \* | [**data**](#variable-data)  <br> |
|  uint16\_t | [**data\_len**](#variable-data_len)  <br> |
|  uint8\_t | [**idx**](#variable-idx)  <br> |
|  uint16\_t | [**wkc**](#variable-wkc)  <br> |












































## Public Attributes Documentation




### variable addr 

```C++
uint32_t SYN_EcatDatagramResult::addr;
```



Datagram address field 


        

<hr>



### variable cmd 

```C++
uint8_t SYN_EcatDatagramResult::cmd;
```



Datagram command code (SYN\_EcatCmd) 


        

<hr>



### variable data 

```C++
const uint8_t* SYN_EcatDatagramResult::data;
```



Pointer to datagram payload bytes 


        

<hr>



### variable data\_len 

```C++
uint16_t SYN_EcatDatagramResult::data_len;
```



Datagram payload length 


        

<hr>



### variable idx 

```C++
uint8_t SYN_EcatDatagramResult::idx;
```



Datagram index 


        

<hr>



### variable wkc 

```C++
uint16_t SYN_EcatDatagramResult::wkc;
```



Working Counter 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

