

# Struct SYN\_EcatHeader



[**ClassList**](annotated.md) **>** [**SYN\_EcatHeader**](structSYN__EcatHeader.md)



_Raw EtherCAT Header (2 bytes)_ 

* `#include <syn_ethercat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**length**](#variable-length)  <br> |
|  uint16\_t | [**reserved**](#variable-reserved)  <br> |
|  uint16\_t | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable length 

```C++
uint16_t SYN_EcatHeader::length;
```



Length of datagrams in bytes 


        

<hr>



### variable reserved 

```C++
uint16_t SYN_EcatHeader::reserved;
```



Reserved bit 


        

<hr>



### variable type 

```C++
uint16_t SYN_EcatHeader::type;
```



Type (1 for EtherCAT datagrams) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

