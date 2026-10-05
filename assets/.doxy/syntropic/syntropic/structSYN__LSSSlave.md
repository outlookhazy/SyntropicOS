

# Struct SYN\_LSSSlave



[**ClassList**](annotated.md) **>** [**SYN\_LSSSlave**](structSYN__LSSSlave.md)



_LSS Slave Instance State._ 

* `#include <syn_lss.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_LSSAddress**](structSYN__LSSAddress.md) | [**addr**](#variable-addr)  <br> |
|  uint8\_t | [**baud\_table**](#variable-baud_table)  <br> |
|  uint8\_t | [**mode**](#variable-mode)  <br> |
|  uint8\_t | [**node\_id**](#variable-node_id)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**store\_req**](#variable-store_req)  <br> |












































## Public Attributes Documentation




### variable addr 

```C++
SYN_LSSAddress SYN_LSSSlave::addr;
```



Static LSS 128-bit Identity Address 


        

<hr>



### variable baud\_table 

```C++
uint8_t SYN_LSSSlave::baud_table;
```



Pending configured baud table entry 


        

<hr>



### variable mode 

```C++
uint8_t SYN_LSSSlave::mode;
```



Current LSS mode (Operation / Configuration) 


        

<hr>



### variable node\_id 

```C++
uint8_t SYN_LSSSlave::node_id;
```



Current configured Node-ID 


        

<hr>



### variable store\_req 

```C++
bool SYN_LSSSlave::store_req;
```



True if store configuration requested 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lss.h`

