

# Struct SYN\_TPM2\_QuoteResult



[**ClassList**](annotated.md) **>** [**SYN\_TPM2\_QuoteResult**](structSYN__TPM2__QuoteResult.md)



_TPM 2.0 Attestation Quote Output Structure._ 

* `#include <syn_tpm2.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**attest\_data**](#variable-attest_data)  <br> |
|  uint16\_t | [**attest\_len**](#variable-attest_len)  <br> |
|  uint8\_t | [**signature**](#variable-signature)  <br> |
|  uint16\_t | [**signature\_len**](#variable-signature_len)  <br> |












































## Public Attributes Documentation




### variable attest\_data 

```C++
uint8_t SYN_TPM2_QuoteResult::attest_data[SYN_TPM2_MAX_QUOTE_LEN];
```



TPMS\_ATTEST serialized data 


        

<hr>



### variable attest\_len 

```C++
uint16_t SYN_TPM2_QuoteResult::attest_len;
```



Length of TPMS\_ATTEST data 


        

<hr>



### variable signature 

```C++
uint8_t SYN_TPM2_QuoteResult::signature[SYN_TPM2_MAX_QUOTE_LEN];
```



TPMT\_SIGNATURE quote signature 


        

<hr>



### variable signature\_len 

```C++
uint16_t SYN_TPM2_QuoteResult::signature_len;
```



Length of signature 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/crypto/syn_tpm2.h`

