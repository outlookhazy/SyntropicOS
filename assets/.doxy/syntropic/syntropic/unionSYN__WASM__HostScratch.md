

# Union SYN\_WASM\_HostScratch



[**ClassList**](annotated.md) **>** [**SYN\_WASM\_HostScratch**](unionSYN__WASM__HostScratch.md)



_Per-call scratch storage preserved across host coroutine yields._ 

* `#include <syn_wasm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_PT**](structSYN__PT.md) | [**child\_pt**](#variable-child_pt)  <br> |
|  struct [**SYN\_WASM\_HostScratch**](unionSYN__WASM__HostScratch.md) | [**pt**](#variable-pt)  <br> |
|  uint16\_t | [**u16**](#variable-u16)  <br> |
|  uint32\_t | [**u32**](#variable-u32)  <br> |
|  uint64\_t | [**u64**](#variable-u64)  <br> |
|  uint8\_t | [**u8**](#variable-u8)  <br> |












































## Public Attributes Documentation




### variable child\_pt 

```C++
SYN_PT SYN_WASM_HostScratch::child_pt;
```



Child protothread control block for nested PT\_SPAWN


Direct child protothread control block 


        

<hr>



### variable pt 

```C++
struct SYN_WASM_HostScratch SYN_WASM_HostScratch::pt;
```



Nested protothread + counter layout 


        

<hr>



### variable u16 

```C++
uint16_t SYN_WASM_HostScratch::u16[7];
```



Auxiliary 16-bit counters alongside child\_pt 


        

<hr>



### variable u32 

```C++
uint32_t SYN_WASM_HostScratch::u32[4];
```



32-bit scratch slots (e.g., standalone PT\_DELAY\_MS target) 


        

<hr>



### variable u64 

```C++
uint64_t SYN_WASM_HostScratch::u64[2];
```



64-bit scratch slots 


        

<hr>



### variable u8 

```C++
uint8_t SYN_WASM_HostScratch::u8[16];
```



Byte scratch buffer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

