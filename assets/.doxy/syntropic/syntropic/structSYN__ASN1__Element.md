

# Struct SYN\_ASN1\_Element



[**ClassList**](annotated.md) **>** [**SYN\_ASN1\_Element**](structSYN__ASN1__Element.md)



_Parsed ASN.1 TLV element header._ 

* `#include <syn_asn1.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**constructed**](#variable-constructed)  <br> |
|  size\_t | [**header\_len**](#variable-header_len)  <br> |
|  size\_t | [**length**](#variable-length)  <br> |
|  uint8\_t | [**tag**](#variable-tag)  <br> |
|  uint8\_t | [**tag\_class**](#variable-tag_class)  <br> |
|  uint8\_t | [**tag\_number**](#variable-tag_number)  <br> |
|  const uint8\_t \* | [**value**](#variable-value)  <br> |












































## Public Attributes Documentation




### variable constructed 

```C++
bool SYN_ASN1_Element::constructed;
```



True if constructed container 


        

<hr>



### variable header\_len 

```C++
size_t SYN_ASN1_Element::header_len;
```



Total length of tag + length bytes 


        

<hr>



### variable length 

```C++
size_t SYN_ASN1_Element::length;
```



Payload length in bytes 


        

<hr>



### variable tag 

```C++
uint8_t SYN_ASN1_Element::tag;
```



Full 8-bit tag byte 


        

<hr>



### variable tag\_class 

```C++
uint8_t SYN_ASN1_Element::tag_class;
```



UNIVERSAL, CONTEXT\_SPECIFIC, etc. 


        

<hr>



### variable tag\_number 

```C++
uint8_t SYN_ASN1_Element::tag_number;
```



Tag type number (0..31) 


        

<hr>



### variable value 

```C++
const uint8_t* SYN_ASN1_Element::value;
```



Pointer to payload bytes 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_asn1.h`

