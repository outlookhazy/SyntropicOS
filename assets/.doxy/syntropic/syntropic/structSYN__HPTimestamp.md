

# Struct SYN\_HPTimestamp



[**ClassList**](annotated.md) **>** [**SYN\_HPTimestamp**](structSYN__HPTimestamp.md)



_Raw high-precision timestamp — three-word snapshot._ [More...](#detailed-description)

* `#include <syn_hpclock.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**lsb**](#variable-lsb)  <br> |
|  uint32\_t | [**msb\_1**](#variable-msb_1)  <br> |
|  uint32\_t | [**msb\_2**](#variable-msb_2)  <br> |












































## Detailed Description


The overflow counter is read before and after the hardware counter register. Resolution compares the two MSB reads to detect whether an overflow occurred during the capture, and uses the LSB value to determine which MSB is correct for the captured instant.


12 bytes, naturally aligned. 


    
## Public Attributes Documentation




### variable lsb 

```C++
uint32_t SYN_HPTimestamp::lsb;
```



Read of hardware counter register 
 


        

<hr>



### variable msb\_1 

```C++
uint32_t SYN_HPTimestamp::msb_1;
```



First read of overflow counter 
 


        

<hr>



### variable msb\_2 

```C++
uint32_t SYN_HPTimestamp::msb_2;
```



Second read of overflow counter 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_hpclock.h`

