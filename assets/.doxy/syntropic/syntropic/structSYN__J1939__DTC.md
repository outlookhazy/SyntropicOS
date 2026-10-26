

# Struct SYN\_J1939\_DTC



[**ClassList**](annotated.md) **>** [**SYN\_J1939\_DTC**](structSYN__J1939__DTC.md)



_J1939 Diagnostic Trouble Code (DTC) Structure (J1939-73)._ 

* `#include <syn_j1939.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**conversion\_method**](#variable-conversion_method)  <br> |
|  uint8\_t | [**fmi**](#variable-fmi)  <br> |
|  uint8\_t | [**occurrence\_count**](#variable-occurrence_count)  <br> |
|  uint32\_t | [**spn**](#variable-spn)  <br> |












































## Public Attributes Documentation




### variable conversion\_method 

```C++
uint8_t SYN_J1939_DTC::conversion_method;
```



1-bit SPN Conversion Method 


        

<hr>



### variable fmi 

```C++
uint8_t SYN_J1939_DTC::fmi;
```



5-bit Failure Mode Identifier 


        

<hr>



### variable occurrence\_count 

```C++
uint8_t SYN_J1939_DTC::occurrence_count;
```



7-bit Occurrence Count 


        

<hr>



### variable spn 

```C++
uint32_t SYN_J1939_DTC::spn;
```



19-bit Suspect Parameter Number 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_j1939.h`

