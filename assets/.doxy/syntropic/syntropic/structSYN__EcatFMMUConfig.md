

# Struct SYN\_EcatFMMUConfig



[**ClassList**](annotated.md) **>** [**SYN\_EcatFMMUConfig**](structSYN__EcatFMMUConfig.md)



_EtherCAT FMMU Configuration._ 

* `#include <syn_ethercat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**enable**](#variable-enable)  <br> |
|  uint16\_t | [**length**](#variable-length)  <br> |
|  uint8\_t | [**logical\_end\_bit**](#variable-logical_end_bit)  <br> |
|  uint32\_t | [**logical\_start**](#variable-logical_start)  <br> |
|  uint8\_t | [**logical\_start\_bit**](#variable-logical_start_bit)  <br> |
|  uint16\_t | [**phys\_start\_addr**](#variable-phys_start_addr)  <br> |
|  uint8\_t | [**phys\_start\_bit**](#variable-phys_start_bit)  <br> |
|  uint8\_t | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable enable 

```C++
bool SYN_EcatFMMUConfig::enable;
```



FMMU active flag 


        

<hr>



### variable length 

```C++
uint16_t SYN_EcatFMMUConfig::length;
```



Mapping byte length 


        

<hr>



### variable logical\_end\_bit 

```C++
uint8_t SYN_EcatFMMUConfig::logical_end_bit;
```



Logical end bit (0..7) 


        

<hr>



### variable logical\_start 

```C++
uint32_t SYN_EcatFMMUConfig::logical_start;
```



Logical start address 


        

<hr>



### variable logical\_start\_bit 

```C++
uint8_t SYN_EcatFMMUConfig::logical_start_bit;
```



Logical start bit offset (0..7) 


        

<hr>



### variable phys\_start\_addr 

```C++
uint16_t SYN_EcatFMMUConfig::phys_start_addr;
```



Physical start address in slave DPRAM 


        

<hr>



### variable phys\_start\_bit 

```C++
uint8_t SYN_EcatFMMUConfig::phys_start_bit;
```



Physical start bit offset (0..7) 


        

<hr>



### variable type 

```C++
uint8_t SYN_EcatFMMUConfig::type;
```



Read (0x01), Write (0x02), or ReadWrite (0x03) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

