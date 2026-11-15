

# Struct SYN\_EcatSMConfig



[**ClassList**](annotated.md) **>** [**SYN\_EcatSMConfig**](structSYN__EcatSMConfig.md)



_EtherCAT SyncManager Configuration._ 

* `#include <syn_ethercat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**control**](#variable-control)  <br> |
|  uint8\_t | [**enable**](#variable-enable)  <br> |
|  uint16\_t | [**length**](#variable-length)  <br> |
|  uint16\_t | [**start\_addr**](#variable-start_addr)  <br> |












































## Public Attributes Documentation




### variable control 

```C++
uint8_t SYN_EcatSMConfig::control;
```



Direction, mode, interrupt control byte 


        

<hr>



### variable enable 

```C++
uint8_t SYN_EcatSMConfig::enable;
```



SM enable byte (0x01 = enable) 


        

<hr>



### variable length 

```C++
uint16_t SYN_EcatSMConfig::length;
```



Buffer length in bytes 


        

<hr>



### variable start\_addr 

```C++
uint16_t SYN_EcatSMConfig::start_addr;
```



Physical start address in slave DPRAM 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

