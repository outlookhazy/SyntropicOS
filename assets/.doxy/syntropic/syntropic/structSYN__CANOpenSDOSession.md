

# Struct SYN\_CANOpenSDOSession



[**ClassList**](annotated.md) **>** [**SYN\_CANOpenSDOSession**](structSYN__CANOpenSDOSession.md)



_Active SDO transfer session tracking._ 

* `#include <syn_canopen.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**index**](#variable-index)  <br> |
|  [**SYN\_CANOpenSDOState**](syn__canopen_8h.md#enum-syn_canopensdostate) | [**state**](#variable-state)  <br> |
|  [**uint8\_t**](structSYN__CANOpenSDOSession.md#variable-index) | [**subindex**](#variable-subindex)  <br> |
|  [**uint8\_t**](structSYN__CANOpenSDOSession.md#variable-index) | [**toggle**](#variable-toggle)  <br> |
|  [**size\_t**](structSYN__CANOpenSDOSession.md#variable-index) | [**total\_bytes**](#variable-total_bytes)  <br> |
|  [**size\_t**](structSYN__CANOpenSDOSession.md#variable-index) | [**transferred\_bytes**](#variable-transferred_bytes)  <br> |












































## Public Attributes Documentation




### variable index 

```C++
uint16_t SYN_CANOpenSDOSession::index;
```



Object dictionary index. 


        

<hr>



### variable state 

```C++
SYN_CANOpenSDOState SYN_CANOpenSDOSession::state;
```



Session state. 


        

<hr>



### variable subindex 

```C++
uint8_t SYN_CANOpenSDOSession::subindex;
```



Object dictionary subindex. 


        

<hr>



### variable toggle 

```C++
uint8_t SYN_CANOpenSDOSession::toggle;
```



Segment toggle bit. 


        

<hr>



### variable total\_bytes 

```C++
size_t SYN_CANOpenSDOSession::total_bytes;
```



Total transfer byte length. 


        

<hr>



### variable transferred\_bytes 

```C++
size_t SYN_CANOpenSDOSession::transferred_bytes;
```



Bytes transferred so far. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen.h`

