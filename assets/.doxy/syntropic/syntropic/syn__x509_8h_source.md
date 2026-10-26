

# File syn\_x509.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_x509.h**](syn__x509_8h.md)

[Go to the documentation of this file](syn__x509_8h.md)


```C++

#ifndef SYN_X509_H
#define SYN_X509_H

#include "syntropic/crypto/syn_asn1.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_X509_MAX_NAME_LEN 128U
#define SYN_X509_PUBKEY_MAX_LEN 128U
#define SYN_X509_SIG_MAX_LEN 256U

typedef enum {
    SYN_X509_ALGO_UNKNOWN = 0,
    SYN_X509_ALGO_ED25519,
    SYN_X509_ALGO_ECDSA_P256,
    SYN_X509_ALGO_RSA_PSS
} SYN_X509_Algo;

typedef struct {
    const uint8_t *tbs_bytes; 
    size_t tbs_len;           
    uint32_t version;         
    const uint8_t *serial;    
    size_t serial_len;        
    SYN_X509_Algo pubkey_algo;               
    uint8_t pubkey[SYN_X509_PUBKEY_MAX_LEN]; 
    size_t pubkey_len;                       
    SYN_X509_Algo sig_algo;                  
    uint8_t signature[SYN_X509_SIG_MAX_LEN]; 
    size_t signature_len;                    
    char subject_cn[SYN_X509_MAX_NAME_LEN]; 
    char issuer_cn[SYN_X509_MAX_NAME_LEN];  
    bool is_ca; 
} SYN_X509_Cert;

bool syn_x509_parse(const uint8_t *der, size_t der_len, SYN_X509_Cert *cert_out);

bool syn_x509_verify_signature(const SYN_X509_Cert *cert, const uint8_t *issuer_pubkey,
                               size_t issuer_pubkey_len, SYN_X509_Algo algo);

bool syn_x509_validate_chain(const SYN_X509_Cert *cert, const SYN_X509_Cert *root_ca,
                             const char *expected_cn);

#ifdef __cplusplus
}
#endif

#endif /* SYN_X509_H */
```


