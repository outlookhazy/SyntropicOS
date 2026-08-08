

# Struct SYN\_ICMP



[**ClassList**](annotated.md) **>** [**SYN\_ICMP**](structSYN__ICMP.md)



[More...](#detailed-description)

* `#include <syn_icmp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**echo\_replies\_rx**](#variable-echo_replies_rx)  <br> |
|  uint32\_t | [**echo\_replies\_tx**](#variable-echo_replies_tx)  <br> |
|  uint32\_t | [**echo\_requests\_rx**](#variable-echo_requests_rx)  <br> |
|  uint32\_t | [**echo\_requests\_tx**](#variable-echo_requests_tx)  <br> |
|  uint32\_t | [**errors**](#variable-errors)  <br> |












































## Detailed Description


ICMP Engine Context Descriptor. 


    
## Public Attributes Documentation




### variable echo\_replies\_rx 

```C++
uint32_t SYN_ICMP::echo_replies_rx;
```



Outbound echo replies received counter 


        

<hr>



### variable echo\_replies\_tx 

```C++
uint32_t SYN_ICMP::echo_replies_tx;
```



Echo replies transmitted counter 


        

<hr>



### variable echo\_requests\_rx 

```C++
uint32_t SYN_ICMP::echo_requests_rx;
```



Echo requests received counter 


        

<hr>



### variable echo\_requests\_tx 

```C++
uint32_t SYN_ICMP::echo_requests_tx;
```



Outbound echo requests transmitted counter 


        

<hr>



### variable errors 

```C++
uint32_t SYN_ICMP::errors;
```



Checksum/format error counter 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_icmp.h`

