

# Struct SYN\_J1939\_Header



[**ClassList**](annotated.md) **>** [**SYN\_J1939\_Header**](structSYN__J1939__Header.md)



_Parsed J1939 29-bit CAN ID Header._ 

* `#include <syn_j1939.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**da**](#variable-da)  <br> |
|  uint8\_t | [**dp**](#variable-dp)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_pdu1**](#variable-is_pdu1)  <br> |
|  uint8\_t | [**pf**](#variable-pf)  <br> |
|  uint32\_t | [**pgn**](#variable-pgn)  <br> |
|  uint8\_t | [**priority**](#variable-priority)  <br> |
|  uint8\_t | [**ps**](#variable-ps)  <br> |
|  uint8\_t | [**sa**](#variable-sa)  <br> |












































## Public Attributes Documentation




### variable da 

```C++
uint8_t SYN_J1939_Header::da;
```



Destination Address (DA if PDU1, 0xFF if PDU2) 


        

<hr>



### variable dp 

```C++
uint8_t SYN_J1939_Header::dp;
```



Data Page bit (0 or 1) 


        

<hr>



### variable is\_pdu1 

```C++
bool SYN_J1939_Header::is_pdu1;
```



true if PF &lt; 240 (destination-specific) 


        

<hr>



### variable pf 

```C++
uint8_t SYN_J1939_Header::pf;
```



PDU Format (0-255) 


        

<hr>



### variable pgn 

```C++
uint32_t SYN_J1939_Header::pgn;
```



Computed 18-bit Parameter Group Number 


        

<hr>



### variable priority 

```C++
uint8_t SYN_J1939_Header::priority;
```



Priority bits (0-7, default 6 or 3) 


        

<hr>



### variable ps 

```C++
uint8_t SYN_J1939_Header::ps;
```



PDU Specific (DA for PDU1 &lt; 240, GE for PDU2 &gt;= 240) 


        

<hr>



### variable sa 

```C++
uint8_t SYN_J1939_Header::sa;
```



Source Address (0-255) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_j1939.h`

