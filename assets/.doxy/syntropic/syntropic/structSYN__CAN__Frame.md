

# Struct SYN\_CAN\_Frame



[**ClassList**](annotated.md) **>** [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md)



_CAN bus frame — standard/extended ID, classic CAN or CAN FD._ 

* `#include <syn_can.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**data**](#variable-data)  <br> |
|  uint8\_t | [**dlc**](#variable-dlc)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**extended**](#variable-extended)  <br> |
|  uint32\_t | [**id**](#variable-id)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**rtr**](#variable-rtr)  <br> |












































## Public Attributes Documentation




### variable data 

```C++
uint8_t SYN_CAN_Frame::data[SYN_CAN_MAX_DATA_LEN];
```



Frame data 
 


        

<hr>



### variable dlc 

```C++
uint8_t SYN_CAN_Frame::dlc;
```



Data length in bytes 
 


        

<hr>



### variable extended 

```C++
bool SYN_CAN_Frame::extended;
```



true = 29-bit ID 
 


        

<hr>



### variable id 

```C++
uint32_t SYN_CAN_Frame::id;
```



11-bit or 29-bit identifier 
 


        

<hr>



### variable rtr 

```C++
bool SYN_CAN_Frame::rtr;
```



Remote transmission request 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_can.h`

