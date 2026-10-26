

# Struct SYN\_MBAP\_Header



[**ClassList**](annotated.md) **>** [**SYN\_MBAP\_Header**](structSYN__MBAP__Header.md)



_Modbus TCP MBAP Header structure._ 

* `#include <syn_modbus_tcp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**length**](#variable-length)  <br> |
|  uint16\_t | [**protocol\_id**](#variable-protocol_id)  <br> |
|  uint16\_t | [**transaction\_id**](#variable-transaction_id)  <br> |
|  uint8\_t | [**unit\_id**](#variable-unit_id)  <br> |












































## Public Attributes Documentation




### variable length 

```C++
uint16_t SYN_MBAP_Header::length;
```



Number of following bytes (Unit ID + PDU) 


        

<hr>



### variable protocol\_id 

```C++
uint16_t SYN_MBAP_Header::protocol_id;
```



Protocol Identifier (0x0000 = Modbus) 


        

<hr>



### variable transaction\_id 

```C++
uint16_t SYN_MBAP_Header::transaction_id;
```



Transaction Identifier 


        

<hr>



### variable unit\_id 

```C++
uint8_t SYN_MBAP_Header::unit_id;
```



Unit Identifier / Slave address 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus_tcp.h`

