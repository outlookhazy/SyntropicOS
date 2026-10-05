

# Struct SYN\_J1939\_Node



[**ClassList**](annotated.md) **>** [**SYN\_J1939\_Node**](structSYN__J1939__Node.md)



_J1939 Node Control Instance._ 

* `#include <syn_j1939.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_J1939\_Name**](structSYN__J1939__Name.md) | [**name**](#variable-name)  <br> |
|  uint8\_t | [**sa**](#variable-sa)  <br> |
|  [**SYN\_J1939\_AddressState**](syn__j1939_8h.md#enum-syn_j1939_addressstate) | [**state**](#variable-state)  <br> |
|  [**SYN\_J1939\_TpRxSession**](structSYN__J1939__TpRxSession.md) | [**tp\_rx**](#variable-tp_rx)  <br> |












































## Public Attributes Documentation




### variable name 

```C++
SYN_J1939_Name SYN_J1939_Node::name;
```



64-bit Device NAME 


        

<hr>



### variable sa 

```C++
uint8_t SYN_J1939_Node::sa;
```



Assigned Source Address 


        

<hr>



### variable state 

```C++
SYN_J1939_AddressState SYN_J1939_Node::state;
```



Current address claim state 


        

<hr>



### variable tp\_rx 

```C++
SYN_J1939_TpRxSession SYN_J1939_Node::tp_rx;
```



Multi-packet Transport Protocol RX session 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_j1939.h`

