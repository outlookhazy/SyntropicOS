

# File syn\_tls.h



[**FileList**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_tls.h**](syn__tls_8h.md)

[Go to the source code of this file](syn__tls_8h_source.md)

_Native Zero-Heap TLS 1.3 Protocol Engine & Transport Adapter (RFC 8446)._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/crypto/syn_hkdf.h"`
* `#include "syntropic/crypto/syn_x509.h"`
































































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_TLS\_ENABLED**](syn__tls_8h.md#define-syn_tls_enabled)  `0`<br>_Non-zero when TLS 1.3 and required crypto/scheduler dependencies are enabled._  |

## Detailed Description


Implements TLS 1.3 Client & Server handshakes and encrypted record layer. Supported modes:
* Pre-Shared Key (PSK / PSK-DHE)
* Raw Public Key (RPK) - RFC 7250
* X.509 Server Authentication & Mutual TLS (mTLS)




Plugs into standard [**SYN\_Transport**](structSYN__Transport.md) interface. 


    
## Macro Definition Documentation





### define SYN\_TLS\_ENABLED 

_Non-zero when TLS 1.3 and required crypto/scheduler dependencies are enabled._ 
```C++
#define SYN_TLS_ENABLED `0`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/net/syn_tls.h`

