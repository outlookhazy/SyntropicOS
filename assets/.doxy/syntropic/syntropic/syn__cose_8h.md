

# File syn\_cose.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_cose.h**](syn__cose_8h.md)

[Go to the source code of this file](syn__cose_8h_source.md)

_Zero-Heap CBOR Object Signing and Encryption (COSE - RFC 9052 / RFC 8152)._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/crypto/syn_ed25519.h"`
* `#include "syntropic/crypto/syn_p256.h"`
































































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_COSE\_ENABLED**](syn__cose_8h.md#define-syn_cose_enabled)  `0`<br>_Non-zero when COSE and required CBOR/crypto dependencies are enabled._  |

## Detailed Description


Implements COSE\_Sign1 (Tag 18) and COSE\_Encrypt0 (Tag 16) for constrained IoT nodes. Supported Cryptographic Algorithms:
* EdDSA (-8): Pure C99 Ed25519 signature scheme (RFC 8032).
* ES256 (-7): ECDSA over NIST P-256 curve with SHA-256 (RFC 9053).
* ChaCha20/Poly1305 (24): Authenticated encryption with associated data (RFC 9053). 




    
## Macro Definition Documentation





### define SYN\_COSE\_ENABLED 

_Non-zero when COSE and required CBOR/crypto dependencies are enabled._ 
```C++
#define SYN_COSE_ENABLED `0`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_cose.h`

