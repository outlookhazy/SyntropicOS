

# Struct SYN\_EcatDatagram



[**ClassList**](annotated.md) **>** [**SYN\_EcatDatagram**](structSYN__EcatDatagram.md)



_EtherCAT Datagram Header (10 bytes header + 2 bytes WKC = 12 bytes overhead)_ 

* `#include <syn_ethercat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**addr**](#variable-addr)  <br> |
|  uint16\_t | [**circ**](#variable-circ)  <br> |
|  uint8\_t | [**cmd**](#variable-cmd)  <br> |
|  uint8\_t | [**idx**](#variable-idx)  <br> |
|  uint16\_t | [**irq**](#variable-irq)  <br> |
|  uint16\_t | [**len**](#variable-len)  <br> |
|  uint16\_t | [**m**](#variable-m)  <br> |
|  uint16\_t | [**res**](#variable-res)  <br> |
|  uint16\_t | [**wkc**](#variable-wkc)  <br> |












































## Public Attributes Documentation




### variable addr 

```C++
uint32_t SYN_EcatDatagram::addr;
```



Address (Auto-inc, Configured, or Logical) 


        

<hr>



### variable circ 

```C++
uint16_t SYN_EcatDatagram::circ;
```



Circulating frame flag 


        

<hr>



### variable cmd 

```C++
uint8_t SYN_EcatDatagram::cmd;
```



Command code (SYN\_EcatCmd) 


        

<hr>



### variable idx 

```C++
uint8_t SYN_EcatDatagram::idx;
```



Index number 


        

<hr>



### variable irq 

```C++
uint16_t SYN_EcatDatagram::irq;
```



Interrupt request 


        

<hr>



### variable len 

```C++
uint16_t SYN_EcatDatagram::len;
```



Datagram data length 


        

<hr>



### variable m 

```C++
uint16_t SYN_EcatDatagram::m;
```



More datagrams flag (1 = another datagram follows) 


        

<hr>



### variable res 

```C++
uint16_t SYN_EcatDatagram::res;
```



Reserved 


        

<hr>



### variable wkc 

```C++
uint16_t SYN_EcatDatagram::wkc;
```



Working Counter 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

