

# File syn\_asn1.h



[**FileList**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_asn1.h**](syn__asn1_8h.md)

[Go to the source code of this file](syn__asn1_8h_source.md)

_Zero-Heap ASN.1 DER (Distinguished Encoding Rules) TLV Parser._ 

* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) <br>_Parsed ASN.1 TLV element header._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_enter\_container**](#function-syn_asn1_enter_container) (const [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* container, const uint8\_t \*\* child\_buf, size\_t \* child\_len) <br>_Unwraps a constructed container (SEQUENCE, SET, or context-specific tag) for child iteration._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_get\_bit\_string**](#function-syn_asn1_get_bit_string) (const [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* elem, const uint8\_t \*\* bits\_out, size\_t \* bit\_len\_out) <br>_Extract Bit String payload (stripping unused bits count prefix byte)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_get\_integer**](#function-syn_asn1_get_integer) (const [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* elem, const uint8\_t \*\* int\_out, size\_t \* int\_len\_out) <br>_Extract raw integer bytes (stripping leading sign zero padding if present)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_match\_oid**](#function-syn_asn1_match_oid) (const [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* elem, const uint8\_t \* expected\_oid, size\_t oid\_len) <br>_Compare ASN.1 Object Identifier (OID) against expected byte array._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_parse\_element**](#function-syn_asn1_parse_element) (const uint8\_t \* buf, size\_t len, [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* elem\_out) <br>_Parse a single ASN.1 DER TLV element from buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_asn1\_step**](#function-syn_asn1_step) (const uint8\_t \*\* buf, size\_t \* len, [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md) \* elem\_out) <br>_Skip/advance past current TLV element in a DER buffer._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_ASN1\_CLASS\_APPLICATION**](syn__asn1_8h.md#define-syn_asn1_class_application)  `0x40U`<br>_APPLICATION tag class (0x40)._  |
| define  | [**SYN\_ASN1\_CLASS\_CONTEXT\_SPECIFIC**](syn__asn1_8h.md#define-syn_asn1_class_context_specific)  `0x80U`<br>_CONTEXT-SPECIFIC tag class (0x80)._  |
| define  | [**SYN\_ASN1\_CLASS\_MASK**](syn__asn1_8h.md#define-syn_asn1_class_mask)  `0xC0U`<br>_ASN.1 Tag Class / Form Masking._  |
| define  | [**SYN\_ASN1\_CLASS\_PRIVATE**](syn__asn1_8h.md#define-syn_asn1_class_private)  `0xC0U`<br>_PRIVATE tag class (0xC0)._  |
| define  | [**SYN\_ASN1\_CLASS\_UNIVERSAL**](syn__asn1_8h.md#define-syn_asn1_class_universal)  `0x00U`<br>_UNIVERSAL tag class (0x00)._  |
| define  | [**SYN\_ASN1\_CONSTRUCTED\_MASK**](syn__asn1_8h.md#define-syn_asn1_constructed_mask)  `0x20U`<br>_Mask for ASN.1 constructed form (bit 5)._  |
| define  | [**SYN\_ASN1\_TAG\_BIT\_STRING**](syn__asn1_8h.md#define-syn_asn1_tag_bit_string)  `0x03U`<br>_ASN.1 BIT STRING universal tag._  |
| define  | [**SYN\_ASN1\_TAG\_BOOLEAN**](syn__asn1_8h.md#define-syn_asn1_tag_boolean)  `0x01U`<br>_ASN.1 Universal Tag Constants._  |
| define  | [**SYN\_ASN1\_TAG\_GENERALIZEDTIME**](syn__asn1_8h.md#define-syn_asn1_tag_generalizedtime)  `0x18U`<br>_ASN.1 GeneralizedTime universal tag._  |
| define  | [**SYN\_ASN1\_TAG\_INTEGER**](syn__asn1_8h.md#define-syn_asn1_tag_integer)  `0x02U`<br>_ASN.1 INTEGER universal tag._  |
| define  | [**SYN\_ASN1\_TAG\_NULL**](syn__asn1_8h.md#define-syn_asn1_tag_null)  `0x05U`<br>_ASN.1 NULL universal tag._  |
| define  | [**SYN\_ASN1\_TAG\_NUM\_MASK**](syn__asn1_8h.md#define-syn_asn1_tag_num_mask)  `0x1FU`<br>_Mask for ASN.1 tag number (lower 5 bits)._  |
| define  | [**SYN\_ASN1\_TAG\_OCTET\_STRING**](syn__asn1_8h.md#define-syn_asn1_tag_octet_string)  `0x04U`<br>_ASN.1 OCTET STRING universal tag._  |
| define  | [**SYN\_ASN1\_TAG\_OID**](syn__asn1_8h.md#define-syn_asn1_tag_oid)  `0x06U`<br>_ASN.1 OID universal tag._  |
| define  | [**SYN\_ASN1\_TAG\_SEQUENCE**](syn__asn1_8h.md#define-syn_asn1_tag_sequence)  `0x30U`<br>_ASN.1 SEQUENCE universal tag._  |
| define  | [**SYN\_ASN1\_TAG\_SET**](syn__asn1_8h.md#define-syn_asn1_tag_set)  `0x31U`<br>_ASN.1 SET universal tag._  |
| define  | [**SYN\_ASN1\_TAG\_UTCTIME**](syn__asn1_8h.md#define-syn_asn1_tag_utctime)  `0x17U`<br>_ASN.1 UTCTime universal tag._  |

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
## Macro Definition Documentation





### define SYN\_ASN1\_CLASS\_APPLICATION 

_APPLICATION tag class (0x40)._ 
```C++
#define SYN_ASN1_CLASS_APPLICATION `0x40U`
```




<hr>



### define SYN\_ASN1\_CLASS\_CONTEXT\_SPECIFIC 

_CONTEXT-SPECIFIC tag class (0x80)._ 
```C++
#define SYN_ASN1_CLASS_CONTEXT_SPECIFIC `0x80U`
```




<hr>



### define SYN\_ASN1\_CLASS\_MASK 

_ASN.1 Tag Class / Form Masking._ 
```C++
#define SYN_ASN1_CLASS_MASK `0xC0U`
```



Mask for ASN.1 tag class (top 2 bits). 


        

<hr>



### define SYN\_ASN1\_CLASS\_PRIVATE 

_PRIVATE tag class (0xC0)._ 
```C++
#define SYN_ASN1_CLASS_PRIVATE `0xC0U`
```




<hr>



### define SYN\_ASN1\_CLASS\_UNIVERSAL 

_UNIVERSAL tag class (0x00)._ 
```C++
#define SYN_ASN1_CLASS_UNIVERSAL `0x00U`
```




<hr>



### define SYN\_ASN1\_CONSTRUCTED\_MASK 

_Mask for ASN.1 constructed form (bit 5)._ 
```C++
#define SYN_ASN1_CONSTRUCTED_MASK `0x20U`
```




<hr>



### define SYN\_ASN1\_TAG\_BIT\_STRING 

_ASN.1 BIT STRING universal tag._ 
```C++
#define SYN_ASN1_TAG_BIT_STRING `0x03U`
```




<hr>



### define SYN\_ASN1\_TAG\_BOOLEAN 

_ASN.1 Universal Tag Constants._ 
```C++
#define SYN_ASN1_TAG_BOOLEAN `0x01U`
```



ASN.1 BOOLEAN universal tag. 


        

<hr>



### define SYN\_ASN1\_TAG\_GENERALIZEDTIME 

_ASN.1 GeneralizedTime universal tag._ 
```C++
#define SYN_ASN1_TAG_GENERALIZEDTIME `0x18U`
```




<hr>



### define SYN\_ASN1\_TAG\_INTEGER 

_ASN.1 INTEGER universal tag._ 
```C++
#define SYN_ASN1_TAG_INTEGER `0x02U`
```




<hr>



### define SYN\_ASN1\_TAG\_NULL 

_ASN.1 NULL universal tag._ 
```C++
#define SYN_ASN1_TAG_NULL `0x05U`
```




<hr>



### define SYN\_ASN1\_TAG\_NUM\_MASK 

_Mask for ASN.1 tag number (lower 5 bits)._ 
```C++
#define SYN_ASN1_TAG_NUM_MASK `0x1FU`
```




<hr>



### define SYN\_ASN1\_TAG\_OCTET\_STRING 

_ASN.1 OCTET STRING universal tag._ 
```C++
#define SYN_ASN1_TAG_OCTET_STRING `0x04U`
```




<hr>



### define SYN\_ASN1\_TAG\_OID 

_ASN.1 OID universal tag._ 
```C++
#define SYN_ASN1_TAG_OID `0x06U`
```




<hr>



### define SYN\_ASN1\_TAG\_SEQUENCE 

_ASN.1 SEQUENCE universal tag._ 
```C++
#define SYN_ASN1_TAG_SEQUENCE `0x30U`
```




<hr>



### define SYN\_ASN1\_TAG\_SET 

_ASN.1 SET universal tag._ 
```C++
#define SYN_ASN1_TAG_SET `0x31U`
```




<hr>



### define SYN\_ASN1\_TAG\_UTCTIME 

_ASN.1 UTCTime universal tag._ 
```C++
#define SYN_ASN1_TAG_UTCTIME `0x17U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_asn1.h`

