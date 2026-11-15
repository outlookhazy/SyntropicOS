

# Struct SYN\_Lz4



[**ClassList**](annotated.md) **>** [**SYN\_Lz4**](structSYN__Lz4.md)



_LZ4 Compression Workspace Context._ [More...](#detailed-description)

* `#include <syn_lz4.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**hash\_table**](#variable-hash_table)  <br> |












































## Detailed Description


Must be allocated by caller and initialized with [**syn\_lz4\_init()**](syn__lz4_8c.md#function-syn_lz4_init) before calling [**syn\_lz4\_compress()**](syn__lz4_8c.md#function-syn_lz4_compress).




**Warning:**

Default [**SYN\_Lz4**](structSYN__Lz4.md) workspace is 8 KB (4096 x uint16\_t) and must be allocated in static/BSS storage rather than on a 1024 B task stack. 





    
## Public Attributes Documentation




### variable hash\_table 

```C++
uint16_t SYN_Lz4::hash_table[SYN_LZ4_HASH_SIZE];
```



Hash table indexed by 4-byte sequence hash 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_lz4.h`

