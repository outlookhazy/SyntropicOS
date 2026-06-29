/**
 * @file syn_modbus.h
 * @brief Modbus RTU slave implementation.
 *
 * Implements a Modbus RTU slave using UART, CRC-16/Modbus (already in
 * syn_crc), and a register map. Supports the most common function codes:
 *
 *   - 0x03: Read Holding Registers
 *   - 0x04: Read Input Registers
 *   - 0x06: Write Single Register
 *   - 0x10: Write Multiple Registers
 *
 * The register map is a flat array of uint16_t that your application
 * reads/writes directly. The Modbus module handles framing, CRC, and
 * exception responses.
 *
 * @par Usage
 * @code
 *   static uint16_t holding_regs[32];
 *   static uint16_t input_regs[16];
 *   static SYN_Modbus mb;
 *   static uint8_t mb_buf[256];
 *
 *   SYN_Modbus_Config cfg = {
 *       .slave_addr = 1,
 *       .uart = SYN_UART0,
 *       .holding_regs = holding_regs,
 *       .holding_count = 32,
 *       .input_regs = input_regs,
 *       .input_count = 16,
 *   };
 *   syn_modbus_init(&mb, &cfg, mb_buf, sizeof(mb_buf));
 *
 *   // In main loop:
 *   syn_modbus_poll(&mb);  // checks UART, processes requests
 *
 *   // Your app writes input registers:
 *   input_regs[0] = adc_value;
 *
 *   // Your app reads holding registers (written by master):
 *   uint16_t setpoint = holding_regs[0];
 * @endcode
 * @ingroup syn_protocol
 */

#ifndef SYN_MODBUS_H
#define SYN_MODBUS_H

#include "../common/syn_defs.h"
#include "../drivers/syn_uart.h"
#include "../util/syn_crc.h"
#include "../port/syn_port_system.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name Modbus function codes
 * @{
 */
#define SYN_MB_FC_READ_HOLDING    0x03  /**< Read holding registers       */
#define SYN_MB_FC_READ_INPUT      0x04  /**< Read input registers         */
#define SYN_MB_FC_WRITE_SINGLE    0x06  /**< Write single register        */
#define SYN_MB_FC_WRITE_MULTIPLE  0x10  /**< Write multiple registers     */
/** @} */

/** @name Modbus exception codes
 * @{
 */
#define SYN_MB_EX_ILLEGAL_FUNC    0x01  /**< Illegal function code        */
#define SYN_MB_EX_ILLEGAL_ADDR    0x02  /**< Illegal data address         */
#define SYN_MB_EX_ILLEGAL_VALUE   0x03  /**< Illegal data value           */
#define SYN_MB_EX_DEVICE_FAILURE  0x04  /**< Device failure               */
/** @} */

/* ── Callbacks ──────────────────────────────────────────────────────────── */

struct SYN_Modbus;

/**
 * @brief Called before a write to holding registers is applied.
 *
 * @param mb     Modbus instance.
 * @param addr   Register start address.
 * @param count  Number of registers.
 * @param ctx    User context.
 * @return true to allow the write, false to reject (exception 0x03).
 */
typedef bool (*SYN_Modbus_WriteCallback)(struct SYN_Modbus *mb,
                                         uint16_t addr, uint16_t count,
                                         void *ctx);

/* ── Configuration ──────────────────────────────────────────────────────── */

/** @brief Modbus RTU slave configuration. */
typedef struct {
    uint8_t          slave_addr;     /**< Modbus slave address (1–247)    */
    SYN_UARTInstance uart;          /**< UART instance to use            */

    /* Register maps (application-owned) */
    uint16_t        *holding_regs;   /**< Read/write holding registers    */
    uint16_t         holding_count;  /**< Number of holding registers     */
    uint16_t        *input_regs;     /**< Read-only input registers       */
    uint16_t         input_count;    /**< Number of input registers       */

    /* Optional */
    SYN_Modbus_WriteCallback on_write;    /**< Write pre-check callback   */
    void            *on_write_ctx;        /**< Context for on_write       */
} SYN_Modbus_Config;

/* ── Modbus instance ────────────────────────────────────────────────────── */

/** @brief Modbus RTU slave instance — frame buffer, config, and statistics. */
typedef struct SYN_Modbus {
    SYN_Modbus_Config cfg;           /**< Configuration snapshot          */

    /* Frame buffer */
    uint8_t     *buf;            /**< RX/TX buffer                       */
    uint16_t     buf_size;       /**< Buffer capacity                    */
    uint16_t     rx_len;         /**< Bytes received in current frame    */
    uint32_t     last_byte_tick; /**< Tick of last received byte         */
    bool         frame_ready;    /**< Complete frame available?           */

    /* Stats */
    uint32_t     frames_rx;      /**< Total frames received              */
    uint32_t     frames_tx;      /**< Total frames sent                  */
    uint32_t     errors;         /**< CRC/framing error count            */
} SYN_Modbus;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the Modbus RTU slave.
 *
 * @param mb       Modbus instance.
 * @param cfg      Configuration.
 * @param buf      Frame buffer (must be at least 256 bytes).
 * @param buf_size Buffer capacity.
 */
void syn_modbus_init(SYN_Modbus *mb, const SYN_Modbus_Config *cfg,
                      uint8_t *buf, uint16_t buf_size);

/**
 * @brief Poll for incoming Modbus requests and process them.
 *
 * Call from your main loop. This reads bytes from UART, detects
 * frame boundaries (3.5 character times silence), validates CRC,
 * and processes the request.
 *
 * @param mb  Modbus instance.
 * @return true if a request was processed.
 */
bool syn_modbus_poll(SYN_Modbus *mb);

/**
 * @brief Feed a byte to the Modbus receiver.
 *
 * Alternative to poll() — call from UART ISR if you want
 * interrupt-driven reception.
 *
 * @param mb    Modbus instance.
 * @param byte  Received byte.
 */
void syn_modbus_feed(SYN_Modbus *mb, uint8_t byte);

/**
 * @brief Process a complete frame (call after feed + silence detection).
 *
 * @param mb  Modbus instance.
 * @return true if a response was sent.
 */
bool syn_modbus_process(SYN_Modbus *mb);

/**
 * @brief Reset the receiver state.
 * @param mb  Modbus instance.
 */
void syn_modbus_reset(SYN_Modbus *mb);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MODBUS_H */
