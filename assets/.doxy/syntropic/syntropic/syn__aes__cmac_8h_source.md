

# File syn\_aes\_cmac.h

[**File List**](files.md) **>** [**crypto**](dir_d4ba40e134a0329a07556f14efd2db9b.md) **>** [**syn\_aes\_cmac.h**](syn__aes__cmac_8h.md)

[Go to the documentation of this file](syn__aes__cmac_8h.md)


```C++

#ifndef SYN_AES_CMAC_H
#define SYN_AES_CMAC_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_AES_CMAC_KEY_SIZE 16U

#define SYN_AES_CMAC_BLOCK_SIZE 16U

#define SYN_AES_CMAC_TAG_SIZE 16U

SYN_Status syn_aes_cmac(const uint8_t key[SYN_AES_CMAC_KEY_SIZE], const uint8_t *msg,
                        size_t msg_len, uint8_t mac[SYN_AES_CMAC_TAG_SIZE]);

bool syn_aes_cmac_verify(const uint8_t key[SYN_AES_CMAC_KEY_SIZE], const uint8_t *msg,
                         size_t msg_len, const uint8_t mac[SYN_AES_CMAC_TAG_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AES_CMAC_H */
```


