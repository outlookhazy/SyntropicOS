

# Struct SYN\_CANOpenPDOMap



[**ClassList**](annotated.md) **>** [**SYN\_CANOpenPDOMap**](structSYN__CANOpenPDOMap.md)



_PDO Mapping Configuration._ 

* `#include <syn_canopen.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**cob\_id**](#variable-cob_id)  <br> |
|  uint8\_t | [**enabled**](#variable-enabled)  <br> |
|  uint16\_t | [**od\_index**](#variable-od_index)  <br> |
|  uint8\_t | [**od\_subindex**](#variable-od_subindex)  <br> |












































## Public Attributes Documentation




### variable cob\_id 

```C++
uint16_t SYN_CANOpenPDOMap::cob_id;
```



COB-ID for PDO 


        

<hr>



### variable enabled 

```C++
uint8_t SYN_CANOpenPDOMap::enabled;
```



1 = Enabled, 0 = Disabled 


        

<hr>



### variable od\_index 

```C++
uint16_t SYN_CANOpenPDOMap::od_index;
```



Mapped OD Index 


        

<hr>



### variable od\_subindex 

```C++
uint8_t SYN_CANOpenPDOMap::od_subindex;
```



Mapped OD Subindex 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen.h`

