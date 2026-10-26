

# File syn\_port\_aes.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_aes.h**](syn__port__aes_8h.md)

[Go to the documentation of this file](syn__port__aes_8h.md)


```C++

#ifndef SYN_PORT_AES_H
#define SYN_PORT_AES_H

#include "../common/syn_defs.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_aes_init(void);

SYN_Status syn_port_aes_encrypt_block(const uint8_t *round_keys, uint8_t nr, const uint8_t in[16],
                                      uint8_t out[16]);

SYN_Status syn_port_aes_decrypt_block(const uint8_t *round_keys, uint8_t nr, const uint8_t in[16],
                                      uint8_t out[16]);

SYN_Status syn_port_ghash_mult(const uint8_t x[16], const uint8_t h[16], uint8_t out[16]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_AES_H */
```


