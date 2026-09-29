

# Struct SYN\_ParamSlotHeader



[**ClassList**](annotated.md) **>** [**SYN\_ParamSlotHeader**](structSYN__ParamSlotHeader.md)



_Slot header — stored at the beginning of each parameter slot._ 

* `#include <syn_param.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**\_pad**](#variable-_pad)  <br> |
|  uint16\_t | [**crc**](#variable-crc)  <br> |
|  uint16\_t | [**data\_size**](#variable-data_size)  <br> |
|  uint16\_t | [**magic**](#variable-magic)  <br> |
|  uint16\_t | [**seq**](#variable-seq)  <br> |












































## Public Attributes Documentation




### variable \_pad 

```C++
uint8_t SYN_ParamSlotHeader::_pad[8];
```



Padding to 16 bytes 
 


        

<hr>



### variable crc 

```C++
uint16_t SYN_ParamSlotHeader::crc;
```



CRC-16 over the data 
 


        

<hr>



### variable data\_size 

```C++
uint16_t SYN_ParamSlotHeader::data_size;
```



Size of the parameter data 
 


        

<hr>



### variable magic 

```C++
uint16_t SYN_ParamSlotHeader::magic;
```



Magic number (SYN\_PARAM\_MAGIC) 
 


        

<hr>



### variable seq 

```C++
uint16_t SYN_ParamSlotHeader::seq;
```



Sequence number (higher = newer) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_param.h`

