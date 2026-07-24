

# File syn\_pack.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pack.h**](syn__pack_8h.md)

[Go to the documentation of this file](syn__pack_8h.md)


```C++

#ifndef SYN_PACK_H
#define SYN_PACK_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Big-endian pack ────────────────────────────────────────────────────── */

static inline void syn_pack_u8(uint8_t *buf, size_t *pos, uint8_t val)
{
    buf[(*pos)++] = val;
}

static inline void syn_pack_i8(uint8_t *buf, size_t *pos, int8_t val)
{
    buf[(*pos)++] = (uint8_t)val;
}

static inline void syn_pack_u16(uint8_t *buf, size_t *pos, uint16_t val)
{
    buf[(*pos)++] = (uint8_t)(val >> 8);
    buf[(*pos)++] = (uint8_t)(val);
}

static inline void syn_pack_i16(uint8_t *buf, size_t *pos, int16_t val)
{
    syn_pack_u16(buf, pos, (uint16_t)val);
}

static inline void syn_pack_u32(uint8_t *buf, size_t *pos, uint32_t val)
{
    buf[(*pos)++] = (uint8_t)(val >> 24);
    buf[(*pos)++] = (uint8_t)(val >> 16);
    buf[(*pos)++] = (uint8_t)(val >> 8);
    buf[(*pos)++] = (uint8_t)(val);
}

static inline void syn_pack_i32(uint8_t *buf, size_t *pos, int32_t val)
{
    syn_pack_u32(buf, pos, (uint32_t)val);
}

static inline void syn_pack_u64(uint8_t *buf, size_t *pos, uint64_t val)
{
    buf[(*pos)++] = (uint8_t)(val >> 56);
    buf[(*pos)++] = (uint8_t)(val >> 48);
    buf[(*pos)++] = (uint8_t)(val >> 40);
    buf[(*pos)++] = (uint8_t)(val >> 32);
    buf[(*pos)++] = (uint8_t)(val >> 24);
    buf[(*pos)++] = (uint8_t)(val >> 16);
    buf[(*pos)++] = (uint8_t)(val >> 8);
    buf[(*pos)++] = (uint8_t)(val);
}

static inline void syn_pack_bytes(uint8_t *buf, size_t *pos,
                                    const uint8_t *data, size_t len)
{
    memcpy(buf + *pos, data, len);
    *pos += len;
}

/* ── Big-endian unpack ──────────────────────────────────────────────────── */

static inline uint8_t syn_unpack_u8(const uint8_t *buf, size_t *pos)
{
    return buf[(*pos)++];
}

static inline int8_t syn_unpack_i8(const uint8_t *buf, size_t *pos)
{
    return (int8_t)buf[(*pos)++];
}

static inline uint16_t syn_unpack_u16(const uint8_t *buf, size_t *pos)
{
    uint16_t val = (uint16_t)((uint16_t)buf[*pos] << 8) |
                   (uint16_t)buf[*pos + 1];
    *pos += 2;
    return val;
}

static inline int16_t syn_unpack_i16(const uint8_t *buf, size_t *pos)
{
    return (int16_t)syn_unpack_u16(buf, pos);
}

static inline uint32_t syn_unpack_u32(const uint8_t *buf, size_t *pos)
{
    uint32_t val = ((uint32_t)buf[*pos]     << 24) |
                   ((uint32_t)buf[*pos + 1] << 16) |
                   ((uint32_t)buf[*pos + 2] << 8)  |
                   ((uint32_t)buf[*pos + 3]);
    *pos += 4;
    return val;
}

static inline int32_t syn_unpack_i32(const uint8_t *buf, size_t *pos)
{
    return (int32_t)syn_unpack_u32(buf, pos);
}

static inline uint64_t syn_unpack_u64(const uint8_t *buf, size_t *pos)
{
    uint64_t val = ((uint64_t)buf[*pos]     << 56) |
                   ((uint64_t)buf[*pos + 1] << 48) |
                   ((uint64_t)buf[*pos + 2] << 40) |
                   ((uint64_t)buf[*pos + 3] << 32) |
                   ((uint64_t)buf[*pos + 4] << 24) |
                   ((uint64_t)buf[*pos + 5] << 16) |
                   ((uint64_t)buf[*pos + 6] << 8)  |
                   ((uint64_t)buf[*pos + 7]);
    *pos += 8;
    return val;
}

static inline void syn_unpack_bytes(const uint8_t *buf, size_t *pos,
                                      uint8_t *out, size_t len)
{
    memcpy(out, buf + *pos, len);
    *pos += len;
}

/* ── Little-endian pack ─────────────────────────────────────────────────── */

static inline void syn_pack_u16_le(uint8_t *buf, size_t *pos, uint16_t val)
{
    buf[(*pos)++] = (uint8_t)(val);
    buf[(*pos)++] = (uint8_t)(val >> 8);
}

static inline void syn_pack_i16_le(uint8_t *buf, size_t *pos, int16_t val)
{
    syn_pack_u16_le(buf, pos, (uint16_t)val);
}

static inline void syn_pack_u32_le(uint8_t *buf, size_t *pos, uint32_t val)
{
    buf[(*pos)++] = (uint8_t)(val);
    buf[(*pos)++] = (uint8_t)(val >> 8);
    buf[(*pos)++] = (uint8_t)(val >> 16);
    buf[(*pos)++] = (uint8_t)(val >> 24);
}

static inline void syn_pack_i32_le(uint8_t *buf, size_t *pos, int32_t val)
{
    syn_pack_u32_le(buf, pos, (uint32_t)val);
}

static inline void syn_pack_u64_le(uint8_t *buf, size_t *pos, uint64_t val)
{
    buf[(*pos)++] = (uint8_t)(val);
    buf[(*pos)++] = (uint8_t)(val >> 8);
    buf[(*pos)++] = (uint8_t)(val >> 16);
    buf[(*pos)++] = (uint8_t)(val >> 24);
    buf[(*pos)++] = (uint8_t)(val >> 32);
    buf[(*pos)++] = (uint8_t)(val >> 40);
    buf[(*pos)++] = (uint8_t)(val >> 48);
    buf[(*pos)++] = (uint8_t)(val >> 56);
}

/* ── Little-endian unpack ───────────────────────────────────────────────── */

static inline uint16_t syn_unpack_u16_le(const uint8_t *buf, size_t *pos)
{
    uint16_t val = (uint16_t)buf[*pos] |
                   (uint16_t)((uint16_t)buf[*pos + 1] << 8);
    *pos += 2;
    return val;
}

static inline int16_t syn_unpack_i16_le(const uint8_t *buf, size_t *pos)
{
    return (int16_t)syn_unpack_u16_le(buf, pos);
}

static inline uint32_t syn_unpack_u32_le(const uint8_t *buf, size_t *pos)
{
    uint32_t val = ((uint32_t)buf[*pos])          |
                   ((uint32_t)buf[*pos + 1] << 8) |
                   ((uint32_t)buf[*pos + 2] << 16) |
                   ((uint32_t)buf[*pos + 3] << 24);
    *pos += 4;
    return val;
}

static inline int32_t syn_unpack_i32_le(const uint8_t *buf, size_t *pos)
{
    return (int32_t)syn_unpack_u32_le(buf, pos);
}

static inline uint64_t syn_unpack_u64_le(const uint8_t *buf, size_t *pos)
{
    uint64_t val = ((uint64_t)buf[*pos])          |
                   ((uint64_t)buf[*pos + 1] << 8) |
                   ((uint64_t)buf[*pos + 2] << 16) |
                   ((uint64_t)buf[*pos + 3] << 24) |
                   ((uint64_t)buf[*pos + 4] << 32) |
                   ((uint64_t)buf[*pos + 5] << 40) |
                   ((uint64_t)buf[*pos + 6] << 48) |
                   ((uint64_t)buf[*pos + 7] << 56);
    *pos += 8;
    return val;
}

/* ── Peek (read without advancing position) ─────────────────────────────── */

static inline uint8_t syn_peek_u8(const uint8_t *buf, size_t pos)
{
    return buf[pos];
}

static inline uint16_t syn_peek_u16(const uint8_t *buf, size_t pos)
{
    return (uint16_t)((uint16_t)buf[pos] << 8) | (uint16_t)buf[pos + 1];
}

static inline uint32_t syn_peek_u32(const uint8_t *buf, size_t pos)
{
    return ((uint32_t)buf[pos]     << 24) |
           ((uint32_t)buf[pos + 1] << 16) |
           ((uint32_t)buf[pos + 2] << 8)  |
           ((uint32_t)buf[pos + 3]);
}

static inline void syn_poke_u16(uint16_t val, uint8_t *buf, size_t pos)
{
    buf[pos]     = (uint8_t)((val >> 8) & 0xFF);
    buf[pos + 1] = (uint8_t)(val & 0xFF);
}

static inline void syn_poke_u32(uint32_t val, uint8_t *buf, size_t pos)
{
    buf[pos]     = (uint8_t)((val >> 24) & 0xFF);
    buf[pos + 1] = (uint8_t)((val >> 16) & 0xFF);
    buf[pos + 2] = (uint8_t)((val >> 8) & 0xFF);
    buf[pos + 3] = (uint8_t)(val & 0xFF);
}

static inline uint16_t syn_peek_u16_le(const uint8_t *buf, size_t pos)
{
    return (uint16_t)buf[pos] | (uint16_t)((uint16_t)buf[pos + 1] << 8);
}

static inline uint32_t syn_peek_u32_le(const uint8_t *buf, size_t pos)
{
    return ((uint32_t)buf[pos]) |
           ((uint32_t)buf[pos + 1] << 8)  |
           ((uint32_t)buf[pos + 2] << 16) |
           ((uint32_t)buf[pos + 3] << 24);
}

static inline void syn_poke_u16_le(uint16_t val, uint8_t *buf, size_t pos)
{
    buf[pos]     = (uint8_t)(val & 0xFF);
    buf[pos + 1] = (uint8_t)((val >> 8) & 0xFF);
}

static inline void syn_poke_u32_le(uint32_t val, uint8_t *buf, size_t pos)
{
    buf[pos]     = (uint8_t)(val & 0xFF);
    buf[pos + 1] = (uint8_t)((val >> 8) & 0xFF);
    buf[pos + 2] = (uint8_t)((val >> 16) & 0xFF);
    buf[pos + 3] = (uint8_t)((val >> 24) & 0xFF);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PACK_H */
```


