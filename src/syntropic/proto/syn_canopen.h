/**
 * @file syn_canopen.h
 * @brief CANopen DS301 Slave Protocol Engine.
 *
 * Implements a lightweight, non-blocking, zero-malloc CANopen DS301 slave stack
 * including Object Dictionary, SDO server, RPDO/TPDO engine, NMT state machine,
 * Heartbeat producer, and Emergency (EMCY) alarms.
 */

#ifndef SYN_CANOPEN_H
#define SYN_CANOPEN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief CANopen NMT Command Specifiers */
#define SYN_CANOPEN_NMT_CMD_START      0x01U /**< Start Remote Node */
#define SYN_CANOPEN_NMT_CMD_STOP       0x02U /**< Stop Remote Node */
#define SYN_CANOPEN_NMT_CMD_PREOP      0x80U /**< Enter Pre-Operational */
#define SYN_CANOPEN_NMT_CMD_RESET_NODE 0x81U /**< Reset Node */
#define SYN_CANOPEN_NMT_CMD_RESET_COMM 0x82U /**< Reset Communication */

/** @brief CANopen NMT States */
#define SYN_CANOPEN_NMT_STATE_BOOTUP      0x00U /**< Bootup State */
#define SYN_CANOPEN_NMT_STATE_STOPPED     0x04U /**< Stopped State */
#define SYN_CANOPEN_NMT_STATE_OPERATIONAL 0x05U /**< Operational State */
#define SYN_CANOPEN_NMT_STATE_PREOP       0x7FU /**< Pre-Operational State */

/** @brief SDO Abort Codes */
#define SYN_CANOPEN_SDO_ABORT_NONE               0x00000000UL /**< No error */
#define SYN_CANOPEN_SDO_ABORT_TOGGLE_BIT         0x05030000UL /**< Toggle bit not altered */
#define SYN_CANOPEN_SDO_ABORT_UNSUPPORTED_ACCESS 0x06010000UL /**< Unsupported access to object */
#define SYN_CANOPEN_SDO_ABORT_WRITE_ONLY         0x06010001UL /**< Attempt to read a write only object */
#define SYN_CANOPEN_SDO_ABORT_READ_ONLY          0x06010002UL /**< Attempt to write a read only object */
#define SYN_CANOPEN_SDO_ABORT_NOT_EXIST          0x06020000UL /**< Object does not exist in dictionary */
#define SYN_CANOPEN_SDO_ABORT_TYPE_MISMATCH      0x06070010UL /**< Data type mismatch, length mismatch */
#define SYN_CANOPEN_SDO_ABORT_SUBINDEX_NOT_EXIST 0x06090011UL /**< Subindex does not exist */

/** @brief OD Access Rights */
#define SYN_CANOPEN_ACCESS_RO 0x01U /**< Read-Only access */
#define SYN_CANOPEN_ACCESS_WO 0x02U /**< Write-Only access */
#define SYN_CANOPEN_ACCESS_RW 0x03U /**< Read/Write access */

/** @brief OD Data Types */
#define SYN_CANOPEN_TYPE_I8  0x02U /**< Signed 8-bit integer */
#define SYN_CANOPEN_TYPE_I16 0x03U /**< Signed 16-bit integer */
#define SYN_CANOPEN_TYPE_I32 0x04U /**< Signed 32-bit integer */
#define SYN_CANOPEN_TYPE_U8  0x05U /**< Unsigned 8-bit integer */
#define SYN_CANOPEN_TYPE_U16 0x06U /**< Unsigned 16-bit integer */
#define SYN_CANOPEN_TYPE_U32 0x07U /**< Unsigned 32-bit integer */

/**
 * @brief Object Dictionary Entry Definition.
 */
typedef struct {
    uint16_t index;       /**< 16-bit OD Index (0x1000..0x9FFF) */
    uint8_t  subindex;    /**< 8-bit Subindex (0x00..0xFF) */
    uint8_t  type;        /**< Data Type (SYN_CANOPEN_TYPE_*) */
    uint8_t  access;      /**< Access Rights (SYN_CANOPEN_ACCESS_*) */
    void    *data_ptr;    /**< Pointer to live variable */
    size_t   size;        /**< Variable size in bytes */
} SYN_CANOpenODEntry;

/**
 * @brief PDO Mapping Configuration.
 */
typedef struct {
    uint16_t cob_id;       /**< COB-ID for PDO */
    uint16_t od_index;     /**< Mapped OD Index */
    uint8_t  od_subindex;  /**< Mapped OD Subindex */
    uint8_t  enabled;      /**< 1 = Enabled, 0 = Disabled */
} SYN_CANOpenPDOMap;

/**
 * @brief CANopen Node Configuration.
 */
typedef struct {
    uint8_t           node_id;       /**< Node-ID (1..127) */
    uint16_t          heartbeat_ms;  /**< Heartbeat period in ms (0 = disabled) */
    SYN_CANOpenPDOMap rpdo[4];       /**< Receive PDO Mappings 1..4 */
    SYN_CANOpenPDOMap tpdo[4];       /**< Transmit PDO Mappings 1..4 */
} SYN_CANOpenNodeConfig;

/**
 * @brief CANopen Node State Handle.
 */
typedef struct {
    uint8_t                 node_id;             /**< Node-ID */
    uint8_t                 nmt_state;           /**< Current NMT state */
    uint16_t                heartbeat_timer_ms;  /**< Heartbeat timer counter */
    SYN_CANOpenNodeConfig   cfg;                 /**< Node configuration copy */
    const SYN_CANOpenODEntry *od_table;          /**< Pointer to Object Dictionary */
    size_t                  od_count;            /**< Number of OD entries */
    uint32_t                tx_cob_id;           /**< Pending transmit COB-ID */
    uint8_t                 tx_data[8];          /**< Pending transmit payload */
    uint8_t                 tx_len;              /**< Pending transmit length */
    bool                    tx_pending;          /**< Transmit frame pending flag */
} SYN_CANOpenNode;

/**
 * @brief Initialize a CANopen DS301 Node.
 *
 * @param node Pointer to node handle.
 * @param cfg Pointer to node configuration.
 * @param od_table Array of Object Dictionary entries.
 * @param od_count Number of entries in od_table.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_canopen_init(SYN_CANOpenNode *node,
                            const SYN_CANOpenNodeConfig *cfg,
                            const SYN_CANOpenODEntry *od_table,
                            size_t od_count);

/**
 * @brief Process an incoming CAN frame.
 *
 * @param node Pointer to node handle.
 * @param cob_id Received 11-bit CAN COB-ID.
 * @param data Payload buffer (up to 8 bytes).
 * @param len Payload length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_canopen_process_rx(SYN_CANOpenNode *node,
                                  uint32_t cob_id,
                                  const uint8_t *data,
                                  uint8_t len);

/**
 * @brief Periodic timer update (call from tick thread or loop).
 *
 * @param node Pointer to node handle.
 * @param dt_ms Elapsed time since last update in milliseconds.
 * @return SYN_OK on success.
 */
SYN_Status syn_canopen_update(SYN_CANOpenNode *node, uint32_t dt_ms);

/**
 * @brief Transmit an Emergency (EMCY) message.
 *
 * @param node Pointer to node handle.
 * @param err_code 16-bit CANopen Emergency Error Code.
 * @param err_reg 8-bit Error Register value (Index 0x1001).
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid parameters.
 */
SYN_Status syn_canopen_send_emcy(SYN_CANOpenNode *node, uint16_t err_code, uint8_t err_reg);

/**
 * @brief Directly read an entry from the Object Dictionary.
 *
 * @param node Pointer to node handle.
 * @param index 16-bit OD Index.
 * @param subindex 8-bit OD Subindex.
 * @param buf Output buffer pointer.
 * @param buf_size Maximum buffer capacity.
 * @param out_len Pointer to store read byte length.
 * @return SYN_OK on success, error code if not found or unauthorized.
 */
SYN_Status syn_canopen_od_read(SYN_CANOpenNode *node,
                               uint16_t index,
                               uint8_t subindex,
                               void *buf,
                               size_t buf_size,
                               size_t *out_len);

/**
 * @brief Directly write an entry to the Object Dictionary.
 *
 * @param node Pointer to node handle.
 * @param index 16-bit OD Index.
 * @param subindex 8-bit OD Subindex.
 * @param data Input data pointer.
 * @param len Input data length in bytes.
 * @return SYN_OK on success, error code if not found or unauthorized.
 */
SYN_Status syn_canopen_od_write(SYN_CANOpenNode *node,
                                uint16_t index,
                                uint8_t subindex,
                                const void *data,
                                size_t len);

/**
 * @brief Retrieve pending transmit CAN frame.
 *
 * @param node Pointer to node handle.
 * @param out_cob_id Output COB-ID pointer.
 * @param out_data Output 8-byte buffer pointer.
 * @param out_len Output frame length pointer.
 * @return true if a frame was popped, false if no frame pending.
 */
bool syn_canopen_get_tx(SYN_CANOpenNode *node,
                        uint32_t *out_cob_id,
                        uint8_t *out_data,
                        uint8_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CANOPEN_H */
