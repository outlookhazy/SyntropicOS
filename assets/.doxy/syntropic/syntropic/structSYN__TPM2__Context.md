

# Struct SYN\_TPM2\_Context



[**ClassList**](annotated.md) **>** [**SYN\_TPM2\_Context**](structSYN__TPM2__Context.md)



_TPM 2.0 Client Instance Context._ 

* `#include <syn_tpm2.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_TPM2\_Config**](structSYN__TPM2__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**initialized**](#variable-initialized)  <br> |
|  uint32\_t | [**last\_rc**](#variable-last_rc)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_TPM2_Config SYN_TPM2_Context::cfg;
```



Configuration 


        

<hr>



### variable initialized 

```C++
bool SYN_TPM2_Context::initialized;
```



Initialized flag 


        

<hr>



### variable last\_rc 

```C++
uint32_t SYN_TPM2_Context::last_rc;
```



Return code from last executed command 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_tpm2.h`

