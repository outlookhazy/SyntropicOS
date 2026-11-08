/**
 * @file syn_ethercat.h
 * @brief EtherCAT (IEEE 802.3 EtherType 0x88A4) Bare-Metal Protocol Stack.
 *
 * Implements EtherCAT Frame packing, Datagram commands (APRD, APWR, APRW,
 * FPRD, FPWR, FPRW, BRD, BWR, LRD, LWR, LRW), Working Counter (WKC) accounting,
 * EtherCAT State Machine (ESM: INIT, PREOP, BOOT, SAFEOP, OP), and CoE
 * (CAN application protocol over EtherCAT) Mailbox SDO & PDO handling.
 *
 * Designed for zero heap allocation and direct L2 raw Ethernet frame processing.
 * @ingroup syn_protocol
 */

#ifndef SYN_ETHERCAT_H
#define SYN_ETHERCAT_H

#include "../common/syn_defs.h"

#if !defined(SYN_USE_ETHERCAT) || SYN_USE_ETHERCAT

#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"

#if !defined(SYN_USE_CANOPEN) || SYN_USE_CANOPEN
/** @brief Non-zero when CANopen Object Dictionary support for CoE is enabled. */
#define SYN_ECAT_COE_OD_ENABLED 1
#include "syn_canopen.h"
#else
/** @brief Non-zero when CANopen Object Dictionary support for CoE is enabled. */
#define SYN_ECAT_COE_OD_ENABLED 0
/** @brief Opaque CANopen node placeholder when SYN_USE_CANOPEN is disabled. */
typedef struct SYN_CANOpenNode_Opaque SYN_CANOpenNode;
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief EtherCAT IEEE 802.3 Ethernet Frame Type */
#define SYN_ETHERCAT_ETHERTYPE 0x88A4

/** @brief EtherCAT Header Type (1 = EtherCAT Datagrams) */
#define SYN_ETHERCAT_TYPE_DATAGRAM 1

/** @brief Maximum EtherCAT datagram payload/frame length (11-bit length field) */
#define SYN_ECAT_MAX_DGRAM_BYTES 0x07FFU

/** @brief Default SyncManager 0/1 CoE mailbox buffer length in bytes */
#define SYN_ECAT_MBOX_SIZE 128U

/* ── EtherCAT Commands ─────────────────────────────────────────────────── */

/** EtherCAT Datagram Commands Enum. */
typedef enum {
    SYN_ECAT_CMD_NOP = 0,   /**< No Operation */
    SYN_ECAT_CMD_APRD = 1,  /**< Auto Increment Read */
    SYN_ECAT_CMD_APWR = 2,  /**< Auto Increment Write */
    SYN_ECAT_CMD_APRW = 3,  /**< Auto Increment Read Write */
    SYN_ECAT_CMD_FPRD = 4,  /**< Configured Address Read */
    SYN_ECAT_CMD_FPWR = 5,  /**< Configured Address Write */
    SYN_ECAT_CMD_FPRW = 6,  /**< Configured Address Read Write */
    SYN_ECAT_CMD_BRD = 7,   /**< Broadcast Read */
    SYN_ECAT_CMD_BWR = 8,   /**< Broadcast Write */
    SYN_ECAT_CMD_BRW = 9,   /**< Broadcast Read Write */
    SYN_ECAT_CMD_LRD = 10,  /**< Logical Read */
    SYN_ECAT_CMD_LWR = 11,  /**< Logical Write */
    SYN_ECAT_CMD_LRW = 12,  /**< Logical Read Write */
    SYN_ECAT_CMD_ARMW = 13, /**< Auto Increment Read Multiple Write */
    SYN_ECAT_CMD_FRMW = 14  /**< Configured Address Read Multiple Write */
} SYN_EcatCmd;

/* ── EtherCAT State Machine (ESM) States ────────────────────────────────── */

/** EtherCAT State Machine (ESM) States. */
typedef enum {
    SYN_ECAT_STATE_NONE = 0x00,   /**< Uninitialized / None */
    SYN_ECAT_STATE_INIT = 0x01,   /**< Init State */
    SYN_ECAT_STATE_PREOP = 0x02,  /**< Pre-Operational State */
    SYN_ECAT_STATE_BOOT = 0x03,   /**< Bootstrap State */
    SYN_ECAT_STATE_SAFEOP = 0x04, /**< Safe-Operational State */
    SYN_ECAT_STATE_OP = 0x08      /**< Operational State */
} SYN_EcatState;

/* ── CoE Mailbox Service Types ──────────────────────────────────────────── */

/** CAN Application Protocol over EtherCAT (CoE) Service Types. */
typedef enum {
    SYN_ECAT_COE_TYPE_EMERGENCY = 1, /**< CoE Emergency message */
    SYN_ECAT_COE_TYPE_SDO_REQ = 2,   /**< SDO Request */
    SYN_ECAT_COE_TYPE_SDO_RESP = 3,  /**< SDO Response */
    SYN_ECAT_COE_TYPE_RXPDO = 4,     /**< RxPDO mapping */
    SYN_ECAT_COE_TYPE_TXPDO = 5,     /**< TxPDO mapping */
    SYN_ECAT_COE_TYPE_SDO_INFO = 6   /**< SDO Information object directory */
} SYN_EcatCoEType;

/* ── Struct Definitions ─────────────────────────────────────────────────── */

/** @brief Raw EtherCAT Header (2 bytes) */
typedef struct {
    uint16_t length : 11;  /**< Length of datagrams in bytes */
    uint16_t reserved : 1; /**< Reserved bit */
    uint16_t type : 4;     /**< Type (1 for EtherCAT datagrams) */
} SYN_EcatHeader;

/** @brief EtherCAT Datagram Header (10 bytes header + 2 bytes WKC = 12 bytes overhead) */
typedef struct {
    uint8_t cmd;       /**< Command code (SYN_EcatCmd) */
    uint8_t idx;       /**< Index number */
    uint32_t addr;     /**< Address (Auto-inc, Configured, or Logical) */
    uint16_t len : 11; /**< Datagram data length */
    uint16_t res : 3;  /**< Reserved */
    uint16_t circ : 1; /**< Circulating frame flag */
    uint16_t m : 1;    /**< More datagrams flag (1 = another datagram follows) */
    uint16_t irq;      /**< Interrupt request */
    uint16_t wkc;      /**< Working Counter */
} SYN_EcatDatagram;

/** @brief CoE (CANoverEtherCAT) Mailbox Header */
typedef struct {
    uint16_t number : 9;       /**< CoE detail/sequence number */
    uint16_t res : 3;          /**< Reserved bits */
    uint16_t service_type : 4; /**< CoE service type enum */
} SYN_EcatCoEHeader;

/* ── EtherCAT Master & Configuration Structs ────────────────────────────── */

#ifndef SYN_ECAT_MAX_SLAVES
#define SYN_ECAT_MAX_SLAVES 16U /**< Maximum connected EtherCAT slaves supported */
#endif

/** @brief Slave process data RAM buffer capacity per direction in bytes */
#define SYN_ECAT_PDO_BUF_SIZE 64U

/** @brief Emulated slave SII EEPROM 32-bit word table size */
#define SYN_ECAT_SII_WORDS 16U

/**
 * @brief EtherCAT SyncManager Configuration.
 */
typedef struct {
    uint16_t start_addr; /**< Physical start address in slave DPRAM */
    uint16_t length;     /**< Buffer length in bytes */
    uint8_t control;     /**< Direction, mode, interrupt control byte */
    uint8_t enable;      /**< SM enable byte (0x01 = enable) */
} SYN_EcatSMConfig;

/**
 * @brief EtherCAT FMMU Configuration.
 */
typedef struct {
    uint32_t logical_start;    /**< Logical start address */
    uint16_t length;           /**< Mapping byte length */
    uint8_t logical_start_bit; /**< Logical start bit offset (0..7) */
    uint8_t logical_end_bit;   /**< Logical end bit (0..7) */
    uint16_t phys_start_addr;  /**< Physical start address in slave DPRAM */
    uint8_t phys_start_bit;    /**< Physical start bit offset (0..7) */
    uint8_t type;              /**< Read (0x01), Write (0x02), or ReadWrite (0x03) */
    bool enable;               /**< FMMU active flag */
} SYN_EcatFMMUConfig;

/** @brief EtherCAT Node (Slave ESC) Descriptor */
typedef struct {
    SYN_EcatState state;                    /**< Current ESM state */
    SYN_EcatState target_state;             /**< Requested ESM state */
    uint16_t station_addr;                  /**< Configured station address (FPRD/FPWR) */
    uint16_t al_status;                     /**< Application Layer Status Code */
    uint16_t wkc_expected;                  /**< Expected Working Counter threshold */
    uint16_t wkc_last;                      /**< Last processed Working Counter */
    uint32_t rx_pdos;                       /**< Processed RxPDO count */
    uint32_t tx_pdos;                       /**< Processed TxPDO count */
    SYN_CANOpenNode *od;                    /**< Optional CANopen Object Dictionary for CoE */
    SYN_EcatSMConfig sm[4];                 /**< Slave SyncManager 0..3 registers */
    SYN_EcatFMMUConfig fmmu[4];             /**< Slave FMMU 0..3 registers */
    uint8_t mbox_out[SYN_ECAT_MBOX_SIZE];   /**< SM0 Master->Slave mailbox DPRAM */
    uint8_t mbox_in[SYN_ECAT_MBOX_SIZE];    /**< SM1 Slave->Master mailbox DPRAM */
    bool mbox_in_full;                      /**< SM1 mailbox response queued flag */
    uint8_t pdo_out[SYN_ECAT_PDO_BUF_SIZE]; /**< RxPDO DPRAM (0x1100..0x113F) */
    uint8_t pdo_in[SYN_ECAT_PDO_BUF_SIZE];  /**< TxPDO DPRAM (0x1140..0x117F) */
    uint32_t sii_addr;                      /**< SII EEPROM word offset register */
    uint16_t sii_ctrl;                      /**< SII EEPROM control/status register */
    uint32_t sii_words[SYN_ECAT_SII_WORDS]; /**< Emulated SII EEPROM 32-bit word table */
} SYN_EcatNode;

/* ── API Functions ──────────────────────────────────────────────────────── */

/**
 * @brief Initialize an EtherCAT Node instance.
 * @param node          EtherCAT node instance.
 * @param station_addr  Configured station address.
 * @param od            Optional CANopen Object Dictionary binding for CoE.
 */
void syn_ecat_init(SYN_EcatNode *node, uint16_t station_addr, SYN_CANOpenNode *od);

/**
 * @brief Build an EtherCAT Frame containing one or more datagrams.
 * @param buf       Output frame buffer (must include Ethernet header space if needed).
 * @param buf_len   Capacity of output buffer.
 * @param datagram  Datagram header descriptor.
 * @param data      Payload bytes to write into datagram.
 * @param data_len  Payload length.
 * @return Total EtherCAT frame length in bytes, or 0 on error.
 */
size_t syn_ecat_build_datagram_frame(uint8_t *buf, size_t buf_len, const SYN_EcatDatagram *datagram,
                                     const uint8_t *data, uint16_t data_len);

/**
 * @brief Parse and validate Working Counter (WKC) from a received raw EtherCAT frame.
 * @param node  EtherCAT node instance.
 * @param frame Raw received frame bytes (after Ethernet header); read-only.
 * @param len   Frame length in bytes.
 * @param wkc   [out] Extracted Working Counter sum across frame datagrams.
 * @return SYN_OK on successful parsing and WKC validation.
 */
SYN_Status syn_ecat_parse_frame(SYN_EcatNode *node, const uint8_t *frame, size_t len,
                                uint16_t *wkc);

/**
 * @brief Process a received raw EtherCAT frame in-place on a slave ESC node.
 * @param node  EtherCAT slave node instance.
 * @param frame Raw received frame bytes (after Ethernet header); updated in-place on reads/WKC.
 * @param len   Frame length in bytes.
 * @param wkc   [out] Resulting Working Counter sum across processed datagrams.
 * @return SYN_OK on successful processing and WKC validation.
 */
SYN_Status syn_ecat_slave_process_frame(SYN_EcatNode *node, uint8_t *frame, size_t len,
                                        uint16_t *wkc);

/**
 * @brief Request an EtherCAT State Machine (ESM) state transition.
 * @param node      EtherCAT node instance.
 * @param new_state Requested state (INIT, PREOP, BOOT, SAFEOP, OP).
 * @return SYN_OK if state transition request is valid.
 */
SYN_Status syn_ecat_set_state(SYN_EcatNode *node, SYN_EcatState new_state);

/**
 * @brief Run ESM state machine step.
 * @param node EtherCAT node instance.
 */
void syn_ecat_update(SYN_EcatNode *node);

/**
 * @brief EtherCAT Slave Information Entry.
 */
typedef struct {
    uint16_t station_addr;   /**< Assigned configured station address */
    uint16_t auto_inc_addr;  /**< Auto-increment ring position */
    uint32_t vendor_id;      /**< Vendor ID read from SII EEPROM */
    uint32_t product_code;   /**< Product code read from SII EEPROM */
    uint32_t revision;       /**< Revision number read from SII EEPROM */
    SYN_EcatState state;     /**< Current AL State */
    uint16_t al_status_code; /**< Last AL status error code */
    uint8_t sm_count;        /**< Configured SyncManager count */
    uint8_t fmmu_count;      /**< Configured FMMU count */
} SYN_EcatSlaveInfo;

/**
 * @brief EtherCAT Master Instance Context (Zero-Heap).
 */
typedef struct {
    SYN_EcatSlaveInfo slaves[SYN_ECAT_MAX_SLAVES]; /**< Slave descriptor table */
    uint8_t slave_count;                           /**< Discovered slave count */

    uint8_t *output_image; /**< Master -> Slaves cyclic process output buffer */
    uint8_t *input_image;  /**< Slaves -> Master cyclic process input buffer */
    uint16_t output_size;  /**< Cyclic process output size in bytes */
    uint16_t input_size;   /**< Cyclic process input size in bytes */
    uint32_t logical_addr; /**< Logical base address for LRW process exchange */

    uint16_t wkc_expected; /**< Expected Working Counter for cyclic frame */
    uint16_t wkc_last;     /**< Last received Working Counter */
    uint32_t cycle_count;  /**< Completed cyclic process exchange counter */
    uint32_t wkc_errors;   /**< Working Counter error accumulator */

    uint8_t *tx_buf;      /**< Pointer to static transmit frame buffer */
    uint8_t *rx_buf;      /**< Pointer to static receive frame buffer */
    uint16_t tx_buf_size; /**< Transmit buffer capacity */
    uint16_t rx_buf_size; /**< Receive buffer capacity */

    /* Transport & Protothread Task State */
    SYN_EcatState master_state; /**< Current Master bus state */
    uint8_t current_slave_idx;  /**< Iteration index for multi-step slave tasks */
    uint16_t tx_frame_len;      /**< Current pending transmit frame length */
    uint16_t rx_frame_len;      /**< Current received frame length */
    bool frame_tx_pending;      /**< Transmit frame ready for PHY */
    bool frame_rx_ready;        /**< Receive frame ready from PHY */

    /* Scratch state for non-blocking task drivers & hardware polling */
    SYN_PT sub_pt;          /**< Sub-protothread handle for nested tasks */
    uint8_t pdo_count;      /**< Scratch PDO count for discovery */
    uint16_t pdo_entry_idx; /**< Scratch PDO entry loop index (uint16_t prevents 255 wrap) */
    uint16_t pdo_map_idx;   /**< Scratch PDO mapping index */
    uint32_t pdo_sdo_val;   /**< Scratch SDO read value */
    size_t pdo_read_len;    /**< Scratch SDO read length */
    uint32_t total_rx_bits; /**< Accumulated RxPDO bits */
    uint32_t total_tx_bits; /**< Accumulated TxPDO bits */
    uint16_t poll_retries;  /**< Retry counter for hardware status polling loops */
    SYN_PT dir_pt;          /**< Child protothread state for PDO direction discovery */
    bool sdo_ok;            /**< Completion status of last nested SDO read task */
} SYN_EcatMaster;

/* ── EtherCAT Master Transport & Protothread Tasks ─────────────────────── */

/**
 * @brief Feed received Ethernet frame into Master context.
 *
 * @param m Pointer to master handle.
 * @param frame Pointer to raw frame payload bytes.
 * @param len Frame length in bytes.
 * @return SYN_OK on success.
 */
SYN_Status syn_ecat_master_set_rx_frame(SYN_EcatMaster *m, const uint8_t *frame, size_t len);

/**
 * @brief Pop pending transmit frame for Ethernet PHY driver.
 *
 * @param m Pointer to master handle.
 * @param out_frame [out] Pointer to receive frame location.
 * @param out_len [out] Pointer to receive frame length.
 * @return true if frame was popped, false if no frame pending.
 */
bool syn_ecat_master_pop_tx_frame(SYN_EcatMaster *m, const uint8_t **out_frame, size_t *out_len);

/**
 * @brief Protothread Task: Scan EtherCAT bus and assign station addresses.
 *
 * @param pt Protothread state handle.
 * @param m Pointer to master handle.
 * @return PT_IDLE if waiting for frames, PT_ENDED when scan complete, or PT_EXITED on abort.
 */
SYN_PT_Status syn_ecat_master_scan_task(SYN_PT *pt, SYN_EcatMaster *m);

/**
 * @brief Protothread Task: Drive EtherCAT slaves through state transition.
 *
 * @param pt Protothread state handle.
 * @param m Pointer to master handle.
 * @param target_state Target ESM state (INIT, PREOP, SAFEOP, OP).
 * @return PT_IDLE if waiting for frames, PT_ENDED when transition complete, or PT_EXITED on abort.
 */
SYN_PT_Status syn_ecat_master_transition_task(SYN_PT *pt, SYN_EcatMaster *m,
                                              SYN_EcatState target_state);

/**
 * @brief Protothread Task: Execute single cyclic process exchange cycle.
 *
 * @param pt Protothread state handle.
 * @param m Pointer to master handle.
 * @return PT_IDLE if waiting for frames, PT_ENDED when cycle complete, or PT_EXITED on abort.
 */
SYN_PT_Status syn_ecat_master_cyclic_task(SYN_PT *pt, SYN_EcatMaster *m);

/**
 * @brief Encode FPRD to read register @p reg_addr from slave @p station_addr.
 * @param m Pointer to master handle.
 * @param station_addr Configured station address of slave.
 * @param reg_addr Register address to read.
 * @param len Number of bytes to read.
 * @return Encoded Ethernet frame length.
 */
size_t syn_ecat_encode_read_reg(SYN_EcatMaster *m, uint16_t station_addr, uint16_t reg_addr,
                                uint16_t len);

/**
 * @brief Encode FPWR to write register @p reg_addr to slave @p station_addr.
 * @param m Pointer to master handle.
 * @param station_addr Configured station address of slave.
 * @param reg_addr Register address to write.
 * @param data Pointer to data bytes to write.
 * @param len Number of bytes to write.
 * @return Encoded Ethernet frame length.
 */
size_t syn_ecat_encode_write_reg(SYN_EcatMaster *m, uint16_t station_addr, uint16_t reg_addr,
                                 const void *data, uint16_t len);

/**
 * @brief Decode FPRD response datagram for register read.
 * @param m Pointer to master handle.
 * @param rx_len Received Ethernet frame length.
 * @param out_data Destination buffer for register data.
 * @param len Expected byte length.
 * @return SYN_OK on success, error code otherwise.
 */
SYN_Status syn_ecat_decode_read_reg(SYN_EcatMaster *m, size_t rx_len, void *out_data, uint16_t len);

/**
 * @brief Protothread Task: Read 32-bit word from slave SII EEPROM via hardware status polling.
 * @param pt Protothread context pointer.
 * @param m Pointer to master handle.
 * @param station_addr Configured station address of slave.
 * @param word_offset 16-bit word offset in SII EEPROM.
 * @param out_data Destination pointer for 32-bit word.
 * @return PT_IDLE if waiting, PT_ENDED when complete, or PT_EXITED on abort.
 */
SYN_PT_Status syn_ecat_master_read_sii_task(SYN_PT *pt, SYN_EcatMaster *m, uint16_t station_addr,
                                            uint16_t word_offset, uint32_t *out_data);

/**
 * @brief Protothread Task: CoE SDO read from slave Object Dictionary with SM1 polling.
 * @param pt Protothread context pointer.
 * @param m Pointer to master handle.
 * @param station_addr Configured station address of slave.
 * @param index 16-bit Object Dictionary index.
 * @param subindex 8-bit sub-index.
 * @param out_data Destination buffer for SDO payload.
 * @param max_len Capacity of destination buffer.
 * @param out_len Destination pointer for actual payload length.
 * @return PT_IDLE if waiting, PT_ENDED when complete, or PT_EXITED on abort.
 */
SYN_PT_Status syn_ecat_master_sdo_read_task(SYN_PT *pt, SYN_EcatMaster *m, uint16_t station_addr,
                                            uint16_t index, uint8_t subindex, void *out_data,
                                            size_t max_len, size_t *out_len);

/**
 * @brief Protothread Task: Read CoE Object Dictionary 0x1C12/0x1C13 to calculate slave Rx/Tx PDO
 * byte lengths.
 * @param pt Protothread context pointer.
 * @param m Pointer to master handle.
 * @param station_addr Configured station address of slave.
 * @param out_rx_bytes Destination pointer for Rx PDO byte size.
 * @param out_tx_bytes Destination pointer for Tx PDO byte size.
 * @return PT_IDLE if waiting, PT_ENDED when complete, or PT_EXITED on abort.
 */
SYN_PT_Status syn_ecat_master_discover_pdo_mapping_task(SYN_PT *pt, SYN_EcatMaster *m,
                                                        uint16_t station_addr,
                                                        uint16_t *out_rx_bytes,
                                                        uint16_t *out_tx_bytes);

/* ── EtherCAT Master API ────────────────────────────────────────────────── */

/**
 * @brief Initialize EtherCAT Master instance context.
 *
 * @param m Pointer to master handle.
 * @param tx_buf Pointer to static transmit frame buffer.
 * @param tx_buf_size Capacity of transmit frame buffer.
 * @param rx_buf Pointer to static receive frame buffer.
 * @param rx_buf_size Capacity of receive frame buffer.
 * @param output_image Output process image buffer pointer.
 * @param output_size Output process image byte size.
 * @param input_image Input process image buffer pointer.
 * @param input_size Input process image byte size.
 * @return SYN_OK on success, SYN_INVALID_PARAM if parameters null/invalid.
 */
SYN_Status syn_ecat_master_init(SYN_EcatMaster *m, uint8_t *tx_buf, uint16_t tx_buf_size,
                                uint8_t *rx_buf, uint16_t rx_buf_size, uint8_t *output_image,
                                uint16_t output_size, uint8_t *input_image, uint16_t input_size);

/**
 * @brief Encode Broadcast Read (BRD) to scan bus slave count into master tx_buf.
 *
 * @param m Pointer to master handle.
 * @return Transmit frame length in bytes, or 0 on error.
 */
size_t syn_ecat_encode_scan_bus(SYN_EcatMaster *m);

/**
 * @brief Decode slave count from scan_bus response frame in rx_buf.
 *
 * @param m Pointer to master handle.
 * @param rx_len Received frame byte length.
 * @return Discovered slave count.
 */
uint8_t syn_ecat_decode_scan_bus(SYN_EcatMaster *m, size_t rx_len);

/**
 * @brief Encode Auto-Increment Write (APWR) to assign station address.
 *
 * @param m Pointer to master handle.
 * @param position Ring position index (0..slave_count-1).
 * @param station_addr Configured station address to set (e.g. 0x1001).
 * @return Transmit frame length in bytes, or 0 on error.
 */
size_t syn_ecat_encode_assign_addr(SYN_EcatMaster *m, uint16_t position, uint16_t station_addr);

/**
 * @brief Encode Configured Read (FPRD) to read SII EEPROM word.
 *
 * @param m Pointer to master handle.
 * @param station_addr Target slave station address.
 * @param word_offset SII EEPROM word offset.
 * @return Transmit frame length in bytes, or 0 on error.
 */
size_t syn_ecat_encode_read_sii(SYN_EcatMaster *m, uint16_t station_addr, uint16_t word_offset);

/**
 * @brief Decode 32-bit word from SII EEPROM read response.
 *
 * @param m Pointer to master handle.
 * @param rx_len Received frame length.
 * @param out_val Pointer to receive parsed 32-bit SII value.
 * @return SYN_OK on success.
 */
SYN_Status syn_ecat_decode_read_sii(SYN_EcatMaster *m, size_t rx_len, uint32_t *out_val);

/**
 * @brief Encode Configured Write (FPWR) to configure a SyncManager register block.
 *
 * @param m Pointer to master handle.
 * @param station_addr Target slave station address.
 * @param sm_index SyncManager index (0..3).
 * @param cfg Pointer to SyncManager configuration settings.
 * @return Transmit frame length in bytes, or 0 on error.
 */
size_t syn_ecat_encode_write_sm(SYN_EcatMaster *m, uint16_t station_addr, uint8_t sm_index,
                                const SYN_EcatSMConfig *cfg);

/**
 * @brief Encode Configured Write (FPWR) to configure an FMMU register block.
 *
 * @param m Pointer to master handle.
 * @param station_addr Target slave station address.
 * @param fmmu_index FMMU index (0..3).
 * @param cfg Pointer to FMMU configuration settings.
 * @return Transmit frame length in bytes, or 0 on error.
 */
size_t syn_ecat_encode_write_fmmu(SYN_EcatMaster *m, uint16_t station_addr, uint8_t fmmu_index,
                                  const SYN_EcatFMMUConfig *cfg);

/**
 * @brief Encode Configured Read (FPRD) to read slave Application Layer (AL) Status.
 *
 * @param m Pointer to master handle.
 * @param station_addr Target slave station address.
 * @return Transmit frame length in bytes, or 0 on error.
 */
size_t syn_ecat_encode_read_al_status(SYN_EcatMaster *m, uint16_t station_addr);

/**
 * @brief Decode Application Layer (AL) Status and Status Code from response.
 *
 * @param m Pointer to master handle.
 * @param rx_len Received frame length.
 * @param state [out] Output location for current ESM state.
 * @param status_code [out] Output location for AL status error code.
 * @return SYN_OK on success.
 */
SYN_Status syn_ecat_decode_read_al_status(SYN_EcatMaster *m, size_t rx_len, SYN_EcatState *state,
                                          uint16_t *status_code);

/* ── Multi-Datagram Frame Builder & Parser ─────────────────────────────── */

#define SYN_ESC_REG_TYPE 0x0000U             /**< ESC Type/Build */
#define SYN_ESC_REG_STATION_ADDR 0x0010U     /**< Configured Station Address */
#define SYN_ESC_REG_STATION_ALIAS 0x0012U    /**< Configured Station Alias */
#define SYN_ESC_REG_AL_CONTROL 0x0120U       /**< Application Layer Control */
#define SYN_ESC_REG_AL_STATUS 0x0130U        /**< Application Layer Status */
#define SYN_ESC_REG_AL_STATUS_CODE 0x0134U   /**< Application Layer Status Code */
#define SYN_ESC_REG_PDI_CONTROL 0x0140U      /**< Process Data Interface Control */
#define SYN_ESC_REG_SII_EEPROM_CTRL 0x0502U  /**< SII EEPROM Control/Status */
#define SYN_ESC_REG_FMMU0 0x0600U            /**< FMMU 0 Base (16 bytes per FMMU) */
#define SYN_ESC_REG_SM0 0x0800U              /**< SyncManager 0 Base (8 bytes per SM) */
#define SYN_ESC_REG_DC_SYSTEM_TIME 0x0900U   /**< DC System Time (64-bit) */
#define SYN_ESC_REG_DC_SYSTEM_OFFSET 0x0920U /**< DC System Time Offset */
#define SYN_ESC_REG_DC_SYNC0_CYCLE 0x09A0U   /**< DC SYNC0 Cycle Time */

/* ── Multi-Datagram Frame Builder & Parser ─────────────────────────────── */

/**
 * @brief Parsed datagram item from frame iteration.
 */
typedef struct {
    uint8_t cmd;         /**< Datagram command code (SYN_EcatCmd) */
    uint8_t idx;         /**< Datagram index */
    uint32_t addr;       /**< Datagram address field */
    const uint8_t *data; /**< Pointer to datagram payload bytes */
    uint16_t data_len;   /**< Datagram payload length */
    uint16_t wkc;        /**< Working Counter */
} SYN_EcatDatagramResult;

/**
 * @brief Initialize an EtherCAT frame buffer header.
 *
 * @param buf Target output buffer.
 * @param buf_size Total capacity of output buffer.
 */
void syn_ecat_frame_begin(uint8_t *buf, size_t buf_size);

/**
 * @brief Add a datagram payload to an in-progress EtherCAT frame.
 *
 * @param buf Target output buffer.
 * @param buf_size Total capacity of output buffer.
 * @param cmd Datagram command type.
 * @param idx Datagram index.
 * @param addr 32-bit address field.
 * @param data Payload data bytes (or NULL if data_len is 0).
 * @param data_len Payload data length in bytes.
 * @param circ True if circulating datagram flag set.
 * @return Output byte offset after added datagram, or 0 on error / insufficient space.
 */
size_t syn_ecat_frame_add_datagram(uint8_t *buf, size_t buf_size, SYN_EcatCmd cmd, uint8_t idx,
                                   uint32_t addr, const uint8_t *data, uint16_t data_len,
                                   bool circ);

/**
 * @brief Finalize EtherCAT frame header length field.
 *
 * @param buf Target output buffer.
 * @return Total frame size in bytes (including 2-byte EtherCAT header).
 */
size_t syn_ecat_frame_finalize(uint8_t *buf);

/**
 * @brief Parse the next datagram from an EtherCAT frame buffer.
 *
 * @param frame Pointer to raw EtherCAT frame bytes (starts with 2-byte EtherCAT header).
 * @param frame_len Total byte length of frame buffer.
 * @param offset In/out byte offset into frame buffer. Set *offset = 2 to start.
 * @param out [out] Result struct to receive parsed datagram details.
 * @return true if datagram parsed successfully, false if no more datagrams or malformed.
 */
bool syn_ecat_frame_parse_next(const uint8_t *frame, size_t frame_len, size_t *offset,
                               SYN_EcatDatagramResult *out);

/**
 * @brief Encode a CoE SDO Download (Write) request into a mailbox buffer.
 * @param buf       Output buffer.
 * @param buf_len   Capacity of output buffer.
 * @param index     Object Dictionary Index (0x0000 - 0xFFFF).
 * @param subindex  Subindex (0x00 - 0xFF).
 * @param data      Data bytes to write.
 * @param data_len  Data length (1-4 bytes for expedited SDO).
 * @return Encoded CoE Mailbox packet length in bytes, or 0 on error.
 */
size_t syn_ecat_coe_encode_sdo_download(uint8_t *buf, size_t buf_len, uint16_t index,
                                        uint8_t subindex, const void *data, size_t data_len);

/**
 * @brief Encode a CoE SDO Upload (Read) request into a mailbox buffer.
 * @param buf       Output buffer.
 * @param buf_len   Capacity of output buffer.
 * @param index     Object Dictionary Index (0x0000 - 0xFFFF).
 * @param subindex  Subindex (0x00 - 0xFF).
 * @return Encoded CoE Mailbox packet length in bytes, or 0 on error.
 */
size_t syn_ecat_coe_encode_sdo_upload(uint8_t *buf, size_t buf_len, uint16_t index,
                                      uint8_t subindex);

/* ── Phase 3: Mailbox & CoE SDO API ─────────────────────────────────────── */

/**
 * @brief Encode a complete Mailbox-wrapped CoE SDO Download request frame into tx_buf.
 *
 * @param m Pointer to master handle.
 * @param station_addr Target slave station address.
 * @param index Object Dictionary Index.
 * @param subindex Subindex.
 * @param data Pointer to payload bytes (<=4 bytes for expedited SDO).
 * @param data_len Payload byte length.
 * @return Transmit frame byte length, or 0 on error.
 */
size_t syn_ecat_encode_coe_sdo_write(SYN_EcatMaster *m, uint16_t station_addr, uint16_t index,
                                     uint8_t subindex, const void *data, size_t data_len);

/**
 * @brief Encode a complete Mailbox-wrapped CoE SDO Upload request frame into tx_buf.
 *
 * @param m Pointer to master handle.
 * @param station_addr Target slave station address.
 * @param index Object Dictionary Index.
 * @param subindex Subindex.
 * @return Transmit frame byte length, or 0 on error.
 */
size_t syn_ecat_encode_coe_sdo_read(SYN_EcatMaster *m, uint16_t station_addr, uint16_t index,
                                    uint8_t subindex);

/**
 * @brief Decode SDO Upload response payload from rx_buf.
 *
 * @param m Pointer to master handle.
 * @param rx_len Received frame length.
 * @param out_data Output destination buffer.
 * @param max_len Maximum output buffer capacity.
 * @param out_len Pointer to receive read byte count.
 * @return SYN_OK on success.
 */
SYN_Status syn_ecat_decode_coe_sdo_response(SYN_EcatMaster *m, size_t rx_len, void *out_data,
                                            size_t max_len, size_t *out_len);

/* ── Phase 4: Cyclic Process Data Exchange API ──────────────────────────── */

/**
 * @brief Encode LRW (Logical Read Write) cyclic process exchange frame into tx_buf.
 *
 * @param m Pointer to master handle.
 * @return Transmit frame length in bytes, or 0 on error.
 */
size_t syn_ecat_encode_cyclic(SYN_EcatMaster *m);

/**
 * @brief Parse LRW cyclic process exchange response from rx_buf and copy inputs to input_image.
 *
 * @param m Pointer to master handle.
 * @param rx_len Received frame byte length.
 * @return SYN_OK if WKC >= wkc_expected, SYN_ERROR on WKC mismatch or parsing error.
 */
SYN_Status syn_ecat_decode_cyclic(SYN_EcatMaster *m, size_t rx_len);

/* ── Phase 5: ESM State Transitions ─────────────────────────────────────── */

/**
 * @brief Encode Configured Write (FPWR) to set slave Application Layer (AL) Control register.
 *
 * @param m Pointer to master handle.
 * @param station_addr Target slave station address.
 * @param state Requested ESM State (INIT, PREOP, SAFEOP, OP).
 * @return Transmit frame length in bytes, or 0 on error.
 */
size_t syn_ecat_encode_set_al_control(SYN_EcatMaster *m, uint16_t station_addr,
                                      SYN_EcatState state);

/* ── Phase 6: Distributed Clocks (DC) API ───────────────────────────────── */

/**
 * @brief Encode Auto-Increment Read Multiple Write (ARMW) for DC System Time propagation.
 *
 * @param m Pointer to master handle.
 * @return Transmit frame length in bytes, or 0 on error.
 */
size_t syn_ecat_encode_dc_read_system_time(SYN_EcatMaster *m);

/**
 * @brief Encode FPWR to configure DC SYNC0 cycle time and shift for a slave.
 *
 * @param m Pointer to master handle.
 * @param station_addr Target slave station address.
 * @param sync0_cycle_ns SYNC0 cycle time in nanoseconds.
 * @param sync0_shift_ns SYNC0 shift time in nanoseconds.
 * @return Transmit frame length in bytes, or 0 on error.
 */
size_t syn_ecat_encode_dc_configure(SYN_EcatMaster *m, uint16_t station_addr,
                                    uint32_t sync0_cycle_ns, int32_t sync0_shift_ns);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_ETHERCAT */

#endif /* SYN_ETHERCAT_H */
