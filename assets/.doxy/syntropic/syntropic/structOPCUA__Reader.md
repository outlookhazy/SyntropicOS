

# Struct OPCUA\_Reader



[**ClassList**](annotated.md) **>** [**OPCUA\_Reader**](structOPCUA__Reader.md)



_Binary deserializer context for OPC UA messages._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t \* | [**data**](#variable-data)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**error**](#variable-error)  <br> |
|  size\_t | [**pos**](#variable-pos)  <br> |
|  size\_t | [**size**](#variable-size)  <br> |












































## Public Attributes Documentation




### variable data 

```C++
const uint8_t* OPCUA_Reader::data;
```



Source buffer pointer 


        

<hr>



### variable error 

```C++
bool OPCUA_Reader::error;
```



Error flag on underflow 


        

<hr>



### variable pos 

```C++
size_t OPCUA_Reader::pos;
```



Current read position 


        

<hr>



### variable size 

```C++
size_t OPCUA_Reader::size;
```



Total buffer size 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.c`

