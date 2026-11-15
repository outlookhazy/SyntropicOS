

# Struct SYN\_SMBUS\_Packet



[**ClassList**](annotated.md) **>** [**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md)



_SMBus transaction packet structure._ 

* `#include <syn_smbus.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**command**](#variable-command)  <br> |
|  uint8\_t | [**data**](#variable-data)  <br> |
|  uint8\_t | [**length**](#variable-length)  <br> |
|  uint8\_t | [**pec**](#variable-pec)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**pec\_enabled**](#variable-pec_enabled)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**pec\_valid**](#variable-pec_valid)  <br> |
|  [**SYN\_SMBUS\_Protocol**](syn__smbus_8h.md#enum-syn_smbus_protocol) | [**proto**](#variable-proto)  <br> |
|  uint8\_t | [**slave\_addr**](#variable-slave_addr)  <br> |












































## Public Attributes Documentation




### variable command 

```C++
uint8_t SYN_SMBUS_Packet::command;
```



Command code byte 


        

<hr>



### variable data 

```C++
uint8_t SYN_SMBUS_Packet::data[SYN_SMBUS_BUF_MAX_LEN];
```



Payload data 


        

<hr>



### variable length 

```C++
uint8_t SYN_SMBUS_Packet::length;
```



Payload length in data buffer 


        

<hr>



### variable pec 

```C++
uint8_t SYN_SMBUS_Packet::pec;
```



Calculated or received PEC byte 


        

<hr>



### variable pec\_enabled 

```C++
bool SYN_SMBUS_Packet::pec_enabled;
```



True if Packet Error Checking is active 


        

<hr>



### variable pec\_valid 

```C++
bool SYN_SMBUS_Packet::pec_valid;
```



True if received PEC matched calculated PEC 


        

<hr>



### variable proto 

```C++
SYN_SMBUS_Protocol SYN_SMBUS_Packet::proto;
```



Protocol type 


        

<hr>



### variable slave\_addr 

```C++
uint8_t SYN_SMBUS_Packet::slave_addr;
```



7-bit slave address 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_smbus.h`

