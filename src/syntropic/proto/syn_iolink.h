/**
 * @file syn_iolink.h
 * @brief IO-Link (IEC 61131-9) Single-Drop Digital Point-to-Point Master & Device Protocol Engine.
 * @ingroup syn_proto
 *
 * Implements a zero-heap, deterministic IO-Link protocol engine:
 * - Physical transmission rate configuration (COM1: 4.8k, COM2: 38.4k, COM3: 230.4k baud).
 * - M-Sequence frame encoding & parsing (Type 0, Type 1_1, Type 1_2, Type 2_1, Type 2_2, Type 2_V)
 * with 6-bit CRC.
 * - Direct Parameter Page 1 handling (Vendor ID, Device ID, Min Cycle Time, Revision ID, IO-Link
 * profile).
 * - Acyclic ISDU (Indexed Service Data Unit) parameter read/write transactions.
 * - Non-blocking master polling step and cooperative protothread runner.
 */

#ifndef SYN_IOLINK_H
#define SYN_IOLINK_H

#include "../common/syn_defs.h"
#include "../net/syn_transport.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_sched.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_IOLINK) || SYN_USE_IOLINK

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Parameter Page Addresses ───────────────────────────────── */

#define SYN_IOLINK_PAGE_MASTER_CYCLE_TIME 0x00U /**< Master cycle time address */
#define SYN_IOLINK_PAGE_MIN_CYCLE_TIME 0x01U    /**< Min cycle time address */
#define SYN_IOLINK_PAGE_MSEQ_CAPABILITY 0x02U   /**< M-Sequence capability */
#define SYN_IOLINK_PAGE_REVISION_ID 0x03U       /**< Protocol revision ID (0x11 = v1.1) */
#define SYN_IOLINK_PAGE_PD_IN_LEN 0x04U         /**< Process data in length */
#define SYN_IOLINK_PAGE_PD_OUT_LEN 0x05U        /**< Process data out length */
#define SYN_IOLINK_PAGE_VENDOR_ID_1 0x06U       /**< Vendor ID MSB */
#define SYN_IOLINK_PAGE_VENDOR_ID_2 0x07U       /**< Vendor ID LSB */
#define SYN_IOLINK_PAGE_DEVICE_ID_1 0x08U       /**< Device ID MSB */
#define SYN_IOLINK_PAGE_DEVICE_ID_2 0x09U       /**< Device ID Mid */
#define SYN_IOLINK_PAGE_DEVICE_ID_3 0x0AU       /**< Device ID LSB */

#define SYN_IOLINK_REV_1_0 0x10U /**< IO-Link v1.0 */
#define SYN_IOLINK_REV_1_1 0x11U /**< IO-Link v1.1 */

#define SYN_IOLINK_MAX_PD_LEN 32U   /**< Max Process Data length in bytes */
#define SYN_IOLINK_MAX_ISDU_LEN 64U /**< Max ISDU payload length */

/* ── Type Definitions ────────────────────────────────────────────────────── */

/**
 * @brief IO-Link transmission speed baud rate.
 */
typedef enum {
    SYN_IOLINK_BAUD_COM1 = 0, /**< 4.8 kbit/s */
    SYN_IOLINK_BAUD_COM2 = 1, /**< 38.4 kbit/s */
    SYN_IOLINK_BAUD_COM3 = 2  /**< 230.4 kbit/s */
} SYN_IOLink_Baud;

/**
 * @brief Port operational state.
 */
typedef enum {
    SYN_IOLINK_PORT_INACTIVE = 0, /**< Port is inactive / unpowered */
    SYN_IOLINK_PORT_SIO,          /**< Standard I/O mode (Digital I/O) */
    SYN_IOLINK_PORT_STARTUP,      /**< Wake-up & parameter negotiation */
    SYN_IOLINK_PORT_OPERATE,      /**< Cyclic process data exchange */
    SYN_IOLINK_PORT_FAULT         /**< Communication failure / fault */
} SYN_IOLink_PortState;

/**
 * @brief M-Sequence message type.
 */
typedef enum {
    SYN_IOLINK_TYPE_0 = 0,   /**< 1 OD byte, 0 PD bytes */
    SYN_IOLINK_TYPE_1_1 = 1, /**< 1 PD byte, 1 OD byte */
    SYN_IOLINK_TYPE_1_2 = 2, /**< 2 PD bytes, 1 OD byte */
    SYN_IOLINK_TYPE_2_1 = 3, /**< 1 PD byte, 2 OD bytes */
    SYN_IOLINK_TYPE_2_2 = 4, /**< 2 PD bytes, 2 OD bytes */
    SYN_IOLINK_TYPE_2_V = 5  /**< Variable PD bytes, 1 OD byte */
} SYN_IOLink_MSeqType;

/**
 * @brief Direct Parameter Page 1 structure.
 */
typedef struct {
    uint8_t master_cycle_time; /**< Configured master cycle time */
    uint8_t min_cycle_time;    /**< Minimum supported device cycle time */
    uint8_t mseq_capability;   /**< Supported M-Sequence capability */
    uint8_t revision_id;       /**< IO-Link revision ID (0x10 or 0x11) */
    uint8_t pd_in_len;         /**< Process data input length in bytes */
    uint8_t pd_out_len;        /**< Process data output length in bytes */
    uint16_t vendor_id;        /**< 16-bit Vendor Identifier */
    uint32_t device_id;        /**< 24-bit Device Identifier */
} SYN_IOLink_DirectParams;

/**
 * @brief Callback for dynamic ISDU parameter read.
 */
typedef SYN_Status (*SYN_IOLink_ISDU_ReadCb)(uint16_t index, uint8_t subindex, uint8_t *out_data,
                                             size_t max_len, size_t *out_len, void *user_data);

/**
 * @brief Callback for dynamic ISDU parameter write.
 */
typedef SYN_Status (*SYN_IOLink_ISDU_WriteCb)(uint16_t index, uint8_t subindex, const uint8_t *data,
                                              size_t len, void *user_data);

/**
 * @brief Master configuration descriptor.
 */
typedef struct {
    SYN_IOLink_Baud target_baud; /**< Target communication baud rate */
    SYN_Transport *transport;    /**< Serial / UART transport interface */
    uint32_t cycle_time_ms;      /**< Master cycle period in milliseconds */
    void *user_data;             /**< User context pointer */

    uint8_t *rx_buf;    /**< Scratch RX buffer */
    size_t rx_buf_size; /**< RX buffer size (>= 64 bytes) */
    uint8_t *tx_buf;    /**< Scratch TX buffer */
    size_t tx_buf_size; /**< TX buffer size (>= 64 bytes) */
} SYN_IOLink_MasterConfig;

/**
 * @brief IO-Link Master instance context.
 */
typedef struct {
    SYN_IOLink_MasterConfig cfg; /**< Configuration */
    SYN_IOLink_PortState state;  /**< Active port state */
    SYN_PT pt;                   /**< Protothread state */

    SYN_IOLink_DirectParams dev_params; /**< Discovered device parameters */
    SYN_IOLink_MSeqType mseq_type;      /**< Active M-Sequence type */
    uint32_t last_cycle_ms;             /**< Timestamp of last cycle execution */
    uint32_t rx_deadline_ms;            /**< Deadline timestamp for pending async RX frame */
    bool awaiting_rx;                   /**< True while waiting for device PD response */

    uint8_t current_pd_in[SYN_IOLINK_MAX_PD_LEN];  /**< Latest input process data */
    size_t current_pd_in_len;                      /**< Input process data length */
    uint8_t current_pd_out[SYN_IOLINK_MAX_PD_LEN]; /**< Active output process data */
    size_t current_pd_out_len;                     /**< Output process data length */
} SYN_IOLink_Master;

/**
 * @brief Device configuration descriptor.
 */
typedef struct {
    SYN_IOLink_DirectParams params;   /**< Device parameter page configuration */
    SYN_IOLink_ISDU_ReadCb on_read;   /**< Dynamic ISDU read callback */
    SYN_IOLink_ISDU_WriteCb on_write; /**< Dynamic ISDU write callback */
    void *user_data;                  /**< User context pointer */
} SYN_IOLink_DeviceConfig;

/**
 * @brief IO-Link Device instance context.
 */
typedef struct {
    SYN_IOLink_DeviceConfig cfg; /**< Configuration */
    SYN_IOLink_PortState state;  /**< Port state */

    uint8_t pd_in[SYN_IOLINK_MAX_PD_LEN];  /**< Sensor input process data buffer */
    uint8_t pd_out[SYN_IOLINK_MAX_PD_LEN]; /**< Actuator output process data buffer */
} SYN_IOLink_Device;

/* ── CRC & Framing Functions ─────────────────────────────────────────────── */

/**
 * @brief Calculate standard IO-Link 6-bit CRC with parity.
 * @param data Data buffer to calculate checksum over.
 * @param len  Data length in bytes.
 * @return 8-bit Checksum byte (6-bit CRC in bits 0..5, parity in bits 6..7).
 */
uint8_t syn_iolink_calc_checksum(const uint8_t *data, size_t len);

/* ── Master Public API ───────────────────────────────────────────────────── */

/**
 * @brief Initialize IO-Link Master.
 * @param master Master instance to initialize.
 * @param cfg    Configuration parameters.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid argument.
 */
SYN_Status syn_iolink_master_init(SYN_IOLink_Master *master, const SYN_IOLink_MasterConfig *cfg);

/**
 * @brief Trigger master startup handshake sequence on connected port.
 * @param master Master instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_iolink_master_start(SYN_IOLink_Master *master);

/**
 * @brief Perform a cyclic Process Data exchange with connected device.
 * @param master  Master instance.
 * @param pd_out  Output process data to send to device (actuator command).
 * @param out_len Length of output process data.
 * @param pd_in   Buffer receiving input process data from device (sensor measurement).
 * @param in_len  Pointer receiving length of input process data.
 * @return SYN_OK on success.
 */
SYN_Status syn_iolink_master_exchange_pd(SYN_IOLink_Master *master, const uint8_t *pd_out,
                                         size_t out_len, uint8_t *pd_in, size_t *in_len);

/**
 * @brief Read an acyclic ISDU parameter from device.
 * @param master   Master instance.
 * @param index    16-bit ISDU Index.
 * @param subindex 8-bit ISDU Subindex.
 * @param data     Buffer receiving parameter data.
 * @param max_len  Capacity of parameter buffer.
 * @param out_len  Pointer receiving parameter data length.
 * @return SYN_OK on success.
 */
SYN_Status syn_iolink_master_read_isdu(SYN_IOLink_Master *master, uint16_t index, uint8_t subindex,
                                       uint8_t *data, size_t max_len, size_t *out_len);

/**
 * @brief Write an acyclic ISDU parameter to device.
 * @param master   Master instance.
 * @param index    16-bit ISDU Index.
 * @param subindex 8-bit ISDU Subindex.
 * @param data     Parameter data to write.
 * @param len      Parameter data length.
 * @return SYN_OK on success.
 */
SYN_Status syn_iolink_master_write_isdu(SYN_IOLink_Master *master, uint16_t index, uint8_t subindex,
                                        const uint8_t *data, size_t len);

/**
 * @brief Step the IO-Link master state machine and cyclic communication loop.
 * @param master Master instance.
 * @param now_ms Current timestamp in milliseconds.
 * @return SYN_OK on success.
 */
SYN_Status syn_iolink_master_step(SYN_IOLink_Master *master, uint32_t now_ms);

/**
 * @brief Protothread continuation function for `syn_sched`.
 * @param pt   Protothread continuation structure.
 * @param task Task handle containing SYN_IOLink_Master in user_data.
 * @return SYN_PT_Status continuation status.
 */
SYN_PT_Status syn_iolink_master_pt(SYN_PT *pt, SYN_Task *task);

/* ── Device Public API ───────────────────────────────────────────────────── */

/**
 * @brief Initialize IO-Link Device.
 * @param dev Device instance to initialize.
 * @param cfg Configuration parameters and parameter page values.
 * @return SYN_OK on success.
 */
SYN_Status syn_iolink_device_init(SYN_IOLink_Device *dev, const SYN_IOLink_DeviceConfig *cfg);

/**
 * @brief Set the device's sensor input process data.
 * @param dev  Device instance.
 * @param data Input process data payload.
 * @param len  Length of process data.
 * @return SYN_OK on success.
 */
SYN_Status syn_iolink_device_set_pd_in(SYN_IOLink_Device *dev, const uint8_t *data, size_t len);

/**
 * @brief Get the device's latest actuator output process data written by master.
 * @param dev     Device instance.
 * @param out_buf Buffer receiving process data.
 * @param max_len Capacity of output buffer.
 * @param out_len Pointer receiving length.
 * @return SYN_OK on success.
 */
SYN_Status syn_iolink_device_get_pd_out(const SYN_IOLink_Device *dev, uint8_t *out_buf,
                                        size_t max_len, size_t *out_len);

/**
 * @brief Process an inbound M-Sequence master frame and generate the device response frame.
 * @param dev     Device instance.
 * @param rx      Master frame payload.
 * @param rx_len  Master frame length.
 * @param tx      Buffer receiving device response frame.
 * @param max_tx  Capacity of response buffer.
 * @param out_len Pointer receiving response length.
 * @return SYN_OK on success.
 */
SYN_Status syn_iolink_device_process_frame(SYN_IOLink_Device *dev, const uint8_t *rx, size_t rx_len,
                                           uint8_t *tx, size_t max_tx, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_IOLINK */

#endif /* SYN_IOLINK_H */
