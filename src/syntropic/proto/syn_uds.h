/**
 * @file syn_uds.h
 * @brief ISO 14229 Unified Diagnostic Services (UDS) Server Implementation.
 *
 * Provides a zero-allocation, lightweight UDS server protocol stack for
 * automotive diagnostic session management, Data Identifier (DID) read/write,
 * security access seed/key unlocks, routine controls, and ECU reset over ISO-TP.
 */

#ifndef SYN_UDS_H
#define SYN_UDS_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name Configuration Constants */
/**@{*/
#ifndef SYN_UDS_MAX_DIDS
#define SYN_UDS_MAX_DIDS 16U /**< Maximum supported DIDs */
#endif
#ifndef SYN_UDS_MAX_SERVICE_OVERRIDES
#define SYN_UDS_MAX_SERVICE_OVERRIDES \
    8U /**< Maximum supported per-service policy overrides (default: 8) */
#endif
/**@}*/

/** @name UDS Service Identifiers (ISO 14229-1 SID) */
/**@{*/
#define SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL 0x10U       /**< DiagnosticSessionControl */
#define SYN_UDS_SID_ECU_RESET 0x11U                        /**< ECUReset */
#define SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION 0x14U     /**< ClearDiagnosticInformation */
#define SYN_UDS_SID_READ_DTC_INFORMATION 0x19U             /**< ReadDTCInformation */
#define SYN_UDS_SID_READ_DATA_BY_IDENTIFIER 0x22U          /**< ReadDataByIdentifier */
#define SYN_UDS_SID_READ_MEMORY_BY_ADDRESS 0x23U           /**< ReadMemoryByAddress */
#define SYN_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER 0x24U  /**< ReadScalingDataByIdentifier */
#define SYN_UDS_SID_SECURITY_ACCESS 0x27U                  /**< SecurityAccess */
#define SYN_UDS_SID_COMMUNICATION_CONTROL 0x28U            /**< CommunicationControl */
#define SYN_UDS_SID_AUTHENTICATION 0x29U                   /**< Authentication */
#define SYN_UDS_SID_READ_DATA_BY_PERIODIC_IDENTIFIER 0x2AU /**< ReadDataByPeriodicIdentifier */
#define SYN_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER                                  \
    0x2CU                                          /**< DynamicallyDefineDataIdentifier \
                                                    */
#define SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER 0x2EU /**< WriteDataByIdentifier */
#define SYN_UDS_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER                                  \
    0x2FU                                           /**< InputOutputControlByIdentifier \
                                                     */
#define SYN_UDS_SID_ROUTINE_CONTROL 0x31U           /**< RoutineControl */
#define SYN_UDS_SID_REQUEST_DOWNLOAD 0x34U          /**< RequestDownload */
#define SYN_UDS_SID_REQUEST_UPLOAD 0x35U            /**< RequestUpload */
#define SYN_UDS_SID_TRANSFER_DATA 0x36U             /**< TransferData */
#define SYN_UDS_SID_REQUEST_TRANSFER_EXIT 0x37U     /**< RequestTransferExit */
#define SYN_UDS_SID_REQUEST_FILE_TRANSFER 0x38U     /**< RequestFileTransfer */
#define SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS 0x3DU   /**< WriteMemoryByAddress */
#define SYN_UDS_SID_TESTER_PRESENT 0x3EU            /**< TesterPresent */
#define SYN_UDS_SID_ACCESS_TIMING_PARAMETER 0x83U   /**< AccessTimingParameter */
#define SYN_UDS_SID_SECURED_DATA_TRANSMISSION 0x84U /**< SecuredDataTransmission */
#define SYN_UDS_SID_CONTROL_DTC_SETTING 0x85U       /**< ControlDTCSetting */
#define SYN_UDS_SID_RESPONSE_ON_EVENT 0x86U         /**< ResponseOnEvent */
#define SYN_UDS_SID_LINK_CONTROL 0x87U              /**< LinkControl */
/**@}*/

/** @name UDS Service 0x11 ECUReset Sub-functions (ISO 14229-1) */
/**@{*/
#define SYN_UDS_RESET_HARD 0x01U                         /**< Hard reset */
#define SYN_UDS_RESET_KEY_OFF_ON 0x02U                   /**< Key off/on reset */
#define SYN_UDS_RESET_SOFT 0x03U                         /**< Soft reset */
#define SYN_UDS_RESET_ENABLE_RAPID_POWER_SHUTDOWN 0x04U  /**< Enable rapid power shutdown */
#define SYN_UDS_RESET_DISABLE_RAPID_POWER_SHUTDOWN 0x05U /**< Disable rapid power shutdown */
/**@}*/

/** @name UDS Service 0x19 ReadDTCInformation Sub-functions */
/**@{*/
#define SYN_UDS_DTC_REPORT_NUMBER_BY_STATUS_MASK                                          \
    0x01U                                       /**< Report number of DTCs by status mask \
                                                 */
#define SYN_UDS_DTC_REPORT_BY_STATUS_MASK 0x02U /**< Report DTCs by status mask */
#define SYN_UDS_DTC_REPORT_SNAPSHOT_IDENTIFICATION                                              \
    0x03U                                               /**< Report DTC snapshot identification \
                                                         */
#define SYN_UDS_DTC_REPORT_SNAPSHOT_RECORD_BY_DTC 0x04U /**< Report DTC snapshot record by DTC */
#define SYN_UDS_DTC_REPORT_STORED_DATA_BY_RECORD_NUM \
    0x05U                                               /**< Report stored data by record number */
#define SYN_UDS_DTC_REPORT_EXT_DATA_RECORD_BY_DTC 0x06U /**< Report extended data record by DTC */
#define SYN_UDS_DTC_REPORT_NUMBER_BY_SEVERITY_MASK \
    0x07U                                          /**< Report number of DTCs by severity mask */
#define SYN_UDS_DTC_REPORT_BY_SEVERITY_MASK 0x08U  /**< Report DTCs by severity mask */
#define SYN_UDS_DTC_REPORT_SEVERITY_INFO 0x09U     /**< Report DTC severity info */
#define SYN_UDS_DTC_REPORT_SUPPORTED 0x0AU         /**< Report supported DTCs */
#define SYN_UDS_DTC_REPORT_FIRST_TEST_FAILED 0x0BU /**< Report first test failed DTC */
#define SYN_UDS_DTC_REPORT_FIRST_CONFIRMED 0x0CU   /**< Report first confirmed DTC */
#define SYN_UDS_DTC_REPORT_MOST_RECENT_TEST_FAILED                                             \
    0x0DU                                              /**< Report most recent test failed DTC \
                                                        */
#define SYN_UDS_DTC_REPORT_MOST_RECENT_CONFIRMED 0x0EU /**< Report most recent confirmed DTC */
#define SYN_UDS_DTC_REPORT_MIRROR_MEMORY_BY_STATUS_MASK \
    0x0FU                                               /**< Report mirror memory by status mask */
#define SYN_UDS_DTC_REPORT_MIRROR_MEMORY_EXT_DATA 0x10U /**< Report mirror memory extended data */
#define SYN_UDS_DTC_REPORT_NUMBER_MIRROR_MEMORY_BY_STATUS_MASK \
    0x11U /**< Report number of mirror memory DTCs */
#define SYN_UDS_DTC_REPORT_NUMBER_EMISSIONS_OBD_BY_STATUS_MASK \
    0x12U                                                     /**< Report number of OBD DTCs */
#define SYN_UDS_DTC_REPORT_EMISSIONS_OBD_BY_STATUS_MASK 0x13U /**< Report OBD DTCs */
#define SYN_UDS_DTC_REPORT_FAULT_DETECTION_COUNTER 0x14U      /**< Report fault detection counter */
#define SYN_UDS_DTC_REPORT_WITH_PERMANENT_STATUS 0x15U /**< Report DTCs with permanent status */
#define SYN_UDS_DTC_REPORT_EXT_DATA_RECORD_BY_RECORD_NUM \
    0x16U /**< Report extended data by record num */
#define SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_BY_STATUS_MASK \
    0x17U /**< Report user-def memory by status mask */
#define SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_SNAPSHOT_BY_DTC \
    0x18U /**< Report user-def snapshot by DTC */
#define SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_EXT_DATA_BY_DTC \
    0x19U                                               /**< Report user-def ext data by DTC */
#define SYN_UDS_DTC_REPORT_WWH_OBD_BY_MASK_RECORD 0x42U /**< Report WWH-OBD by mask record */
#define SYN_UDS_DTC_REPORT_WWH_OBD_WITH_PERMANENT_STATUS \
    0x55U /**< Report WWH-OBD with permanent status */
/**@}*/

/** @name UDS GroupOfDTC Definitions (ISO 14229-1 / SAE J2012) */
/**@{*/
#define SYN_UDS_DTC_GROUP_EMISSIONS 0x000000U  /**< Emissions-related systems DTC group */
#define SYN_UDS_DTC_GROUP_POWERTRAIN 0x100000U /**< Powertrain DTC group */
#define SYN_UDS_DTC_GROUP_CHASSIS 0x400000U    /**< Chassis DTC group */
#define SYN_UDS_DTC_GROUP_BODY 0x800000U       /**< Body DTC group */
#define SYN_UDS_DTC_GROUP_NETWORK 0xC00000U    /**< Network DTC group */
#define SYN_UDS_DTC_GROUP_ALL 0xFFFFFFU        /**< All DTC groups */
/**@}*/

/** @name UDS Response Identifiers */
/**@{*/
#define SYN_UDS_RESPONSE_NEGATIVE 0x7FU /**< Negative response code header */
/**@}*/

/** @name UDS Negative Response Codes (NRC) */
/**@{*/
#define SYN_UDS_NRC_SUCCESS 0x00U                   /**< Positive response (0x00) */
#define SYN_UDS_NRC_SERVICE_NOT_SUPPORTED 0x11U     /**< Service not supported (0x11) */
#define SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED 0x12U /**< Sub-function not supported (0x12) */
#define SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH \
    0x13U /**< Incorrect message length or invalid format (0x13) */
#define SYN_UDS_NRC_RESPONSE_TOO_LONG 0x14U           /**< Response too long (0x14) */
#define SYN_UDS_NRC_CONDITIONS_NOT_CORRECT 0x22U      /**< Conditions not correct (0x22) */
#define SYN_UDS_NRC_REQUEST_OUT_OF_RANGE 0x31U        /**< Request out of range (0x31) */
#define SYN_UDS_NRC_SECURITY_ACCESS_DENIED 0x33U      /**< Security access denied (0x33) */
#define SYN_UDS_NRC_INVALID_KEY 0x35U                 /**< Invalid key (0x35) */
#define SYN_UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS 0x36U /**< Exceeded number of attempts (0x36) */
#define SYN_UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED \
    0x37U                                             /**< Required time delay not expired (0x37) */
#define SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR 0x24U      /**< Request sequence error (0x24) */
#define SYN_UDS_NRC_TRANSFER_DATA_SUSPENDED 0x71U     /**< Transfer data suspended (0x71) */
#define SYN_UDS_NRC_GENERAL_PROGRAMMING_FAILURE 0x72U /**< General programming failure (0x72) */
#define SYN_UDS_NRC_WRONG_BLOCK_SEQUENCE_COUNTER 0x73U /**< Wrong block sequence counter (0x73) */
#define SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION \
    0x7EU /**< Sub-function not supported in active session (0x7E) */
/**@}*/

/** @brief UDS Addressing Modes (ISO 14229-1) */
typedef enum {
    SYN_UDS_ADDR_PHYSICAL = 0U,  /**< Physical (1:1 point-to-point) addressing mode */
    SYN_UDS_ADDR_FUNCTIONAL = 1U /**< Functional (1:N broadcast) addressing mode */
} SYN_UDS_AddrMode;

/** @brief UDS Data Transfer States */
typedef enum {
    SYN_UDS_TRANSFER_IDLE = 0U,     /**< Idle state */
    SYN_UDS_TRANSFER_DOWNLOAD = 1U, /**< Active download transfer */
    SYN_UDS_TRANSFER_UPLOAD = 2U    /**< Active upload transfer */
} SYN_UDS_TransferState;

/** @brief UDS Diagnostic Session Types (ISO 14229-1) */
typedef enum {
    SYN_UDS_SESSION_DEFAULT = 0x01U,      /**< Default session (0x01) */
    SYN_UDS_SESSION_PROGRAMMING = 0x02U,  /**< Programming session (0x02) */
    SYN_UDS_SESSION_EXTENDED = 0x03U,     /**< Extended diagnostic session (0x03) */
    SYN_UDS_SESSION_SAFETY_SYSTEM = 0x04U /**< Safety system diagnostic session (0x04) */
} SYN_UDS_Session;

/** @name ISO 14229-1 Session Permission Bitmask Definitions */
/**@{*/
#define SYN_UDS_SESSION_MASK_DEFAULT (1U << 0)     /**< Default Session */
#define SYN_UDS_SESSION_MASK_PROGRAMMING (1U << 1) /**< Programming Session */
#define SYN_UDS_SESSION_MASK_EXTENDED (1U << 2)    /**< Extended Diagnostic Session */
#define SYN_UDS_SESSION_MASK_SAFETY (1U << 3)      /**< Safety System Session */
#define SYN_UDS_SESSION_MASK_ALL (0x0FU)           /**< Allowed in all sessions */
/**@}*/

/** @name ISO 14229-1 Security Level Bitmask Definitions */
/**@{*/
#define SYN_UDS_SECURITY_MASK_NONE (1U << 0)    /**< Unlocked without security (Level 0 / Locked) */
#define SYN_UDS_SECURITY_MASK_LEVEL_1 (1U << 1) /**< Security Level 1 required */
#define SYN_UDS_SECURITY_MASK_LEVEL_2 (1U << 2) /**< Security Level 2 required */
#define SYN_UDS_SECURITY_MASK_LEVEL_3 (1U << 3) /**< Security Level 3 required */
#define SYN_UDS_SECURITY_MASK_ALL (0xFFFFU)     /**< Allowed in all security states */
/**@}*/

/**
 * @brief Data Identifier (DID) Registry Entry.
 */
typedef struct {
    uint16_t did;           /**< 16-bit DID identifier code */
    uint8_t *data;          /**< Pointer to DID data buffer */
    uint16_t len;           /**< Byte length of DID data */
    bool writable;          /**< True if DID is writable */
    uint8_t session_mask;   /**< Permitted session bitmask */
    uint16_t security_mask; /**< Permitted security level bitmask */
} SYN_UDS_DIDEntry;

/** @brief UDS Security Access Unlock States */
typedef enum {
    SYN_UDS_SECURITY_LOCKED = 0x00U,    /**< Security locked */
    SYN_UDS_SECURITY_SEED_SENT = 0x01U, /**< Seed sent, awaiting key */
    SYN_UDS_SECURITY_UNLOCKED = 0x02U   /**< Security unlocked */
} SYN_UDS_SecurityState;

/** @brief UDS CommunicationControl (0x28) Subfunctions */
typedef enum {
    SYN_UDS_COMM_ENABLE_RX_AND_TX = 0x00U,      /**< Enable Rx and Tx */
    SYN_UDS_COMM_ENABLE_RX_DISABLE_TX = 0x01U,  /**< Enable Rx, disable Tx */
    SYN_UDS_COMM_DISABLE_RX_ENABLE_TX = 0x02U,  /**< Disable Rx, enable Tx */
    SYN_UDS_COMM_DISABLE_RX_AND_TX = 0x03U,     /**< Disable Rx and Tx */
    SYN_UDS_COMM_ENABLE_RX_TX_ENHANCED = 0x04U, /**< Enhanced Rx and Tx */
    SYN_UDS_COMM_ENABLE_RX_ENHANCED = 0x05U     /**< Enhanced Rx */
} SYN_UDS_CommControlType;

/** @brief UDS AccessTimingParameter (0x83) Subfunctions */
typedef enum {
    SYN_UDS_TIMING_READ_EXTENDED = 0x01U,  /**< Read extended timing limits */
    SYN_UDS_TIMING_SET_TO_DEFAULT = 0x02U, /**< Reset timing parameters to default */
    SYN_UDS_TIMING_READ_ACTIVE = 0x03U,    /**< Read currently active timing parameters */
    SYN_UDS_TIMING_SET_TO_GIVEN = 0x04U    /**< Set timing parameters to requested values */
} SYN_UDS_AccessTimingType;

/**
 * @brief CommunicationControl (0x28) callback function signature.
 */
typedef bool (*SYN_UDS_CommControlHandler)(SYN_UDS_CommControlType control_type, uint8_t comm_type,
                                           void *ctx);

/**
 * @brief AccessTimingParameter (0x83) callback function signature.
 */
typedef bool (*SYN_UDS_AccessTimingHandler)(SYN_UDS_AccessTimingType timing_type,
                                            uint16_t *p2_max_ms, uint16_t *p2_star_max_10ms,
                                            void *ctx);

/**
 * @brief SecuredDataTransmission (0x84) callback function signature.
 */
typedef bool (*SYN_UDS_SecuredDataHandler)(const uint8_t *in_data, uint16_t in_len,
                                           uint8_t *out_buf, uint16_t max_out_len,
                                           uint16_t *out_len, void *ctx);

/**
 * @brief ReadMemoryByAddress (0x23) and WriteMemoryByAddress (0x3D) callback function signature.
 */
typedef bool (*SYN_UDS_MemoryHandler)(bool is_write, uint32_t address, uint32_t size,
                                      uint8_t *data_buf, void *ctx);

/**
 * @brief Authentication (0x29) callback function signature.
 */
typedef bool (*SYN_UDS_AuthHandler)(uint8_t subfunction, const uint8_t *in_data, uint16_t in_len,
                                    uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len,
                                    void *ctx);

/**
 * @brief RequestFileTransfer (0x38) callback function signature.
 */
typedef bool (*SYN_UDS_FileTransferHandler)(uint8_t mode, const char *file_path, uint16_t path_len,
                                            uint8_t *out_buf, uint16_t max_out_len,
                                            uint16_t *out_len, void *ctx);

/**
 * @brief RoutineControl (0x31) callback function signature.
 */
typedef bool (*SYN_UDS_RoutineControlHandler)(uint8_t subfunction, uint16_t routine_id,
                                              const uint8_t *in_data, uint16_t in_len,
                                              uint8_t *out_buf, uint16_t max_out_len,
                                              uint16_t *out_len, void *ctx);

/**
 * @brief InputOutputControlByIdentifier (0x2F) callback function signature.
 */
typedef bool (*SYN_UDS_IOControlHandler)(uint16_t did, uint8_t control_opt, const uint8_t *in_data,
                                         uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len,
                                         uint16_t *out_len, void *ctx);

/**
 * @brief LinkControl (0x87) callback function signature.
 */
typedef bool (*SYN_UDS_LinkControlHandler)(uint8_t subfunction, const uint8_t *in_data,
                                           uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len,
                                           uint16_t *out_len, void *ctx);

/**
 * @brief ResponseOnEvent (0x86) callback function signature.
 */
typedef bool (*SYN_UDS_ResponseOnEventHandler)(uint8_t subfunction, const uint8_t *in_data,
                                               uint16_t in_len, uint8_t *out_buf,
                                               uint16_t max_out_len, uint16_t *out_len, void *ctx);

/**
 * @brief ReadScalingDataByIdentifier (0x24) callback function signature.
 */
typedef bool (*SYN_UDS_ScalingDataHandler)(uint16_t did, uint8_t *out_buf, uint16_t max_out_len,
                                           uint16_t *out_len, void *ctx);

/**
 * @brief ReadDataByPeriodicIdentifier (0x2A) callback function signature.
 */
typedef bool (*SYN_UDS_PeriodicDataHandler)(uint8_t mode, const uint8_t *in_data, uint16_t in_len,
                                            uint8_t *out_buf, uint16_t max_out_len,
                                            uint16_t *out_len, void *ctx);

/**
 * @brief DynamicallyDefineDataIdentifier (0x2C) callback function signature.
 */
typedef bool (*SYN_UDS_DynamicDIDHandler)(uint8_t subfunction, uint16_t dyn_did,
                                          const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf,
                                          uint16_t max_out_len, uint16_t *out_len, void *ctx);

/** @name UDS Timing & Security Constants */
/**@{*/
#ifndef SYN_UDS_MAX_DTCS
#define SYN_UDS_MAX_DTCS 32U /**< Maximum stored DTC count */
#endif

#define SYN_UDS_DTC_STATUS_AVAILABILITY_MASK 0xFFU /**< DTC status availability bitmask */
/**@}*/

/** @name ISO 14229-1 DTCStatusByte Bit Definitions */
/**@{*/
#define SYN_UDS_DTC_STATUS_TEST_FAILED (1U << 0) /**< Test failed bit */
#define SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE \
    (1U << 1)                                      /**< Test failed this operation cycle */
#define SYN_UDS_DTC_STATUS_PENDING_DTC (1U << 2)   /**< Pending DTC bit */
#define SYN_UDS_DTC_STATUS_CONFIRMED_DTC (1U << 3) /**< Confirmed DTC bit */
#define SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR \
    (1U << 4) /**< Test not completed since clear */
#define SYN_UDS_DTC_STATUS_TEST_FAILED_SINCE_LAST_CLEAR (1U << 5) /**< Test failed since clear */
#define SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE \
    (1U << 6) /**< Test not completed this op cycle */
#define SYN_UDS_DTC_STATUS_WARNING_INDICATOR_REQUESTED \
    (1U << 7) /**< Warning indicator requested         \
               */
/**@}*/

/** @name ISO 14229-1 DTCSeverityByte Definitions (Bits 7..5) */
/**@{*/
#define SYN_UDS_DTC_SEVERITY_NO_SEVERITY (0x00U << 5)          /**< No severity */
#define SYN_UDS_DTC_SEVERITY_MAINTENANCE_REQUIRED (0x01U << 5) /**< Maintenance required */
#define SYN_UDS_DTC_SEVERITY_CHECK_AT_NEXT_HALT (0x02U << 5)   /**< Check at next halt */
#define SYN_UDS_DTC_SEVERITY_CHECK_IMMEDIATELY (0x03U << 5)    /**< Check immediately */
#define SYN_UDS_DTC_SEVERITY_MASK (0x07U << 5)                 /**< DTC severity mask */
#define SYN_UDS_DTC_CLASS_MASK (0x1FU)                         /**< DTC class mask */
/**@}*/

/** @name ISO 14229-1 DTCFormatIdentifier Definitions */
/**@{*/
#define SYN_UDS_DTC_FORMAT_ISO14229_1 0x00U   /**< ISO 14229-1 DTC format */
#define SYN_UDS_DTC_FORMAT_ISO15031_6 0x01U   /**< ISO 15031-6 DTC format */
#define SYN_UDS_DTC_FORMAT_SAE_J1939_73 0x02U /**< SAE J1939-73 DTC format */
#define SYN_UDS_DTC_FORMAT_ISO27145_4 0x03U   /**< ISO 27145-4 DTC format */
/**@}*/

/**
 * @brief Diagnostic Trouble Code (DTC) Registry Entry.
 */
typedef struct {
    uint32_t dtc;     /**< 24-bit Diagnostic Trouble Code (e.g., 0x012345). */
    uint8_t status;   /**< DTCStatusByte bitmask.                           */
    uint8_t severity; /**< DTCSeverityByte (High 3 bits severity, low 5 bits class). */
    int8_t fault_cnt; /**< Fault detection counter (-128 to 127).          */
} SYN_UDS_DTCEntry;

/**
 * @brief ReadDTCInformation (0x19) callback function signature.
 */
typedef bool (*SYN_UDS_DTCHandler)(uint8_t subfunction, const uint8_t *in_data, uint16_t in_len,
                                   uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len,
                                   void *ctx);

/**
 * @brief ECUReset (0x11) deferred post-TX reset callback function signature.
 */
typedef void (*SYN_UDS_ResetHandler)(uint8_t reset_type, void *ctx);

/**
 * @brief DiagnosticSessionControl (0x10) session transition rule callback signature.
 */
typedef bool (*SYN_UDS_SessionTransitionHandler)(SYN_UDS_Session from_session,
                                                 SYN_UDS_Session to_session, void *ctx);

/** @name UDS Security & S3 Session Timeout Parameters */
/**@{*/
#ifndef SYN_UDS_S3_TIMEOUT_MS
#define SYN_UDS_S3_TIMEOUT_MS 5000U /**< S3 session timeout in ms */
#endif

#ifndef SYN_UDS_SECURITY_MAX_ATTEMPTS
#define SYN_UDS_SECURITY_MAX_ATTEMPTS 3U /**< Max security unlock attempts */
#endif

#ifndef SYN_UDS_SECURITY_DELAY_MS
#define SYN_UDS_SECURITY_DELAY_MS 10000U /**< Security delay penalty in ms */
#endif

#ifndef SYN_UDS_DEFAULT_RESET_TX_WAIT_MS
#define SYN_UDS_DEFAULT_RESET_TX_WAIT_MS 50U /**< Default post-TX ECU reset wait time in ms */
#endif
/**@}*/

/**
 * @brief UDS Server Instance Context.
 */
typedef struct {
    SYN_UDS_Session session;              /**< Current diagnostic session state */
    SYN_UDS_SecurityState security_state; /**< Security access unlock state */
    uint8_t
        security_level;    /**< Unlocked security level (0 = locked, 1 = level 1, 2 = level 2...) */
    uint32_t current_seed; /**< Master template security seed value */
    uint32_t active_seed;  /**< Issued transaction security seed value */
    uint8_t active_seed_subfunction;  /**< Active seed request subfunction (e.g. 0x01, 0x03) */
    bool use_aes128_security;         /**< True if AES-128 security mode is active */
    uint8_t aes_security_key[16];     /**< AES-128 security secret key (16 bytes) */
    uint8_t current_seed_bytes[16];   /**< Master template AES-128 security seed (16 bytes) */
    uint8_t active_seed_bytes[16];    /**< Issued transaction AES-128 security seed (16 bytes) */
    uint32_t s3_timer_ms;             /**< S3 session timer in ms */
    uint8_t security_error_count;     /**< Failed security unlock attempts counter */
    uint32_t security_delay_timer_ms; /**< Security delay penalty timer in ms */
    SYN_UDS_CommControlType comm_control_state;   /**< CommunicationControl state */
    uint8_t comm_type;                            /**< Communication type byte */
    bool dtc_setting_enabled;                     /**< True when DTC status updates are enabled */
    SYN_UDS_CommControlHandler comm_control_cb;   /**< CommunicationControl callback */
    void *comm_control_ctx;                       /**< CommunicationControl context pointer */
    uint16_t p2_max_ms;                           /**< Default P2Server_max timing (ms). */
    uint16_t p2_star_max_10ms;                    /**< Default P2*Server_max timing (10ms units). */
    uint16_t active_p2_max_ms;                    /**< Active P2Server_max timing (ms). */
    uint16_t active_p2_star_max_10ms;             /**< Active P2*Server_max timing (10ms units). */
    SYN_UDS_AccessTimingHandler timing_cb;        /**< AccessTimingParameter callback. */
    void *timing_ctx;                             /**< Context pointer for timing callback. */
    SYN_UDS_SecuredDataHandler secured_data_cb;   /**< SecuredDataTransmission callback. */
    void *secured_data_ctx;                       /**< Context pointer for secured data callback. */
    SYN_UDS_MemoryHandler memory_cb;              /**< Memory (0x23/0x3D) callback. */
    void *memory_ctx;                             /**< Context pointer for memory callback. */
    SYN_UDS_AuthHandler auth_cb;                  /**< Authentication (0x29) callback. */
    void *auth_ctx;                               /**< Context pointer for auth callback. */
    SYN_UDS_FileTransferHandler file_transfer_cb; /**< File transfer (0x38) callback. */
    void *file_transfer_ctx;                    /**< Context pointer for file transfer callback. */
    SYN_UDS_RoutineControlHandler routine_cb;   /**< RoutineControl (0x31) callback. */
    void *routine_ctx;                          /**< Context pointer for routine callback. */
    SYN_UDS_IOControlHandler io_control_cb;     /**< IOControl (0x2F) callback. */
    void *io_control_ctx;                       /**< Context pointer for IO control callback. */
    SYN_UDS_LinkControlHandler link_control_cb; /**< LinkControl (0x87) callback. */
    void *link_control_ctx;                     /**< Context pointer for link control callback. */
    SYN_UDS_ResponseOnEventHandler roe_cb;      /**< ResponseOnEvent (0x86) callback. */
    void *roe_ctx;                              /**< Context pointer for ROE callback. */
    SYN_UDS_ScalingDataHandler scaling_cb;      /**< ScalingData (0x24) callback. */
    void *scaling_ctx;                          /**< Context pointer for scaling callback. */
    SYN_UDS_PeriodicDataHandler periodic_cb;    /**< PeriodicData (0x2A) callback. */
    void *periodic_ctx;                         /**< Context pointer for periodic callback. */
    SYN_UDS_DynamicDIDHandler dynamic_did_cb;   /**< DynamicDID (0x2C) callback. */
    void *dynamic_did_ctx;                      /**< Context pointer for dynamic DID callback. */
    SYN_UDS_DTCHandler dtc_cb;                  /**< ReadDTCInformation (0x19) callback. */
    void *dtc_ctx;                              /**< Context pointer for DTC callback. */
    SYN_UDS_ResetHandler reset_cb;              /**< ECUReset (0x11) deferred callback. */
    void *reset_ctx;                            /**< Context pointer for reset callback. */
    uint16_t reset_tx_wait_ms;                  /**< Post-TX ECU reset delay window in ms. */
    uint32_t reset_wait_elapsed_ms;             /**< Internal reset delay accumulator in ms. */
    SYN_UDS_SessionTransitionHandler
        session_transition_cb;                    /**< Session transition policy callback. */
    void *session_transition_ctx;                 /**< Session transition context pointer. */
    SYN_UDS_DIDEntry did_table[SYN_UDS_MAX_DIDS]; /**< Registered DID entries array */
    uint8_t did_count;                            /**< Registered DID count */
    SYN_UDS_DTCEntry dtc_table[SYN_UDS_MAX_DTCS]; /**< Registered DTC entries array */
    uint8_t dtc_count;                            /**< Registered DTC count */
    SYN_UDS_TransferState transfer_state;         /**< Active data transfer state */
    uint32_t transfer_address;                    /**< Target transfer memory address */
    uint32_t transfer_size;                       /**< Total transfer size in bytes */
    uint32_t transfer_bytes_processed;            /**< Transferred byte count */
    uint8_t expected_block_seq;                   /**< Expected block sequence counter */
    uint8_t reset_type_requested;                 /**< Pending ECU reset type requested */
    uint8_t custom_session_sids[SYN_UDS_MAX_SERVICE_OVERRIDES];  /**< Custom session mask SID
                                                                    overrides */
    uint8_t custom_session_masks[SYN_UDS_MAX_SERVICE_OVERRIDES]; /**< Custom session mask values */
    uint8_t custom_session_count; /**< Custom session override count */
    uint8_t custom_security_sids[SYN_UDS_MAX_SERVICE_OVERRIDES]; /**< Custom security mask SID
                                                                    overrides */
    uint16_t
        custom_security_masks[SYN_UDS_MAX_SERVICE_OVERRIDES]; /**< Custom security mask values */
    uint8_t custom_security_count;                            /**< Custom security override count */
} SYN_UDS_Server;

/**
 * @brief Enable AES-128 algorithm for UDS SecurityAccess (0x27) seed/key unlock.
 *
 * @param server Pointer to UDS server instance.
 * @param key 16-byte AES-128 secret key.
 * @return true on success, false if server or key is NULL.
 */
bool syn_uds_enable_aes128_security(SYN_UDS_Server *server, const uint8_t key[16]);

/**
 * @brief Disable AES-128 security mode and revert to standard XOR security key calculation.
 *
 * @param server Pointer to UDS server instance.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_disable_aes128_security(SYN_UDS_Server *server);

/**
 * @brief Set custom 16-byte seed for AES-128 SecurityAccess.
 *
 * @param server Pointer to UDS server instance.
 * @param seed 16-byte seed buffer.
 * @return true on success, false if server or seed is NULL.
 */
bool syn_uds_set_security_seed_bytes(SYN_UDS_Server *server, const uint8_t seed[16]);

/**
 * @brief Get currently unlocked security level (0 = locked, 1 = level 1, 2 = level 2...).
 *
 * @param server Pointer to UDS server instance.
 * @return Active security level uint8_t.
 */
uint8_t syn_uds_get_security_level(const SYN_UDS_Server *server);

/**
 * @brief Initialize UDS Server context.
 *
 * @param server Pointer to UDS server instance.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_init(SYN_UDS_Server *server);

/**
 * @brief Advance periodic S3 server timer by dt_ms milliseconds.
 * Reverts session to DEFAULT and locks security state if S3 timer expires (5000ms).
 *
 * @param server Pointer to UDS server instance.
 * @param dt_ms Milliseconds elapsed since last tick.
 */
void syn_uds_tick(SYN_UDS_Server *server, uint32_t dt_ms);

/**
 * @brief Register optional session transition policy callback.
 *
 * @param server Pointer to UDS server instance.
 * @param cb Callback function to validate session transition graph permissions.
 * @param ctx Context pointer passed to callback function.
 */
void syn_uds_set_session_transition_handler(SYN_UDS_Server *server,
                                            SYN_UDS_SessionTransitionHandler cb, void *ctx);

/**
 * @brief Register deferred post-TX ECU reset handler callback.
 *
 * @param server Pointer to UDS server instance.
 * @param cb Callback function to execute after post-TX reset delay.
 * @param ctx Context pointer passed to callback function.
 */
void syn_uds_set_reset_handler(SYN_UDS_Server *server, SYN_UDS_ResetHandler cb, void *ctx);

/**
 * @brief Set post-TX ECU reset delay window duration in milliseconds.
 *
 * @param server Pointer to UDS server instance.
 * @param wait_ms Delay duration in ms before executing reset callback.
 */
void syn_uds_set_reset_wait_ms(SYN_UDS_Server *server, uint16_t wait_ms);

/**
 * @brief Get pending ECU reset sub-function requested by 0x11 service.
 *
 * @param server Pointer to UDS server instance.
 * @return Pending reset type (e.g. 0x01 = hard, 0x02 = keyOffOn, 0x03 = soft), or 0 if none.
 */
uint8_t syn_uds_get_pending_reset(const SYN_UDS_Server *server);

/**
 * @brief Clear pending ECU reset flag after executing reset or transmitting response.
 *
 * @param server Pointer to UDS server instance.
 */
void syn_uds_clear_pending_reset(SYN_UDS_Server *server);

/**
 * @brief Register Data Identifier (DID) mapping in UDS Server table.
 *
 * @param server Pointer to UDS server instance.
 * @param did 16-bit Data Identifier code (e.g., 0xF190 for VIN).
 * @param data Pointer to data memory buffer.
 * @param len Data byte length.
 * @param writable True if DID allows WriteDataByIdentifier (0x2E).
 * @return true on success, false if table full or invalid params.
 */
bool syn_uds_register_did(SYN_UDS_Server *server, uint16_t did, uint8_t *data, uint16_t len,
                          bool writable);

/**
 * @brief Register Data Identifier (DID) with custom session & security permission bitmask.
 *
 * @param server Pointer to UDS server instance.
 * @param did 16-bit Data Identifier code (e.g., 0xF190 for VIN).
 * @param data Pointer to data memory buffer.
 * @param len Data byte length.
 * @param writable True if DID allows WriteDataByIdentifier (0x2E).
 * @param session_mask Permitted session bitmask (SYN_UDS_SESSION_MASK_*).
 * @param security_mask Permitted security level bitmask (SYN_UDS_SECURITY_MASK_*).
 * @return true on success, false if table full or invalid params.
 */
bool syn_uds_register_did_ext(SYN_UDS_Server *server, uint16_t did, uint8_t *data, uint16_t len,
                              bool writable, uint8_t session_mask, uint16_t security_mask);

/**
 * @brief Register Diagnostic Trouble Code (DTC) in UDS Server table.
 *
 * @param server Pointer to UDS server instance.
 * @param dtc 24-bit Diagnostic Trouble Code (e.g., 0x012345).
 * @param status Initial DTCStatusByte bitmask.
 * @param severity DTCSeverityByte (severity & class).
 * @return true on success, false if table full or invalid params.
 */
bool syn_uds_register_dtc(SYN_UDS_Server *server, uint32_t dtc, uint8_t status, uint8_t severity);

/**
 * @brief Register ReadDTCInformation (0x19) callback handler.
 *
 * @param server Pointer to UDS server instance.
 * @param handler Callback function invoked for custom DTC subfunctions.
 * @param ctx User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_dtc_handler(SYN_UDS_Server *server, SYN_UDS_DTCHandler handler, void *ctx);

/**
 * @brief Register CommunicationControl (0x28) callback handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x28 is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_comm_control(SYN_UDS_Server *server, SYN_UDS_CommControlHandler handler,
                                   void *ctx);

/**
 * @brief Register AccessTimingParameter (0x83) callback handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x83 is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_access_timing(SYN_UDS_Server *server, SYN_UDS_AccessTimingHandler handler,
                                    void *ctx);

/**
 * @brief Register SecuredDataTransmission (0x84) callback handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x84 is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_secured_data(SYN_UDS_Server *server, SYN_UDS_SecuredDataHandler handler,
                                   void *ctx);

/**
 * @brief Register ReadMemoryByAddress (0x23) and WriteMemoryByAddress (0x3D) handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when memory services are processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_memory_handler(SYN_UDS_Server *server, SYN_UDS_MemoryHandler handler,
                                     void *ctx);

/**
 * @brief Register Authentication (0x29) handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x29 is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_auth_handler(SYN_UDS_Server *server, SYN_UDS_AuthHandler handler, void *ctx);

/**
 * @brief Register RequestFileTransfer (0x38) handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x38 is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_file_transfer(SYN_UDS_Server *server, SYN_UDS_FileTransferHandler handler,
                                    void *ctx);

/**
 * @brief Register RoutineControl (0x31) callback handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x31 is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_routine_control(SYN_UDS_Server *server, SYN_UDS_RoutineControlHandler handler,
                                      void *ctx);

/**
 * @brief Register InputOutputControlByIdentifier (0x2F) callback handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x2F is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_io_control(SYN_UDS_Server *server, SYN_UDS_IOControlHandler handler,
                                 void *ctx);

/**
 * @brief Register LinkControl (0x87) callback handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x87 is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_link_control(SYN_UDS_Server *server, SYN_UDS_LinkControlHandler handler,
                                   void *ctx);

/**
 * @brief Register ResponseOnEvent (0x86) callback handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x86 is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_roe_handler(SYN_UDS_Server *server, SYN_UDS_ResponseOnEventHandler handler,
                                  void *ctx);

/**
 * @brief Register ReadScalingDataByIdentifier (0x24) callback handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x24 is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_scaling_data_handler(SYN_UDS_Server *server,
                                           SYN_UDS_ScalingDataHandler handler, void *ctx);

/**
 * @brief Register ReadDataByPeriodicIdentifier (0x2A) callback handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x2A is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_periodic_data_handler(SYN_UDS_Server *server,
                                            SYN_UDS_PeriodicDataHandler handler, void *ctx);

/**
 * @brief Register DynamicallyDefineDataIdentifier (0x2C) callback handler.
 *
 * @param server  Pointer to UDS server instance.
 * @param handler Callback function invoked when Service 0x2C is processed.
 * @param ctx     User context passed to handler.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_register_dynamic_did_handler(SYN_UDS_Server *server, SYN_UDS_DynamicDIDHandler handler,
                                          void *ctx);

/**
 * @brief Configure allowed diagnostic session mask for a specific Service Identifier.
 *
 * @param server Pointer to UDS server instance.
 * @param sid Service Identifier (e.g. 0x27, 0x34).
 * @param session_mask Allowed session bitmask (SYN_UDS_SESSION_MASK_*).
 * @return true on success, false if server is NULL.
 */
bool syn_uds_set_service_session_mask(SYN_UDS_Server *server, uint8_t sid, uint8_t session_mask);

/**
 * @brief Configure required security level mask for a specific Service Identifier.
 *
 * @param server Pointer to UDS server instance.
 * @param sid Service Identifier (e.g. 0x27, 0x34).
 * @param security_mask Required security level bitmask (SYN_UDS_SECURITY_MASK_*).
 * @return true on success, false if server is NULL.
 */
bool syn_uds_set_service_security_mask(SYN_UDS_Server *server, uint8_t sid, uint16_t security_mask);

/**
 * @brief Process incoming UDS request diagnostic payload and format response.
 *
 * @param server Pointer to UDS server instance.
 * @param req Pointer to input request bytes.
 * @param req_len Length of request payload in bytes.
 * @param resp_buf Output buffer for response bytes.
 * @param max_resp_len Capacity of output response buffer.
 * @param resp_len Pointer to store generated response byte length.
 * @param addr_mode Addressing mode (SYN_UDS_ADDR_PHYSICAL or SYN_UDS_ADDR_FUNCTIONAL).
 * @return true if response frame generated, false on invalid parameters.
 */
bool syn_uds_process_request(SYN_UDS_Server *server, const uint8_t *req, uint16_t req_len,
                             uint8_t *resp_buf, uint16_t max_resp_len, uint16_t *resp_len,
                             SYN_UDS_AddrMode addr_mode);

/**
 * @brief Check if Service Identifier (SID) supports Functional Addressing (1:N Broadcast).
 * @param sid Service Identifier (SID).
 * @return true if service supports functional addressing, false if physical addressing only.
 */
bool syn_uds_is_sid_functional_supported(uint8_t sid);

/**
 * @brief Report diagnostic test result for a registered DTC according to ISO 14229-1 state
 * transitions.
 *
 * Diagnostic monitors must invoke this function whenever a periodic or event-driven test completes:
 * - When @p failed is true (qualified failure):
 *   - Sets @c testFailed (bit 0), @c testFailedThisOperationCycle (bit 1),
 *     @c pendingDTC (bit 2), @c confirmedDTC (bit 3), and @c testFailedSinceLastClear (bit 5).
 *   - Clears @c testNotCompletedSinceLastClear (bit 4) and @c testNotCompletedThisOperationCycle
 * (bit 6).
 *   - Sets Fault Detection Counter (FDC) to +127.
 * - When @p failed is false (qualified pass):
 *   - Clears @c testFailed (bit 0), @c testNotCompletedSinceLastClear (bit 4), and
 *     @c testNotCompletedThisOperationCycle (bit 6).
 *   - Sets Fault Detection Counter (FDC) to -128.
 *
 * @param server Pointer to UDS server instance.
 * @param dtc 24-bit Diagnostic Trouble Code.
 * @param failed True if test qualified failed, false if test qualified passed.
 * @return true if DTC found and updated, false otherwise.
 */
bool syn_uds_dtc_report_test_result(SYN_UDS_Server *server, uint32_t dtc, bool failed);

/**
 * @brief Advance server state to a new operation cycle.
 *
 * Must be invoked at ECU ignition ON / startup and upon completion/boundary of each driving
 * or operation cycle:
 * - If a registered DTC completed testing during the previous cycle without any failure
 *   (@c testNotCompletedThisOperationCycle == 0 and @c testFailedThisOperationCycle == 0),
 *   clears @c pendingDTC (bit 2) per ISO 14229-1 Table 242.
 * - Clears @c testFailedThisOperationCycle (bit 1) to 0.
 * - Sets @c testNotCompletedThisOperationCycle (bit 6) to 1.
 *
 * @param server Pointer to UDS server instance.
 * @return true on success, false if server is NULL.
 */
bool syn_uds_dtc_start_operation_cycle(SYN_UDS_Server *server);

/**
 * @brief Get current 8-bit status byte for a registered DTC.
 *
 * Returns the authoritative ISO 14229-1 DTCStatusByte tracked by the UDS server.
 *
 * @param server Pointer to UDS server instance.
 * @param dtc 24-bit Diagnostic Trouble Code.
 * @param out_status Output pointer to store DTC status byte.
 * @return true if DTC found, false otherwise.
 */
bool syn_uds_dtc_get_status(SYN_UDS_Server *server, uint32_t dtc, uint8_t *out_status);

#ifdef __cplusplus
}
#endif

#endif /* SYN_UDS_H */
