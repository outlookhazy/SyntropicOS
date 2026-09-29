

# File syn\_bits.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_bits.h**](syn__bits_8h.md)

[Go to the source code of this file](syn__bits_8h_source.md)

_Bit manipulation macros._ [More...](#detailed-description)

* `#include <stdint.h>`
































































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_BIT**](syn__bits_8h.md#define-syn_bit) (n) `((uint32\_t)1U &lt;&lt; (n))`<br> |
| define  | [**SYN\_BITMASK**](syn__bits_8h.md#define-syn_bitmask) (width, offset) `(((1U &lt;&lt; (width)) - 1U) &lt;&lt; (offset))`<br> |
| define  | [**SYN\_BITS\_GET**](syn__bits_8h.md#define-syn_bits_get) (reg, width, offset) `(((reg) &gt;&gt; (offset)) & ((1U &lt;&lt; (width)) - 1U))`<br> |
| define  | [**SYN\_BITS\_SET**](syn__bits_8h.md#define-syn_bits_set) (reg, width, offset, value) `/* multi line expression */`<br> |
| define  | [**SYN\_BITS\_TO\_BYTES**](syn__bits_8h.md#define-syn_bits_to_bytes) (bits) `(((bits) + 7U) / 8U)`<br> |
| define  | [**SYN\_BIT\_CHECK**](syn__bits_8h.md#define-syn_bit_check) (reg, bit) `((reg) & [**SYN\_BIT**](syn__bits_8h.md#define-syn_bit)(bit))`<br> |
| define  | [**SYN\_BIT\_CLEAR**](syn__bits_8h.md#define-syn_bit_clear) (reg, bit) `((reg) &= ~[**SYN\_BIT**](syn__bits_8h.md#define-syn_bit)(bit))`<br> |
| define  | [**SYN\_BIT\_SET**](syn__bits_8h.md#define-syn_bit_set) (reg, bit) `((reg) \|= [**SYN\_BIT**](syn__bits_8h.md#define-syn_bit)(bit))`<br> |
| define  | [**SYN\_BIT\_TOGGLE**](syn__bits_8h.md#define-syn_bit_toggle) (reg, bit) `((reg) ^= [**SYN\_BIT**](syn__bits_8h.md#define-syn_bit)(bit))`<br> |

## Detailed Description


All macros are pure preprocessor — zero overhead, no function calls. 


    
## Macro Definition Documentation





### define SYN\_BIT 

```C++
#define SYN_BIT (
    n
) `((uint32_t)1U << (n))`
```



Produce a bitmask with bit `n` set (0-indexed). 


        

<hr>



### define SYN\_BITMASK 

```C++
#define SYN_BITMASK (
    width,
    offset
) `(((1U << (width)) - 1U) << (offset))`
```



Produce a bitmask of `width` bits starting at bit position `offset`. Example: [**SYN\_BITMASK(3, 4)**](syn__bits_8h.md#define-syn_bitmask) → 0b0000'0000'0111'0000 → 0x70 


        

<hr>



### define SYN\_BITS\_GET 

```C++
#define SYN_BITS_GET (
    reg,
    width,
    offset
) `(((reg) >> (offset)) & ((1U << (width)) - 1U))`
```



Extract a bit-field of `width` bits starting at `offset` from `reg`. 


        

<hr>



### define SYN\_BITS\_SET 

```C++
#define SYN_BITS_SET (
    reg,
    width,
    offset,
    value
) `/* multi line expression */`
```



Set a bit-field: clear the field in `reg`, then OR in `value`. 


        

<hr>



### define SYN\_BITS\_TO\_BYTES 

```C++
#define SYN_BITS_TO_BYTES (
    bits
) `(((bits) + 7U) / 8U)`
```



Number of bytes needed to store `bits`. 


        

<hr>



### define SYN\_BIT\_CHECK 

```C++
#define SYN_BIT_CHECK (
    reg,
    bit
) `((reg) & SYN_BIT (bit))`
```



Check if bit `bit` is set in `reg`. Evaluates to non-zero if set. 


        

<hr>



### define SYN\_BIT\_CLEAR 

```C++
#define SYN_BIT_CLEAR (
    reg,
    bit
) `((reg) &= ~ SYN_BIT (bit))`
```



Clear bit `bit` in register/variable `reg`. 


        

<hr>



### define SYN\_BIT\_SET 

```C++
#define SYN_BIT_SET (
    reg,
    bit
) `((reg) |= SYN_BIT (bit))`
```



Set bit `bit` in register/variable `reg`. 


        

<hr>



### define SYN\_BIT\_TOGGLE 

```C++
#define SYN_BIT_TOGGLE (
    reg,
    bit
) `((reg) ^= SYN_BIT (bit))`
```



Toggle bit `bit` in register/variable `reg`. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_bits.h`

