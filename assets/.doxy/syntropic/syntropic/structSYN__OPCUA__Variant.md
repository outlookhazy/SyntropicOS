

# Struct SYN\_OPCUA\_Variant



[**ClassList**](annotated.md) **>** [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md)



_OPC UA Variant containing dynamic value._ 

* `#include <syn_opcua.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**boolean**](#variable-boolean)  <br> |
|  uint8\_t | [**byte**](#variable-byte)  <br> |
|  double | [**double\_val**](#variable-double_val)  <br> |
|  float | [**float\_val**](#variable-float_val)  <br> |
|  int16\_t | [**int16**](#variable-int16)  <br> |
|  int32\_t | [**int32**](#variable-int32)  <br> |
|  int64\_t | [**int64**](#variable-int64)  <br> |
|  int8\_t | [**sbyte**](#variable-sbyte)  <br> |
|  const char \* | [**string**](#variable-string)  <br> |
|  [**SYN\_OPCUA\_DataType**](syn__opcua_8h.md#enum-syn_opcua_datatype) | [**type**](#variable-type)  <br> |
|  uint16\_t | [**uint16**](#variable-uint16)  <br> |
|  uint32\_t | [**uint32**](#variable-uint32)  <br> |
|  uint64\_t | [**uint64**](#variable-uint64)  <br> |
|  union [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) | [**val**](#variable-val)  <br> |












































## Public Attributes Documentation




### variable boolean 

```C++
bool SYN_OPCUA_Variant::boolean;
```



Boolean value 


        

<hr>



### variable byte 

```C++
uint8_t SYN_OPCUA_Variant::byte;
```



Unsigned 8-bit integer 


        

<hr>



### variable double\_val 

```C++
double SYN_OPCUA_Variant::double_val;
```



64-bit IEEE-754 double precision float 


        

<hr>



### variable float\_val 

```C++
float SYN_OPCUA_Variant::float_val;
```



32-bit IEEE-754 single precision float 


        

<hr>



### variable int16 

```C++
int16_t SYN_OPCUA_Variant::int16;
```



Signed 16-bit integer 


        

<hr>



### variable int32 

```C++
int32_t SYN_OPCUA_Variant::int32;
```



Signed 32-bit integer 


        

<hr>



### variable int64 

```C++
int64_t SYN_OPCUA_Variant::int64;
```



Signed 64-bit integer 


        

<hr>



### variable sbyte 

```C++
int8_t SYN_OPCUA_Variant::sbyte;
```



Signed 8-bit integer 


        

<hr>



### variable string 

```C++
const char* SYN_OPCUA_Variant::string;
```



Null-terminated string 


        

<hr>



### variable type 

```C++
SYN_OPCUA_DataType SYN_OPCUA_Variant::type;
```



Value data type 


        

<hr>



### variable uint16 

```C++
uint16_t SYN_OPCUA_Variant::uint16;
```



Unsigned 16-bit integer 


        

<hr>



### variable uint32 

```C++
uint32_t SYN_OPCUA_Variant::uint32;
```



Unsigned 32-bit integer 


        

<hr>



### variable uint64 

```C++
uint64_t SYN_OPCUA_Variant::uint64;
```



Unsigned 64-bit integer 


        

<hr>



### variable val 

```C++
union SYN_OPCUA_Variant SYN_OPCUA_Variant::val;
```



Value payload union 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.h`

