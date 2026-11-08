#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_MODBUS) || SYN_USE_MODBUS

/**
 * @file syn_modbus.c
 * @brief Modbus RTU slave implementation.
 */

#include "../util/syn_assert.h"
#include "syn_modbus.h"

#include <string.h>

/* ── Constants ──────────────────────────────────────────────────────────── */

/*
 * Inter-frame silence: 3.5 character times.
 * At 9600 baud: ~4ms. We use a conservative 5ms minimum.
 * At higher baud rates it's shorter, but we still need a minimum.
 */
#define MB_SILENCE_MS 5     /**< Inter-frame silence (ms).       */
#define MB_MIN_FRAME_LEN 4  /**< Minimum: addr + func + CRC16.   */
#define MB_MAX_PDU_DATA 252 /**< Max PDU data bytes.             */

#include "../util/syn_pack.h"

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Read big-endian 16-bit unsigned integer from buffer.
 * @param buf Pointer to 2-byte buffer.
 * @return 16-bit value.
 */
static inline uint16_t read_u16(const uint8_t *buf)
{
    return syn_peek_u16(buf, 0);
}

/**
 * @brief Write big-endian 16-bit unsigned integer into buffer.
 * @param buf Output buffer.
 * @param val 16-bit value to write.
 */
static inline void write_u16(uint8_t *buf, uint16_t val)
{
    size_t pos = 0;
    syn_pack_u16(buf, &pos, val);
}

/**
 * @brief Verify the CRC-16 Modbus trailer.
 * @param buf  Frame buffer.
 * @param len  Total frame length (including CRC).
 * @return true if CRC matches.
 */
static bool check_crc(const uint8_t *buf, uint16_t len)
{
    if (len < 2)
        return false; /* LCOV_EXCL_LINE: Unreachable guard; caller verifies frame_len >= 4 before
                         CRC verification */
    uint16_t received = syn_peek_u16_le(buf, len - 2);
    uint16_t computed = syn_crc16_modbus(buf, len - 2);
    return received == computed;
}

/**
 * @brief Append CRC-16 Modbus to a frame buffer.
 * @param buf  Frame buffer.
 * @param len  Frame length (excluding CRC).
 */
static void append_crc(uint8_t *buf, uint16_t len)
{
    uint16_t crc = syn_crc16_modbus(buf, len);
    syn_poke_u16_le(crc, buf, len);
}

/* ── Send helpers ───────────────────────────────────────────────────────── */

/**
 * @brief Append CRC and transmit a response frame.
 * @param mb   Modbus instance.
 * @param len  PDU length (excluding CRC).
 */
static void send_response(SYN_Modbus *mb, uint16_t len)
{
    if (mb->buf[0] == 0) {
        return; /* Do not respond to broadcast */
    }
    append_crc(mb->buf, len);
    if (!mb->suppress_uart_tx) {
        syn_port_uart_transmit(mb->cfg.uart, mb->buf, len + 2U, 0);
    }
    mb->frames_tx++;
}

/**
 * @brief Send a Modbus exception response.
 * @param mb       Modbus instance.
 * @param func     Function code that caused the exception.
 * @param ex_code  Exception code.
 */
static void send_exception(SYN_Modbus *mb, uint8_t func, uint8_t ex_code)
{
    if (mb->buf[0] == 0U || mb->buf_size < 5U) {
        mb->errors++;
        return; /* Do not respond to broadcast or runt TX buffers */
    }
    mb->buf[0] = mb->cfg.slave_addr;
    mb->buf[1] = (uint8_t)(func | 0x80U);
    mb->buf[2] = ex_code;
    send_response(mb, 3U);
    mb->errors++;
}

/**
 * @brief Verify buffer capacity for RTU response payload plus 2-byte CRC.
 * @param mb   Modbus instance.
 * @param func Function code for exception fallback.
 * @param len  Response length before 2-byte CRC.
 * @return true if buffer fits len + 2 bytes, false if exception sent or dropped.
 */
static bool mb_check_tx_cap(SYN_Modbus *mb, uint8_t func, uint16_t len)
{
    if ((uint32_t)len + 2U > (uint32_t)mb->buf_size) {
        send_exception(mb, func, SYN_MB_EX_DEVICE_FAILURE);
        return false;
    }
    return true;
}

/* ── Function code handlers ─────────────────────────────────────────────── */

/**
 * @brief Handle Modbus read holding/input register request.
 * @param mb         Modbus instance.
 * @param frame_len  Received frame length.
 * @param regs       Register array.
 * @param reg_count  Total register count.
 */
static void handle_read_regs(SYN_Modbus *mb, uint16_t frame_len, const uint16_t *regs,
                             uint16_t reg_count)
{
    if (frame_len < 8U) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t addr = read_u16(&mb->buf[2]);
    uint16_t count = read_u16(&mb->buf[4]);

    if (count == 0U || count > 125U) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    if (regs == NULL || (uint32_t)addr + count > reg_count) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    uint16_t resp_len = (uint16_t)(3U + count * 2U);
    if (!mb_check_tx_cap(mb, mb->buf[1], resp_len)) {
        return;
    }

    /* Build response */
    mb->buf[2] = (uint8_t)(count * 2U); /* byte count */
    for (uint16_t i = 0U; i < count; i++) {
        write_u16(&mb->buf[3U + i * 2U], regs[addr + i]);
    }
    send_response(mb, resp_len);
}

/**
 * @brief Handle Modbus write single register request.
 * @param mb         Modbus instance.
 * @param frame_len  Received frame length.
 */
static void handle_write_single(SYN_Modbus *mb, uint16_t frame_len)
{
    if (frame_len < 8U) {
        send_exception(mb, SYN_MB_FC_WRITE_SINGLE, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t addr = read_u16(&mb->buf[2]);
    uint16_t value = read_u16(&mb->buf[4]);

    if (mb->cfg.holding_regs == NULL || addr >= mb->cfg.holding_count) {
        send_exception(mb, SYN_MB_FC_WRITE_SINGLE, SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    /* Check write callback */
    if (mb->cfg.on_write != NULL) {
        if (!mb->cfg.on_write(mb, addr, 1U, mb->cfg.on_write_ctx)) {
            send_exception(mb, SYN_MB_FC_WRITE_SINGLE, SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }
    }

    mb->cfg.holding_regs[addr] = value;

    /* Echo request as response */
    send_response(mb, 6U);
}

/**
 * @brief Handle Modbus write multiple registers request.
 * @param mb         Modbus instance.
 * @param frame_len  Received frame length.
 */
static void handle_write_multiple(SYN_Modbus *mb, uint16_t frame_len)
{
    if (frame_len < 9U) {
        send_exception(mb, SYN_MB_FC_WRITE_MULTIPLE, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t addr = read_u16(&mb->buf[2]);
    uint16_t count = read_u16(&mb->buf[4]);
    uint8_t bytes = mb->buf[6];

    if (count == 0U || count > 123U || bytes != count * 2U || frame_len < (uint16_t)(9U + bytes)) {
        send_exception(mb, SYN_MB_FC_WRITE_MULTIPLE, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    if (mb->cfg.holding_regs == NULL || (uint32_t)addr + count > mb->cfg.holding_count) {
        send_exception(mb, SYN_MB_FC_WRITE_MULTIPLE, SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    /* Check write callback */
    if (mb->cfg.on_write != NULL) {
        if (!mb->cfg.on_write(mb, addr, count, mb->cfg.on_write_ctx)) {
            send_exception(mb, SYN_MB_FC_WRITE_MULTIPLE, SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }
    }

    /* Apply writes */
    for (uint16_t i = 0U; i < count; i++) {
        mb->cfg.holding_regs[addr + i] = read_u16(&mb->buf[7U + i * 2U]);
    }

    /* Response: addr, func, start addr, quantity */
    send_response(mb, 6U);
}

/**
 * @brief Handle Modbus read exception status (FC 0x07).
 * @param mb Modbus instance.
 */
static void handle_read_exception_status(SYN_Modbus *mb)
{
    if (!mb_check_tx_cap(mb, SYN_MB_FC_READ_EXCEPTION_STATUS, 3U)) {
        return;
    }
    mb->buf[2] = mb->cfg.exception_status;
    send_response(mb, 3U);
}

/**
 * @brief Validate quantity and byte-count fields for FC 0x17 request.
 * @param frame_len   Received RTU frame length.
 * @param read_count  Requested read register count.
 * @param write_count Requested write register count.
 * @param write_bytes Byte count field from request.
 * @return true if counts and frame length are valid, false otherwise.
 */
static bool mb_validate_fc17_counts(uint16_t frame_len, uint16_t read_count, uint16_t write_count,
                                    uint8_t write_bytes)
{
    if (read_count == 0U || read_count > 125U || write_count == 0U || write_count > 121U) {
        return false;
    }
    return (write_bytes == (uint8_t)(write_count * 2U)) &&
           (frame_len >= (uint16_t)(13U + write_bytes));
}

/**
 * @brief Handle Modbus read/write multiple registers (FC 0x17).
 * @param mb         Modbus instance.
 * @param frame_len  Received frame length.
 */
static void handle_read_write_multiple(SYN_Modbus *mb, uint16_t frame_len)
{
    /* Minimum: addr(1) func(1) read_addr(2) read_count(2) write_addr(2) write_count(2)
     *          write_bytes(1) CRC(2) = 13 bytes */
    if (frame_len < 13U) {
        send_exception(mb, SYN_MB_FC_READ_WRITE_MULTIPLE, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t read_addr = read_u16(&mb->buf[2]);
    uint16_t read_count = read_u16(&mb->buf[4]);
    uint16_t write_addr = read_u16(&mb->buf[6]);
    uint16_t write_count = read_u16(&mb->buf[8]);
    uint8_t write_bytes = mb->buf[10];

    if (!mb_validate_fc17_counts(frame_len, read_count, write_count, write_bytes)) {
        send_exception(mb, SYN_MB_FC_READ_WRITE_MULTIPLE, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    if (mb->cfg.holding_regs == NULL || (uint32_t)read_addr + read_count > mb->cfg.holding_count ||
        (uint32_t)write_addr + write_count > mb->cfg.holding_count) {
        send_exception(mb, SYN_MB_FC_READ_WRITE_MULTIPLE, SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    uint16_t resp_len = (uint16_t)(3U + read_count * 2U);
    if (!mb_check_tx_cap(mb, SYN_MB_FC_READ_WRITE_MULTIPLE, resp_len)) {
        return;
    }

    if (mb->cfg.on_write != NULL) {
        if (!mb->cfg.on_write(mb, write_addr, write_count, mb->cfg.on_write_ctx)) {
            send_exception(mb, SYN_MB_FC_READ_WRITE_MULTIPLE, SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }
    }

    /* Write performed first according to spec */
    for (uint16_t i = 0U; i < write_count; i++) {
        mb->cfg.holding_regs[write_addr + i] = read_u16(&mb->buf[11U + i * 2U]);
    }

    /* Read performed second */
    mb->buf[2] = (uint8_t)(read_count * 2U);
    for (uint16_t i = 0U; i < read_count; i++) {
        write_u16(&mb->buf[3U + i * 2U], mb->cfg.holding_regs[read_addr + i]);
    }
    send_response(mb, resp_len);
}

/**
 * @brief Resolve device identification string for a given MEI object ID.
 * @param info Configured device identification table (may be NULL).
 * @param id   Object identifier (0x00..0x06).
 * @return Pointer to object string, or NULL if unpopulated optional object.
 */
static const char *mb_get_device_info_str(const SYN_Modbus_DeviceInfo *info, uint8_t id)
{
    const char *str = NULL;
    if (info != NULL) {
        switch (id) {
        case 0x00U:
            str = info->vendor_name;
            break;
        case 0x01U:
            str = info->product_code;
            break;
        case 0x02U:
            str = info->revision;
            break;
        case 0x03U:
            str = info->vendor_url;
            break;
        case 0x04U:
            str = info->product_name;
            break;
        case 0x05U:
            str = info->model_name;
            break;
        default:
            str = info->user_app_name;
            break;
        }
    }
    if (str == NULL) {
        if (id == 0x00U) {
            str = "SyntropicOS";
        } else if (id == 0x01U) {
            str = "SYN-MB";
        } else if (id == 0x02U) {
            str = "1.0.0";
        }
    }
    return str;
}

/**
 * @brief Handle Modbus read device identification (FC 0x2B / MEI 0x0E).
 * @param mb         Modbus instance.
 * @param frame_len  Received frame length.
 */
static void handle_read_device_info(SYN_Modbus *mb, uint16_t frame_len)
{
    if (frame_len < 7U || mb->buf[2] != SYN_MB_MEI_TYPE_READ_DEVICE_ID) {
        send_exception(mb, SYN_MB_FC_READ_DEVICE_INFO, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint8_t read_code = mb->buf[3];
    uint8_t object_id = mb->buf[4];

    if (read_code < 1U || read_code > 4U) {
        send_exception(mb, SYN_MB_FC_READ_DEVICE_INFO, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    if (!mb_check_tx_cap(mb, SYN_MB_FC_READ_DEVICE_INFO, 8U)) {
        return;
    }

    mb->buf[2] = SYN_MB_MEI_TYPE_READ_DEVICE_ID;
    mb->buf[3] = read_code;
    mb->buf[4] = 0x01U; /* Conformity level: Basic stream */
    mb->buf[5] = 0x00U; /* More follows: 0 */
    mb->buf[6] = 0x00U; /* Next object id */

    uint16_t pos = 8U;
    uint8_t obj_count = 0U;

    uint16_t start_obj = (read_code == 0x04U) ? (uint16_t)object_id : 0U;
    uint16_t max_obj = (read_code == 0x01U) ? 2U : 6U;
    if (read_code == 0x04U) {
        max_obj = (uint16_t)object_id;
    }

    for (uint16_t id = start_obj; id <= max_obj; id++) {
        const char *str =
            (id <= 0x06U) ? mb_get_device_info_str(mb->cfg.device_info, (uint8_t)id) : NULL;
        if (str == NULL) {
            continue;
        }

        size_t slen = strlen(str);
        if (slen > 245U) {
            slen = 245U;
        }

        if ((uint32_t)pos + 2U + (uint32_t)slen + 2U > (uint32_t)mb->buf_size) {
            break;
        }

        mb->buf[pos++] = (uint8_t)id;
        mb->buf[pos++] = (uint8_t)slen;
        memcpy(&mb->buf[pos], str, slen);
        pos = (uint16_t)(pos + slen);
        obj_count++;
    }

    mb->buf[7] = obj_count;
    send_response(mb, pos);
}

/**
 * @brief Compute clamped maximum response position (addr + PDU) for FC 0x14.
 * @param buf_size  Configured RTU buffer size.
 * @param stack_cap Size of local response staging buffer.
 * @return Maximum safe response byte offset before 2-byte CRC (capped at 253).
 */
static uint16_t mb_file_record_max_pos(uint16_t buf_size, uint16_t stack_cap)
{
    uint16_t min_cap = (buf_size < stack_cap) ? buf_size : stack_cap;
    uint16_t max_pos = (uint16_t)(min_cap - 2U);
    return (max_pos > 253U) ? 253U : max_pos;
}

/**
 * @brief Handle Modbus read file record (FC 0x14).
 * @param mb         Modbus instance.
 * @param frame_len  Received frame length.
 */
static void handle_read_file_record(SYN_Modbus *mb, uint16_t frame_len)
{
    if (frame_len < 5U) {
        send_exception(mb, SYN_MB_FC_READ_FILE_RECORD, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint8_t byte_count = mb->buf[2];
    if (byte_count < 7U || (byte_count % 7U) != 0U || frame_len < (uint16_t)(5U + byte_count) ||
        mb->cfg.on_read_file == NULL) {
        send_exception(mb, SYN_MB_FC_READ_FILE_RECORD, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint8_t sub_req_count = byte_count / 7U;
    uint8_t req_pos = 3U;

    uint8_t resp_buf[256];
    uint16_t resp_pos = 3U;
    uint16_t max_resp_pos = mb_file_record_max_pos(mb->buf_size, (uint16_t)sizeof(resp_buf));

    for (uint8_t i = 0U; i < sub_req_count; i++) {
        uint8_t ref_type = mb->buf[req_pos];
        uint16_t file_num = read_u16(&mb->buf[req_pos + 1U]);
        uint16_t rec_num = read_u16(&mb->buf[req_pos + 3U]);
        uint16_t rec_len = read_u16(&mb->buf[req_pos + 5U]);
        req_pos = (uint8_t)(req_pos + 7U);

        if (ref_type != 0x06U || rec_len == 0U || rec_len > 120U) {
            send_exception(mb, SYN_MB_FC_READ_FILE_RECORD, SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }

        uint16_t rec_data[120];
        if (!mb->cfg.on_read_file(mb, file_num, rec_num, rec_len, rec_data, mb->cfg.file_cb_ctx)) {
            send_exception(mb, SYN_MB_FC_READ_FILE_RECORD, SYN_MB_EX_ILLEGAL_ADDR);
            return;
        }

        uint8_t sub_resp_len = (uint8_t)(1U + rec_len * 2U);
        if ((uint32_t)resp_pos + 2U + (uint32_t)rec_len * 2U > (uint32_t)max_resp_pos) {
            send_exception(mb, SYN_MB_FC_READ_FILE_RECORD, SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }

        resp_buf[resp_pos++] = sub_resp_len;
        resp_buf[resp_pos++] = 0x06U;
        for (uint16_t j = 0U; j < rec_len; j++) {
            write_u16(&resp_buf[resp_pos], rec_data[j]);
            resp_pos = (uint16_t)(resp_pos + 2U);
        }
    }

    resp_buf[0] = mb->cfg.slave_addr;
    resp_buf[1] = SYN_MB_FC_READ_FILE_RECORD;
    resp_buf[2] = (uint8_t)(resp_pos - 3U);
    memcpy(mb->buf, resp_buf, resp_pos);
    send_response(mb, resp_pos);
}

/**
 * @brief Handle Modbus write file record (FC 0x15).
 * @param mb         Modbus instance.
 * @param frame_len  Received frame length.
 */
static void handle_write_file_record(SYN_Modbus *mb, uint16_t frame_len)
{
    if (frame_len < 5) {
        send_exception(mb, SYN_MB_FC_WRITE_FILE_RECORD, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint8_t data_len = mb->buf[2];
    if (data_len < 9 || frame_len < (uint16_t)(5 + data_len) || mb->cfg.on_write_file == NULL) {
        send_exception(mb, SYN_MB_FC_WRITE_FILE_RECORD, SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t pos = 3;
    uint16_t end = 3 + data_len;

    while (pos < end) {
        if (pos + 7 > end) {
            send_exception(mb, SYN_MB_FC_WRITE_FILE_RECORD, SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }
        uint8_t ref_type = mb->buf[pos];
        uint16_t file_num = read_u16(&mb->buf[pos + 1]);
        uint16_t rec_num = read_u16(&mb->buf[pos + 3]);
        uint16_t rec_len = read_u16(&mb->buf[pos + 5]);
        pos += 7;

        if (ref_type != 0x06 || rec_len == 0 || pos + rec_len * 2 > end) {
            send_exception(mb, SYN_MB_FC_WRITE_FILE_RECORD, SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }

        uint16_t rec_data[120];
        if (rec_len > 120) {
            send_exception(mb, SYN_MB_FC_WRITE_FILE_RECORD, SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }
        for (uint16_t j = 0; j < rec_len; j++) {
            rec_data[j] = read_u16(&mb->buf[pos + j * 2]);
        }
        pos += rec_len * 2;

        if (!mb->cfg.on_write_file(mb, file_num, rec_num, rec_len, rec_data, mb->cfg.file_cb_ctx)) {
            send_exception(mb, SYN_MB_FC_WRITE_FILE_RECORD, SYN_MB_EX_ILLEGAL_ADDR);
            return;
        }
    }

    send_response(mb, (uint16_t)(3 + data_len));
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_modbus_init(SYN_Modbus *mb, const SYN_Modbus_Config *cfg, uint8_t *buf, uint16_t buf_size)
{
    SYN_ASSERT(mb != NULL);
    SYN_ASSERT(cfg != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(buf_size >= MB_MIN_FRAME_LEN);

    memset(mb, 0, sizeof(*mb));
    mb->cfg = *cfg;
    mb->buf = buf;
    mb->buf_size = buf_size;
}

void syn_modbus_feed(SYN_Modbus *mb, uint8_t byte)
{
    SYN_ASSERT(mb != NULL);

    uint32_t now = syn_port_get_tick_ms();
    uint32_t silence_gap = (mb->cfg.silence_ms > 0) ? mb->cfg.silence_ms : MB_SILENCE_MS;

    /* Detect inter-frame gap (3.5 char times) */
    if (mb->rx_len > 0 && (now - mb->last_byte_tick) >= silence_gap) {
        /* Previous frame timed out — start fresh */
        mb->frame_ready = (mb->rx_len >= MB_MIN_FRAME_LEN);
        if (!mb->frame_ready) {
            mb->rx_len = 0;
        }
        /* Don't accumulate into old frame — process first */
        return;
    }

    if (mb->rx_len < mb->buf_size) {
        mb->buf[mb->rx_len++] = byte;
    }
    mb->last_byte_tick = now;
}

/**
 * @brief Helper to get bit from array.
 * @param bits Pointer to bit array.
 * @param idx Bit index.
 * @return True if bit set, false if clear.
 */
static bool get_bit(const uint8_t *bits, uint16_t idx)
{
    return (bits[idx / 8U] & (uint8_t)(1U << (idx % 8U))) != 0U;
}

/**
 * @brief Helper to set bit in array.
 * @param bits Pointer to bit array.
 * @param idx Bit index.
 * @param val Value to set (true/false).
 */
static void set_bit(uint8_t *bits, uint16_t idx, bool val)
{
    if (val) {
        bits[idx / 8U] |= (uint8_t)(1U << (idx % 8U));
    } else {
        bits[idx / 8U] &= (uint8_t)~(1U << (idx % 8U));
    }
}

/**
 * @brief Handle Read Coils (FC 0x01) and Read Discrete Inputs (FC 0x02).
 * @param mb Pointer to Modbus instance.
 * @param frame_len Length of received frame.
 * @param bits Pointer to coils or discrete inputs array.
 * @param total_bits Total available bits in array.
 */
static void handle_read_bits(SYN_Modbus *mb, uint16_t frame_len, const uint8_t *bits,
                             uint16_t total_bits)
{
    if (bits == NULL) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    if (frame_len < 8) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t addr = read_u16(&mb->buf[2]);
    uint16_t count = read_u16(&mb->buf[4]);

    if (count == 0 || count > 2000) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    if ((uint32_t)addr + count > total_bits) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    uint8_t num_bytes = (uint8_t)((count + 7U) / 8U);
    if (!mb_check_tx_cap(mb, mb->buf[1], (uint16_t)(3U + num_bytes))) {
        return;
    }
    mb->buf[2] = num_bytes;
    (void)memset(&mb->buf[3], 0, num_bytes);

    for (uint16_t i = 0; i < count; i++) {
        if (get_bit(bits, (uint16_t)(addr + i))) {
            set_bit(&mb->buf[3], i, true);
        }
    }

    send_response(mb, (uint16_t)(3 + num_bytes));
    mb->comm_event_counter++;
}

/**
 * @brief Handle Write Single Coil (FC 0x05).
 * @param mb Pointer to Modbus instance.
 * @param frame_len Length of received frame.
 */
static void handle_write_single_coil(SYN_Modbus *mb, uint16_t frame_len)
{
    if (mb->cfg.coils == NULL) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    if (frame_len < 8) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t addr = read_u16(&mb->buf[2]);
    uint16_t val = read_u16(&mb->buf[4]);

    if (val != 0xFF00U && val != 0x0000U) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    if (addr >= mb->cfg.coils_count) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    set_bit(mb->cfg.coils, addr, (val == 0xFF00U));
    send_response(mb, 6);
    mb->comm_event_counter++;
}

/**
 * @brief Handle Write Multiple Coils (FC 0x0F).
 * @param mb Pointer to Modbus instance.
 * @param frame_len Length of received frame.
 */
static void handle_write_multiple_coils(SYN_Modbus *mb, uint16_t frame_len)
{
    if (mb->cfg.coils == NULL) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    if (frame_len < 9) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t addr = read_u16(&mb->buf[2]);
    uint16_t count = read_u16(&mb->buf[4]);
    uint8_t byte_count = mb->buf[6];

    uint8_t expected_bytes = (uint8_t)((count + 7U) / 8U);
    if (count == 0 || count > 1968 || byte_count != expected_bytes ||
        frame_len < (uint16_t)(9 + byte_count)) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    if ((uint32_t)addr + count > mb->cfg.coils_count) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    for (uint16_t i = 0; i < count; i++) {
        bool b = get_bit(&mb->buf[7], i);
        set_bit(mb->cfg.coils, (uint16_t)(addr + i), b);
    }

    send_response(mb, 6);
    mb->comm_event_counter++;
}

/**
 * @brief Handle Diagnostics (FC 0x08).
 * @param mb Pointer to Modbus instance.
 * @param frame_len Length of received frame.
 */
static void handle_diagnostics(SYN_Modbus *mb, uint16_t frame_len)
{
    /* Minimum: addr(1) func(1) subfunc(2) data(2) CRC(2) = 8 bytes */
    if (frame_len < 8U) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }
    uint16_t subfunc = read_u16(&mb->buf[2]);
    if (subfunc == 0x0000U) { /* Return Query Data (Echo) */
        send_response(mb, 6);
        mb->comm_event_counter++;
    } else {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
    }
}

/**
 * @brief Handle Get Comm Event Counter (FC 0x0B).
 * @param mb Pointer to Modbus instance.
 */
static void handle_get_comm_event_cnt(SYN_Modbus *mb)
{
    if (!mb_check_tx_cap(mb, SYN_MB_FC_GET_COMM_EVENT_CNT, 6U)) {
        return;
    }
    mb->buf[2] = 0x00U;
    mb->buf[3] = 0x00U;
    write_u16(&mb->buf[4], mb->comm_event_counter);
    send_response(mb, 6U);
}

/**
 * @brief Handle Get Comm Event Log (FC 0x0C).
 * @param mb Pointer to Modbus instance.
 */
static void handle_get_comm_event_log(SYN_Modbus *mb)
{
    if (!mb_check_tx_cap(mb, SYN_MB_FC_GET_COMM_EVENT_LOG, 11U)) {
        return;
    }
    mb->buf[2] = 0x08U; /* byte count */
    mb->buf[3] = 0x00U; /* status high */
    mb->buf[4] = 0x00U; /* status low */
    write_u16(&mb->buf[5], mb->comm_event_counter);
    write_u16(&mb->buf[7], mb->bus_message_count);
    mb->buf[9] = 0x00U;  /* event log byte 0 */
    mb->buf[10] = 0x00U; /* event log byte 1 */
    send_response(mb, 11U);
}

/**
 * @brief Handle Report Server ID (FC 0x11).
 * @param mb Pointer to Modbus instance.
 */
static void handle_report_server_id(SYN_Modbus *mb)
{
    const uint8_t *id_bytes = mb->cfg.server_id;
    uint8_t id_len = mb->cfg.server_id_len;

    if (id_bytes == NULL) {
        id_bytes = (const uint8_t *)"SYN-MB";
        id_len = 6U;
    }

    if (id_len > 240U) {
        id_len = 240U;
    }

    if (!mb_check_tx_cap(mb, SYN_MB_FC_REPORT_SERVER_ID, (uint16_t)(4U + id_len))) {
        return;
    }

    mb->buf[2] = (uint8_t)(id_len + 1U); /* Byte count: ID bytes + Run Status indicator */
    memcpy(&mb->buf[3], id_bytes, id_len);
    mb->buf[3U + id_len] = 0xFFU; /* Run status: 0xFF = ON */

    send_response(mb, (uint16_t)(4U + id_len));
    mb->comm_event_counter++;
}

/**
 * @brief Handle Mask Write Register (FC 0x16).
 * @param mb Pointer to Modbus instance.
 * @param frame_len Length of received frame.
 */
static void handle_mask_write_register(SYN_Modbus *mb, uint16_t frame_len)
{
    if (mb->cfg.holding_regs == NULL) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    /* Minimum: addr(1) func(1) ref_addr(2) and_mask(2) or_mask(2) CRC(2) = 10 bytes */
    if (frame_len < 10) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t addr = read_u16(&mb->buf[2]);
    uint16_t and_mask = read_u16(&mb->buf[4]);
    uint16_t or_mask = read_u16(&mb->buf[6]);

    if (addr >= mb->cfg.holding_count) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    if (mb->cfg.on_write != NULL) {
        if (!mb->cfg.on_write(mb, addr, 1, mb->cfg.on_write_ctx)) {
            send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }
    }

    uint16_t cur = mb->cfg.holding_regs[addr];
    mb->cfg.holding_regs[addr] = (uint16_t)((cur & and_mask) | (or_mask & (uint16_t)~and_mask));
    send_response(mb, 8);
    mb->comm_event_counter++;
}

/**
 * @brief Handle Read FIFO Queue (FC 0x18).
 * @param mb         Pointer to Modbus instance.
 * @param frame_len  Length of received frame.
 */
static void handle_read_fifo_queue(SYN_Modbus *mb, uint16_t frame_len)
{
    if (frame_len < 6) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    if (mb->cfg.fifo_queue == NULL) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    uint16_t fifo_count = mb->cfg.fifo_count;
    if (fifo_count > 31U) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_DEVICE_FAILURE);
        return;
    }

    uint16_t resp_len = (uint16_t)(6U + fifo_count * 2U);
    if (!mb_check_tx_cap(mb, mb->buf[1], resp_len)) {
        return;
    }

    uint16_t byte_count = (uint16_t)(2U + fifo_count * 2U);
    write_u16(&mb->buf[2], byte_count);
    write_u16(&mb->buf[4], fifo_count);

    for (uint16_t i = 0U; i < fifo_count; i++) {
        write_u16(&mb->buf[6U + i * 2U], mb->cfg.fifo_queue[i]);
    }

    send_response(mb, resp_len);
    mb->comm_event_counter++;
}

bool syn_modbus_process(SYN_Modbus *mb)
{
    SYN_ASSERT(mb != NULL);

    if (mb->rx_len == 0) {
        return false;
    }

    uint32_t now = syn_port_get_tick_ms();
    uint32_t silence_gap = (mb->cfg.silence_ms > 0) ? mb->cfg.silence_ms : MB_SILENCE_MS;
    bool silence_elapsed = (mb->last_byte_tick == 0) || ((now - mb->last_byte_tick) >= silence_gap);

    /* Do not touch or discard partial frames while inter-frame silence gap is still running */
    if (!mb->frame_ready && !silence_elapsed) {
        return false;
    }

    if (mb->rx_len < MB_MIN_FRAME_LEN) {
        mb->rx_len = 0;
        mb->frame_ready = false;
        return false;
    }

    uint16_t len = mb->rx_len;
    mb->rx_len = 0;
    mb->frame_ready = false;

    /* Check CRC */
    if (!check_crc(mb->buf, len)) {
        mb->errors++;
        return false;
    }

    mb->frames_rx++;
    mb->bus_message_count++;

    /* Check slave address (0 = broadcast, we process but don't respond) */
    uint8_t addr = mb->buf[0];
    bool is_broadcast = (addr == 0);
    if (!is_broadcast && addr != mb->cfg.slave_addr) {
        return false; /* not for us */
    }

    uint8_t func = mb->buf[1];

    switch (func) {
    case SYN_MB_FC_READ_COILS:
        if (is_broadcast)
            break;
        handle_read_bits(mb, len, mb->cfg.coils, mb->cfg.coils_count);
        return true;

    case SYN_MB_FC_READ_DISCRETE_INPUTS:
        if (is_broadcast)
            break;
        handle_read_bits(mb, len, mb->cfg.discrete_inputs, mb->cfg.discrete_count);
        return true;

    case SYN_MB_FC_READ_HOLDING:
        if (is_broadcast)
            break;
        handle_read_regs(mb, len, mb->cfg.holding_regs, mb->cfg.holding_count);
        mb->comm_event_counter++;
        return true;

    case SYN_MB_FC_READ_INPUT:
        if (is_broadcast)
            break;
        handle_read_regs(mb, len, mb->cfg.input_regs, mb->cfg.input_count);
        mb->comm_event_counter++;
        return true;

    case SYN_MB_FC_WRITE_SINGLE_COIL:
        handle_write_single_coil(mb, len);
        return !is_broadcast;

    case SYN_MB_FC_WRITE_SINGLE:
        handle_write_single(mb, len);
        mb->comm_event_counter++;
        return !is_broadcast;

    case SYN_MB_FC_READ_EXCEPTION_STATUS:
        if (is_broadcast)
            break;
        handle_read_exception_status(mb);
        return true;

    case SYN_MB_FC_DIAGNOSTICS:
        if (is_broadcast)
            break;
        handle_diagnostics(mb, len);
        return true;

    case SYN_MB_FC_GET_COMM_EVENT_CNT:
        if (is_broadcast)
            break;
        handle_get_comm_event_cnt(mb);
        return true;

    case SYN_MB_FC_GET_COMM_EVENT_LOG:
        if (is_broadcast)
            break;
        handle_get_comm_event_log(mb);
        return true;

    case SYN_MB_FC_WRITE_MULTIPLE_COILS:
        handle_write_multiple_coils(mb, len);
        return !is_broadcast;

    case SYN_MB_FC_WRITE_MULTIPLE:
        handle_write_multiple(mb, len);
        mb->comm_event_counter++;
        return !is_broadcast;

    case SYN_MB_FC_REPORT_SERVER_ID:
        if (is_broadcast)
            break;
        handle_report_server_id(mb);
        return true;

    case SYN_MB_FC_READ_FILE_RECORD:
        if (is_broadcast)
            break;
        handle_read_file_record(mb, len);
        mb->comm_event_counter++;
        return true;

    case SYN_MB_FC_WRITE_FILE_RECORD:
        handle_write_file_record(mb, len);
        mb->comm_event_counter++;
        return !is_broadcast;

    case SYN_MB_FC_MASK_WRITE_REGISTER:
        handle_mask_write_register(mb, len);
        return !is_broadcast;

    case SYN_MB_FC_READ_WRITE_MULTIPLE:
        handle_read_write_multiple(mb, len);
        mb->comm_event_counter++;
        return !is_broadcast;

    case SYN_MB_FC_READ_FIFO_QUEUE:
        if (is_broadcast)
            break;
        handle_read_fifo_queue(mb, len);
        return true;

    case SYN_MB_FC_READ_DEVICE_INFO:
        if (is_broadcast)
            break;
        handle_read_device_info(mb, len);
        mb->comm_event_counter++;
        return true;

    default:
        if (!is_broadcast) {
            send_exception(mb, func, SYN_MB_EX_ILLEGAL_FUNC);
        }
        return false;
    }

    return false;
}

bool syn_modbus_poll(SYN_Modbus *mb)
{
    SYN_ASSERT(mb != NULL);

    /* Read available bytes */
    uint8_t byte;
    while (syn_port_uart_receive_byte(mb->cfg.uart, &byte, 0) == SYN_OK) {
        if (mb->rx_len < mb->buf_size) {
            mb->buf[mb->rx_len++] = byte;
        }
        mb->last_byte_tick = syn_port_get_tick_ms();
    }

    /* Check for inter-frame silence */
    if (mb->rx_len > 0U) {
        uint32_t silence_gap = (mb->cfg.silence_ms > 0U) ? mb->cfg.silence_ms : MB_SILENCE_MS;
        uint32_t silence = syn_port_get_tick_ms() - mb->last_byte_tick;
        if (silence >= silence_gap) {
            return syn_modbus_process(mb);
        }
    }

    return false;
}

void syn_modbus_reset(SYN_Modbus *mb)
{
    SYN_ASSERT(mb != NULL);
    mb->rx_len = 0;
    mb->frame_ready = false;
}

#endif /* SYN_USE_MODBUS */
