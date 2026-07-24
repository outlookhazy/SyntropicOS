

# Struct SYN\_CANOpenODEntry



[**ClassList**](annotated.md) **>** [**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md)



_Object Dictionary Entry Definition._ 

* `#include <syn_canopen.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**uint8\_t**](structSYN__CANOpenODEntry.md#variable-index) | [**access**](#variable-access)  <br> |
|  [**void**](structSYN__CANOpenODEntry.md#variable-index) \* | [**data\_ptr**](#variable-data_ptr)  <br> |
|  uint16\_t | [**index**](#variable-index)  <br> |
|  [**size\_t**](structSYN__CANOpenODEntry.md#variable-index) | [**size**](#variable-size)  <br> |
|  [**uint8\_t**](structSYN__CANOpenODEntry.md#variable-index) | [**subindex**](#variable-subindex)  <br> |
|  [**uint8\_t**](structSYN__CANOpenODEntry.md#variable-index) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable access 

```C++
uint8_t SYN_CANOpenODEntry::access;
```



Access Rights (SYN\_CANOPEN\_ACCESS\_\*) 


        

<hr>



### variable data\_ptr 

```C++
void* SYN_CANOpenODEntry::data_ptr;
```



Pointer to live variable 


        

<hr>



### variable index 

```C++
uint16_t SYN_CANOpenODEntry::index;
```



16-bit OD Index (0x1000..0x9FFF) 


        

<hr>



### variable size 

```C++
size_t SYN_CANOpenODEntry::size;
```



Variable size in bytes 


        

<hr>



### variable subindex 

```C++
uint8_t SYN_CANOpenODEntry::subindex;
```



8-bit Subindex (0x00..0xFF) 


        

<hr>



### variable type 

```C++
uint8_t SYN_CANOpenODEntry::type;
```



Data Type (SYN\_CANOPEN\_TYPE\_\*) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen.h`

