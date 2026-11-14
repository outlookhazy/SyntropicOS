

# Struct SYN\_SHA1\_Ctx



[**ClassList**](annotated.md) **>** [**SYN\_SHA1\_Ctx**](structSYN__SHA1__Ctx.md)



_SHA-1 hashing algorithm context._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**buffer**](#variable-buffer)  <br> |
|  uint32\_t | [**count**](#variable-count)  <br> |
|  uint32\_t | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable buffer 

```C++
uint8_t SYN_SHA1_Ctx::buffer[64];
```



Input buffer staging area 


        

<hr>



### variable count 

```C++
uint32_t SYN_SHA1_Ctx::count[2];
```



Number of bits processed 


        

<hr>



### variable state 

```C++
uint32_t SYN_SHA1_Ctx::state[5];
```



Internal state registers 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_websocket.c`

