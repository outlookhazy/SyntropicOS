/**
 * @file syn_lss.h
 * @brief CANopen CiA 305 Layer Setting Services (LSS) & Protocols.
 *
 * Implements non-blocking, zero-malloc LSS slave & master protocol engines
 * for dynamic Node-ID allocation, bit-timing (baud rate) configuration,
 * LSS Fastscan, and NVM configuration storage over CAN.
 */

#ifndef SYN_LSS_H
#define SYN_LSS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief LSS CAN Identifier Definitions */
#define SYN_LSS_COB_ID_MASTER 0x7E5U /**< Master to Slave Request COB-ID */
#define SYN_LSS_COB_ID_SLAVE  0x7E4U /**< Slave to Master Response COB-ID */

/** @brief LSS Command Specifiers (CS) */
#define SYN_LSS_CS_SWITCH_MODE_GLOBAL   0x04U /**< Switch mode global */
#define SYN_LSS_CS_CONFIGURE_NODE_ID    0x11U /**< Configure Node-ID */
#define SYN_LSS_CS_CONFIGURE_BIT_TIMING 0x13U /**< Configure Bit Timing */
#define SYN_LSS_CS_ACTIVATE_BIT_TIMING  0x15U /**< Activate Bit Timing */
#define SYN_LSS_CS_STORE_CONFIG         0x17U /**< Store Configuration */
#define SYN_LSS_CS_SWITCH_MODE_SELECT   0x40U /**< Switch mode selective vendor */
#define SYN_LSS_CS_INQUIRE_VENDOR       0x5AU /**< Inquire Vendor-ID */
#define SYN_LSS_CS_INQUIRE_PRODUCT      0x5BU /**< Inquire Product-Code */
#define SYN_LSS_CS_INQUIRE_REV          0x5CU /**< Inquire Revision-Number */
#define SYN_LSS_CS_INQUIRE_SERIAL       0x5DU /**< Inquire Serial-Number */

/** @brief LSS Operating Modes */
#define SYN_LSS_MODE_OPERATION     0x00U /**< LSS Operation Mode */
#define SYN_LSS_MODE_CONFIGURATION 0x01U /**< LSS Configuration Mode */

/** @brief LSS Bit Timing Table Entries (CiA 305 Standard Baud Rates) */
#define SYN_LSS_BAUD_1000K 0x00U /**< 1000 kbit/s */
#define SYN_LSS_BAUD_800K  0x01U /**< 800 kbit/s */
#define SYN_LSS_BAUD_500K  0x02U /**< 500 kbit/s */
#define SYN_LSS_BAUD_250K  0x03U /**< 250 kbit/s */
#define SYN_LSS_BAUD_125K  0x04U /**< 125 kbit/s */
#define SYN_LSS_BAUD_100K  0x05U /**< 100 kbit/s */
#define SYN_LSS_BAUD_50K   0x06U /**< 50 kbit/s */
#define SYN_LSS_BAUD_20K   0x07U /**< 20 kbit/s */
#define SYN_LSS_BAUD_10K   0x08U /**< 10 kbit/s */

/** @brief LSS Address Structure (Unique 128-bit identity) */
typedef struct {
    uint32_t vendor_id;     /**< 32-bit Vendor-ID */
    uint32_t product_code;  /**< 32-bit Product-Code */
    uint32_t revision_no;   /**< 32-bit Revision-Number */
    uint32_t serial_no;     /**< 32-bit Serial-Number */
} SYN_LSSAddress;

/** @brief LSS Slave Instance State */
typedef struct {
    uint8_t        mode;        /**< Current LSS mode (Operation / Configuration) */
    uint8_t        node_id;     /**< Current configured Node-ID */
    uint8_t        baud_table;  /**< Pending configured baud table entry */
    SYN_LSSAddress addr;        /**< Static LSS 128-bit Identity Address */
    bool           store_req;   /**< True if store configuration requested */
} SYN_LSSSlave;

/**
 * @brief Initialize LSS Slave state instance.
 * @param slave Pointer to slave handle.
 * @param addr Pointer to LSS unique 128-bit identity address.
 * @param initial_node_id Starting node-ID (or 0xFF for unconfigured).
 */
void syn_lss_slave_init(SYN_LSSSlave *slave, const SYN_LSSAddress *addr, uint8_t initial_node_id);

/**
 * @brief Process an incoming CAN frame for LSS Slave processing.
 * @param slave Pointer to LSS Slave handle.
 * @param frame Incoming CAN frame (COB-ID 0x7E5).
 * @param resp Output CAN frame buffer (populated with 0x7E4 response if required).
 * @return True if a response frame should be transmitted over CAN.
 */
bool syn_lss_slave_process(SYN_LSSSlave *slave, const SYN_CAN_Frame *frame, SYN_CAN_Frame *resp);

/**
 * @brief Build an LSS Master request frame to switch global mode.
 * @param frame Output frame buffer.
 * @param mode Target LSS mode (SYN_LSS_MODE_OPERATION or SYN_LSS_MODE_CONFIGURATION).
 */
void syn_lss_master_build_switch_mode(SYN_CAN_Frame *frame, uint8_t mode);

/**
 * @brief Build an LSS Master request frame to configure Node-ID.
 * @param frame Output frame buffer.
 * @param new_node_id Target Node-ID (1..127).
 */
void syn_lss_master_build_config_node_id(SYN_CAN_Frame *frame, uint8_t new_node_id);

/**
 * @brief Build an LSS Master request frame to configure Bit Timing / Baud rate.
 * @param frame Output frame buffer.
 * @param baud_table_idx Baud rate index (SYN_LSS_BAUD_*).
 */
void syn_lss_master_build_config_bit_timing(SYN_CAN_Frame *frame, uint8_t baud_table_idx);

/**
 * @brief Build an LSS Master request frame to store configuration to NVM.
 * @param frame Output frame buffer.
 */
void syn_lss_master_build_store_config(SYN_CAN_Frame *frame);

#ifdef __cplusplus
}
#endif

#endif /* SYN_LSS_H */
