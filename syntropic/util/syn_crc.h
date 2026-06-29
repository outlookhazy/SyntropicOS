/**
 * @file syn_crc.h
 * @brief CRC calculation for communication protocols.
 *
 * Provides CRC-8, CRC-16 (CCITT, Modbus), and CRC-32 (Ethernet).
 * Configure SYN_CRC_USE_TABLE to trade ROM for speed:
 *   1 = 256-entry lookup table (fast, ~256–1024 bytes ROM per variant)
 *   0 = bit-by-bit computation (small, slower)
 *
 * All variants support incremental (streaming) computation.
 *
 * @par Usage
 * @code
 *   uint16_t crc = syn_crc16_ccitt(data, len);
 *
 *   // Incremental:
 *   uint16_t crc = SYN_CRC16_CCITT_INIT;
 *   crc = syn_crc16_ccitt_update(crc, chunk1, len1);
 *   crc = syn_crc16_ccitt_update(crc, chunk2, len2);
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_CRC_H
#define SYN_CRC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

/** Set to 1 for lookup-table CRC (fast), 0 for bit-by-bit (small). */
#ifndef SYN_CRC_USE_TABLE
  #define SYN_CRC_USE_TABLE  1
#endif

/* ── CRC-8 (MAXIM / Dallas 1-Wire) ─────────────────────────────────────── */
/* Polynomial: x^8 + x^5 + x^4 + 1  (0x31)                                */

/** @brief CRC-8 initial value. */
#define SYN_CRC8_INIT  0x00u

/**
 * @brief Update CRC-8 with a block of data.
 * @param crc   Running CRC value (start with SYN_CRC8_INIT).
 * @param data  Data buffer.
 * @param len   Length in bytes.
 * @return Updated CRC-8.
 */
uint8_t syn_crc8_update(uint8_t crc, const void *data, size_t len);

/**
 * @brief Compute CRC-8 of a complete buffer.
 * @param data  Data buffer.
 * @param len   Length in bytes.
 * @return CRC-8 value.
 */
static inline uint8_t syn_crc8(const void *data, size_t len)
{
    return syn_crc8_update(SYN_CRC8_INIT, data, len);
}

/* ── CRC-16/CCITT (XModem) ──────────────────────────────────────────────── */
/* Polynomial: x^16 + x^12 + x^5 + 1  (0x1021)                             */

/** @brief CRC-16 CCITT initial value. */
#define SYN_CRC16_CCITT_INIT  0xFFFFu

/**
 * @brief Update CRC-16 CCITT with a block of data.
 * @param crc   Running CRC value (start with SYN_CRC16_CCITT_INIT).
 * @param data  Data buffer.
 * @param len   Length in bytes.
 * @return Updated CRC-16.
 */
uint16_t syn_crc16_ccitt_update(uint16_t crc, const void *data, size_t len);

/**
 * @brief Compute CRC-16 CCITT of a complete buffer.
 * @param data  Data buffer.
 * @param len   Length in bytes.
 * @return CRC-16 CCITT value.
 */
static inline uint16_t syn_crc16_ccitt(const void *data, size_t len)
{
    return syn_crc16_ccitt_update(SYN_CRC16_CCITT_INIT, data, len);
}

/* ── CRC-16/Modbus ──────────────────────────────────────────────────────── */
/* Polynomial: x^16 + x^15 + x^2 + 1  (0x8005, reflected)                  */

/** @brief CRC-16 Modbus initial value. */
#define SYN_CRC16_MODBUS_INIT  0xFFFFu

/**
 * @brief Update CRC-16 Modbus with a block of data.
 * @param crc   Running CRC value (start with SYN_CRC16_MODBUS_INIT).
 * @param data  Data buffer.
 * @param len   Length in bytes.
 * @return Updated CRC-16.
 */
uint16_t syn_crc16_modbus_update(uint16_t crc, const void *data, size_t len);

/**
 * @brief Compute CRC-16 Modbus of a complete buffer.
 * @param data  Data buffer.
 * @param len   Length in bytes.
 * @return CRC-16 Modbus value.
 */
static inline uint16_t syn_crc16_modbus(const void *data, size_t len)
{
    return syn_crc16_modbus_update(SYN_CRC16_MODBUS_INIT, data, len);
}

/* ── CRC-32 (Ethernet / zlib) ───────────────────────────────────────────── */
/* Polynomial: 0x04C11DB7 (reflected: 0xEDB88320)                           */

/** @brief CRC-32 initial value. */
#define SYN_CRC32_INIT  0xFFFFFFFFu

/**
 * @brief Update CRC-32 with a block of data.
 * @param crc   Running CRC value (start with SYN_CRC32_INIT).
 * @param data  Data buffer.
 * @param len   Length in bytes.
 * @return Updated CRC-32 (call syn_crc32_final to finalize).
 */
uint32_t syn_crc32_update(uint32_t crc, const void *data, size_t len);

/**
 * @brief Finalize CRC-32 (XOR with 0xFFFFFFFF).
 * @param crc  Running CRC-32 value.
 * @return Final CRC-32.
 */
static inline uint32_t syn_crc32_final(uint32_t crc)
{
    return crc ^ 0xFFFFFFFFu;
}

/**
 * @brief Compute CRC-32 of a complete buffer.
 * @param data  Data buffer.
 * @param len   Length in bytes.
 * @return CRC-32 value.
 */
static inline uint32_t syn_crc32(const void *data, size_t len)
{
    return syn_crc32_final(syn_crc32_update(SYN_CRC32_INIT, data, len));
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_CRC_H */
