

# Struct SYN\_LwM2M\_Value



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md)



_LwM2M Generic Value Container._ 

* `#include <syn_lwm2m.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**boolean**](#variable-boolean)  <br> |
|  const uint8\_t \* | [**data**](#variable-data)  <br> |
|  double | [**floating**](#variable-floating)  <br> |
|  uint16\_t | [**inst\_id**](#variable-inst_id)  <br> |
|  int64\_t | [**integer**](#variable-integer)  <br> |
|  size\_t | [**len**](#variable-len)  <br> |
|  uint16\_t | [**obj\_id**](#variable-obj_id)  <br> |
|  struct [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md) | [**objlnk**](#variable-objlnk)  <br> |
|  struct [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md) | [**opaque**](#variable-opaque)  <br> |
|  const char \* | [**str**](#variable-str)  <br> |
|  [**SYN\_LwM2M\_ValType**](syn__lwm2m_8h.md#enum-syn_lwm2m_valtype) | [**type**](#variable-type)  <br> |
|  union [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md) | [**val**](#variable-val)  <br> |












































## Public Attributes Documentation




### variable boolean 

```C++
bool SYN_LwM2M_Value::boolean;
```



Boolean value 


        

<hr>



### variable data 

```C++
const uint8_t* SYN_LwM2M_Value::data;
```



Pointer to binary data 


        

<hr>



### variable floating 

```C++
double SYN_LwM2M_Value::floating;
```



Float value 


        

<hr>



### variable inst\_id 

```C++
uint16_t SYN_LwM2M_Value::inst_id;
```



Target Instance ID 


        

<hr>



### variable integer 

```C++
int64_t SYN_LwM2M_Value::integer;
```



Integer value 


        

<hr>



### variable len 

```C++
size_t SYN_LwM2M_Value::len;
```



Data length in bytes 


        

<hr>



### variable obj\_id 

```C++
uint16_t SYN_LwM2M_Value::obj_id;
```



Target Object ID 


        

<hr>



### variable objlnk 

```C++
struct SYN_LwM2M_Value SYN_LwM2M_Value::objlnk;
```



Object link value 


        

<hr>



### variable opaque 

```C++
struct SYN_LwM2M_Value SYN_LwM2M_Value::opaque;
```



Opaque binary data 


        

<hr>



### variable str 

```C++
const char* SYN_LwM2M_Value::str;
```



Pointer to string 


        

<hr>



### variable type 

```C++
SYN_LwM2M_ValType SYN_LwM2M_Value::type;
```



Value type tag 


        

<hr>



### variable val 

```C++
union SYN_LwM2M_Value SYN_LwM2M_Value::val;
```



Value storage union 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.h`

