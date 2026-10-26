

# Struct SYN\_AUTOIP



[**ClassList**](annotated.md) **>** [**SYN\_AUTOIP**](structSYN__AUTOIP.md)



[More...](#detailed-description)

* `#include <syn_autoip.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**announce\_count**](#variable-announce_count)  <br> |
|  uint32\_t | [**collisions**](#variable-collisions)  <br> |
|  uint32\_t | [**ip\_addr**](#variable-ip_addr)  <br> |
|  uint8\_t | [**probe\_count**](#variable-probe_count)  <br> |
|  [**SYN\_AUTOIP\_State**](syn__autoip_8h.md#enum-syn_autoip_state) | [**state**](#variable-state)  <br> |












































## Detailed Description


AutoIP Context Descriptor. 


    
## Public Attributes Documentation




### variable announce\_count 

```C++
uint8_t SYN_AUTOIP::announce_count;
```



Number of ARP announcements sent 


        

<hr>



### variable collisions 

```C++
uint32_t SYN_AUTOIP::collisions;
```



Collision counter 


        

<hr>



### variable ip\_addr 

```C++
uint32_t SYN_AUTOIP::ip_addr;
```



Candidate Link-Local IP 


        

<hr>



### variable probe\_count 

```C++
uint8_t SYN_AUTOIP::probe_count;
```



Number of ARP probes sent 


        

<hr>



### variable state 

```C++
SYN_AUTOIP_State SYN_AUTOIP::state;
```



Active state machine state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_autoip.h`

