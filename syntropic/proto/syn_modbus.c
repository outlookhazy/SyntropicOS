#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_MODBUS) || SYN_USE_MODBUS

/**
 * @file syn_modbus.c
 * @brief Modbus RTU slave implementation.
 */

#include "syn_modbus.h"
#include "../util/syn_assert.h"

#include <string.h>

/* ── Constants ──────────────────────────────────────────────────────────── */

/*
 * Inter-frame silence: 3.5 character times.
 * At 9600 baud: ~4ms. We use a conservative 5ms minimum.
 * At higher baud rates it's shorter, but we still need a minimum.
 */
#define MB_SILENCE_MS      5    /**< Inter-frame silence (ms).       */
#define MB_MIN_FRAME_LEN   4    /**< Minimum: addr + func + CRC16.   */
#define MB_MAX_PDU_DATA    252  /**< Max PDU data bytes.             */

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Read big-endian uint16 from a byte buffer.
 * @param buf  Source buffer.
 * @return Decoded value.
 */
static uint16_t read_u16(const uint8_t *buf)
{
    return (uint16_t)((buf[0] << 8) | buf[1]);
}

/**
 * @brief Write big-endian uint16 to a byte buffer.
 * @param buf  Destination buffer.
 * @param val  Value to encode.
 */
static void write_u16(uint8_t *buf, uint16_t val)
{
    buf[0] = (uint8_t)(val >> 8);
    buf[1] = (uint8_t)(val & 0xFF);
}

/**
 * @brief Verify the CRC-16 Modbus trailer.
 * @param buf  Frame buffer.
 * @param len  Total frame length (including CRC).
 * @return true if CRC matches.
 */
static bool check_crc(const uint8_t *buf, uint16_t len)
{
    if (len < 2) return false;
    uint16_t received = (uint16_t)(buf[len - 1] << 8 | buf[len - 2]);
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
    buf[len]     = (uint8_t)(crc & 0xFF);         /* CRC low */
    buf[len + 1] = (uint8_t)((crc >> 8) & 0xFF);  /* CRC high */
}

/* ── Send helpers ───────────────────────────────────────────────────────── */

/**
 * @brief Append CRC and transmit a response frame.
 * @param mb   Modbus instance.
 * @param len  PDU length (excluding CRC).
 */
static void send_response(SYN_Modbus *mb, uint16_t len)
{
    append_crc(mb->buf, len);
    syn_port_uart_transmit(mb->cfg.uart, mb->buf, len + 2, 100);
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
    mb->buf[0] = mb->cfg.slave_addr;
    mb->buf[1] = (uint8_t)(func | 0x80);
    mb->buf[2] = ex_code;
    send_response(mb, 3);
    mb->errors++;
}

/* ── Function code handlers ─────────────────────────────────────────────── */

/**
 * @brief Handle Modbus read holding/input register request.
 * @param mb         Modbus instance.
 * @param regs       Register array.
 * @param reg_count  Total register count.
 */
static void handle_read_regs(SYN_Modbus *mb, const uint16_t *regs,
                             uint16_t reg_count)
{
    uint16_t addr  = read_u16(&mb->buf[2]);
    uint16_t count = read_u16(&mb->buf[4]);

    if (count == 0 || count > 125) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    if ((uint32_t)addr + count > reg_count) {
        send_exception(mb, mb->buf[1], SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    /* Build response */
    mb->buf[2] = (uint8_t)(count * 2); /* byte count */
    for (uint16_t i = 0; i < count; i++) {
        write_u16(&mb->buf[3 + i * 2], regs[addr + i]);
    }
    send_response(mb, (uint16_t)(3 + count * 2));
}

/**
 * @brief Handle Modbus write single register request.
 * @param mb  Modbus instance.
 */
static void handle_write_single(SYN_Modbus *mb)
{
    uint16_t addr  = read_u16(&mb->buf[2]);
    uint16_t value = read_u16(&mb->buf[4]);

    if (addr >= mb->cfg.holding_count) {
        send_exception(mb, SYN_MB_FC_WRITE_SINGLE, SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    /* Check write callback */
    if (mb->cfg.on_write != NULL) {
        if (!mb->cfg.on_write(mb, addr, 1, mb->cfg.on_write_ctx)) {
            send_exception(mb, SYN_MB_FC_WRITE_SINGLE,
                           SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }
    }

    mb->cfg.holding_regs[addr] = value;

    /* Echo request as response */
    send_response(mb, 6);
}

/**
 * @brief Handle Modbus write multiple registers request.
 * @param mb  Modbus instance.
 */
static void handle_write_multiple(SYN_Modbus *mb)
{
    uint16_t addr  = read_u16(&mb->buf[2]);
    uint16_t count = read_u16(&mb->buf[4]);
    uint8_t  bytes = mb->buf[6];

    if (count == 0 || count > 123 || bytes != count * 2) {
        send_exception(mb, SYN_MB_FC_WRITE_MULTIPLE,
                       SYN_MB_EX_ILLEGAL_VALUE);
        return;
    }

    if ((uint32_t)addr + count > mb->cfg.holding_count) {
        send_exception(mb, SYN_MB_FC_WRITE_MULTIPLE,
                       SYN_MB_EX_ILLEGAL_ADDR);
        return;
    }

    /* Check write callback */
    if (mb->cfg.on_write != NULL) {
        if (!mb->cfg.on_write(mb, addr, count, mb->cfg.on_write_ctx)) {
            send_exception(mb, SYN_MB_FC_WRITE_MULTIPLE,
                           SYN_MB_EX_ILLEGAL_VALUE);
            return;
        }
    }

    /* Apply writes */
    for (uint16_t i = 0; i < count; i++) {
        mb->cfg.holding_regs[addr + i] = read_u16(&mb->buf[7 + i * 2]);
    }

    /* Response: addr, func, start addr, quantity */
    mb->buf[0] = mb->cfg.slave_addr;
    mb->buf[1] = SYN_MB_FC_WRITE_MULTIPLE;
    write_u16(&mb->buf[2], addr);
    write_u16(&mb->buf[4], count);
    send_response(mb, 6);
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_modbus_init(SYN_Modbus *mb, const SYN_Modbus_Config *cfg,
                      uint8_t *buf, uint16_t buf_size)
{
    SYN_ASSERT(mb != NULL);
    SYN_ASSERT(cfg != NULL);
    SYN_ASSERT(buf != NULL);
    SYN_ASSERT(buf_size >= MB_MIN_FRAME_LEN);

    memset(mb, 0, sizeof(*mb));
    mb->cfg      = *cfg;
    mb->buf      = buf;
    mb->buf_size = buf_size;
}

void syn_modbus_feed(SYN_Modbus *mb, uint8_t byte)
{
    SYN_ASSERT(mb != NULL);

    uint32_t now = syn_port_get_tick_ms();

    /* Detect inter-frame gap (3.5 char times) */
    if (mb->rx_len > 0 && (now - mb->last_byte_tick) >= MB_SILENCE_MS) {
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

bool syn_modbus_process(SYN_Modbus *mb)
{
    SYN_ASSERT(mb != NULL);

    if (mb->rx_len < MB_MIN_FRAME_LEN) {
        mb->rx_len = 0;
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

    /* Check slave address (0 = broadcast, we process but don't respond) */
    uint8_t addr = mb->buf[0];
    bool is_broadcast = (addr == 0);
    if (!is_broadcast && addr != mb->cfg.slave_addr) {
        return false; /* not for us */
    }

    uint8_t func = mb->buf[1];

    switch (func) {
    case SYN_MB_FC_READ_HOLDING:
        if (is_broadcast) break;
        handle_read_regs(mb, mb->cfg.holding_regs, mb->cfg.holding_count);
        return true;

    case SYN_MB_FC_READ_INPUT:
        if (is_broadcast) break;
        handle_read_regs(mb, mb->cfg.input_regs, mb->cfg.input_count);
        return true;

    case SYN_MB_FC_WRITE_SINGLE:
        handle_write_single(mb);
        return !is_broadcast;

    case SYN_MB_FC_WRITE_MULTIPLE:
        handle_write_multiple(mb);
        return !is_broadcast;

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
    if (mb->rx_len > 0) {
        uint32_t silence = syn_port_get_tick_ms() - mb->last_byte_tick;
        if (silence >= MB_SILENCE_MS) {
            return syn_modbus_process(mb);
        }
    }

    return false;
}

void syn_modbus_reset(SYN_Modbus *mb)
{
    SYN_ASSERT(mb != NULL);
    mb->rx_len      = 0;
    mb->frame_ready = false;
}

#endif /* SYN_USE_MODBUS */
