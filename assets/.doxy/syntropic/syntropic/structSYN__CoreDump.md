

# Struct SYN\_CoreDump



[**ClassList**](annotated.md) **>** [**SYN\_CoreDump**](structSYN__CoreDump.md)



_Persistent core dump — written to flash on fault._ [More...](#detailed-description)

* `#include <syn_coredump.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**crc**](#variable-crc)  <br> |
|  uint32\_t | [**magic**](#variable-magic)  <br> |
|  [**SYN\_FaultContext**](structSYN__FaultContext.md) | [**regs**](#variable-regs)  <br> |
|  uint8\_t | [**stack**](#variable-stack)  <br> |
|  uint32\_t | [**stack\_bytes**](#variable-stack_bytes)  <br> |
|  uint32\_t | [**uptime\_ms**](#variable-uptime_ms)  <br> |












































## Detailed Description


Total size: ~180 bytes with 128-byte stack capture. 


    
## Public Attributes Documentation




### variable crc 

```C++
uint32_t SYN_CoreDump::crc;
```



CRC-32 over preceding fields 
 


        

<hr>



### variable magic 

```C++
uint32_t SYN_CoreDump::magic;
```



SYN\_COREDUMP\_MAGIC when valid 


        

<hr>



### variable regs 

```C++
SYN_FaultContext SYN_CoreDump::regs;
```



CPU register snapshot 
 


        

<hr>



### variable stack 

```C++
uint8_t SYN_CoreDump::stack[SYN_COREDUMP_STACK_SIZE];
```



Partial stack snapshot 
 


        

<hr>



### variable stack\_bytes 

```C++
uint32_t SYN_CoreDump::stack_bytes;
```



Actual stack bytes captured 
 


        

<hr>



### variable uptime\_ms 

```C++
uint32_t SYN_CoreDump::uptime_ms;
```



System uptime at fault 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_coredump.h`

