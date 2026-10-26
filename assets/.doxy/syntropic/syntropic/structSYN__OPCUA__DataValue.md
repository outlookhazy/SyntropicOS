

# Struct SYN\_OPCUA\_DataValue



[**ClassList**](annotated.md) **>** [**SYN\_OPCUA\_DataValue**](structSYN__OPCUA__DataValue.md)



_DataValue with value, status code, and source timestamp._ 

* `#include <syn_opcua.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint64\_t | [**source\_ts\_ms**](#variable-source_ts_ms)  <br> |
|  uint32\_t | [**status\_code**](#variable-status_code)  <br> |
|  [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) | [**value**](#variable-value)  <br> |












































## Public Attributes Documentation




### variable source\_ts\_ms 

```C++
uint64_t SYN_OPCUA_DataValue::source_ts_ms;
```



Source timestamp in milliseconds 


        

<hr>



### variable status\_code 

```C++
uint32_t SYN_OPCUA_DataValue::status_code;
```



Quality status code 


        

<hr>



### variable value 

```C++
SYN_OPCUA_Variant SYN_OPCUA_DataValue::value;
```



Actual variant value 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.h`

