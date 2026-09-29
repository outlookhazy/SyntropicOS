

# Struct OPCUA\_Writer



[**ClassList**](annotated.md) **>** [**OPCUA\_Writer**](structOPCUA__Writer.md)



_Binary serializer context for OPC UA messages._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**data**](#variable-data)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**error**](#variable-error)  <br> |
|  size\_t | [**pos**](#variable-pos)  <br> |
|  size\_t | [**size**](#variable-size)  <br> |












































## Public Attributes Documentation




### variable data 

```C++
uint8_t* OPCUA_Writer::data;
```



Destination buffer pointer 


        

<hr>



### variable error 

```C++
bool OPCUA_Writer::error;
```



Error flag on overflow 


        

<hr>



### variable pos 

```C++
size_t OPCUA_Writer::pos;
```



Current write position 


        

<hr>



### variable size 

```C++
size_t OPCUA_Writer::size;
```



Total buffer capacity 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.c`

