/**
 * @file syn_kwp2000.h
 * @brief ISO 14230-3 Key Word Protocol 2000 (KWP2000) Server Implementation.
 * @ingroup syn_proto
 *
 * Provides a zero-allocation, lightweight KWP2000 server protocol stack over
 * ISO 15765-2 ISO-TP for automotive diagnostic session management, Local/Common
 * Identifier (LID/CID) read/write, security access seed/key unlocks, routine
 * control, memory transfer, and ECU reset.
 */

#ifndef SYN_KWP2000_H
#define SYN_KWP2000_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name Configuration Constants */
/**@{*/
#ifndef SYN_KWP2000_MAX_LIDS
#define SYN_KWP2000_MAX_LIDS 16U /**< Maximum registered Local Identifiers (LID) */
#endif
#ifndef SYN_KWP2000_MAX_CIDS
#define SYN_KWP2000_MAX_CIDS 16U /**< Maximum registered Common Identifiers (CID) */
#endif
/**@}*/

/** @name KWP2000 Service Identifiers (ISO 14230-3 SID) */
/**@{*/
#define SYN_KWP2000_SID_START_DIAGNOSTIC_SESSION 0x10U     /**< StartDiagnosticSession */
#define SYN_KWP2000_SID_ECU_RESET 0x11U                    /**< ECUReset */
#define SYN_KWP2000_SID_CLEAR_DIAGNOSTIC_INFORMATION 0x14U /**< ClearDiagnosticInformation */
#define SYN_KWP2000_SID_READ_DTC_BY_STATUS 0x18U      /**< ReadDiagnosticTroubleCodesByStatus */
#define SYN_KWP2000_SID_READ_ECU_IDENTIFICATION 0x1AU /**< ReadECUIdentification */
#define SYN_KWP2000_SID_READ_DATA_BY_LOCAL_IDENTIFIER 0x21U   /**< ReadDataByLocalIdentifier */
#define SYN_KWP2000_SID_READ_DATA_BY_COMMON_IDENTIFIER 0x22U  /**< ReadDataByCommonIdentifier */
#define SYN_KWP2000_SID_SECURITY_ACCESS 0x27U                 /**< SecurityAccess */
#define SYN_KWP2000_SID_WRITE_DATA_BY_COMMON_IDENTIFIER 0x2EU /**< WriteDataByCommonIdentifier */
#define SYN_KWP2000_SID_START_ROUTINE_BY_LOCAL_IDENTIFIER \
    0x31U                                                    /**< StartRoutineByLocalIdentifier */
#define SYN_KWP2000_SID_REQUEST_DOWNLOAD 0x34U               /**< RequestDownload */
#define SYN_KWP2000_SID_TRANSFER_DATA 0x36U                  /**< TransferData */
#define SYN_KWP2000_SID_REQUEST_TRANSFER_EXIT 0x37U          /**< RequestTransferExit */
#define SYN_KWP2000_SID_WRITE_DATA_BY_LOCAL_IDENTIFIER 0x3BU /**< WriteDataByLocalIdentifier */
#define SYN_KWP2000_SID_TESTER_PRESENT 0x3EU                 /**< TesterPresent */
#define SYN_KWP2000_SID_NEGATIVE_RESPONSE 0x7FU              /**< NegativeResponse */
/**@}*/

/** @name KWP2000 Session Modes (ISO 14230-3) */
/**@{*/
#define SYN_KWP2000_SESSION_DEFAULT 0x81U     /**< Standard diagnostic session */
#define SYN_KWP2000_SESSION_PROGRAMMING 0x85U /**< ECU programming / bootloader session */
#define SYN_KWP2000_SESSION_EXTENDED 0x86U    /**< Extended diagnostic session */
/**@}*/

/** @name KWP2000 Timing Constants (ISO 14230-3 §5.3) */
/**@{*/
#ifndef SYN_KWP2000_S3_TIMEOUT_MS
#define SYN_KWP2000_S3_TIMEOUT_MS 5000U /**< S3 server session timeout in ms */
#endif
/**@}*/

/** @name KWP2000 Negative Response Codes (NRC) */
/**@{*/
#define SYN_KWP2000_NRC_SERVICE_NOT_SUPPORTED 0x11U     /**< Service not supported */
#define SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED 0x12U /**< Subfunction not supported */
#define SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH \
    0x13U /**< Incorrect message length or invalid format */
#define SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT 0x22U      /**< Conditions not correct */
#define SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE 0x31U        /**< Request out of range */
#define SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED 0x33U      /**< Security access denied */
#define SYN_KWP2000_NRC_INVALID_KEY 0x35U                 /**< Invalid key */
#define SYN_KWP2000_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS 0x36U /**< Exceeded number of attempts */
#define SYN_KWP2000_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED \
    0x37U /**< Required time delay not expired */
#define SYN_KWP2000_NRC_RESPONSE_PENDING                    \
    0x78U /**< Request correctly received, response pending \
           */
/**@}*/

/** @brief KWP2000 session type. */
typedef uint8_t SYN_KWP2000_Session;

/** @brief Registered Local Identifier (LID) descriptor. */
typedef struct {
    uint8_t lid;   /**< 1-byte Local Identifier (e.g. 0x01, 0x10) */
    void *data;    /**< Pointer to data buffer                     */
    uint16_t len;  /**< Length of data buffer in bytes             */
    bool writable; /**< Is LID writable via Service 0x3B?          */
} SYN_KWP2000_LID;

/** @brief Registered Common Identifier (CID) descriptor. */
typedef struct {
    uint16_t cid;  /**< 2-byte Common Identifier (e.g. 0xF190)    */
    void *data;    /**< Pointer to data buffer                     */
    uint16_t len;  /**< Length of data buffer in bytes             */
    bool writable; /**< Is CID writable via Service 0x2E?          */
} SYN_KWP2000_CID;

/** @brief Session transition callback function pointer. */
typedef bool (*SYN_KWP2000_SessionHandler)(SYN_KWP2000_Session from_session,
                                           SYN_KWP2000_Session to_session, void *user_ctx);

/** @brief ECU reset callback function pointer. */
typedef void (*SYN_KWP2000_ResetHandler)(uint8_t reset_type, void *user_ctx);

/** @brief Security Access seed request callback. */
typedef bool (*SYN_KWP2000_SeedHandler)(uint8_t level, uint8_t *seed_buf, uint16_t *seed_len,
                                        void *user_ctx);

/** @brief Security Access key verification callback. */
typedef bool (*SYN_KWP2000_KeyHandler)(uint8_t level, const uint8_t *key, uint16_t key_len,
                                       void *user_ctx);

/** @brief Routine control callback. */
typedef bool (*SYN_KWP2000_RoutineHandler)(uint8_t routine_id, const uint8_t *in_data,
                                           uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len,
                                           uint16_t *out_len, void *user_ctx);

/** @brief Memory access callback for 0x34 RequestDownload & 0x36 TransferData. */
typedef bool (*SYN_KWP2000_MemoryHandler)(bool is_write, uint32_t address, uint32_t size,
                                          uint8_t *data_buf, void *user_ctx);

/** @brief KWP2000 Server instance state. */
typedef struct {
    SYN_KWP2000_Session current_session; /**< Current diagnostic session          */
    bool security_unlocked;              /**< Is security access unlocked?        */
    uint8_t security_level;              /**< Current active security level       */
    uint32_t s3_timer_ms;                /**< S3 session keep-alive timer ms      */

    SYN_KWP2000_LID lids[SYN_KWP2000_MAX_LIDS]; /**< Registered LIDs             */
    uint16_t lid_count;                         /**< Active LID count                    */

    SYN_KWP2000_CID cids[SYN_KWP2000_MAX_CIDS]; /**< Registered CIDs             */
    uint16_t cid_count;                         /**< Active CID count                    */

    SYN_KWP2000_SessionHandler session_cb; /**< Session transition handler          */
    void *session_ctx;                     /**< Session context                     */

    SYN_KWP2000_ResetHandler reset_cb; /**< Reset handler                       */
    void *reset_ctx;                   /**< Reset context                       */
    uint8_t pending_reset_type;        /**< Pending reset type (0 if none)      */
    uint32_t reset_wait_ms;            /**< Delay before executing reset        */

    SYN_KWP2000_SeedHandler seed_cb; /**< Security seed handler               */
    SYN_KWP2000_KeyHandler key_cb;   /**< Security key handler                */
    void *security_ctx;              /**< Security context                    */

    SYN_KWP2000_RoutineHandler routine_cb; /**< Routine control handler             */
    void *routine_ctx;                     /**< Routine context                     */

    SYN_KWP2000_MemoryHandler memory_cb; /**< Memory access handler               */
    void *memory_ctx;                    /**< Memory context                      */

    bool is_downloading;        /**< Is RequestDownload active?          */
    uint8_t expected_block_seq; /**< Expected 0x36 block sequence number */
    uint32_t download_addr;     /**< Target download address             */
    uint32_t download_size;     /**< Expected total transfer size        */
    uint32_t download_received; /**< Bytes received so far               */
} SYN_KWP2000_Server;

/* ── API Declarations ────────────────────────────────────────────────── */

/**
 * @brief Initialize a KWP2000 diagnostic server instance.
 * @param server Pointer to server instance.
 * @return SYN_OK on success, SYN_INVALID_PARAM if server is NULL.
 */
SYN_Status syn_kwp2000_init(SYN_KWP2000_Server *server);

/**
 * @brief Register a Local Identifier (LID, 1-byte) for Service 0x21/0x3B.
 * @param server Pointer to server instance.
 * @param lid 1-byte Local Identifier.
 * @param data Pointer to mutable data buffer.
 * @param len Data length in bytes.
 * @param writable Is LID writable?
 * @return SYN_OK on success, SYN_INVALID_PARAM or SYN_ERROR on error.
 */
SYN_Status syn_kwp2000_register_lid(SYN_KWP2000_Server *server, uint8_t lid, void *data,
                                    uint16_t len, bool writable);

/**
 * @brief Register a Common Identifier (CID, 2-byte) for Service 0x22/0x2E.
 * @param server Pointer to server instance.
 * @param cid 2-byte Common Identifier.
 * @param data Pointer to mutable data buffer.
 * @param len Data length in bytes.
 * @param writable Is CID writable?
 * @return SYN_OK on success, SYN_INVALID_PARAM or SYN_ERROR on error.
 */
SYN_Status syn_kwp2000_register_cid(SYN_KWP2000_Server *server, uint16_t cid, void *data,
                                    uint16_t len, bool writable);

/**
 * @brief Set session transition callback handler.
 * @param server Pointer to server instance.
 * @param cb Session handler callback.
 * @param user_ctx User context pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if server is NULL.
 */
SYN_Status syn_kwp2000_set_session_handler(SYN_KWP2000_Server *server,
                                           SYN_KWP2000_SessionHandler cb, void *user_ctx);

/**
 * @brief Set ECU reset callback handler and deferred wait delay.
 * @param server Pointer to server instance.
 * @param cb Reset handler callback.
 * @param user_ctx User context pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if server is NULL.
 */
SYN_Status syn_kwp2000_set_reset_handler(SYN_KWP2000_Server *server, SYN_KWP2000_ResetHandler cb,
                                         void *user_ctx);

/**
 * @brief Set security access seed and key callbacks.
 * @param server Pointer to server instance.
 * @param seed_cb Seed request callback.
 * @param key_cb Key verification callback.
 * @param user_ctx User context pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if server is NULL.
 */
SYN_Status syn_kwp2000_set_security_handler(SYN_KWP2000_Server *server,
                                            SYN_KWP2000_SeedHandler seed_cb,
                                            SYN_KWP2000_KeyHandler key_cb, void *user_ctx);

/**
 * @brief Register RoutineControl handler.
 * @param server Pointer to server instance.
 * @param cb Routine handler callback.
 * @param user_ctx User context pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if server is NULL.
 */
SYN_Status syn_kwp2000_set_routine_handler(SYN_KWP2000_Server *server,
                                           SYN_KWP2000_RoutineHandler cb, void *user_ctx);

/**
 * @brief Register memory access handler for 0x34 RequestDownload & 0x36 TransferData.
 * @param server Pointer to server instance.
 * @param cb Memory handler callback.
 * @param user_ctx User context pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if server is NULL.
 */
SYN_Status syn_kwp2000_set_memory_handler(SYN_KWP2000_Server *server, SYN_KWP2000_MemoryHandler cb,
                                          void *user_ctx);

/**
 * @brief Process an incoming KWP2000 request message PDU and generate response.
 * @param server Pointer to server instance.
 * @param req_buf Pointer to request payload buffer.
 * @param req_len Length of request payload in bytes.
 * @param resp_buf Pointer to output response buffer.
 * @param max_resp_len Capacity of output response buffer.
 * @param resp_len Pointer to store output response length.
 * @return SYN_OK on success, SYN_INVALID_PARAM or SYN_ERROR on failure.
 */
SYN_Status syn_kwp2000_process_request(SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                       uint16_t req_len, uint8_t *resp_buf, uint16_t max_resp_len,
                                       uint16_t *resp_len);

/**
 * @brief Tick KWP2000 server S3 timer and deferred reset state.
 * @param server Pointer to server instance.
 * @param dt_ms Elapsed time step in milliseconds.
 */
void syn_kwp2000_tick(SYN_KWP2000_Server *server, uint32_t dt_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYN_KWP2000_H */
