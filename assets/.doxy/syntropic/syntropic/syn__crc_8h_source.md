

# File syn\_crc.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_crc.h**](syn__crc_8h.md)

[Go to the documentation of this file](syn__crc_8h.md)


```C++

#ifndef SYN_CRC_H
#define SYN_CRC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

#ifndef SYN_CRC_USE_TABLE
  #define SYN_CRC_USE_TABLE  1
#endif

/* ── CRC-8 (MAXIM / Dallas 1-Wire) ─────────────────────────────────────── */
/* Polynomial: x^8 + x^5 + x^4 + 1  (0x31)                                */

#define SYN_CRC8_INIT  0x00u

uint8_t syn_crc8_update(uint8_t crc, const void *data, size_t len);

static inline uint8_t syn_crc8(const void *data, size_t len)
{
    return syn_crc8_update(SYN_CRC8_INIT, data, len);
}

/* ── CRC-16/CCITT (XModem) ──────────────────────────────────────────────── */
/* Polynomial: x^16 + x^12 + x^5 + 1  (0x1021)                             */

#define SYN_CRC16_CCITT_INIT  0xFFFFu

uint16_t syn_crc16_ccitt_update(uint16_t crc, const void *data, size_t len);

static inline uint16_t syn_crc16_ccitt(const void *data, size_t len)
{
    return syn_crc16_ccitt_update(SYN_CRC16_CCITT_INIT, data, len);
}

/* ── CRC-16/Modbus ──────────────────────────────────────────────────────── */
/* Polynomial: x^16 + x^15 + x^2 + 1  (0x8005, reflected)                  */

#define SYN_CRC16_MODBUS_INIT  0xFFFFu

uint16_t syn_crc16_modbus_update(uint16_t crc, const void *data, size_t len);

static inline uint16_t syn_crc16_modbus(const void *data, size_t len)
{
    return syn_crc16_modbus_update(SYN_CRC16_MODBUS_INIT, data, len);
}

/* ── CRC-32 (Ethernet / zlib) ───────────────────────────────────────────── */
/* Polynomial: 0x04C11DB7 (reflected: 0xEDB88320)                           */

#define SYN_CRC32_INIT  0xFFFFFFFFu

uint32_t syn_crc32_update(uint32_t crc, const void *data, size_t len);

static inline uint32_t syn_crc32_final(uint32_t crc)
{
    return crc ^ 0xFFFFFFFFu;
}

static inline uint32_t syn_crc32(const void *data, size_t len)
{
    return syn_crc32_final(syn_crc32_update(SYN_CRC32_INIT, data, len));
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_CRC_H */
```


