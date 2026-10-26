

# Struct SYN\_ModbusMasterQuery



[**ClassList**](annotated.md) **>** [**SYN\_ModbusMasterQuery**](structSYN__ModbusMasterQuery.md)



_Modbus Master query descriptor._ 

* `#include <syn_modbus_master.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_ModbusMasterCallback**](syn__modbus__master_8h.md#typedef-syn_modbusmastercallback) | [**callback**](#variable-callback)  <br> |
|  uint16\_t | [**count**](#variable-count)  <br> |
|  uint8\_t | [**func\_code**](#variable-func_code)  <br> |
|  uint8\_t | [**slave\_addr**](#variable-slave_addr)  <br> |
|  uint16\_t | [**start\_addr**](#variable-start_addr)  <br> |
|  void \* | [**user\_ctx**](#variable-user_ctx)  <br> |
|  uint16\_t | [**write\_value**](#variable-write_value)  <br> |












































## Public Attributes Documentation




### variable callback 

```C++
SYN_ModbusMasterCallback SYN_ModbusMasterQuery::callback;
```



Asynchronous response callback. 


        

<hr>



### variable count 

```C++
uint16_t SYN_ModbusMasterQuery::count;
```



Number of registers/coils. 


        

<hr>



### variable func\_code 

```C++
uint8_t SYN_ModbusMasterQuery::func_code;
```



Modbus function code. 


        

<hr>



### variable slave\_addr 

```C++
uint8_t SYN_ModbusMasterQuery::slave_addr;
```



Target slave device address. 


        

<hr>



### variable start\_addr 

```C++
uint16_t SYN_ModbusMasterQuery::start_addr;
```



Starting register/coil address. 


        

<hr>



### variable user\_ctx 

```C++
void* SYN_ModbusMasterQuery::user_ctx;
```



User context pointer passed to callback. 


        

<hr>



### variable write\_value 

```C++
uint16_t SYN_ModbusMasterQuery::write_value;
```



Value for single-write requests. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus_master.h`

