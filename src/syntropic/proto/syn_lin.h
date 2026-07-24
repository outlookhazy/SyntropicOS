/**
 * @file syn_lin.h
 * @brief LIN (Local Interconnect Network 2.1 / 2.2a & ISO 17987) Protocol Engine.
 *
 * Provides non-blocking, zero-allocation LIN frame parsing, PID parity logic,
 * classic/enhanced checksum verification, Master schedule table execution,
 * and Slave response publishing/subscription state machines.
 * @ingroup syn_proto
 */

#ifndef SYN_LIN_H
#define SYN_LIN_H

#include "../common/syn_defs.h"

#if defined(SYN_USE_LIN) && SYN_USE_LIN

#ifdef __cplusplus
extern "C" {
#endif

/* ── LIN Frame Identifiers & Limits ──────────────────────────────────────── */

#define SYN_LIN_ID_MAX             0x3F  /**< Maximum 6-bit LIN Frame ID (0..63) */
#define SYN_LIN_DATA_MAX           8     /**< Maximum payload bytes per LIN frame */

#define SYN_LIN_ID_MASTER_REQ      0x3C  /**< LIN Diagnostic Master Request ID */
#define SYN_LIN_ID_SLAVE_RESP      0x3D  /**< LIN Diagnostic Slave Response ID */
#define SYN_LIN_ID_EXTENDED        0x3E  /**< LIN Extended Frame ID */
#define SYN_LIN_ID_RESERVED        0x3F  /**< LIN Reserved Frame ID */

#define SYN_LIN_SYNC_BYTE          0x55  /**< LIN Sync Byte */

/* ── LIN Checksum Modes ─────────────────────────────────────────────────── */

/**
 * @brief LIN Checksum calculation mode.
 */
typedef enum {
    SYN_LIN_CHECKSUM_CLASSIC = 0,  /**< Classic LIN 1.3 checksum (data bytes only; forced for 0x3C/0x3D) */
    SYN_LIN_CHECKSUM_ENHANCED = 1  /**< Enhanced LIN 2.0+ checksum (PID + data bytes) */
} SYN_LIN_ChecksumMode;

/* ── LIN Frame Data Structure ────────────────────────────────────────────── */

/**
 * @brief LIN Frame representation.
 */
typedef struct {
    uint8_t id;                       /**< 6-bit Frame ID (0..63) */
    uint8_t pid;                      /**< Protected Identifier byte (ID + Parity P0/P1) */
    uint8_t len;                      /**< Data payload length (1..8 bytes) */
    uint8_t data[SYN_LIN_DATA_MAX];   /**< Payload data bytes */
    uint8_t checksum;                 /**< Received or computed checksum byte */
    bool    valid_pid;                /**< True if PID parity bits are valid */
    bool    valid_checksum;           /**< True if checksum matches frame payload */
} SYN_LIN_Frame;

/* ── LIN Master Schedule Table Slot ──────────────────────────────────────── */

/**
 * @brief Direction of data for a Master schedule table slot.
 */
typedef enum {
    SYN_LIN_SLOT_PUBLISH = 0,   /**< Master transmits header and response data */
    SYN_LIN_SLOT_SUBSCRIBE = 1  /**< Master transmits header and receives slave response data */
} SYN_LIN_SlotDirection;

/**
 * @brief Schedule table slot entry for LIN Master.
 */
typedef struct {
    uint8_t              id;          /**< 6-bit LIN Frame ID (0..63) */
    uint8_t              len;         /**< Payload length (1..8 bytes) */
    SYN_LIN_ChecksumMode checksum_mode; /**< Classic vs. Enhanced checksum */
    SYN_LIN_SlotDirection dir;        /**< Master publish or subscribe */
    uint8_t              data[SYN_LIN_DATA_MAX]; /**< Data to publish (if PUBLISH) */
    uint32_t             delay_ms;    /**< Delay slot duration in milliseconds */
} SYN_LIN_ScheduleSlot;

/* ── LIN Master Handle ──────────────────────────────────────────────────── */

/**
 * @brief LIN Master state machine handle.
 */
typedef struct {
    const SYN_LIN_ScheduleSlot *slots;       /**< Pointer to schedule table array */
    size_t                      slot_count;  /**< Total number of slots in schedule table */
    size_t                      current_slot;/**< Currently active slot index */
    uint32_t                    slot_timer;  /**< Millisecond timer accumulator */
    bool                        running;     /**< Schedule table execution status */
} SYN_LIN_Master;

/* ── LIN Slave Published/Subscribed Frame Filter ─────────────────────────── */

/**
 * @brief Configuration for a single frame ID handled by a LIN Slave.
 */
typedef struct {
    uint8_t              id;            /**< 6-bit Frame ID (0..63) */
    uint8_t              len;           /**< Payload length (1..8 bytes) */
    SYN_LIN_ChecksumMode checksum_mode; /**< Classic vs. Enhanced checksum */
    SYN_LIN_SlotDirection dir;          /**< SUBSCRIBE = slave receives, PUBLISH = slave transmits */
    uint8_t              data[SYN_LIN_DATA_MAX]; /**< Published data buffer */
    bool                 active;        /**< Entry active flag */
} SYN_LIN_SlaveFrame;

/* ── LIN Slave Handle ───────────────────────────────────────────────────── */

#ifndef SYN_LIN_SLAVE_MAX_FRAMES
  #define SYN_LIN_SLAVE_MAX_FRAMES 8  /**< Maximum configured frame slots per LIN Slave */
#endif

typedef enum {
    SYN_LIN_STATE_IDLE = 0,
    SYN_LIN_STATE_SYNC,
    SYN_LIN_STATE_PID,
    SYN_LIN_STATE_DATA,
    SYN_LIN_STATE_CHECKSUM
} SYN_LIN_State;

/**
 * @brief LIN Slave state machine handle.
 */
typedef struct {
    uint8_t            nad;                                /**< LIN Node Address (for 0x3C/0x3D diagnostics) */
    SYN_LIN_SlaveFrame frames[SYN_LIN_SLAVE_MAX_FRAMES];   /**< Configured frame filter table */
    size_t             frame_count;                        /**< Number of configured frame filters */
    SYN_LIN_State      state;                              /**< Current byte stream parser state */
    SYN_LIN_Frame      rx_frame;                           /**< Scratch RX frame buffer */
    uint8_t            rx_idx;                             /**< Current payload byte index */
    uint8_t            expected_len;                       /**< Expected payload length for active frame */
    SYN_LIN_ChecksumMode expected_checksum_mode;          /**< Active checksum mode */
} SYN_LIN_Slave;

/* ── API Functions ──────────────────────────────────────────────────────── */

/**
 * @brief Compute LIN Protected Identifier (PID) from a 6-bit Frame ID.
 * @param id 6-bit LIN Frame ID (0..63).
 * @return 8-bit Protected Identifier byte with P0/P1 parity bits set.
 */
uint8_t syn_lin_calc_pid(uint8_t id);

/**
 * @brief Verify Protected Identifier (PID) parity bits.
 * @param pid 8-bit PID byte.
 * @return True if P0 and P1 parity bits match the 6-bit Frame ID.
 */
bool syn_lin_verify_pid(uint8_t pid);

/**
 * @brief Compute LIN Checksum (Classic or Enhanced).
 * @param pid 8-bit Protected Identifier (used only in Enhanced mode).
 * @param data Pointer to payload data bytes.
 * @param len Data length (1..8 bytes).
 * @param mode Classic (data only) or Enhanced (PID + data).
 * @return Inverted modulo-255 sum byte.
 */
uint8_t syn_lin_calc_checksum(uint8_t pid, const uint8_t *data, uint8_t len, SYN_LIN_ChecksumMode mode);

/**
 * @brief Initialize LIN Master engine with a schedule table.
 * @param master Pointer to LIN Master handle.
 * @param slots Pointer to array of schedule table slots.
 * @param count Number of slots in table.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_lin_master_init(SYN_LIN_Master *master, const SYN_LIN_ScheduleSlot *slots, size_t count);

/**
 * @brief Step LIN Master schedule table timer.
 * @param master Pointer to LIN Master handle.
 * @param dt_ms Elapsed time since last call in milliseconds.
 * @param active_slot Pointer to store currently executed slot reference (can be NULL).
 * @return True if a new schedule slot trigger occurred.
 */
bool syn_lin_master_step(SYN_LIN_Master *master, uint32_t dt_ms, const SYN_LIN_ScheduleSlot **active_slot);

/**
 * @brief Initialize LIN Slave engine.
 * @param slave Pointer to LIN Slave handle.
 * @param nad Node Address (0..255, 0x7F = wildcard/broadcast).
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid inputs.
 */
SYN_Status syn_lin_slave_init(SYN_LIN_Slave *slave, uint8_t nad);

/**
 * @brief Register a frame filter on LIN Slave.
 * @param slave Pointer to LIN Slave handle.
 * @param id Frame ID (0..63).
 * @param len Payload length (1..8 bytes).
 * @param dir SUBSCRIBE (slave receives) or PUBLISH (slave transmits).
 * @param mode Classic or Enhanced checksum mode.
 * @return SYN_OK on success, SYN_ERROR if slave table is full.
 */
SYN_Status syn_lin_slave_add_frame(SYN_LIN_Slave *slave, uint8_t id, uint8_t len, SYN_LIN_SlotDirection dir, SYN_LIN_ChecksumMode mode);

/**
 * @brief Set published payload data for a specific Frame ID on LIN Slave.
 * @param slave Pointer to LIN Slave handle.
 * @param id Frame ID (0..63).
 * @param data Pointer to payload data bytes.
 * @param len Payload data length.
 * @return SYN_OK on success, SYN_ERROR if ID not registered as PUBLISH.
 */
SYN_Status syn_lin_slave_set_publish_data(SYN_LIN_Slave *slave, uint8_t id, const uint8_t *data, uint8_t len);

/**
 * @brief Process an incoming byte stream byte by byte on LIN Slave.
 * @param slave Pointer to LIN Slave handle.
 * @param byte Incoming UART byte.
 * @param out_frame Pointer to output structure populated when a complete frame is received.
 * @return True if a complete valid frame was processed.
 */
bool syn_lin_slave_process_byte(SYN_LIN_Slave *slave, uint8_t byte, SYN_LIN_Frame *out_frame);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_LIN */

#endif /* SYN_LIN_H */
