/**
 * @file syn_modbus_master.h
 * @brief Non-blocking Modbus RTU Master / Client protocol engine.
 */

#ifndef SYN_MODBUS_MASTER_H
#define SYN_MODBUS_MASTER_H

#include "../common/syn_defs.h"
#include "syn_modbus.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Modbus Master transaction state. */
typedef enum {
    SYN_MB_MASTER_STATE_IDLE = 0,
    SYN_MB_MASTER_STATE_WAITING_RESPONSE,
    SYN_MB_MASTER_STATE_COMPLETE,
    SYN_MB_MASTER_STATE_TIMEOUT,
    SYN_MB_MASTER_STATE_ERROR,
} SYN_ModbusMaster_State;

/** @brief Modbus Master instance configuration & transaction state. */
typedef struct {
    SYN_ModbusMaster_State state;               /**< Master transaction state           */

    uint8_t   buf[256];                          /**< Frame buffer                      */
    uint16_t  rx_len;                            /**< Received response length           */
    uint16_t  tx_len;                            /**< Transmitted request length         */

    uint8_t   slave_addr;                        /**< Target slave address (1–247)       */
    uint8_t   func_code;                         /**< Active function code               */
    uint16_t  start_addr;                        /**< Target register start address      */
    uint16_t  count;                             /**< Register quantity / write value    */

    uint32_t  request_tick_ms;                   /**< Tick when request was dispatched   */
    uint32_t  timeout_ms;                        /**< Response timeout limit in ms       */
    uint32_t  last_byte_tick_ms;                 /**< Tick of last received RX byte      */

    uint8_t   response_fc;                       /**< Response function code             */
    uint8_t   exception_code;                    /**< Exception code if error            */

    uint16_t  read_data[125];                    /**< Received read data buffer          */
    uint16_t  read_count;                        /**< Number of registers read           */
} SYN_ModbusMaster;

/**
 * @brief Initialize a Modbus Master instance.
 * @param m           Master instance.
 * @param timeout_ms  Response timeout in milliseconds (e.g. 500 ms).
 */
void syn_modbus_master_init(SYN_ModbusMaster *m, uint32_t timeout_ms);

/**
 * @brief Issue a Read Holding Registers (FC 0x03) request.
 * @param m           Master instance.
 * @param slave_addr  Target slave address (1–247).
 * @param start_addr  Starting register address.
 * @param count       Number of registers to read (1–125).
 * @return SYN_OK on success, SYN_BUSY if transaction in progress.
 */
SYN_Status syn_modbus_master_read_holding(SYN_ModbusMaster *m, uint8_t slave_addr,
                                           uint16_t start_addr, uint16_t count);

/**
 * @brief Issue a Read Input Registers (FC 0x04) request.
 * @param m           Master instance.
 * @param slave_addr  Target slave address (1–247).
 * @param start_addr  Starting register address.
 * @param count       Number of registers to read (1–125).
 * @return SYN_OK on success, SYN_BUSY if transaction in progress.
 */
SYN_Status syn_modbus_master_read_input(SYN_ModbusMaster *m, uint8_t slave_addr,
                                         uint16_t start_addr, uint16_t count);

/**
 * @brief Issue a Write Single Register (FC 0x06) request.
 * @param m           Master instance.
 * @param slave_addr  Target slave address (1–247).
 * @param reg_addr    Register address.
 * @param value       Value to write.
 * @return SYN_OK on success, SYN_BUSY if transaction in progress.
 */
SYN_Status syn_modbus_master_write_single(SYN_ModbusMaster *m, uint8_t slave_addr,
                                           uint16_t reg_addr, uint16_t value);

/**
 * @brief Issue a Write Multiple Registers (FC 0x10) request.
 * @param m           Master instance.
 * @param slave_addr  Target slave address (1–247).
 * @param start_addr  Starting register address.
 * @param count       Number of registers (1–121).
 * @param values      Array of register values to write.
 * @return SYN_OK on success, SYN_BUSY if transaction in progress.
 */
SYN_Status syn_modbus_master_write_multiple(SYN_ModbusMaster *m, uint8_t slave_addr,
                                            uint16_t start_addr, uint16_t count,
                                            const uint16_t *values);

/**
 * @brief Issue a Read Coils (FC 0x01) request.
 * @param m           Master instance.
 * @param slave_addr  Target slave address (1–247).
 * @param start_addr  Starting coil address.
 * @param count       Number of coils to read (1–2000).
 * @return SYN_OK on success, SYN_BUSY if transaction in progress.
 */
SYN_Status syn_modbus_master_read_coils(SYN_ModbusMaster *m, uint8_t slave_addr,
                                         uint16_t start_addr, uint16_t count);

/**
 * @brief Issue a Read Discrete Inputs (FC 0x02) request.
 * @param m           Master instance.
 * @param slave_addr  Target slave address (1–247).
 * @param start_addr  Starting input address.
 * @param count       Number of inputs to read (1–2000).
 * @return SYN_OK on success, SYN_BUSY if transaction in progress.
 */
SYN_Status syn_modbus_master_read_discrete_inputs(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                   uint16_t start_addr, uint16_t count);

/**
 * @brief Issue a Write Single Coil (FC 0x05) request.
 * @param m           Master instance.
 * @param slave_addr  Target slave address (1–247).
 * @param coil_addr   Coil address.
 * @param state       Coil state (true = ON / 0xFF00, false = OFF / 0x0000).
 * @return SYN_OK on success, SYN_BUSY if transaction in progress.
 */
SYN_Status syn_modbus_master_write_single_coil(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                 uint16_t coil_addr, bool state);

/**
 * @brief Issue a Write Multiple Coils (FC 0x0F) request.
 * @param m           Master instance.
 * @param slave_addr  Target slave address (1–247).
 * @param start_addr  Starting coil address.
 * @param count       Number of coils (1–1968).
 * @param coil_bytes  Packed byte array containing coil states.
 * @return SYN_OK on success, SYN_BUSY if transaction in progress.
 */
SYN_Status syn_modbus_master_write_multiple_coils(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                   uint16_t start_addr, uint16_t count,
                                                   const uint8_t *coil_bytes);

/**
 * @brief Issue a Mask Write Register (FC 0x16) request.
 * @param m           Master instance.
 * @param slave_addr  Target slave address (1–247).
 * @param reg_addr    Register address.
 * @param and_mask    AND mask.
 * @param or_mask     OR mask.
 * @return SYN_OK on success, SYN_BUSY if transaction in progress.
 */
SYN_Status syn_modbus_master_mask_write_register(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                  uint16_t reg_addr, uint16_t and_mask,
                                                  uint16_t or_mask);

/**
 * @brief Issue a Read FIFO Queue (FC 0x18) request.
 * @param m           Master instance.
 * @param slave_addr  Target slave address (1–247).
 * @param fifo_addr   FIFO register address.
 * @return SYN_OK on success, SYN_BUSY if transaction in progress.
 */
SYN_Status syn_modbus_master_read_fifo_queue(SYN_ModbusMaster *m, uint8_t slave_addr,
                                              uint16_t fifo_addr);

/**
 * @brief Feed a received byte into the Master RX stream buffer.
 * @param m     Master instance.
 * @param byte  Received byte.
 */
void syn_modbus_master_feed(SYN_ModbusMaster *m, uint8_t byte);

/**
 * @brief Non-blocking state machine processor.
 * @param m               Master instance.
 * @param current_tick_ms Current system tick in milliseconds.
 * @return Current Master state.
 */
SYN_ModbusMaster_State syn_modbus_master_process(SYN_ModbusMaster *m, uint32_t current_tick_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MODBUS_MASTER_H */
