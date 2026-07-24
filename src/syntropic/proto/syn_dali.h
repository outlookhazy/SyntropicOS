/**
 * @file syn_dali.h
 * @brief Digital Addressable Lighting Interface (DALI / IEC 62386) Protocol Stack.
 *
 * Implements non-blocking, zero-malloc DALI (IEC 62386-101/102) Manchester bi-phase
 * frame encoding/decoding, address matching (Short, Group, Broadcast), Direct Arc Power
 * control ($0..254$), standard commands, configuration commands, and query responses.
 *
 * @par Usage
 * @code
 *   static SYN_DALI_SlaveState dali_slave;
 *   SYN_DALI_SlaveConfig cfg = {
 *       .short_address = 5,
 *       .min_level = 10,
 *       .max_level = 254,
 *       .power_on_level = 254,
 *       .system_failure_level = 254
 *   };
 *   syn_dali_slave_init(&dali_slave, &cfg);
 *
 *   SYN_DALI_ForwardFrame req;
 *   if (syn_dali_decode_forward(raw_16, &req)) {
 *       uint8_t resp = 0;
 *       bool has_resp = false;
 *       syn_dali_slave_process(&dali_slave, &req, &resp, &has_resp);
 *   }
 * @endcode
 * @ingroup syn_proto
 */

#ifndef SYN_DALI_H
#define SYN_DALI_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── DALI Constants ─────────────────────────────────────────────────────── */

#define SYN_DALI_SHORT_ADDR_UNASSIGNED   0xFFU   /**< Unassigned Short Address */
#define SYN_DALI_MASK_LEVEL              255U    /**< MASK level (no change)   */

/** @brief DALI Address Types */
typedef enum {
    SYN_DALI_ADDR_SHORT = 0,    /**< Short Address (0..63)                    */
    SYN_DALI_ADDR_GROUP,        /**< Group Address (0..15)                    */
    SYN_DALI_ADDR_BROADCAST,    /**< Broadcast Address (all control gear)     */
    SYN_DALI_ADDR_SPECIAL,      /**< Special / Configuration Address          */
} SYN_DALI_AddressType;

/** @brief DALI Forward Frame (Master -> Control Gear, 16-bit payload) */
typedef struct {
    SYN_DALI_AddressType  addr_type;  /**< Decoded address type             */
    uint8_t               address;    /**< Target address value             */
    bool                  is_direct;  /**< true = Direct Arc Level, false = Cmd */
    uint8_t               data_cmd;   /**< Direct Arc Level (0..254) or Cmd */
    uint16_t              raw;        /**< Raw 16-bit payload               */
} SYN_DALI_ForwardFrame;

/** @brief DALI Backward Frame (Control Gear -> Master, 8-bit payload) */
typedef struct {
    uint8_t               data;       /**< 8-bit response payload           */
} SYN_DALI_BackwardFrame;

/** @brief DALI Slave Configuration */
typedef struct {
    uint8_t               short_address;        /**< Short Address (0..63 or 0xFF) */
    uint16_t              group_mask;           /**< Bitmask of assigned Groups (0..15) */
    uint8_t               min_level;            /**< Minimum physical arc level (1..254) */
    uint8_t               max_level;            /**< Maximum physical arc level (1..254) */
    uint8_t               power_on_level;       /**< Power-on arc level (0..254, 255=MASK) */
    uint8_t               system_failure_level; /**< Failure arc level (0..254, 255=MASK) */
    uint8_t               fade_rate;            /**< Fade rate setting (1..15) */
    uint8_t               fade_time;            /**< Fade time setting (1..15) */
} SYN_DALI_SlaveConfig;

/** @brief DALI Slave Runtime State */
typedef struct {
    SYN_DALI_SlaveConfig  cfg;
    uint8_t               actual_level;         /**< Current arc power level (0..254) */
    uint8_t               scenes[16];           /**< Scene levels 0..15 (255=MASK) */
    uint32_t              random_address;       /**< 24-bit Random Search Address */
    uint32_t              search_address;       /**< 24-bit Search Address comparator */
    bool                  initialise_active;    /**< True when initialisation active */
    bool                  lamp_failure;         /**< Lamp failure flag */
    bool                  control_gear_failure; /**< Gear failure flag */
    bool                  lamp_on;              /**< Lamp status flag */
    uint8_t               dtr0;                 /**< Data Transfer Register 0 */
    uint8_t               dtr1;                 /**< Data Transfer Register 1 */
    uint8_t               dtr2;                 /**< Data Transfer Register 2 */
    bool                  write_mem_enabled;    /**< True when write memory enabled */
} SYN_DALI_SlaveState;

/* ── Standard DALI Command Definitions (IEC 62386-102) ─────────────────── */

#define SYN_DALI_CMD_OFF                         0x00U
#define SYN_DALI_CMD_UP                          0x01U
#define SYN_DALI_CMD_DOWN                        0x02U
#define SYN_DALI_CMD_STEP_UP                     0x03U
#define SYN_DALI_CMD_STEP_DOWN                   0x04U
#define SYN_DALI_CMD_RECALL_MAX                  0x05U
#define SYN_DALI_CMD_RECALL_MIN                  0x06U
#define SYN_DALI_CMD_STEP_DOWN_AND_OFF           0x07U
#define SYN_DALI_CMD_ON_AND_STEP_UP              0x08U
#define SYN_DALI_CMD_GO_TO_SCENE_BASE            0x10U /* 0x10..0x1F */

#define SYN_DALI_CMD_RESET                       0x20U
#define SYN_DALI_CMD_STORE_ACTUAL_LEVEL_IN_DTR   0x21U
#define SYN_DALI_CMD_STORE_DTR_AS_MAX_LEVEL      0x2AU
#define SYN_DALI_CMD_STORE_DTR_AS_MIN_LEVEL      0x2BU
#define SYN_DALI_CMD_STORE_DTR_AS_SYS_FAIL_LEVEL 0x2CU
#define SYN_DALI_CMD_STORE_DTR_AS_POWER_ON_LEVEL 0x2DU
#define SYN_DALI_CMD_STORE_DTR_AS_FADE_TIME      0x2EU
#define SYN_DALI_CMD_STORE_DTR_AS_FADE_RATE      0x2FU

#define SYN_DALI_CMD_STORE_DTR_AS_SCENE_BASE     0x40U /* 0x40..0x4F */
#define SYN_DALI_CMD_REMOVE_FROM_SCENE_BASE      0x50U /* 0x50..0x5F */
#define SYN_DALI_CMD_ADD_TO_GROUP_BASE           0x60U /* 0x60..0x6F */
#define SYN_DALI_CMD_REMOVE_FROM_GROUP_BASE      0x70U /* 0x70..0x7F */
#define SYN_DALI_CMD_STORE_DTR_AS_SHORT_ADDR     0x80U
#define SYN_DALI_CMD_ENABLE_WRITE_MEMORY         0x81U

#define SYN_DALI_CMD_QUERY_STATUS                0x90U
#define SYN_DALI_CMD_QUERY_CONTROL_GEAR          0x91U
#define SYN_DALI_CMD_QUERY_LAMP_FAILURE          0x92U
#define SYN_DALI_CMD_QUERY_LAMP_POWER_ON         0x93U
#define SYN_DALI_CMD_QUERY_CONTENT_DTR           0x98U
#define SYN_DALI_CMD_QUERY_DEVICE_TYPE           0x99U
#define SYN_DALI_CMD_QUERY_PHYSICAL_MIN_LEVEL    0x9AU
#define SYN_DALI_CMD_QUERY_CONTENT_DTR1          0x9CU
#define SYN_DALI_CMD_QUERY_CONTENT_DTR2          0x9DU
#define SYN_DALI_CMD_QUERY_ACTUAL_LEVEL          0xA0U
#define SYN_DALI_CMD_QUERY_MAX_LEVEL             0xA1U
#define SYN_DALI_CMD_QUERY_MIN_LEVEL             0xA2U
#define SYN_DALI_CMD_QUERY_POWER_ON_LEVEL        0xA3U
#define SYN_DALI_CMD_QUERY_SYS_FAIL_LEVEL        0xA4U

#define SYN_DALI_CMD_QUERY_GROUPS_0_7            0xC0U
#define SYN_DALI_CMD_QUERY_GROUPS_8_15           0xC1U
#define SYN_DALI_CMD_QUERY_RANDOM_ADDR_H         0xC2U
#define SYN_DALI_CMD_QUERY_RANDOM_ADDR_M         0xC3U
#define SYN_DALI_CMD_QUERY_RANDOM_ADDR_L         0xC4U

/* Special / Configuration Commands */
#define SYN_DALI_SPEC_TERMINATE                  0x00U
#define SYN_DALI_SPEC_DTR0                       0xA3U
#define SYN_DALI_SPEC_INITIALISE                 0x01U
#define SYN_DALI_SPEC_RANDOMISE                  0x02U
#define SYN_DALI_SPEC_COMPARE                    0x03U
#define SYN_DALI_SPEC_WITHDRAW                   0x04U
#define SYN_DALI_SPEC_SEARCHADDRH                0x05U
#define SYN_DALI_SPEC_SEARCHADDRM                0x06U
#define SYN_DALI_SPEC_SEARCHADDRL                0x07U
#define SYN_DALI_SPEC_PROGRAM_SHORT_ADDR         0x08U
#define SYN_DALI_SPEC_VERIFY_SHORT_ADDR          0x09U
#define SYN_DALI_SPEC_QUERY_SHORT_ADDR           0x0AU
#define SYN_DALI_SPEC_DTR1                       0xC3U
#define SYN_DALI_SPEC_DTR2                       0xC5U

/* ── API Functions ──────────────────────────────────────────────────────── */

/**
 * @brief Encode a 16-bit Master Forward Frame.
 * @param addr      Address byte (Short, Group, Broadcast).
 * @param data_cmd  Data / Command byte.
 * @return Raw 16-bit frame integer.
 */
uint16_t syn_dali_encode_forward(uint8_t addr, uint8_t data_cmd);

/**
 * @brief Decode a 16-bit Master Forward Frame.
 * @param raw_16  Raw 16-bit frame integer.
 * @param frame   [out] Decoded frame structure.
 * @return true on valid frame decoding.
 */
bool syn_dali_decode_forward(uint16_t raw_16, SYN_DALI_ForwardFrame *frame);

/**
 * @brief Encode an 8-bit Slave Backward Frame.
 * @param data  8-bit response payload.
 * @return Raw 8-bit frame integer.
 */
uint8_t syn_dali_encode_backward(uint8_t data);

/**
 * @brief Decode an 8-bit Slave Backward Frame.
 * @param raw_8  Raw 8-bit frame integer.
 * @param frame  [out] Decoded frame structure.
 * @return true on valid frame decoding.
 */
bool syn_dali_decode_backward(uint8_t raw_8, SYN_DALI_BackwardFrame *frame);

/**
 * @brief Initialize a DALI Slave Control Gear instance.
 * @param slave  Slave state handle.
 * @param cfg    Slave configuration pointer.
 * @return SYN_OK on success, error code otherwise.
 */
SYN_Status syn_dali_slave_init(SYN_DALI_SlaveState *slave, const SYN_DALI_SlaveConfig *cfg);

/**
 * @brief Process an incoming DALI Forward Frame on a Slave Control Gear node.
 * @param slave      Slave state handle.
 * @param req        Incoming forward frame request.
 * @param resp_data  [out] 8-bit response payload (if has_resp is set to true).
 * @param has_resp   [out] Set to true if a backward response frame must be transmitted.
 * @return SYN_OK on success, error code on invalid parameters.
 */
SYN_Status syn_dali_slave_process(SYN_DALI_SlaveState *slave,
                                  const SYN_DALI_ForwardFrame *req,
                                  uint8_t *resp_data,
                                  bool *has_resp);

/**
 * @brief Encode a byte sequence into Manchester bi-phase bits.
 * @param val      Input byte value.
 * @param bit_out  Output uint16_t array (minimum 16 elements for bi-phase pairs).
 * @return Number of bi-phase transitions written.
 */
size_t syn_dali_manchester_encode_byte(uint8_t val, uint8_t *bit_out);

/**
 * @brief Decode Manchester bi-phase bit pairs into a byte value.
 * @param bit_in  Input uint8_t array containing bi-phase pairs.
 * @param val_out [out] Decoded byte.
 * @return true on valid Manchester alignment.
 */
bool syn_dali_manchester_decode_byte(const uint8_t *bit_in, uint8_t *val_out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DALI_H */
