

# Struct SYN\_FaultContext



[**ClassList**](annotated.md) **>** [**SYN\_FaultContext**](structSYN__FaultContext.md)



_Register context dumped on CPU hard faults (ARM Cortex-M style)._ 

* `#include <syn_fault.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**lr**](#variable-lr)  <br> |
|  uint32\_t | [**pc**](#variable-pc)  <br> |
|  uint32\_t | [**r0**](#variable-r0)  <br> |
|  uint32\_t | [**r1**](#variable-r1)  <br> |
|  uint32\_t | [**r12**](#variable-r12)  <br> |
|  uint32\_t | [**r2**](#variable-r2)  <br> |
|  uint32\_t | [**r3**](#variable-r3)  <br> |
|  uint32\_t | [**sp**](#variable-sp)  <br> |
|  uint32\_t | [**xpsr**](#variable-xpsr)  <br> |












































## Public Attributes Documentation




### variable lr 

```C++
uint32_t SYN_FaultContext::lr;
```



Link register (return address) 
 


        

<hr>



### variable pc 

```C++
uint32_t SYN_FaultContext::pc;
```



Program counter at fault 
 


        

<hr>



### variable r0 

```C++
uint32_t SYN_FaultContext::r0;
```



General-purpose register R0 
 


        

<hr>



### variable r1 

```C++
uint32_t SYN_FaultContext::r1;
```



General-purpose register R1 
 


        

<hr>



### variable r12 

```C++
uint32_t SYN_FaultContext::r12;
```



General-purpose register R12 
 


        

<hr>



### variable r2 

```C++
uint32_t SYN_FaultContext::r2;
```



General-purpose register R2 
 


        

<hr>



### variable r3 

```C++
uint32_t SYN_FaultContext::r3;
```



General-purpose register R3 
 


        

<hr>



### variable sp 

```C++
uint32_t SYN_FaultContext::sp;
```



Stack pointer at fault 
 


        

<hr>



### variable xpsr 

```C++
uint32_t SYN_FaultContext::xpsr;
```



Program status register 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fault.h`

