/**
 * @file syn_uds.c
 * @brief ISO 14229 Unified Diagnostic Services (UDS) Server Implementation.
 */

#include "syn_uds.h"

#include "../crypto/syn_aes.h"
#include "../util/syn_pack.h"

#include <string.h>

/** @brief Internal UDS request processing context. */
typedef struct {
    SYN_UDS_Server *server;     /**< UDS server instance */
    uint8_t sid;                /**< Request service identifier */
    const uint8_t *req;         /**< Request PDU buffer */
    uint16_t req_len;           /**< Request PDU byte length */
    uint8_t *resp_buf;          /**< Response PDU output buffer */
    uint16_t max_resp_len;      /**< Capacity of resp_buf in bytes */
    uint16_t *resp_len;         /**< Output response PDU byte length */
    SYN_UDS_AddrMode addr_mode; /**< Physical or functional addressing mode */
    bool success;               /**< Set true when positive response is built */
} UDS_ReqCtx;

/** @brief UDS service handler function signature. */
typedef bool (*UDS_ServiceFn)(UDS_ReqCtx *ctx);

/** @brief UDS service dispatch table entry. */
typedef struct {
    uint8_t sid;      /**< UDS Service Identifier */
    UDS_ServiceFn fn; /**< Service handler function */
} UDS_ServiceEntry;

bool syn_uds_init(SYN_UDS_Server *server)
{
    if (server == NULL) {
        return false;
    }

    static const uint8_t default_seed_16[16] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF,
                                                0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

    memset(server, 0, sizeof(*server));
    server->session = SYN_UDS_SESSION_DEFAULT;
    server->security_state = SYN_UDS_SECURITY_LOCKED;
    server->security_level = 0U;
    server->current_seed = 0x12345678U;
    server->active_seed = 0x12345678U;
    server->use_aes128_security = false;
    memcpy(server->current_seed_bytes, default_seed_16, 16);
    memcpy(server->active_seed_bytes, default_seed_16, 16);
    server->comm_control_state = SYN_UDS_COMM_ENABLE_RX_AND_TX;
    server->dtc_setting_enabled = true;
    server->p2_max_ms = 50U;
    server->p2_star_max_10ms = 500U;
    server->active_p2_max_ms = 50U;
    server->active_p2_star_max_10ms = 500U;
    server->transfer_state = SYN_UDS_TRANSFER_IDLE;
    server->expected_block_seq = 1U;
    server->reset_tx_wait_ms = SYN_UDS_DEFAULT_RESET_TX_WAIT_MS;

    return true;
}

/**
 * @brief Find the table index of a registered 16-bit Data Identifier (DID).
 * @param server UDS server instance.
 * @param did 16-bit Data Identifier to look up.
 * @return Zero-based table index on match, or -1 if not found.
 */
static int find_did_index(const SYN_UDS_Server *server, uint16_t did)
{
    for (uint8_t i = 0U; i < server->did_count; i++) {
        if (server->did_table[i].did == did) {
            return (int)i;
        }
    }
    return -1;
}

/**
 * @brief Find the table index of a registered 24-bit Diagnostic Trouble Code (DTC).
 * @param server UDS server instance.
 * @param dtc 24-bit DTC identifier to look up.
 * @return Zero-based table index on match, or -1 if not found.
 */
static int find_dtc_index(const SYN_UDS_Server *server, uint32_t dtc)
{
    for (uint8_t i = 0U; i < server->dtc_count; i++) {
        if (server->dtc_table[i].dtc == dtc) {
            return (int)i;
        }
    }
    return -1;
}

bool syn_uds_enable_aes128_security(SYN_UDS_Server *server, const uint8_t key[16])
{
    if ((server == NULL) || (key == NULL)) {
        return false;
    }
    server->use_aes128_security = true;
    memcpy(server->aes_security_key, key, 16);
    return true;
}

bool syn_uds_disable_aes128_security(SYN_UDS_Server *server)
{
    if (server == NULL) {
        return false;
    }
    server->use_aes128_security = false;
    memset(server->aes_security_key, 0, sizeof(server->aes_security_key));
    return true;
}

bool syn_uds_set_security_seed_bytes(SYN_UDS_Server *server, const uint8_t seed[16])
{
    if ((server == NULL) || (seed == NULL)) {
        return false;
    }
    memcpy(server->current_seed_bytes, seed, 16);
    return true;
}

/**
 * @brief Reset session, security, communication, DTC setting, and transfer state to default.
 * @param server UDS server instance.
 */
static void uds_reset_default_session_state(SYN_UDS_Server *server)
{
    server->session = SYN_UDS_SESSION_DEFAULT;
    server->security_state = SYN_UDS_SECURITY_LOCKED;
    server->security_level = 0U;
    server->comm_control_state = SYN_UDS_COMM_ENABLE_RX_AND_TX;
    server->dtc_setting_enabled = true;
    server->transfer_state = SYN_UDS_TRANSFER_IDLE;
    server->s3_timer_ms = 0U;
}

void syn_uds_tick(SYN_UDS_Server *server, uint32_t dt_ms)
{
    if (server == NULL) {
        return;
    }

    if (server->security_delay_timer_ms > 0U) {
        if (dt_ms >= server->security_delay_timer_ms) {
            server->security_delay_timer_ms = 0U;
            if (server->security_error_count > 0U) {
                server->security_error_count--;
            }
        } else {
            server->security_delay_timer_ms -= dt_ms;
        }
    }

    if (server->session != SYN_UDS_SESSION_DEFAULT) {
        if (dt_ms >= server->s3_timer_ms) {
            uds_reset_default_session_state(server);
        } else {
            server->s3_timer_ms -= dt_ms;
        }
    } else {
        server->s3_timer_ms = 0U;
    }

    if (server->reset_type_requested > 0U) {
        if (dt_ms > (UINT32_MAX - server->reset_wait_elapsed_ms)) {
            server->reset_wait_elapsed_ms = UINT32_MAX;
        } else {
            server->reset_wait_elapsed_ms += dt_ms;
        }
        if (server->reset_wait_elapsed_ms >= (uint32_t)server->reset_tx_wait_ms) {
            uint8_t reset_type = server->reset_type_requested;
            server->reset_type_requested = 0U;
            server->reset_wait_elapsed_ms = 0U;
            if (server->reset_cb != NULL) {
                server->reset_cb(reset_type, server->reset_ctx);
            }
        }
    } else {
        server->reset_wait_elapsed_ms = 0U;
    }
}

void syn_uds_set_session_transition_handler(SYN_UDS_Server *server,
                                            SYN_UDS_SessionTransitionHandler cb, void *ctx)
{
    if (server != NULL) {
        server->session_transition_cb = cb;
        server->session_transition_ctx = ctx;
    }
}

void syn_uds_set_reset_handler(SYN_UDS_Server *server, SYN_UDS_ResetHandler cb, void *ctx)
{
    if (server != NULL) {
        server->reset_cb = cb;
        server->reset_ctx = ctx;
    }
}

void syn_uds_set_reset_wait_ms(SYN_UDS_Server *server, uint16_t wait_ms)
{
    if (server != NULL) {
        server->reset_tx_wait_ms = wait_ms;
    }
}

uint8_t syn_uds_get_pending_reset(const SYN_UDS_Server *server)
{
    if (server == NULL) {
        return 0U;
    }
    return server->reset_type_requested;
}

void syn_uds_clear_pending_reset(SYN_UDS_Server *server)
{
    if (server != NULL) {
        server->reset_type_requested = 0U;
        server->reset_wait_elapsed_ms = 0U;
    }
}

bool syn_uds_register_did_ext(SYN_UDS_Server *server, uint16_t did, uint8_t *data, uint16_t len,
                              bool writable, uint8_t session_mask, uint16_t security_mask)
{
    if ((server == NULL) || (data == NULL) || (len == 0U)) {
        return false;
    }

    if (server->did_count >= SYN_UDS_MAX_DIDS) {
        return false;
    }

    SYN_UDS_DIDEntry *entry = &server->did_table[server->did_count];
    entry->did = did;
    entry->data = data;
    entry->len = len;
    entry->writable = writable;
    entry->session_mask = session_mask;
    entry->security_mask = security_mask;

    server->did_count++;
    return true;
}

bool syn_uds_register_did(SYN_UDS_Server *server, uint16_t did, uint8_t *data, uint16_t len,
                          bool writable)
{
    return syn_uds_register_did_ext(server, did, data, len, writable, SYN_UDS_SESSION_MASK_ALL,
                                    SYN_UDS_SECURITY_MASK_ALL);
}

bool syn_uds_register_comm_control(SYN_UDS_Server *server, SYN_UDS_CommControlHandler handler,
                                   void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->comm_control_cb = handler;
    server->comm_control_ctx = ctx;
    return true;
}

bool syn_uds_register_access_timing(SYN_UDS_Server *server, SYN_UDS_AccessTimingHandler handler,
                                    void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->timing_cb = handler;
    server->timing_ctx = ctx;
    return true;
}

bool syn_uds_register_secured_data(SYN_UDS_Server *server, SYN_UDS_SecuredDataHandler handler,
                                   void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->secured_data_cb = handler;
    server->secured_data_ctx = ctx;
    return true;
}

bool syn_uds_register_memory_handler(SYN_UDS_Server *server, SYN_UDS_MemoryHandler handler,
                                     void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->memory_cb = handler;
    server->memory_ctx = ctx;
    return true;
}

bool syn_uds_register_auth_handler(SYN_UDS_Server *server, SYN_UDS_AuthHandler handler, void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->auth_cb = handler;
    server->auth_ctx = ctx;
    return true;
}

bool syn_uds_register_file_transfer(SYN_UDS_Server *server, SYN_UDS_FileTransferHandler handler,
                                    void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->file_transfer_cb = handler;
    server->file_transfer_ctx = ctx;
    return true;
}

bool syn_uds_register_routine_control(SYN_UDS_Server *server, SYN_UDS_RoutineControlHandler handler,
                                      void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->routine_cb = handler;
    server->routine_ctx = ctx;
    return true;
}

bool syn_uds_register_io_control(SYN_UDS_Server *server, SYN_UDS_IOControlHandler handler,
                                 void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->io_control_cb = handler;
    server->io_control_ctx = ctx;
    return true;
}

bool syn_uds_register_link_control(SYN_UDS_Server *server, SYN_UDS_LinkControlHandler handler,
                                   void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->link_control_cb = handler;
    server->link_control_ctx = ctx;
    return true;
}

bool syn_uds_register_roe_handler(SYN_UDS_Server *server, SYN_UDS_ResponseOnEventHandler handler,
                                  void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->roe_cb = handler;
    server->roe_ctx = ctx;
    return true;
}

bool syn_uds_register_scaling_data_handler(SYN_UDS_Server *server,
                                           SYN_UDS_ScalingDataHandler handler, void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->scaling_cb = handler;
    server->scaling_ctx = ctx;
    return true;
}

bool syn_uds_register_periodic_data_handler(SYN_UDS_Server *server,
                                            SYN_UDS_PeriodicDataHandler handler, void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->periodic_cb = handler;
    server->periodic_ctx = ctx;
    return true;
}

bool syn_uds_register_dynamic_did_handler(SYN_UDS_Server *server, SYN_UDS_DynamicDIDHandler handler,
                                          void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->dynamic_did_cb = handler;
    server->dynamic_did_ctx = ctx;
    return true;
}

bool syn_uds_register_dtc(SYN_UDS_Server *server, uint32_t dtc, uint8_t status, uint8_t severity)
{
    if ((server == NULL) || (server->dtc_count >= SYN_UDS_MAX_DTCS)) {
        return false;
    }
    server->dtc_table[server->dtc_count].dtc = dtc & 0xFFFFFFU;
    server->dtc_table[server->dtc_count].status = status;
    server->dtc_table[server->dtc_count].severity = severity;
    server->dtc_table[server->dtc_count].fault_cnt = 0;
    server->dtc_count++;
    return true;
}

bool syn_uds_register_dtc_handler(SYN_UDS_Server *server, SYN_UDS_DTCHandler handler, void *ctx)
{
    if (server == NULL) {
        return false;
    }
    server->dtc_cb = handler;
    server->dtc_ctx = ctx;
    return true;
}

/**
 * @brief Encode a UDS Negative Response Code (0x7F) or suppress it per functional addressing rules.
 * @param ctx UDS request processing context.
 * @param nrc ISO 14229-1 Negative Response Code byte.
 * @return true after populating or suppressing the negative response.
 */
static bool uds_nrc(UDS_ReqCtx *ctx, uint8_t nrc)
{
    if ((ctx->addr_mode == SYN_UDS_ADDR_FUNCTIONAL) &&
        ((nrc == SYN_UDS_NRC_SERVICE_NOT_SUPPORTED) ||
         (nrc == SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED) ||
         (nrc == SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION))) {
        *ctx->resp_len = 0U;
        return true;
    }
    ctx->resp_buf[0] = SYN_UDS_RESPONSE_NEGATIVE;
    ctx->resp_buf[1] = ctx->sid;
    ctx->resp_buf[2] = nrc;
    *ctx->resp_len = 3U;
    return true;
}

bool syn_uds_is_sid_functional_supported(uint8_t sid)
{
    switch (sid) {
    case SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL:
    case SYN_UDS_SID_ECU_RESET:
    case SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION:
    case SYN_UDS_SID_READ_DTC_INFORMATION:
    case SYN_UDS_SID_READ_DATA_BY_IDENTIFIER:
    case SYN_UDS_SID_COMMUNICATION_CONTROL:
    case SYN_UDS_SID_AUTHENTICATION:
    case SYN_UDS_SID_ROUTINE_CONTROL:
    case SYN_UDS_SID_TESTER_PRESENT:
    case SYN_UDS_SID_ACCESS_TIMING_PARAMETER:
    case SYN_UDS_SID_CONTROL_DTC_SETTING:
    case SYN_UDS_SID_LINK_CONTROL:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Check whether a UDS service uses a sub-function byte with suppressPosRspMsgIndicationBit.
 * @param sid UDS Service Identifier.
 * @return true if byte 1 carries a sub-function field, false otherwise.
 */
static bool uds_sid_has_subfunction(uint8_t sid)
{
    switch (sid) {
    case SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL:
    case SYN_UDS_SID_ECU_RESET:
    case SYN_UDS_SID_READ_DTC_INFORMATION:
    case SYN_UDS_SID_SECURITY_ACCESS:
    case SYN_UDS_SID_COMMUNICATION_CONTROL:
    case SYN_UDS_SID_AUTHENTICATION:
    case SYN_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER:
    case SYN_UDS_SID_ROUTINE_CONTROL:
    case SYN_UDS_SID_TESTER_PRESENT:
    case SYN_UDS_SID_ACCESS_TIMING_PARAMETER:
    case SYN_UDS_SID_CONTROL_DTC_SETTING:
    case SYN_UDS_SID_RESPONSE_ON_EVENT:
    case SYN_UDS_SID_LINK_CONTROL:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Convert a UDS diagnostic session enum into its single-bit permission mask.
 * @param session Active UDS diagnostic session.
 * @return Session permission bitmask.
 */
static uint8_t session_to_mask(SYN_UDS_Session session)
{
    if (session == SYN_UDS_SESSION_PROGRAMMING) {
        return SYN_UDS_SESSION_MASK_PROGRAMMING;
    }
    if (session == SYN_UDS_SESSION_EXTENDED) {
        return SYN_UDS_SESSION_MASK_EXTENDED;
    }
    if (session == SYN_UDS_SESSION_SAFETY_SYSTEM) {
        return SYN_UDS_SESSION_MASK_SAFETY;
    }
    return SYN_UDS_SESSION_MASK_DEFAULT;
}

/**
 * @brief Look up the permitted session bitmask configured for a UDS service identifier.
 * @param server UDS server instance.
 * @param sid UDS Service Identifier.
 * @return Permitted session bitmask (defaults to SYN_UDS_SESSION_MASK_ALL).
 */
static uint8_t get_sid_allowed_session_mask(const SYN_UDS_Server *server, uint8_t sid)
{
    for (uint8_t i = 0U; i < server->custom_session_count; i++) {
        if (server->custom_session_sids[i] == sid) {
            return server->custom_session_masks[i];
        }
    }
    return SYN_UDS_SESSION_MASK_ALL;
}

/**
 * @brief Look up the required security level bitmask configured for a UDS service identifier.
 * @param server UDS server instance.
 * @param sid UDS Service Identifier.
 * @return Required security level bitmask, or 0 if no custom security override exists.
 */
static uint16_t get_sid_required_security_mask(const SYN_UDS_Server *server, uint8_t sid)
{
    for (uint8_t i = 0U; i < server->custom_security_count; i++) {
        if (server->custom_security_sids[i] == sid) {
            return server->custom_security_masks[i];
        }
    }
    return 0U;
}

/**
 * @brief Compute the bitmask of currently satisfied security levels on the UDS server.
 * @param server UDS server instance.
 * @return Active security level bitmask including SYN_UDS_SECURITY_MASK_NONE.
 */
static uint16_t get_active_security_mask(const SYN_UDS_Server *server)
{
    uint16_t mask = (uint16_t)SYN_UDS_SECURITY_MASK_NONE;
    if ((server->security_state == SYN_UDS_SECURITY_UNLOCKED) && (server->security_level > 0U) &&
        (server->security_level <= 15U)) {
        mask |= (uint16_t)(1U << server->security_level);
    }
    return mask;
}

bool syn_uds_set_service_session_mask(SYN_UDS_Server *server, uint8_t sid, uint8_t session_mask)
{
    if (server == NULL) {
        return false;
    }
    for (uint8_t i = 0U; i < server->custom_session_count; i++) {
        if (server->custom_session_sids[i] == sid) {
            server->custom_session_masks[i] = session_mask;
            return true;
        }
    }
    if (server->custom_session_count < SYN_UDS_MAX_SERVICE_OVERRIDES) {
        uint8_t idx = server->custom_session_count++;
        server->custom_session_sids[idx] = sid;
        server->custom_session_masks[idx] = session_mask;
        return true;
    }
    return false;
}

bool syn_uds_set_service_security_mask(SYN_UDS_Server *server, uint8_t sid, uint16_t security_mask)
{
    if (server == NULL) {
        return false;
    }
    for (uint8_t i = 0U; i < server->custom_security_count; i++) {
        if (server->custom_security_sids[i] == sid) {
            server->custom_security_masks[i] = security_mask;
            return true;
        }
    }
    if (server->custom_security_count < SYN_UDS_MAX_SERVICE_OVERRIDES) {
        uint8_t idx = server->custom_security_count++;
        server->custom_security_sids[idx] = sid;
        server->custom_security_masks[idx] = security_mask;
        return true;
    }
    return false;
}

/**
 * @brief Handle DiagnosticSessionControl (0x10) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_session_control(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if ((sub < (uint8_t)SYN_UDS_SESSION_DEFAULT) ||
        (sub > (uint8_t)SYN_UDS_SESSION_SAFETY_SYSTEM)) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    if (ctx->max_resp_len < 6U) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }

    if (server->session_transition_cb != NULL) {
        if (!server->session_transition_cb(server->session, (SYN_UDS_Session)sub,
                                           server->session_transition_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    }

    /* ISO 14229-1 9.2.2: Lock security on every session transition while keeping lockout delay */
    if ((SYN_UDS_Session)sub == SYN_UDS_SESSION_DEFAULT) {
        uds_reset_default_session_state(server);
    } else {
        server->session = (SYN_UDS_Session)sub;
        server->security_state = SYN_UDS_SECURITY_LOCKED;
        server->security_level = 0U;
        server->s3_timer_ms = SYN_UDS_S3_TIMEOUT_MS;
    }

    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    syn_poke_u16(server->active_p2_max_ms, ctx->resp_buf, 2);
    syn_poke_u16(server->active_p2_star_max_10ms, ctx->resp_buf, 4);
    *ctx->resp_len = 6U;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle ECUReset (0x11) request and arm deferred reset timer.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_ecu_reset(UDS_ReqCtx *ctx)
{
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if ((sub < SYN_UDS_RESET_HARD) || (sub > SYN_UDS_RESET_DISABLE_RAPID_POWER_SHUTDOWN)) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    ctx->server->reset_type_requested = sub;
    ctx->server->reset_wait_elapsed_ms = 0U;
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    *ctx->resp_len = 2U;
    ctx->success = true;
    return true;
}

/**
 * @brief Record a failed SecurityAccess key attempt, arm lockout delay if needed, and emit NRC.
 * @param ctx UDS request processing context.
 * @return true after encoding NRC 0x35 or 0x36.
 */
static bool uds_security_fail(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    server->security_state = SYN_UDS_SECURITY_LOCKED;
    server->security_level = 0U;
    server->security_error_count++;
    if (server->security_error_count >= SYN_UDS_SECURITY_MAX_ATTEMPTS) {
        server->security_delay_timer_ms = SYN_UDS_SECURITY_DELAY_MS;
        return uds_nrc(ctx, SYN_UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS);
    }
    return uds_nrc(ctx, SYN_UDS_NRC_INVALID_KEY);
}

/**
 * @brief Verify SecurityAccess sendKey (even sub-function) payload and unlock security level.
 * @param ctx UDS request processing context.
 * @param sub Masked even SecurityAccess sub-function byte.
 * @return true after encoding positive or negative response.
 */
static bool uds_security_send_key(UDS_ReqCtx *ctx, uint8_t sub)
{
    SYN_UDS_Server *server = ctx->server;
    if (server->security_state != SYN_UDS_SECURITY_SEED_SENT) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR);
    }
    if (sub != (uint8_t)(server->active_seed_subfunction + 1U)) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR);
    }

    if (server->use_aes128_security) {
        if (ctx->req_len < 18U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        SYN_AES_Context aes_ctx;
        (void)syn_aes_init(&aes_ctx, server->aes_security_key, 16U);
        uint8_t expected_key[16];
        syn_aes_encrypt_block(&aes_ctx, server->active_seed_bytes, expected_key);
        if (memcmp(&ctx->req[2], expected_key, 16) != 0) {
            return uds_security_fail(ctx);
        }
    } else {
        if (ctx->req_len < 6U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        uint32_t key = syn_peek_u32(ctx->req, 2);
        uint32_t expected_key = server->active_seed ^ 0xA5A5A5A5U;
        if (key != expected_key) {
            return uds_security_fail(ctx);
        }
    }

    server->security_state = SYN_UDS_SECURITY_UNLOCKED;
    server->security_level = sub / 2U;
    server->security_error_count = 0U;
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    *ctx->resp_len = 2U;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle SecurityAccess (0x27) requestSeed and sendKey sub-functions.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_security_access(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if ((sub == 0x00U) || (sub == 0x7FU)) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    if ((sub & 1U) == 0U) {
        return uds_security_send_key(ctx, sub);
    }

    if (server->security_delay_timer_ms > 0U) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED);
    }
    uint16_t seed_len = server->use_aes128_security ? 16U : 4U;
    if ((uint16_t)(2U + seed_len) > ctx->max_resp_len) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    server->security_state = SYN_UDS_SECURITY_SEED_SENT;
    server->active_seed_subfunction = sub;
    if (server->use_aes128_security) {
        memcpy(server->active_seed_bytes, server->current_seed_bytes, 16);
        memcpy(&ctx->resp_buf[2], server->current_seed_bytes, 16);
        *ctx->resp_len = 18U;
    } else {
        server->active_seed = server->current_seed;
        syn_poke_u32(server->current_seed, ctx->resp_buf, 2);
        *ctx->resp_len = 6U;
    }
    ctx->success = true;
    return true;
}

/**
 * @brief Handle CommunicationControl (0x28) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_comm_control(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 3U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if (sub > 0x05U) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    uint8_t comm_type = ctx->req[2];
    if (comm_type == 0U) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }

    if (server->comm_control_cb != NULL) {
        if (!server->comm_control_cb((SYN_UDS_CommControlType)sub, comm_type,
                                     server->comm_control_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    }

    server->comm_control_state = (SYN_UDS_CommControlType)sub;
    server->comm_type = comm_type;
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    *ctx->resp_len = 2U;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle ReadDataByIdentifier (0x22) request for one or more DIDs.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_read_did(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if ((ctx->req_len < 3U) || (((ctx->req_len - 1U) % 2U) != 0U)) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }

    ctx->resp_buf[0] = ctx->sid + 0x40U;
    size_t out_pos = 1U;

    for (size_t req_pos = 1U; req_pos < ctx->req_len; req_pos += 2U) {
        uint16_t target_did = syn_peek_u16(ctx->req, req_pos);
        int did_idx = find_did_index(server, target_did);
        if (did_idx < 0) {
            return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
        }
        SYN_UDS_DIDEntry *matched_entry = &server->did_table[did_idx];
        if ((matched_entry->session_mask & session_to_mask(server->session)) == 0U) {
            return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION);
        }
        if ((get_active_security_mask(server) & matched_entry->security_mask) == 0U) {
            return uds_nrc(ctx, SYN_UDS_NRC_SECURITY_ACCESS_DENIED);
        }
        if ((out_pos + 2U + matched_entry->len) > ctx->max_resp_len) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }

        syn_poke_u16(target_did, ctx->resp_buf, out_pos);
        out_pos += 2U;
        memcpy(&ctx->resp_buf[out_pos], matched_entry->data, matched_entry->len);
        out_pos += matched_entry->len;
    }

    *ctx->resp_len = (uint16_t)out_pos;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle WriteDataByIdentifier (0x2E) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_write_did(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 4U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint16_t target_did = syn_peek_u16(ctx->req, 1);
    int did_idx = find_did_index(server, target_did);
    if (did_idx < 0) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }
    SYN_UDS_DIDEntry *matched_entry = &server->did_table[did_idx];
    if ((matched_entry->session_mask & session_to_mask(server->session)) == 0U) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION);
    }
    uint16_t data_len = ctx->req_len - 3U;
    if (data_len != matched_entry->len) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    if (!matched_entry->writable) {
        return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
    }
    bool locked_default = (server->security_state != SYN_UDS_SECURITY_UNLOCKED) &&
                          (matched_entry->security_mask == SYN_UDS_SECURITY_MASK_ALL);
    if (locked_default ||
        ((get_active_security_mask(server) & matched_entry->security_mask) == 0U)) {
        return uds_nrc(ctx, SYN_UDS_NRC_SECURITY_ACCESS_DENIED);
    }

    memcpy(matched_entry->data, &ctx->req[3], data_len);
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    syn_poke_u16(target_did, ctx->resp_buf, 1);
    *ctx->resp_len = 3U;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle RoutineControl (0x31) start, stop, and request-results sub-functions.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_routine_control(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 4U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if ((sub < 0x01U) || (sub > 0x03U)) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    if (ctx->max_resp_len < 4U) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }
    uint16_t routine_id = syn_peek_u16(ctx->req, 2);
    uint16_t out_payload_len = 0U;
    if (server->routine_cb != NULL) {
        if (!server->routine_cb(sub, routine_id, &ctx->req[4], ctx->req_len - 4U, &ctx->resp_buf[4],
                                ctx->max_resp_len - 4U, &out_payload_len, server->routine_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    syn_poke_u16(routine_id, ctx->resp_buf, 2);
    *ctx->resp_len = 4U + out_payload_len;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle TesterPresent (0x3E) keepalive request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_tester_present(UDS_ReqCtx *ctx)
{
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if (sub != 0x00U) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    *ctx->resp_len = 2U;
    ctx->success = true;
    return true;
}

/**
 * @brief Encode P2 and P2* timing parameters for AccessTimingParameter read sub-functions.
 * @param ctx UDS request processing context.
 * @param mode AccessTimingParameter read mode sub-function.
 * @param p2 Default or active P2Server_max value in milliseconds.
 * @param p2_star Default or active P2*Server_max value in 10 ms units.
 * @return true after encoding positive or negative response.
 */
static bool uds_timing_read(UDS_ReqCtx *ctx, SYN_UDS_AccessTimingType mode, uint16_t p2,
                            uint16_t p2_star)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len != 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    if (ctx->max_resp_len < 6U) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }
    if (server->timing_cb != NULL) {
        if (!server->timing_cb(mode, &p2, &p2_star, server->timing_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = (uint8_t)mode;
    syn_poke_u16(p2, ctx->resp_buf, 2);
    syn_poke_u16(p2_star, ctx->resp_buf, 4);
    *ctx->resp_len = 6U;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle AccessTimingParameter setTimingParametersToDefaultValues (0x83 0x02).
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_timing_set_default(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len != 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint16_t p2 = server->p2_max_ms;
    uint16_t p2_star = server->p2_star_max_10ms;
    if ((server->timing_cb != NULL) &&
        !server->timing_cb(SYN_UDS_TIMING_SET_TO_DEFAULT, &p2, &p2_star, server->timing_ctx)) {
        return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
    }
    server->active_p2_max_ms = p2;
    server->active_p2_star_max_10ms = p2_star;
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = (uint8_t)SYN_UDS_TIMING_SET_TO_DEFAULT;
    *ctx->resp_len = 2U;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle AccessTimingParameter (0x83) read and set sub-functions.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_access_timing(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if (sub == SYN_UDS_TIMING_READ_EXTENDED) {
        return uds_timing_read(ctx, SYN_UDS_TIMING_READ_EXTENDED, server->p2_max_ms,
                               server->p2_star_max_10ms);
    }
    if (sub == SYN_UDS_TIMING_READ_ACTIVE) {
        return uds_timing_read(ctx, SYN_UDS_TIMING_READ_ACTIVE, server->active_p2_max_ms,
                               server->active_p2_star_max_10ms);
    }
    if (sub == SYN_UDS_TIMING_SET_TO_DEFAULT) {
        return uds_timing_set_default(ctx);
    }
    if (sub != SYN_UDS_TIMING_SET_TO_GIVEN) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    if (ctx->req_len != 6U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint16_t given_p2 = syn_peek_u16(ctx->req, 2);
    uint16_t given_p2_star = syn_peek_u16(ctx->req, 4);
    if ((server->timing_cb != NULL) && !server->timing_cb(SYN_UDS_TIMING_SET_TO_GIVEN, &given_p2,
                                                          &given_p2_star, server->timing_ctx)) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }
    server->active_p2_max_ms = given_p2;
    server->active_p2_star_max_10ms = given_p2_star;

    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    *ctx->resp_len = 2U;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle SecuredDataTransmission (0x84) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_secured_data(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    if (server->security_state != SYN_UDS_SECURITY_UNLOCKED) {
        return uds_nrc(ctx, SYN_UDS_NRC_SECURITY_ACCESS_DENIED);
    }
    uint16_t sec_in_len = ctx->req_len - 1U;
    const uint8_t *sec_in_data = &ctx->req[1];
    uint16_t sec_out_len = 0U;

    if (server->secured_data_cb != NULL) {
        if (!server->secured_data_cb(sec_in_data, sec_in_len, &ctx->resp_buf[1],
                                     ctx->max_resp_len - 1U, &sec_out_len,
                                     server->secured_data_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    } else {
        if (sec_in_len > (ctx->max_resp_len - 1U)) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
        memcpy(&ctx->resp_buf[1], sec_in_data, sec_in_len);
        sec_out_len = sec_in_len;
    }

    ctx->resp_buf[0] = ctx->sid + 0x40U;
    *ctx->resp_len = 1U + sec_out_len;
    ctx->success = true;
    return true;
}

/**
 * @brief Check whether a 24-bit group identifier matches an ISO 14229-1 standard DTC group.
 * @param group 24-bit groupOfDTC value.
 * @return true if group is a predefined standard DTC group, false otherwise.
 */
static bool uds_is_standard_dtc_group(uint32_t group)
{
    switch (group) {
    case SYN_UDS_DTC_GROUP_ALL:
    case SYN_UDS_DTC_GROUP_EMISSIONS:
    case SYN_UDS_DTC_GROUP_POWERTRAIN:
    case SYN_UDS_DTC_GROUP_CHASSIS:
    case SYN_UDS_DTC_GROUP_BODY:
    case SYN_UDS_DTC_GROUP_NETWORK:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Check whether a 24-bit DTC belongs to the specified DTC group or exact DTC value.
 * @param dtc 24-bit Diagnostic Trouble Code.
 * @param group 24-bit groupOfDTC selector.
 * @return true if dtc falls within group, false otherwise.
 */
static bool uds_dtc_in_group(uint32_t dtc, uint32_t group)
{
    if ((group == SYN_UDS_DTC_GROUP_ALL) || (dtc == group)) {
        return true;
    }
    if (group == SYN_UDS_DTC_GROUP_EMISSIONS) {
        return dtc <= 0x0FFFFFU;
    }
    if (group == SYN_UDS_DTC_GROUP_POWERTRAIN) {
        return (dtc >= 0x100000U) && (dtc <= 0x3FFFFFU);
    }
    if (group == SYN_UDS_DTC_GROUP_CHASSIS) {
        return (dtc >= 0x400000U) && (dtc <= 0x7FFFFFU);
    }
    if (group == SYN_UDS_DTC_GROUP_BODY) {
        return (dtc >= 0x800000U) && (dtc <= 0xBFFFFFU);
    }
    return (group == SYN_UDS_DTC_GROUP_NETWORK) && (dtc >= 0xC00000U) && (dtc <= 0xFEFFFFU);
}

/**
 * @brief Handle ClearDiagnosticInformation (0x14) request for a DTC or DTC group.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_clear_dtc(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len != 4U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint32_t group_of_dtc =
        ((uint32_t)ctx->req[1] << 16U) | ((uint32_t)ctx->req[2] << 8U) | (uint32_t)ctx->req[3];

    if (!uds_is_standard_dtc_group(group_of_dtc) && (find_dtc_index(server, group_of_dtc) < 0)) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }

    for (uint8_t i = 0U; i < server->dtc_count; i++) {
        if (uds_dtc_in_group(server->dtc_table[i].dtc, group_of_dtc)) {
            server->dtc_table[i].status =
                (uint8_t)(SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR |
                          SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE);
            server->dtc_table[i].fault_cnt = 0;
        }
    }

    ctx->resp_buf[0] = ctx->sid + 0x40U;
    *ctx->resp_len = 1U;
    ctx->success = true;
    return true;
}

/**
 * @brief Pack a 24-bit DTC value in big-endian byte order into an output buffer.
 * @param buf Target output buffer.
 * @param pos Write offset within buf.
 * @param dtc 24-bit Diagnostic Trouble Code value.
 */
static void uds_pack_dtc24(uint8_t *buf, uint16_t pos, uint32_t dtc)
{
    buf[pos] = (uint8_t)(dtc >> 16U);
    buf[pos + 1U] = (uint8_t)(dtc >> 8U);
    buf[pos + 2U] = (uint8_t)dtc;
}

/**
 * @brief Encode a list of 24-bit DTCs and status bytes filtered by status mask.
 * @param ctx UDS request processing context.
 * @param sub ReadDTCInformation sub-function byte.
 * @param mask Status bitmask filter.
 * @param match_all true to emit all registered DTCs regardless of mask, false to filter.
 * @return true after encoding positive or negative response.
 */
static bool uds_dtc_list_by_mask(UDS_ReqCtx *ctx, uint8_t sub, uint8_t mask, bool match_all)
{
    SYN_UDS_Server *server = ctx->server;
    uint16_t pos = 3U;
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    ctx->resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
    for (uint8_t i = 0U; i < server->dtc_count; i++) {
        if (match_all || ((server->dtc_table[i].status & mask) != 0U)) {
            if ((pos + 4U) > ctx->max_resp_len) {
                return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
            }
            uds_pack_dtc24(ctx->resp_buf, pos, server->dtc_table[i].dtc);
            ctx->resp_buf[pos + 3U] = server->dtc_table[i].status;
            pos += 4U;
        }
    }
    *ctx->resp_len = pos;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle ReadDTCInformation sub-functions 0x0B..0x0E (first/most recent failed/confirmed).
 * @param ctx UDS request processing context.
 * @param sub ReadDTCInformation sub-function byte.
 * @return true after encoding positive or negative response.
 */
static bool uds_dtc_first_or_recent(UDS_ReqCtx *ctx, uint8_t sub)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len != 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    ctx->resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
    uint8_t match_idx = 0xFFU;
    bool is_failed_query = (sub == SYN_UDS_DTC_REPORT_FIRST_TEST_FAILED) ||
                           (sub == SYN_UDS_DTC_REPORT_MOST_RECENT_TEST_FAILED);
    uint8_t target_mask = is_failed_query ? 0x01U : 0x08U;
    bool stop_first = (sub == SYN_UDS_DTC_REPORT_FIRST_TEST_FAILED) ||
                      (sub == SYN_UDS_DTC_REPORT_FIRST_CONFIRMED);

    for (uint8_t i = 0U; i < server->dtc_count; i++) {
        if ((server->dtc_table[i].status & target_mask) != 0U) {
            match_idx = i;
            if (stop_first) {
                break;
            }
        }
    }

    if (match_idx != 0xFFU) {
        if (ctx->max_resp_len < 7U) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
        uds_pack_dtc24(ctx->resp_buf, 3U, server->dtc_table[match_idx].dtc);
        ctx->resp_buf[6] = server->dtc_table[match_idx].status;
        *ctx->resp_len = 7U;
    } else {
        *ctx->resp_len = 3U;
    }
    ctx->success = true;
    return true;
}

/**
 * @brief Handle ReadDTCInformation severity-mask queries (0x07, 0x08).
 * @param ctx UDS request processing context.
 * @param sub ReadDTCInformation sub-function byte.
 * @return true if sub-function was handled, false otherwise.
 */
static bool uds_dtc_severity_and_fdc(UDS_ReqCtx *ctx, uint8_t sub)
{
    SYN_UDS_Server *server = ctx->server;
    if (sub == SYN_UDS_DTC_REPORT_NUMBER_BY_SEVERITY_MASK) {
        if (ctx->req_len != 4U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        if (ctx->max_resp_len < 6U) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
        uint16_t match_cnt = 0U;
        for (uint8_t i = 0U; i < server->dtc_count; i++) {
            if (((server->dtc_table[i].severity & ctx->req[2]) != 0U) &&
                ((server->dtc_table[i].status & ctx->req[3]) != 0U)) {
                match_cnt++;
            }
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        ctx->resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
        ctx->resp_buf[3] = SYN_UDS_DTC_FORMAT_ISO14229_1;
        syn_poke_u16(match_cnt, ctx->resp_buf, 4);
        *ctx->resp_len = 6U;
        ctx->success = true;
        return true;
    }
    if (sub == SYN_UDS_DTC_REPORT_BY_SEVERITY_MASK) {
        if (ctx->req_len != 4U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        uint16_t pos = 3U;
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        ctx->resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
        for (uint8_t i = 0U; i < server->dtc_count; i++) {
            if (((server->dtc_table[i].severity & ctx->req[2]) != 0U) &&
                ((server->dtc_table[i].status & ctx->req[3]) != 0U)) {
                if ((pos + 6U) > ctx->max_resp_len) {
                    return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
                }
                ctx->resp_buf[pos] = server->dtc_table[i].severity;
                ctx->resp_buf[pos + 1U] = 0x00U;
                uds_pack_dtc24(ctx->resp_buf, pos + 2U, server->dtc_table[i].dtc);
                ctx->resp_buf[pos + 5U] = server->dtc_table[i].status;
                pos += 6U;
            }
        }
        *ctx->resp_len = pos;
        ctx->success = true;
        return true;
    }
    return false;
}

/**
 * @brief Handle ReadDTCInformation severity-by-DTC (0x09) and fault detection counter (0x14).
 * @param ctx UDS request processing context.
 * @param sub ReadDTCInformation sub-function byte.
 * @return true if sub-function was handled, false otherwise.
 */
static bool uds_dtc_info_by_dtc(UDS_ReqCtx *ctx, uint8_t sub)
{
    SYN_UDS_Server *server = ctx->server;
    if (sub == SYN_UDS_DTC_REPORT_SEVERITY_INFO) {
        if (ctx->req_len != 5U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        uint32_t req_dtc =
            ((uint32_t)ctx->req[2] << 16U) | ((uint32_t)ctx->req[3] << 8U) | (uint32_t)ctx->req[4];
        int match_idx = find_dtc_index(server, req_dtc);
        if (match_idx >= 0) {
            if (ctx->max_resp_len < 8U) {
                return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
            }
            ctx->resp_buf[0] = ctx->sid + 0x40U;
            ctx->resp_buf[1] = sub;
            ctx->resp_buf[2] = server->dtc_table[match_idx].severity;
            ctx->resp_buf[3] = 0x00U;
            memcpy(&ctx->resp_buf[4], &ctx->req[2], 3U);
            ctx->resp_buf[7] = server->dtc_table[match_idx].status;
            *ctx->resp_len = 8U;
        } else {
            ctx->resp_buf[0] = ctx->sid + 0x40U;
            ctx->resp_buf[1] = sub;
            *ctx->resp_len = 2U;
        }
        ctx->success = true;
        return true;
    }
    if (sub == SYN_UDS_DTC_REPORT_FAULT_DETECTION_COUNTER) {
        if (ctx->req_len != 2U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        uint16_t pos = 2U;
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        for (uint8_t i = 0U; i < server->dtc_count; i++) {
            if ((pos + 4U) > ctx->max_resp_len) {
                return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
            }
            uds_pack_dtc24(ctx->resp_buf, pos, server->dtc_table[i].dtc);
            ctx->resp_buf[pos + 3U] = (uint8_t)server->dtc_table[i].fault_cnt;
            pos += 4U;
        }
        *ctx->resp_len = pos;
        ctx->success = true;
        return true;
    }
    return false;
}

/**
 * @brief Handle ReadDTCInformation snapshot and extended-data record queries by DTC.
 * @param ctx UDS request processing context.
 * @param sub ReadDTCInformation sub-function byte.
 * @return true if sub-function was handled, false otherwise.
 */
static bool uds_dtc_records(UDS_ReqCtx *ctx, uint8_t sub)
{
    SYN_UDS_Server *server = ctx->server;
    if ((sub == SYN_UDS_DTC_REPORT_SNAPSHOT_RECORD_BY_DTC) ||
        (sub == SYN_UDS_DTC_REPORT_EXT_DATA_RECORD_BY_DTC) ||
        (sub == SYN_UDS_DTC_REPORT_MIRROR_MEMORY_EXT_DATA)) {
        if (ctx->req_len != 6U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        uint32_t req_dtc =
            ((uint32_t)ctx->req[2] << 16U) | ((uint32_t)ctx->req[3] << 8U) | (uint32_t)ctx->req[4];
        int match_idx = find_dtc_index(server, req_dtc);
        if (match_idx < 0) {
            return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
        }
        if (ctx->max_resp_len < 6U) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        memcpy(&ctx->resp_buf[2], &ctx->req[2], 3U);
        ctx->resp_buf[5] = server->dtc_table[match_idx].status;
        *ctx->resp_len = 6U;
        ctx->success = true;
        return true;
    }
    if ((sub == SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_SNAPSHOT_BY_DTC) ||
        (sub == SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_EXT_DATA_BY_DTC)) {
        if (ctx->req_len != 7U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        uint32_t req_dtc =
            ((uint32_t)ctx->req[2] << 16U) | ((uint32_t)ctx->req[3] << 8U) | (uint32_t)ctx->req[4];
        int match_idx = find_dtc_index(server, req_dtc);
        if (match_idx < 0) {
            return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
        }
        if (ctx->max_resp_len < 7U) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        ctx->resp_buf[2] = ctx->req[6];
        memcpy(&ctx->resp_buf[3], &ctx->req[2], 3U);
        ctx->resp_buf[6] = server->dtc_table[match_idx].status;
        *ctx->resp_len = 7U;
        ctx->success = true;
        return true;
    }
    return false;
}

/**
 * @brief Handle ReadDTCInformation snapshot ID, record-number, user-memory, and WWH-OBD queries.
 * @param ctx UDS request processing context.
 * @param sub ReadDTCInformation sub-function byte.
 * @return true if sub-function was handled, false otherwise.
 */
static bool uds_dtc_misc_records(UDS_ReqCtx *ctx, uint8_t sub)
{
    if (sub == SYN_UDS_DTC_REPORT_SNAPSHOT_IDENTIFICATION) {
        if (ctx->req_len != 2U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        *ctx->resp_len = 2U;
        ctx->success = true;
        return true;
    }
    if ((sub == SYN_UDS_DTC_REPORT_STORED_DATA_BY_RECORD_NUM) ||
        (sub == SYN_UDS_DTC_REPORT_EXT_DATA_RECORD_BY_RECORD_NUM)) {
        if (ctx->req_len != 3U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        ctx->resp_buf[2] = ctx->req[2];
        *ctx->resp_len = 3U;
        ctx->success = true;
        return true;
    }
    if (sub == SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_BY_STATUS_MASK) {
        if (ctx->req_len != 4U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        if (ctx->max_resp_len < 4U) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        ctx->resp_buf[2] = ctx->req[3];
        ctx->resp_buf[3] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
        *ctx->resp_len = 4U;
        ctx->success = true;
        return true;
    }
    if (sub == SYN_UDS_DTC_REPORT_WWH_OBD_BY_MASK_RECORD) {
        if (ctx->req_len != 8U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        if (ctx->max_resp_len < 4U) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        ctx->resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
        ctx->resp_buf[3] = ctx->req[6];
        *ctx->resp_len = 4U;
        ctx->success = true;
        return true;
    }
    return false;
}

/**
 * @brief Encode matching DTC count for ReadDTCInformation count-by-status-mask sub-functions.
 * @param ctx UDS request processing context.
 * @param sub ReadDTCInformation sub-function byte.
 * @return true after encoding positive or negative response.
 */
static bool uds_dtc_count_by_mask(UDS_ReqCtx *ctx, uint8_t sub)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len != 3U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    if (ctx->max_resp_len < 6U) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }
    uint16_t match_cnt = 0U;
    for (uint8_t i = 0U; i < server->dtc_count; i++) {
        if ((server->dtc_table[i].status & ctx->req[2]) != 0U) {
            match_cnt++;
        }
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    ctx->resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
    ctx->resp_buf[3] = SYN_UDS_DTC_FORMAT_ISO14229_1;
    syn_poke_u16(match_cnt, ctx->resp_buf, 4);
    *ctx->resp_len = 6U;
    ctx->success = true;
    return true;
}

/**
 * @brief Dispatch ReadDTCInformation status-mask, supported-DTC, and permanent-DTC sub-functions.
 * @param ctx UDS request processing context.
 * @param sub ReadDTCInformation sub-function byte.
 * @return true if sub-function was handled, false otherwise.
 */
static bool uds_dtc_status_queries(UDS_ReqCtx *ctx, uint8_t sub)
{
    if ((sub == SYN_UDS_DTC_REPORT_NUMBER_BY_STATUS_MASK) ||
        (sub == SYN_UDS_DTC_REPORT_NUMBER_MIRROR_MEMORY_BY_STATUS_MASK) ||
        (sub == SYN_UDS_DTC_REPORT_NUMBER_EMISSIONS_OBD_BY_STATUS_MASK)) {
        return uds_dtc_count_by_mask(ctx, sub);
    }
    if ((sub == SYN_UDS_DTC_REPORT_BY_STATUS_MASK) ||
        (sub == SYN_UDS_DTC_REPORT_MIRROR_MEMORY_BY_STATUS_MASK) ||
        (sub == SYN_UDS_DTC_REPORT_EMISSIONS_OBD_BY_STATUS_MASK)) {
        if (ctx->req_len != 3U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        return uds_dtc_list_by_mask(ctx, sub, ctx->req[2], false);
    }
    if ((sub == SYN_UDS_DTC_REPORT_SUPPORTED) ||
        (sub == SYN_UDS_DTC_REPORT_WITH_PERMANENT_STATUS)) {
        if (ctx->req_len != 2U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        return uds_dtc_list_by_mask(ctx, sub, 0U, true);
    }
    if (sub == SYN_UDS_DTC_REPORT_WWH_OBD_WITH_PERMANENT_STATUS) {
        if (ctx->req_len != 4U) {
            return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        }
        return uds_dtc_list_by_mask(ctx, sub, ctx->req[3], false);
    }
    return false;
}

/**
 * @brief Handle ReadDTCInformation (0x19) request across all supported sub-functions.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_read_dtc_info(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if ((sub >= SYN_UDS_DTC_REPORT_FIRST_TEST_FAILED) &&
        (sub <= SYN_UDS_DTC_REPORT_MOST_RECENT_CONFIRMED)) {
        return uds_dtc_first_or_recent(ctx, sub);
    }
    if (server->dtc_cb != NULL) {
        uint16_t cb_out_len = 0U;
        if (!server->dtc_cb(sub, &ctx->req[2], ctx->req_len - 2U, &ctx->resp_buf[2],
                            ctx->max_resp_len - 2U, &cb_out_len, server->dtc_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        *ctx->resp_len = 2U + cb_out_len;
        ctx->success = true;
        return true;
    }

    if (uds_dtc_status_queries(ctx, sub) || uds_dtc_severity_and_fdc(ctx, sub) ||
        uds_dtc_info_by_dtc(ctx, sub) || uds_dtc_records(ctx, sub) ||
        uds_dtc_misc_records(ctx, sub)) {
        return true;
    }
    return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
}

/**
 * @brief Handle ControlDTCSetting (0x85) on/off request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_control_dtc_setting(UDS_ReqCtx *ctx)
{
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if ((sub != 0x01U) && (sub != 0x02U)) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    ctx->server->dtc_setting_enabled = (sub == 0x01U);
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    *ctx->resp_len = 2U;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle ResponseOnEvent (0x86) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_roe(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if (sub > 0x07U) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    if (server->roe_cb != NULL) {
        uint16_t out_payload_len = 0U;
        if (!server->roe_cb(sub, &ctx->req[2], ctx->req_len - 2U, &ctx->resp_buf[2],
                            ctx->max_resp_len - 2U, &out_payload_len, server->roe_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        *ctx->resp_len = 2U + out_payload_len;
    } else {
        if (ctx->max_resp_len < 4U) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = sub;
        ctx->resp_buf[2] = 0x00U;
        if (sub == 0x04U) {
            *ctx->resp_len = 3U;
        } else {
            ctx->resp_buf[3] = 0x02U;
            *ctx->resp_len = 4U;
        }
    }
    ctx->success = true;
    return true;
}

/**
 * @brief Decode a big-endian UDS address or size parameter up to 32-bit range.
 * @param buf Source byte buffer.
 * @param len Parameter width in bytes (0..15 from ALFID).
 * @param out_val Output decoded 32-bit unsigned value.
 * @return true if value fits in uint32_t, false if non-zero upper bytes overflow 32 bits.
 */
static bool uds_decode_be_var(const uint8_t *buf, uint8_t len, uint32_t *out_val)
{
    uint8_t offset = 0U;
    if (len > 4U) {
        offset = (uint8_t)(len - 4U);
        for (uint8_t i = 0U; i < offset; i++) {
            if (buf[i] != 0U) {
                return false;
            }
        }
    }
    uint32_t val = 0U;
    for (uint8_t i = offset; i < len; i++) {
        val = (val << 8U) | buf[i];
    }
    *out_val = val;
    return true;
}

/**
 * @brief Handle RequestDownload (0x34) and RequestUpload (0x35) transfer initialization.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_request_transfer(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 3U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t alfid = ctx->req[2];
    uint8_t addr_len = alfid & 0x0FU;
    uint8_t size_len = (alfid >> 4U) & 0x0FU;
    if (ctx->req_len < (uint16_t)(3U + addr_len + size_len)) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    if (server->security_state != SYN_UDS_SECURITY_UNLOCKED) {
        return uds_nrc(ctx, SYN_UDS_NRC_SECURITY_ACCESS_DENIED);
    }
    if (ctx->max_resp_len < 4U) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }

    uint32_t transfer_addr = 0U;
    uint32_t transfer_size = 0U;
    if (!uds_decode_be_var(&ctx->req[3], addr_len, &transfer_addr) ||
        !uds_decode_be_var(&ctx->req[3U + addr_len], size_len, &transfer_size)) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }

    server->transfer_state = (ctx->sid == SYN_UDS_SID_REQUEST_DOWNLOAD) ? SYN_UDS_TRANSFER_DOWNLOAD
                                                                        : SYN_UDS_TRANSFER_UPLOAD;
    server->transfer_address = transfer_addr;
    server->transfer_size = transfer_size;
    server->transfer_bytes_processed = 0U;
    server->transfer_last_chunk_len = 0U;
    server->expected_block_seq = 1U;

    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = 0x20U;
    syn_poke_u16(0x0400U, ctx->resp_buf, 2);
    *ctx->resp_len = 4U;
    ctx->success = true;
    return true;
}

/**
 * @brief Process one TransferData (0x36) block write during an active download session.
 * @param ctx UDS request processing context.
 * @param block_seq Validated block sequence counter byte.
 * @return true after encoding positive or negative response.
 */
static bool uds_transfer_download(UDS_ReqCtx *ctx, uint8_t block_seq)
{
    SYN_UDS_Server *server = ctx->server;
    uint16_t payload_len = ctx->req_len - 2U;
    if ((server->transfer_size > 0U) &&
        ((server->transfer_bytes_processed >= server->transfer_size) ||
         ((uint32_t)payload_len > (server->transfer_size - server->transfer_bytes_processed)))) {
        return uds_nrc(ctx, SYN_UDS_NRC_TRANSFER_DATA_SUSPENDED);
    }
    if (server->memory_cb != NULL) {
        if (!server->memory_cb(true, server->transfer_address + server->transfer_bytes_processed,
                               payload_len, (uint8_t *)&ctx->req[2], server->memory_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_GENERAL_PROGRAMMING_FAILURE);
        }
    }
    server->transfer_bytes_processed += payload_len;
    server->expected_block_seq++;
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = block_seq;
    *ctx->resp_len = 2U;
    ctx->success = true;
    return true;
}

/**
 * @brief Encode one TransferData (0x36) upload response block from server memory.
 * @param ctx UDS request processing context.
 * @param block_seq Block sequence counter byte for response header.
 * @param offset Byte offset from transfer_address to read.
 * @param chunk_len Payload byte count to read into response buffer.
 * @return true after encoding positive or negative response.
 */
static bool uds_transfer_upload_chunk(UDS_ReqCtx *ctx, uint8_t block_seq, uint32_t offset,
                                      uint16_t chunk_len)
{
    SYN_UDS_Server *server = ctx->server;
    if (((uint32_t)2U + chunk_len) > ctx->max_resp_len) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }
    if (server->memory_cb != NULL) {
        if (!server->memory_cb(false, server->transfer_address + offset, chunk_len,
                               &ctx->resp_buf[2], server->memory_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_GENERAL_PROGRAMMING_FAILURE);
        }
    } else {
        memset(&ctx->resp_buf[2], 0, chunk_len);
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = block_seq;
    *ctx->resp_len = 2U + chunk_len;
    ctx->success = true;
    return true;
}

/**
 * @brief Process one TransferData (0x36) block read during an active upload session.
 * @param ctx UDS request processing context.
 * @param block_seq Validated block sequence counter byte.
 * @return true after encoding positive or negative response.
 */
static bool uds_transfer_upload(UDS_ReqCtx *ctx, uint8_t block_seq)
{
    SYN_UDS_Server *server = ctx->server;
    uint16_t chunk_len = 0x03FEU;
    if (server->transfer_size > 0U) {
        if (server->transfer_bytes_processed >= server->transfer_size) {
            return uds_nrc(ctx, SYN_UDS_NRC_TRANSFER_DATA_SUSPENDED);
        }
        uint32_t remaining = server->transfer_size - server->transfer_bytes_processed;
        if (remaining < chunk_len) {
            chunk_len = (uint16_t)remaining;
        }
    }
    (void)uds_transfer_upload_chunk(ctx, block_seq, server->transfer_bytes_processed, chunk_len);
    if (ctx->success) {
        server->transfer_bytes_processed += chunk_len;
        server->transfer_last_chunk_len = chunk_len;
        server->expected_block_seq++;
    }
    return true;
}

/**
 * @brief Handle TransferData (0x36) block sequence verification and download/upload dispatch.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_transfer_data(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    if (server->transfer_state == SYN_UDS_TRANSFER_IDLE) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR);
    }

    uint8_t block_seq = ctx->req[1];
    if ((server->transfer_bytes_processed > 0U) &&
        (block_seq == (uint8_t)(server->expected_block_seq - 1U))) {
        if (server->transfer_state == SYN_UDS_TRANSFER_UPLOAD) {
            uint32_t prev_offset =
                server->transfer_bytes_processed - (uint32_t)server->transfer_last_chunk_len;
            return uds_transfer_upload_chunk(ctx, block_seq, prev_offset,
                                             server->transfer_last_chunk_len);
        }
        ctx->resp_buf[0] = ctx->sid + 0x40U;
        ctx->resp_buf[1] = block_seq;
        *ctx->resp_len = 2U;
        ctx->success = true;
        return true;
    }

    if (block_seq != server->expected_block_seq) {
        return uds_nrc(ctx, SYN_UDS_NRC_WRONG_BLOCK_SEQUENCE_COUNTER);
    }

    if (server->transfer_state == SYN_UDS_TRANSFER_DOWNLOAD) {
        return uds_transfer_download(ctx, block_seq);
    }
    return uds_transfer_upload(ctx, block_seq);
}

/**
 * @brief Handle RequestTransferExit (0x37) request to terminate an active transfer session.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_transfer_exit(UDS_ReqCtx *ctx)
{
    if (ctx->server->transfer_state == SYN_UDS_TRANSFER_IDLE) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR);
    }
    ctx->server->transfer_state = SYN_UDS_TRANSFER_IDLE;
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    *ctx->resp_len = 1U;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle ReadMemoryByAddress (0x23) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_read_memory(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 3U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    if (server->security_state != SYN_UDS_SECURITY_UNLOCKED) {
        return uds_nrc(ctx, SYN_UDS_NRC_SECURITY_ACCESS_DENIED);
    }
    uint8_t alfid = ctx->req[1];
    uint8_t addr_len = alfid & 0x0FU;
    uint8_t size_len = (alfid >> 4U) & 0x0FU;
    if (ctx->req_len < (uint16_t)(2U + addr_len + size_len)) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint32_t address = 0U;
    uint32_t size = 0U;
    if (!uds_decode_be_var(&ctx->req[2], addr_len, &address) ||
        !uds_decode_be_var(&ctx->req[2U + addr_len], size_len, &size)) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }
    if (size > (uint32_t)(ctx->max_resp_len - 1U)) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }
    if (server->memory_cb != NULL) {
        if (!server->memory_cb(false, address, size, &ctx->resp_buf[1], server->memory_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    } else {
        memset(&ctx->resp_buf[1], 0, size);
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    *ctx->resp_len = 1U + (uint16_t)size;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle WriteMemoryByAddress (0x3D) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_write_memory(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 3U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    if (server->security_state != SYN_UDS_SECURITY_UNLOCKED) {
        return uds_nrc(ctx, SYN_UDS_NRC_SECURITY_ACCESS_DENIED);
    }
    uint8_t alfid = ctx->req[1];
    uint8_t addr_len = alfid & 0x0FU;
    uint8_t size_len = (alfid >> 4U) & 0x0FU;
    uint16_t header_len = (uint16_t)(2U + addr_len + size_len);
    if (ctx->req_len < header_len) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint32_t address = 0U;
    uint32_t size = 0U;
    if (!uds_decode_be_var(&ctx->req[2], addr_len, &address) ||
        !uds_decode_be_var(&ctx->req[2U + addr_len], size_len, &size)) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }
    if (size > (uint32_t)(ctx->req_len - header_len)) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    if (header_len > ctx->max_resp_len) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }
    if (server->memory_cb != NULL) {
        if (!server->memory_cb(true, address, size, (uint8_t *)&ctx->req[header_len],
                               server->memory_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = alfid;
    memcpy(&ctx->resp_buf[2], &ctx->req[2], (size_t)addr_len + (size_t)size_len);
    *ctx->resp_len = header_len;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle ReadScalingDataByIdentifier (0x24) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_read_scaling(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 3U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint16_t target_did = syn_peek_u16(ctx->req, 1);
    int did_idx = find_did_index(server, target_did);
    if ((did_idx < 0) && (server->scaling_cb == NULL)) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }
    if (ctx->max_resp_len < 4U) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    syn_poke_u16(target_did, ctx->resp_buf, 1);
    if (server->scaling_cb != NULL) {
        uint16_t out_payload_len = 0U;
        if (!server->scaling_cb(target_did, &ctx->resp_buf[3], ctx->max_resp_len - 3U,
                                &out_payload_len, server->scaling_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
        *ctx->resp_len = 3U + out_payload_len;
    } else {
        ctx->resp_buf[3] = 0x01U;
        *ctx->resp_len = 4U;
    }
    ctx->success = true;
    return true;
}

/**
 * @brief Handle Authentication (0x29) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_authentication(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if (sub > 0x08U) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    uint16_t out_payload_len = 0U;
    if (server->auth_cb != NULL) {
        if (!server->auth_cb(sub, &ctx->req[2], ctx->req_len - 2U, &ctx->resp_buf[2],
                             ctx->max_resp_len - 2U, &out_payload_len, server->auth_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
        if (out_payload_len > (uint16_t)(ctx->max_resp_len - 2U)) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    *ctx->resp_len = 2U + out_payload_len;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle ReadDataByPeriodicIdentifier (0x2A) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_read_periodic(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 3U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t mode = ctx->req[1];
    if ((mode < 0x01U) || (mode > 0x04U)) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }
    uint16_t out_payload_len = 0U;
    if (server->periodic_cb != NULL) {
        if (!server->periodic_cb(mode, &ctx->req[2], ctx->req_len - 2U, &ctx->resp_buf[2],
                                 ctx->max_resp_len - 2U, &out_payload_len, server->periodic_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = ctx->req[2];
    *ctx->resp_len = 2U + out_payload_len;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle DynamicallyDefineDataIdentifier (0x2C) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_dynamic_did(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 4U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if ((sub < 0x01U) || (sub > 0x03U)) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    if (ctx->max_resp_len < 4U) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }
    uint16_t dyn_did = syn_peek_u16(ctx->req, 2);
    uint16_t out_payload_len = 0U;
    if (server->dynamic_did_cb != NULL) {
        if (!server->dynamic_did_cb(sub, dyn_did, &ctx->req[4], ctx->req_len - 4U,
                                    &ctx->resp_buf[4], ctx->max_resp_len - 4U, &out_payload_len,
                                    server->dynamic_did_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    syn_poke_u16(dyn_did, ctx->resp_buf, 2);
    *ctx->resp_len = 4U + out_payload_len;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle InputOutputControlByIdentifier (0x2F) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_io_control(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 4U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint16_t target_did = syn_peek_u16(ctx->req, 1);
    uint8_t control_opt = ctx->req[3];
    if (control_opt > 0x03U) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }
    int did_idx = find_did_index(server, target_did);
    if ((did_idx < 0) && (server->io_control_cb == NULL)) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }
    if (ctx->max_resp_len < 4U) {
        return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
    }
    uint16_t out_payload_len = 0U;
    if (server->io_control_cb != NULL) {
        if (!server->io_control_cb(target_did, control_opt, &ctx->req[4], ctx->req_len - 4U,
                                   &ctx->resp_buf[4], ctx->max_resp_len - 4U, &out_payload_len,
                                   server->io_control_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    syn_poke_u16(target_did, ctx->resp_buf, 1);
    ctx->resp_buf[3] = control_opt;
    *ctx->resp_len = 4U + out_payload_len;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle RequestFileTransfer (0x38) request.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_file_transfer(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 3U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t mode = ctx->req[1];
    if ((mode < 0x01U) || (mode > 0x05U)) {
        return uds_nrc(ctx, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE);
    }
    uint16_t out_len = 0U;
    if (server->file_transfer_cb != NULL) {
        uint16_t path_len = ctx->req_len - 2U;
        if (!server->file_transfer_cb(mode, (const char *)&ctx->req[2], path_len, &ctx->resp_buf[2],
                                      ctx->max_resp_len - 2U, &out_len,
                                      server->file_transfer_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
        if (out_len > (uint16_t)(ctx->max_resp_len - 2U)) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
    } else {
        if (ctx->max_resp_len < 5U) {
            return uds_nrc(ctx, SYN_UDS_NRC_RESPONSE_TOO_LONG);
        }
        ctx->resp_buf[2] = 0x20U;
        syn_poke_u16(0x0400U, ctx->resp_buf, 3);
        out_len = 3U;
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = mode;
    *ctx->resp_len = 2U + out_len;
    ctx->success = true;
    return true;
}

/**
 * @brief Handle LinkControl (0x87) baudrate verification and transition sub-functions.
 * @param ctx UDS request processing context.
 * @return true after encoding positive or negative response.
 */
static bool uds_sid_link_control(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    if (ctx->req_len < 2U) {
        return uds_nrc(ctx, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH);
    }
    uint8_t sub = ctx->req[1] & 0x7FU;
    if ((sub < 0x01U) || (sub > 0x03U)) {
        return uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    }
    uint16_t out_payload_len = 0U;
    if (server->link_control_cb != NULL) {
        if (!server->link_control_cb(sub, &ctx->req[2], ctx->req_len - 2U, &ctx->resp_buf[2],
                                     ctx->max_resp_len - 2U, &out_payload_len,
                                     server->link_control_ctx)) {
            return uds_nrc(ctx, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT);
        }
    }
    ctx->resp_buf[0] = ctx->sid + 0x40U;
    ctx->resp_buf[1] = sub;
    *ctx->resp_len = 2U + out_payload_len;
    ctx->success = true;
    return true;
}

/** @brief Static UDS service identifier to handler dispatch table. */
static const UDS_ServiceEntry g_uds_services[] = {
    {SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL, uds_sid_session_control},
    {SYN_UDS_SID_ECU_RESET, uds_sid_ecu_reset},
    {SYN_UDS_SID_SECURITY_ACCESS, uds_sid_security_access},
    {SYN_UDS_SID_COMMUNICATION_CONTROL, uds_sid_comm_control},
    {SYN_UDS_SID_READ_DATA_BY_IDENTIFIER, uds_sid_read_did},
    {SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER, uds_sid_write_did},
    {SYN_UDS_SID_ROUTINE_CONTROL, uds_sid_routine_control},
    {SYN_UDS_SID_TESTER_PRESENT, uds_sid_tester_present},
    {SYN_UDS_SID_ACCESS_TIMING_PARAMETER, uds_sid_access_timing},
    {SYN_UDS_SID_SECURED_DATA_TRANSMISSION, uds_sid_secured_data},
    {SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, uds_sid_clear_dtc},
    {SYN_UDS_SID_READ_DTC_INFORMATION, uds_sid_read_dtc_info},
    {SYN_UDS_SID_CONTROL_DTC_SETTING, uds_sid_control_dtc_setting},
    {SYN_UDS_SID_RESPONSE_ON_EVENT, uds_sid_roe},
    {SYN_UDS_SID_REQUEST_DOWNLOAD, uds_sid_request_transfer},
    {SYN_UDS_SID_REQUEST_UPLOAD, uds_sid_request_transfer},
    {SYN_UDS_SID_TRANSFER_DATA, uds_sid_transfer_data},
    {SYN_UDS_SID_REQUEST_TRANSFER_EXIT, uds_sid_transfer_exit},
    {SYN_UDS_SID_READ_MEMORY_BY_ADDRESS, uds_sid_read_memory},
    {SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS, uds_sid_write_memory},
    {SYN_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER, uds_sid_read_scaling},
    {SYN_UDS_SID_AUTHENTICATION, uds_sid_authentication},
    {SYN_UDS_SID_READ_DATA_BY_PERIODIC_IDENTIFIER, uds_sid_read_periodic},
    {SYN_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER, uds_sid_dynamic_did},
    {SYN_UDS_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER, uds_sid_io_control},
    {SYN_UDS_SID_REQUEST_FILE_TRANSFER, uds_sid_file_transfer},
    {SYN_UDS_SID_LINK_CONTROL, uds_sid_link_control},
};

/**
 * @brief Look up the UDS service handler function registered for a Service Identifier.
 * @param sid UDS Service Identifier byte.
 * @return Matching UDS_ServiceFn handler, or NULL if unsupported.
 */
static UDS_ServiceFn uds_find_service(uint8_t sid)
{
    size_t count = sizeof(g_uds_services) / sizeof(g_uds_services[0]);
    for (size_t i = 0U; i < count; i++) {
        if (g_uds_services[i].sid == sid) {
            return g_uds_services[i].fn;
        }
    }
    return NULL;
}

/**
 * @brief Validate functional addressing support, session mask, and security mask for a request.
 * @param ctx UDS request processing context.
 * @return true if preconditions pass, false if request was suppressed or rejected with an NRC.
 */
static bool uds_check_preconditions(UDS_ReqCtx *ctx)
{
    if ((ctx->addr_mode == SYN_UDS_ADDR_FUNCTIONAL) &&
        !syn_uds_is_sid_functional_supported(ctx->sid)) {
        *ctx->resp_len = 0U;
        return false;
    }

    uint8_t allowed_mask = get_sid_allowed_session_mask(ctx->server, ctx->sid);
    uint8_t current_mask = session_to_mask(ctx->server->session);
    if ((ctx->sid != SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL) &&
        ((allowed_mask & current_mask) == 0U)) {
        (void)uds_nrc(ctx, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION);
        return false;
    }

    uint16_t req_sec_mask = get_sid_required_security_mask(ctx->server, ctx->sid);
    if ((req_sec_mask != 0U) && ((get_active_security_mask(ctx->server) & req_sec_mask) == 0U)) {
        (void)uds_nrc(ctx, SYN_UDS_NRC_SECURITY_ACCESS_DENIED);
        return false;
    }

    return true;
}

/**
 * @brief Reload non-default S3 session timer and apply positive-response suppression bit.
 * @param ctx UDS request processing context.
 */
static void uds_finalize_response(UDS_ReqCtx *ctx)
{
    SYN_UDS_Server *server = ctx->server;
    bool valid_nrc = (*ctx->resp_len == 3U) && (ctx->resp_buf[0] == SYN_UDS_RESPONSE_NEGATIVE);
    if ((server->session != SYN_UDS_SESSION_DEFAULT) && (ctx->success || valid_nrc)) {
        server->s3_timer_ms = SYN_UDS_S3_TIMEOUT_MS;
    }
    if (ctx->success && (ctx->req_len >= 2U) && uds_sid_has_subfunction(ctx->sid) &&
        ((ctx->req[1] & 0x80U) != 0U)) {
        *ctx->resp_len = 0U;
    }
}

bool syn_uds_process_request(SYN_UDS_Server *server, const uint8_t *req, uint16_t req_len,
                             uint8_t *resp_buf, uint16_t max_resp_len, uint16_t *resp_len,
                             SYN_UDS_AddrMode addr_mode)
{
    if ((server == NULL) || (req == NULL) || (req_len == 0U) || (resp_buf == NULL) ||
        (max_resp_len < 3U) || (resp_len == NULL)) {
        return false;
    }

    *resp_len = 0U;
    UDS_ReqCtx ctx = {
        .server = server,
        .sid = req[0],
        .req = req,
        .req_len = req_len,
        .resp_buf = resp_buf,
        .max_resp_len = max_resp_len,
        .resp_len = resp_len,
        .addr_mode = addr_mode,
        .success = false,
    };

    if (uds_check_preconditions(&ctx)) {
        UDS_ServiceFn fn = uds_find_service(ctx.sid);
        if (fn != NULL) {
            (void)fn(&ctx);
        } else {
            (void)uds_nrc(&ctx, SYN_UDS_NRC_SERVICE_NOT_SUPPORTED);
        }
    }

    uds_finalize_response(&ctx);
    return true;
}

bool syn_uds_dtc_report_test_result(SYN_UDS_Server *server, uint32_t dtc, bool failed)
{
    if ((server == NULL) || (!server->dtc_setting_enabled)) {
        return false;
    }

    int idx = find_dtc_index(server, dtc);
    if (idx < 0) {
        return false;
    }

    uint8_t status = server->dtc_table[idx].status;
    if (failed) {
        status |= (uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED;
        status |= (uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE;
        status |= (uint8_t)SYN_UDS_DTC_STATUS_PENDING_DTC;
        status |= (uint8_t)SYN_UDS_DTC_STATUS_CONFIRMED_DTC;
        status |= (uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED_SINCE_LAST_CLEAR;
        status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
        status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;
        server->dtc_table[idx].fault_cnt = 127;
    } else {
        status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED;
        status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
        status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;
        server->dtc_table[idx].fault_cnt = -128;
    }

    server->dtc_table[idx].status = status;
    return true;
}

bool syn_uds_dtc_start_operation_cycle(SYN_UDS_Server *server)
{
    if (server == NULL) {
        return false;
    }

    for (uint8_t i = 0U; i < server->dtc_count; i++) {
        uint8_t status = server->dtc_table[i].status;

        if (((status & (uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE) == 0U) &&
            ((status & (uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE) == 0U)) {
            status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_PENDING_DTC;
        }

        status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE;
        status |= (uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;
        server->dtc_table[i].status = status;
    }

    return true;
}

bool syn_uds_dtc_get_status(SYN_UDS_Server *server, uint32_t dtc, uint8_t *out_status)
{
    if ((server == NULL) || (out_status == NULL)) {
        return false;
    }

    int idx = find_dtc_index(server, dtc);
    if (idx < 0) {
        return false;
    }

    *out_status = server->dtc_table[idx].status;
    return true;
}

uint8_t syn_uds_get_security_level(const SYN_UDS_Server *server)
{
    if ((server == NULL) || (server->security_state != SYN_UDS_SECURITY_UNLOCKED)) {
        return 0U;
    }
    return server->security_level;
}
