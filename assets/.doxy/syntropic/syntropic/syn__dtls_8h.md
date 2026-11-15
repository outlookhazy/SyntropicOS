

# File syn\_dtls.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dtls.h**](syn__dtls_8h.md)

[Go to the source code of this file](syn__dtls_8h_source.md)

_Native Zero-Heap DTLS 1.3 Datagram Protocol Engine (RFC 9147)._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/net/syn_tls.h"`
































































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_DTLS\_ENABLED**](syn__dtls_8h.md#define-syn_dtls_enabled)  `0`<br>_Non-zero when DTLS 1.3 and required TLS/crypto dependencies are enabled._  |

## Detailed Description


Implements Datagram Transport Layer Security (DTLS) version 1.3:
* Unified record header format (RFC 9147 Section 4)
* 64-packet sliding window anti-replay protection (RFC 9147 Section 4.5.2)
* Epoch-based key scheduling & AEAD record encryption/decryption
* Pre-Shared Key (PSK), Raw Public Key (RPK), and X.509 modes
* Full pluggability into [**SYN\_Transport**](structSYN__Transport.md) (UDP, wireless, serial, etc.) 




    
## Macro Definition Documentation





### define SYN\_DTLS\_ENABLED 

_Non-zero when DTLS 1.3 and required TLS/crypto dependencies are enabled._ 
```C++
#define SYN_DTLS_ENABLED `0`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_dtls.h`

