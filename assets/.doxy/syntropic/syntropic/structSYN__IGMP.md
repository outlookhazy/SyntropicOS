

# Struct SYN\_IGMP



[**ClassList**](annotated.md) **>** [**SYN\_IGMP**](structSYN__IGMP.md)



[More...](#detailed-description)

* `#include <syn_igmp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_IGMP\_Group**](structSYN__IGMP__Group.md) | [**groups**](#variable-groups)  <br> |
|  uint32\_t | [**leaves\_sent**](#variable-leaves_sent)  <br> |
|  uint32\_t | [**queries\_received**](#variable-queries_received)  <br> |
|  uint32\_t | [**reports\_sent**](#variable-reports_sent)  <br> |












































## Detailed Description


IGMP Engine Context Descriptor. 


    
## Public Attributes Documentation




### variable groups 

```C++
SYN_IGMP_Group SYN_IGMP::groups[SYN_IGMP_MAX_GROUPS];
```



Multicast groups tracking table 


        

<hr>



### variable leaves\_sent 

```C++
uint32_t SYN_IGMP::leaves_sent;
```



Total IGMP leave messages sent 


        

<hr>



### variable queries\_received 

```C++
uint32_t SYN_IGMP::queries_received;
```



Total IGMP queries received 


        

<hr>



### variable reports\_sent 

```C++
uint32_t SYN_IGMP::reports_sent;
```



Total IGMP reports sent 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_igmp.h`

