

# Struct SYN\_EcatCoEHeader



[**ClassList**](annotated.md) **>** [**SYN\_EcatCoEHeader**](structSYN__EcatCoEHeader.md)



_CoE (CANoverEtherCAT) Mailbox Header._ 

* `#include <syn_ethercat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**number**](#variable-number)  <br> |
|  uint16\_t | [**res**](#variable-res)  <br> |
|  uint16\_t | [**service\_type**](#variable-service_type)  <br> |












































## Public Attributes Documentation




### variable number 

```C++
uint16_t SYN_EcatCoEHeader::number;
```



CoE detail/sequence number 


        

<hr>



### variable res 

```C++
uint16_t SYN_EcatCoEHeader::res;
```



Reserved bits 


        

<hr>



### variable service\_type 

```C++
uint16_t SYN_EcatCoEHeader::service_type;
```



CoE service type enum 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

