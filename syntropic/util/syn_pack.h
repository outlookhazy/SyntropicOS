/**
 * @file syn_pack.h
 * @brief Binary message packer / unpacker (header-only).
 *
 * Endianness-safe serialization for packing sensor telemetry,
 * config blobs, register maps, and protocol payloads.
 *
 * All functions take a buffer and a position pointer that auto-advances.
 *
 * Usage:
 * @code
 *   uint8_t buf[32];
 *   size_t pos = 0;
 *
 *   // Pack (big-endian)
 *   syn_pack_u8(buf, &pos, 0x42);
 *   syn_pack_u16(buf, &pos, 0x1234);
 *   syn_pack_i32(buf, &pos, -12345);
 *   syn_pack_bytes(buf, &pos, payload, 4);
 *
 *   // Unpack
 *   pos = 0;
 *   uint8_t  a = syn_unpack_u8(buf, &pos);
 *   uint16_t b = syn_unpack_u16(buf, &pos);
 *   int32_t  c = syn_unpack_i32(buf, &pos);
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_PACK_H
#define SYN_PACK_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Big-endian pack ────────────────────────────────────────────────────── */

/**
 * @brief Pack a uint8_t value (Big Endian).
 * @param buf Target buffer.
 * @param pos Cursor position (auto-advanced).
 * @param val Value to pack.
 */
static inline void syn_pack_u8(uint8_t *buf, size_t *pos, uint8_t val)
{
    buf[(*pos)++] = val;
}

/**
 * @brief Pack an int8_t value (Big Endian).
 * @param buf Target buffer.
 * @param pos Cursor position (auto-advanced).
 * @param val Value to pack.
 */
static inline void syn_pack_i8(uint8_t *buf, size_t *pos, int8_t val)
{
    buf[(*pos)++] = (uint8_t)val;
}

/**
 * @brief Pack a uint16_t value (Big Endian).
 * @param buf Target buffer.
 * @param pos Cursor position (auto-advanced).
 * @param val Value to pack.
 */
static inline void syn_pack_u16(uint8_t *buf, size_t *pos, uint16_t val)
{
    buf[(*pos)++] = (uint8_t)(val >> 8);
    buf[(*pos)++] = (uint8_t)(val);
}

/**
 * @brief Pack an int16_t value (Big Endian).
 * @param buf Target buffer.
 * @param pos Cursor position (auto-advanced).
 * @param val Value to pack.
 */
static inline void syn_pack_i16(uint8_t *buf, size_t *pos, int16_t val)
{
    syn_pack_u16(buf, pos, (uint16_t)val);
}

/**
 * @brief Pack a uint32_t value (Big Endian).
 * @param buf Target buffer.
 * @param pos Cursor position (auto-advanced).
 * @param val Value to pack.
 */
static inline void syn_pack_u32(uint8_t *buf, size_t *pos, uint32_t val)
{
    buf[(*pos)++] = (uint8_t)(val >> 24);
    buf[(*pos)++] = (uint8_t)(val >> 16);
    buf[(*pos)++] = (uint8_t)(val >> 8);
    buf[(*pos)++] = (uint8_t)(val);
}

/**
 * @brief Pack an int32_t value (Big Endian).
 * @param buf Target buffer.
 * @param pos Cursor position (auto-advanced).
 * @param val Value to pack.
 */
static inline void syn_pack_i32(uint8_t *buf, size_t *pos, int32_t val)
{
    syn_pack_u32(buf, pos, (uint32_t)val);
}

/**
 * @brief Pack a raw byte array.
 * @param buf  Target buffer.
 * @param pos  Cursor position (auto-advanced).
 * @param data Array to copy.
 * @param len  Number of bytes to copy.
 */
static inline void syn_pack_bytes(uint8_t *buf, size_t *pos,
                                    const uint8_t *data, size_t len)
{
    memcpy(buf + *pos, data, len);
    *pos += len;
}

/* ── Big-endian unpack ──────────────────────────────────────────────────── */

/**
 * @brief Unpack a uint8_t value (Big Endian).
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @return Unpacked value.
 */
static inline uint8_t syn_unpack_u8(const uint8_t *buf, size_t *pos)
{
    return buf[(*pos)++];
}

/**
 * @brief Unpack an int8_t value (Big Endian).
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @return Unpacked value.
 */
static inline int8_t syn_unpack_i8(const uint8_t *buf, size_t *pos)
{
    return (int8_t)buf[(*pos)++];
}

/**
 * @brief Unpack a uint16_t value (Big Endian).
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @return Unpacked value.
 */
static inline uint16_t syn_unpack_u16(const uint8_t *buf, size_t *pos)
{
    uint16_t val = (uint16_t)((uint16_t)buf[*pos] << 8) |
                   (uint16_t)buf[*pos + 1];
    *pos += 2;
    return val;
}

/**
 * @brief Unpack an int16_t value (Big Endian).
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @return Unpacked value.
 */
static inline int16_t syn_unpack_i16(const uint8_t *buf, size_t *pos)
{
    return (int16_t)syn_unpack_u16(buf, pos);
}

/**
 * @brief Unpack a uint32_t value (Big Endian).
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @return Unpacked value.
 */
static inline uint32_t syn_unpack_u32(const uint8_t *buf, size_t *pos)
{
    uint32_t val = ((uint32_t)buf[*pos]     << 24) |
                   ((uint32_t)buf[*pos + 1] << 16) |
                   ((uint32_t)buf[*pos + 2] << 8)  |
                   ((uint32_t)buf[*pos + 3]);
    *pos += 4;
    return val;
}

/**
 * @brief Unpack an int32_t value (Big Endian).
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @return Unpacked value.
 */
static inline int32_t syn_unpack_i32(const uint8_t *buf, size_t *pos)
{
    return (int32_t)syn_unpack_u32(buf, pos);
}

/**
 * @brief Unpack a raw byte array.
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @param out Output buffer to copy into.
 * @param len Number of bytes to copy.
 */
static inline void syn_unpack_bytes(const uint8_t *buf, size_t *pos,
                                      uint8_t *out, size_t len)
{
    memcpy(out, buf + *pos, len);
    *pos += len;
}

/* ── Little-endian pack ─────────────────────────────────────────────────── */

/**
 * @brief Pack a uint16_t value (Little Endian).
 * @param buf Target buffer.
 * @param pos Cursor position (auto-advanced).
 * @param val Value to pack.
 */
static inline void syn_pack_u16_le(uint8_t *buf, size_t *pos, uint16_t val)
{
    buf[(*pos)++] = (uint8_t)(val);
    buf[(*pos)++] = (uint8_t)(val >> 8);
}

/**
 * @brief Pack an int16_t value (Little Endian).
 * @param buf Target buffer.
 * @param pos Cursor position (auto-advanced).
 * @param val Value to pack.
 */
static inline void syn_pack_i16_le(uint8_t *buf, size_t *pos, int16_t val)
{
    syn_pack_u16_le(buf, pos, (uint16_t)val);
}

/**
 * @brief Pack a uint32_t value (Little Endian).
 * @param buf Target buffer.
 * @param pos Cursor position (auto-advanced).
 * @param val Value to pack.
 */
static inline void syn_pack_u32_le(uint8_t *buf, size_t *pos, uint32_t val)
{
    buf[(*pos)++] = (uint8_t)(val);
    buf[(*pos)++] = (uint8_t)(val >> 8);
    buf[(*pos)++] = (uint8_t)(val >> 16);
    buf[(*pos)++] = (uint8_t)(val >> 24);
}

/**
 * @brief Pack an int32_t value (Little Endian).
 * @param buf Target buffer.
 * @param pos Cursor position (auto-advanced).
 * @param val Value to pack.
 */
static inline void syn_pack_i32_le(uint8_t *buf, size_t *pos, int32_t val)
{
    syn_pack_u32_le(buf, pos, (uint32_t)val);
}

/* ── Little-endian unpack ───────────────────────────────────────────────── */

/**
 * @brief Unpack a uint16_t value (Little Endian).
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @return Unpacked value.
 */
static inline uint16_t syn_unpack_u16_le(const uint8_t *buf, size_t *pos)
{
    uint16_t val = (uint16_t)buf[*pos] |
                   (uint16_t)((uint16_t)buf[*pos + 1] << 8);
    *pos += 2;
    return val;
}

/**
 * @brief Unpack an int16_t value (Little Endian).
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @return Unpacked value.
 */
static inline int16_t syn_unpack_i16_le(const uint8_t *buf, size_t *pos)
{
    return (int16_t)syn_unpack_u16_le(buf, pos);
}

/**
 * @brief Unpack a uint32_t value (Little Endian).
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @return Unpacked value.
 */
static inline uint32_t syn_unpack_u32_le(const uint8_t *buf, size_t *pos)
{
    uint32_t val = ((uint32_t)buf[*pos])          |
                   ((uint32_t)buf[*pos + 1] << 8) |
                   ((uint32_t)buf[*pos + 2] << 16) |
                   ((uint32_t)buf[*pos + 3] << 24);
    *pos += 4;
    return val;
}

/**
 * @brief Unpack an int32_t value (Little Endian).
 * @param buf Source buffer.
 * @param pos Cursor position (auto-advanced).
 * @return Unpacked value.
 */
static inline int32_t syn_unpack_i32_le(const uint8_t *buf, size_t *pos)
{
    return (int32_t)syn_unpack_u32_le(buf, pos);
}

/* ── Peek (read without advancing position) ─────────────────────────────── */

/**
 * @brief Peek a uint8_t value (Big Endian) without advancing position.
 * @param buf Source buffer.
 * @param pos Source offset byte index.
 * @return Value at the offset index.
 */
static inline uint8_t syn_peek_u8(const uint8_t *buf, size_t pos)
{
    return buf[pos];
}

/**
 * @brief Peek a uint16_t value (Big Endian) without advancing position.
 * @param buf Source buffer.
 * @param pos Source offset byte index.
 * @return Value at the offset index.
 */
static inline uint16_t syn_peek_u16(const uint8_t *buf, size_t pos)
{
    return (uint16_t)((uint16_t)buf[pos] << 8) | (uint16_t)buf[pos + 1];
}

/**
 * @brief Peek a uint32_t value (Big Endian) without advancing position.
 * @param buf Source buffer.
 * @param pos Source offset byte index.
 * @return Value at the offset index.
 */
static inline uint32_t syn_peek_u32(const uint8_t *buf, size_t pos)
{
    return ((uint32_t)buf[pos]     << 24) |
           ((uint32_t)buf[pos + 1] << 16) |
           ((uint32_t)buf[pos + 2] << 8)  |
           ((uint32_t)buf[pos + 3]);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PACK_H */
