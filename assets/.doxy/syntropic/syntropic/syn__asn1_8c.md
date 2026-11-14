

# File syn\_asn1.c



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_asn1.c**](syn__asn1_8c.md)

[Go to the source code of this file](syn__asn1_8c_source.md)

_Zero-Heap ASN.1 DER (Distinguished Encoding Rules) TLV Parser implementation._ 

* `#include "syntropic/crypto/syn_asn1.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_enter\_container**](#function-syn_asn1_enter_container) (const [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* container, const uint8\_t \*\* child\_buf, size\_t \* child\_len) <br>_Unwraps a constructed container (SEQUENCE, SET, or context-specific tag) for child iteration._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_get\_bit\_string**](#function-syn_asn1_get_bit_string) (const [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* elem, const uint8\_t \*\* bits\_out, size\_t \* bit\_len\_out) <br>_Extract Bit String payload (stripping unused bits count prefix byte)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_get\_integer**](#function-syn_asn1_get_integer) (const [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* elem, const uint8\_t \*\* int\_out, size\_t \* int\_len\_out) <br>_Extract raw integer bytes (stripping leading sign zero padding if present)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_match\_oid**](#function-syn_asn1_match_oid) (const [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* elem, const uint8\_t \* expected\_oid, size\_t oid\_len) <br>_Compare ASN.1 Object Identifier (OID) against expected byte array._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_parse\_element**](#function-syn_asn1_parse_element) (const uint8\_t \* buf, size\_t len, [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* elem\_out) <br>_Parse a single ASN.1 DER TLV element from buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_step**](#function-syn_asn1_step) (const uint8\_t \*\* buf, size\_t \* len, [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* elem\_out) <br>_Skip/advance past current TLV element in a DER buffer._  |




























## Public Functions Documentation




### function syn\_asn1\_enter\_container 

_Unwraps a constructed container (SEQUENCE, SET, or context-specific tag) for child iteration._ 
```C++
bool syn_asn1_enter_container (
    const SYN_ASN1_Element * container,
    const uint8_t ** child_buf,
    size_t * child_len
) 
```





**Parameters:**


* `container` Pointer to container TLV element header. 
* `child_buf` [out] Sub-slice buffer pointer to container payload. 
* `child_len` [out] Length of container payload. 



**Returns:**

true if element is constructed and valid payload extracted. 





        

<hr>



### function syn\_asn1\_get\_bit\_string 

_Extract Bit String payload (stripping unused bits count prefix byte)._ 
```C++
bool syn_asn1_get_bit_string (
    const SYN_ASN1_Element * elem,
    const uint8_t ** bits_out,
    size_t * bit_len_out
) 
```





**Parameters:**


* `elem` Parsed ASN.1 BIT STRING element. 
* `bits_out` [out] Pointer to raw bit string bytes. 
* `bit_len_out` [out] Number of valid bits in payload. 



**Returns:**

true on success. 





        

<hr>



### function syn\_asn1\_get\_integer 

_Extract raw integer bytes (stripping leading sign zero padding if present)._ 
```C++
bool syn_asn1_get_integer (
    const SYN_ASN1_Element * elem,
    const uint8_t ** int_out,
    size_t * int_len_out
) 
```





**Parameters:**


* `elem` Parsed ASN.1 INTEGER element. 
* `int_out` [out] Pointer to integer payload bytes. 
* `int_len_out` [out] Length of integer payload bytes. 



**Returns:**

true on success. 





        

<hr>



### function syn\_asn1\_match\_oid 

_Compare ASN.1 Object Identifier (OID) against expected byte array._ 
```C++
bool syn_asn1_match_oid (
    const SYN_ASN1_Element * elem,
    const uint8_t * expected_oid,
    size_t oid_len
) 
```





**Parameters:**


* `elem` Parsed ASN.1 OID element. 
* `expected_oid` Expected encoded OID payload bytes. 
* `oid_len` Length of expected OID payload bytes. 



**Returns:**

true if OIDs match. 





        

<hr>



### function syn\_asn1\_parse\_element 

_Parse a single ASN.1 DER TLV element from buffer._ 
```C++
bool syn_asn1_parse_element (
    const uint8_t * buf,
    size_t len,
    SYN_ASN1_Element * elem_out
) 
```





**Parameters:**


* `buf` Data buffer. 
* `len` Length of available bytes in buffer. 
* `elem_out` [out] Output parsed element. 



**Returns:**

true if valid TLV element parsed, false on truncation or encoding error. 





        

<hr>



### function syn\_asn1\_step 

_Skip/advance past current TLV element in a DER buffer._ 
```C++
bool syn_asn1_step (
    const uint8_t ** buf,
    size_t * len,
    SYN_ASN1_Element * elem_out
) 
```





**Parameters:**


* `buf` Pointer to buffer pointer (updated on success). 
* `len` Pointer to available length (updated on success). 
* `elem_out` [out] Optional parsed element header. 



**Returns:**

true on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_asn1.c`

