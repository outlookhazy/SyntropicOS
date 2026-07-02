

# File syn\_bits.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_bits.h**](syn__bits_8h.md)

[Go to the documentation of this file](syn__bits_8h.md)


```C++

#ifndef SYN_BITS_H
#define SYN_BITS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_BIT(n)                ((uint32_t)1U << (n))

#define SYN_BIT_SET(reg, bit)     ((reg) |= SYN_BIT(bit))

#define SYN_BIT_CLEAR(reg, bit)   ((reg) &= ~SYN_BIT(bit))

#define SYN_BIT_TOGGLE(reg, bit)  ((reg) ^= SYN_BIT(bit))

#define SYN_BIT_CHECK(reg, bit)   ((reg) & SYN_BIT(bit))

#define SYN_BITMASK(width, offset) \
    (((1U << (width)) - 1U) << (offset))

#define SYN_BITS_GET(reg, width, offset) \
    (((reg) >> (offset)) & ((1U << (width)) - 1U))

#define SYN_BITS_SET(reg, width, offset, value)             \
    do {                                                      \
        (reg) &= ~SYN_BITMASK(width, offset);               \
        (reg) |= (((uint32_t)(value) & ((1U << (width)) - 1U)) << (offset)); \
    } while (0)

#define SYN_BITS_TO_BYTES(bits)   (((bits) + 7U) / 8U)

#ifdef __cplusplus
}
#endif

#endif /* SYN_BITS_H */
```


