

# Struct SYN\_CJT188\_MeterData



[**ClassList**](annotated.md) **>** [**SYN\_CJT188\_MeterData**](structSYN__CJT188__MeterData.md)



_Parsed CJ/T 188 Meter Response Data._ 

* `#include <syn_cjt188.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**current\_flow\_bcd**](#variable-current_flow_bcd)  <br> |
|  uint32\_t | [**month\_flow\_bcd**](#variable-month_flow_bcd)  <br> |
|  uint16\_t | [**status**](#variable-status)  <br> |
|  uint8\_t | [**unit**](#variable-unit)  <br> |












































## Public Attributes Documentation




### variable current\_flow\_bcd 

```C++
uint32_t SYN_CJT188_MeterData::current_flow_bcd;
```



Current cumulative flow (8-digit BCD) 


        

<hr>



### variable month\_flow\_bcd 

```C++
uint32_t SYN_CJT188_MeterData::month_flow_bcd;
```



Month usage flow (8-digit BCD) 
 


        

<hr>



### variable status 

```C++
uint16_t SYN_CJT188_MeterData::status;
```



2-byte meter status flags 
 


        

<hr>



### variable unit 

```C++
uint8_t SYN_CJT188_MeterData::unit;
```



Unit code (e.g., 0x2C for m³) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cjt188.h`

