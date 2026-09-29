

# Struct SYN\_UDS\_DTCEntry



[**ClassList**](annotated.md) **>** [**SYN\_UDS\_DTCEntry**](structSYN__UDS__DTCEntry.md)



_Diagnostic Trouble Code (DTC) Registry Entry._ 

* `#include <syn_uds.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**dtc**](#variable-dtc)  <br> |
|  int8\_t | [**fault\_cnt**](#variable-fault_cnt)  <br> |
|  uint8\_t | [**severity**](#variable-severity)  <br> |
|  uint8\_t | [**status**](#variable-status)  <br> |












































## Public Attributes Documentation




### variable dtc 

```C++
uint32_t SYN_UDS_DTCEntry::dtc;
```



24-bit Diagnostic Trouble Code (e.g., 0x012345). 


        

<hr>



### variable fault\_cnt 

```C++
int8_t SYN_UDS_DTCEntry::fault_cnt;
```



Fault detection counter (-128 to 127). 
 


        

<hr>



### variable severity 

```C++
uint8_t SYN_UDS_DTCEntry::severity;
```



DTCSeverityByte (High 3 bits severity, low 5 bits class). 


        

<hr>



### variable status 

```C++
uint8_t SYN_UDS_DTCEntry::status;
```



DTCStatusByte bitmask. 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_uds.h`

