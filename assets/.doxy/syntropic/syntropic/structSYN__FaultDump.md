

# Struct SYN\_FaultDump



[**ClassList**](annotated.md) **>** [**SYN\_FaultDump**](structSYN__FaultDump.md)



_Fault dump stored in noinit RAM across resets._ 

* `#include <syn_fault.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_FaultContext**](structSYN__FaultContext.md) | [**context**](#variable-context)  <br> |
|  uint32\_t | [**signature**](#variable-signature)  <br> |












































## Public Attributes Documentation




### variable context 

```C++
SYN_FaultContext SYN_FaultDump::context;
```



Saved register context 
 


        

<hr>



### variable signature 

```C++
uint32_t SYN_FaultDump::signature;
```



Magic value (SYN\_FAULT\_SIGNATURE when valid) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_fault.h`

