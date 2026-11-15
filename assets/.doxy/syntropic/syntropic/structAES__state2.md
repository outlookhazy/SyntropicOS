

# Struct AES\_state2



[**ClassList**](annotated.md) **>** [**AES\_state2**](structAES__state2.md)



_Bitsliced AES state matrix (8 slices of 32-bit words)._ [More...](#detailed-description)






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**slice**](#variable-slice)  <br> |












































## Detailed Description


Each 32-bit slice variable slice[b] contains bit b of 2 independent 16-byte blocks: Block 0 in bits 0..15, Block 1 in bits 16..31. Bit index in slice = 16 \* blk + 4 \* r + c. 


    
## Public Attributes Documentation




### variable slice 

```C++
uint32_t AES_state2::slice[8];
```



Bit-slice planes across up to 2 state blocks 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_aes.c`

