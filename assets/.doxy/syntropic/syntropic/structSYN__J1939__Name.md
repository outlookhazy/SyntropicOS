

# Struct SYN\_J1939\_Name



[**ClassList**](annotated.md) **>** [**SYN\_J1939\_Name**](structSYN__J1939__Name.md)



_64-bit J1939 Device NAME Structure (J1939-81)._ 

* `#include <syn_j1939.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**arbitrary\_addr\_cap**](#variable-arbitrary_addr_cap)  <br> |
|  uint8\_t | [**ecu\_instance**](#variable-ecu_instance)  <br> |
|  uint8\_t | [**function**](#variable-function)  <br> |
|  uint8\_t | [**function\_instance**](#variable-function_instance)  <br> |
|  uint32\_t | [**identity\_number**](#variable-identity_number)  <br> |
|  uint8\_t | [**industry\_group**](#variable-industry_group)  <br> |
|  uint16\_t | [**manufacturer\_code**](#variable-manufacturer_code)  <br> |
|  uint8\_t | [**vehicle\_system**](#variable-vehicle_system)  <br> |
|  uint8\_t | [**vehicle\_system\_inst**](#variable-vehicle_system_inst)  <br> |












































## Public Attributes Documentation




### variable arbitrary\_addr\_cap 

```C++
bool SYN_J1939_Name::arbitrary_addr_cap;
```



1-bit Arbitrary Address Capable flag 


        

<hr>



### variable ecu\_instance 

```C++
uint8_t SYN_J1939_Name::ecu_instance;
```



3-bit ECU Instance 


        

<hr>



### variable function 

```C++
uint8_t SYN_J1939_Name::function;
```



8-bit Defined Function 


        

<hr>



### variable function\_instance 

```C++
uint8_t SYN_J1939_Name::function_instance;
```



5-bit Function Instance 


        

<hr>



### variable identity\_number 

```C++
uint32_t SYN_J1939_Name::identity_number;
```



21-bit Unique Serial / Identity Number 


        

<hr>



### variable industry\_group 

```C++
uint8_t SYN_J1939_Name::industry_group;
```



3-bit Industry Group (e.g. 0=Global, 1=On-Highway, 2=Agricultural) 


        

<hr>



### variable manufacturer\_code 

```C++
uint16_t SYN_J1939_Name::manufacturer_code;
```



11-bit SAE Manufacturer Code 


        

<hr>



### variable vehicle\_system 

```C++
uint8_t SYN_J1939_Name::vehicle_system;
```



7-bit Vehicle System 


        

<hr>



### variable vehicle\_system\_inst 

```C++
uint8_t SYN_J1939_Name::vehicle_system_inst;
```



4-bit Vehicle System Instance 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_j1939.h`

