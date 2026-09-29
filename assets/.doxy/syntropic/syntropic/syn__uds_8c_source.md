

# File syn\_uds.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_uds.c**](syn__uds_8c.md)

[Go to the documentation of this file](syn__uds_8c.md)


```C++

#include "syn_uds.h"

#include "../crypto/syn_aes.h"
#include "../util/syn_pack.h"

#include <string.h>

bool syn_uds_init(SYN_UDS_Server *server)
{
    if (server == NULL) {
        return false;
    }

    memset(server, 0, sizeof(*server));
    server->session = SYN_UDS_SESSION_DEFAULT;
    server->security_state = SYN_UDS_SECURITY_LOCKED;
    server->current_seed = 0x12345678U;
    server->active_seed = 0x12345678U;
    server->use_aes128_security = false;
    memset(server->aes_security_key, 0, sizeof(server->aes_security_key));
    static const uint8_t default_seed_16[16] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF,
                                                0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    memcpy(server->current_seed_bytes, default_seed_16, 16);
    memcpy(server->active_seed_bytes, default_seed_16, 16);
    server->s3_timer_ms = 0U;
    server->security_error_count = 0U;
    server->security_delay_timer_ms = 0U;
    server->comm_control_state = SYN_UDS_COMM_ENABLE_RX_AND_TX;
    server->p2_max_ms = 50U;
    server->p2_star_max_10ms = 500U;
    server->active_p2_max_ms = 50U;
    server->active_p2_star_max_10ms = 500U;
    server->timing_cb = NULL;
    server->timing_ctx = NULL;
    server->memory_cb = NULL;
    server->memory_ctx = NULL;
    server->auth_cb = NULL;
    server->auth_ctx = NULL;
    server->file_transfer_cb = NULL;
    server->file_transfer_ctx = NULL;
    server->dtc_cb = NULL;
    server->dtc_ctx = NULL;
    server->did_count = 0U;
    server->dtc_count = 0U;
    server->transfer_state = SYN_UDS_TRANSFER_IDLE;
    server->transfer_address = 0U;
    server->transfer_size = 0U;
    server->transfer_bytes_processed = 0U;
    server->expected_block_seq = 1U;
    server->reset_type_requested = 0U;
    server->reset_cb = NULL;
    server->reset_ctx = NULL;
    server->reset_tx_wait_ms = SYN_UDS_DEFAULT_RESET_TX_WAIT_MS;
    server->reset_wait_elapsed_ms = 0U;
    server->session_transition_cb = NULL;
    server->session_transition_ctx = NULL;
    server->custom_session_count = 0U;
    server->custom_security_count = 0U;

    return true;
}

static int find_did_index(const SYN_UDS_Server *server, uint16_t did)
{
    /* LCOV_EXCL_START: Defensive null check */
    if (server == NULL) {
        return -1;
    }
    /* LCOV_EXCL_STOP */
    for (uint8_t i = 0U; i < server->did_count; i++) {
        if (server->did_table[i].did == did) {
            return (int)i;
        }
    }
    return -1;
}

bool syn_uds_enable_aes128_security(SYN_UDS_Server *server, const uint8_t key[16])
{
    if (server == NULL || key == NULL) {
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
    if (server == NULL || seed == NULL) {
        return false;
    }
    memcpy(server->current_seed_bytes, seed, 16);
    return true;
}

void syn_uds_tick(SYN_UDS_Server *server, uint32_t dt_ms)
{
    if (server == NULL) {
        return;
    }

    /* 1. Security delay countdown */
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

    /* 2. S3 server session timeout countdown */
    if (server->session != SYN_UDS_SESSION_DEFAULT) {
        if (dt_ms >= server->s3_timer_ms) {
            /* S3 timer expired -> revert to DEFAULT session, lock security, and restore comm */
            server->session = SYN_UDS_SESSION_DEFAULT;
            server->security_state = SYN_UDS_SECURITY_LOCKED;
            server->comm_control_state = SYN_UDS_COMM_ENABLE_RX_AND_TX;
            server->transfer_state = SYN_UDS_TRANSFER_IDLE;
            server->s3_timer_ms = 0U;
        } else {
            server->s3_timer_ms -= dt_ms;
        }
    } else {
        server->s3_timer_ms = 0U;
    }

    /* 3. Deferred ECU reset countdown */
    if (server->reset_type_requested > 0U) {
        server->reset_wait_elapsed_ms += dt_ms;
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
    if (server == NULL || server->dtc_count >= SYN_UDS_MAX_DTCS) {
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

static bool make_negative_response(uint8_t sid, uint8_t nrc, uint8_t *resp_buf, uint16_t *resp_len,
                                   SYN_UDS_AddrMode addr_mode)
{
    if (addr_mode == SYN_UDS_ADDR_FUNCTIONAL &&
        (nrc == SYN_UDS_NRC_SERVICE_NOT_SUPPORTED || nrc == SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED ||
         nrc == SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION)) {
        *resp_len = 0U;
        return true;
    }
    resp_buf[0] = SYN_UDS_RESPONSE_NEGATIVE;
    resp_buf[1] = sid;
    resp_buf[2] = nrc;
    *resp_len = 3U;
    return true;
}
bool syn_uds_is_sid_functional_supported(uint8_t sid)
{
    switch (sid) {
    case SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL:   /* 0x10 */
    case SYN_UDS_SID_ECU_RESET:                    /* 0x11 */
    case SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION: /* 0x14 */
    case SYN_UDS_SID_READ_DTC_INFORMATION:         /* 0x19 */
    case SYN_UDS_SID_READ_DATA_BY_IDENTIFIER:      /* 0x22 */
    case SYN_UDS_SID_COMMUNICATION_CONTROL:        /* 0x28 */
    case SYN_UDS_SID_AUTHENTICATION:               /* 0x29 */
    case SYN_UDS_SID_ROUTINE_CONTROL:              /* 0x31 */
    case SYN_UDS_SID_TESTER_PRESENT:               /* 0x3E */
    case SYN_UDS_SID_ACCESS_TIMING_PARAMETER:      /* 0x83 */
    case SYN_UDS_SID_CONTROL_DTC_SETTING:          /* 0x85 */
    case SYN_UDS_SID_LINK_CONTROL:                 /* 0x87 */
        return true;
    default:
        return false;
    }
}

static bool get_sid_functional_support(uint8_t sid)
{
    return syn_uds_is_sid_functional_supported(sid);
}

static uint8_t session_to_mask(SYN_UDS_Session session)
{
    switch (session) {
    case SYN_UDS_SESSION_DEFAULT:
        return SYN_UDS_SESSION_MASK_DEFAULT;
    case SYN_UDS_SESSION_PROGRAMMING:
        return SYN_UDS_SESSION_MASK_PROGRAMMING;
    case SYN_UDS_SESSION_EXTENDED:
        return SYN_UDS_SESSION_MASK_EXTENDED;
    case SYN_UDS_SESSION_SAFETY_SYSTEM:
        return SYN_UDS_SESSION_MASK_SAFETY;
    /* LCOV_EXCL_START: Unreachable session enum guard */
    default:
        return 0U;
        /* LCOV_EXCL_STOP */
    }
}

static uint8_t get_sid_allowed_session_mask(const SYN_UDS_Server *server, uint8_t sid)
{
    if (server != NULL) {
        for (uint8_t i = 0U; i < server->custom_session_count; i++) {
            if (server->custom_session_sids[i] == sid) {
                return server->custom_session_masks[i];
            }
        }
    }
    return SYN_UDS_SESSION_MASK_ALL;
}

static uint16_t get_sid_required_security_mask(const SYN_UDS_Server *server, uint8_t sid)
{
    if (server != NULL) {
        for (uint8_t i = 0U; i < server->custom_security_count; i++) {
            if (server->custom_security_sids[i] == sid) {
                return server->custom_security_masks[i];
            }
        }
    }
    return 0U;
}

static uint16_t get_active_security_mask(const SYN_UDS_Server *server)
{
    uint16_t mask = (1U << 0);
    if (server != NULL && server->security_state == SYN_UDS_SECURITY_UNLOCKED) {
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
            /* LCOV_EXCL_START: Update existing service session mask override */
            server->custom_session_masks[i] = session_mask;
            return true;
            /* LCOV_EXCL_STOP */
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
            /* LCOV_EXCL_START: Update existing service security mask override */
            server->custom_security_masks[i] = security_mask;
            return true;
            /* LCOV_EXCL_STOP */
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

bool syn_uds_process_request(SYN_UDS_Server *server, const uint8_t *req, uint16_t req_len,
                             uint8_t *resp_buf, uint16_t max_resp_len, uint16_t *resp_len,
                             SYN_UDS_AddrMode addr_mode)
{
    if ((server == NULL) || (req == NULL) || (req_len == 0U) || (resp_buf == NULL) ||
        (max_resp_len < 3U) || (resp_len == NULL)) {
        return false;
    }

    uint8_t sid = req[0];
    bool success = false;

    if (addr_mode == SYN_UDS_ADDR_FUNCTIONAL && !get_sid_functional_support(sid)) {
        *resp_len = 0U;
        return true;
    }

    uint8_t allowed_mask = get_sid_allowed_session_mask(server, sid);
    uint8_t current_mask = session_to_mask(server->session);
    if ((sid != SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL) && ((allowed_mask & current_mask) == 0U)) {
        return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION,
                                      resp_buf, resp_len, addr_mode);
    }

    uint16_t req_sec_mask = get_sid_required_security_mask(server, sid);
    if (req_sec_mask != 0U) {
        if ((get_active_security_mask(server) & req_sec_mask) == 0U) {
            return make_negative_response(sid, SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp_buf,
                                          resp_len, addr_mode);
        }
    }

    switch (sid) {
    case SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if ((sub != SYN_UDS_SESSION_DEFAULT) && (sub != SYN_UDS_SESSION_PROGRAMMING) &&
            (sub != SYN_UDS_SESSION_EXTENDED) && (sub != SYN_UDS_SESSION_SAFETY_SYSTEM)) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }

        if (server->session_transition_cb != NULL) {
            if (!server->session_transition_cb(server->session, (SYN_UDS_Session)sub,
                                               server->session_transition_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
        }

        server->session = (SYN_UDS_Session)sub;
        if (server->session == SYN_UDS_SESSION_DEFAULT) {
            server->security_state = SYN_UDS_SECURITY_LOCKED;
            server->comm_control_state = SYN_UDS_COMM_ENABLE_RX_AND_TX;
            server->transfer_state = SYN_UDS_TRANSFER_IDLE;
            server->s3_timer_ms = 0U;
        } else {
            server->s3_timer_ms = SYN_UDS_S3_TIMEOUT_MS;
            if (server->session == SYN_UDS_SESSION_PROGRAMMING) {
                /* ISO 14229: Switching to programming session clears power-on safety delay */
                server->security_delay_timer_ms = 0U;
            }
        }

        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = sub;
        syn_poke_u16(server->active_p2_max_ms, resp_buf, 2);
        syn_poke_u16(server->active_p2_star_max_10ms, resp_buf, 4);
        *resp_len = 6U;
        success = true;
        break;
    }

    case SYN_UDS_SID_ECU_RESET: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if ((sub < SYN_UDS_RESET_HARD) || (sub > SYN_UDS_RESET_DISABLE_RAPID_POWER_SHUTDOWN)) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        server->reset_type_requested = sub;
        server->reset_wait_elapsed_ms = 0U;
        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = sub;
        *resp_len = 2U;
        success = true;
        break;
    }

    case SYN_UDS_SID_SECURITY_ACCESS: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if (sub == 0x00U || sub == 0x7FU) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        if ((sub & 1U) != 0U) { /* Odd subfunction = Request Seed (0x01, 0x03, 0x05...) */
            if (server->security_delay_timer_ms > 0U) {
                return make_negative_response(sid, SYN_UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED,
                                              resp_buf, resp_len, addr_mode);
            }
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            server->security_state = SYN_UDS_SECURITY_SEED_SENT;
            server->active_seed_subfunction = sub;
            if (server->use_aes128_security) {
                memcpy(server->active_seed_bytes, server->current_seed_bytes, 16);
                memcpy(&resp_buf[2], server->current_seed_bytes, 16);
                *resp_len = 18U;
            } else {
                server->active_seed = server->current_seed;
                syn_poke_u32(server->current_seed, resp_buf, 2);
                *resp_len = 6U;
            }
            success = true;
        } else if ((sub & 1U) == 0U &&
                   sub != 0x00U) { /* Even subfunction = Send Key (0x02, 0x04...) */
            if (server->security_state != SYN_UDS_SECURITY_SEED_SENT) {
                return make_negative_response(sid, SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR, resp_buf,
                                              resp_len, addr_mode);
            }
            if (sub != (server->active_seed_subfunction + 1U)) {
                return make_negative_response(sid, SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR, resp_buf,
                                              resp_len, addr_mode);
            }
            if (server->use_aes128_security) {
                if (req_len < 18U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                SYN_AES_Context aes_ctx;
                (void)syn_aes_init(&aes_ctx, server->aes_security_key, 16U);
                uint8_t expected_key[16];
                syn_aes_encrypt_block(&aes_ctx, server->active_seed_bytes, expected_key);

                if (memcmp(&req[2], expected_key, 16) == 0) {
                    server->security_state = SYN_UDS_SECURITY_UNLOCKED;
                    server->security_level = sub / 2U;
                    server->security_error_count = 0U;
                    resp_buf[0] = sid + 0x40U;
                    resp_buf[1] = sub;
                    *resp_len = 2U;
                    success = true;
                } else {
                    server->security_state = SYN_UDS_SECURITY_LOCKED;
                    server->security_level = 0U;
                    server->security_error_count++;
                    if (server->security_error_count >= SYN_UDS_SECURITY_MAX_ATTEMPTS) {
                        server->security_delay_timer_ms = SYN_UDS_SECURITY_DELAY_MS;
                        return make_negative_response(sid, SYN_UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS,
                                                      resp_buf, resp_len, addr_mode);
                    }
                    return make_negative_response(sid, SYN_UDS_NRC_INVALID_KEY, resp_buf, resp_len,
                                                  addr_mode);
                }
            } else {
                if (req_len < 6U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                uint32_t key = syn_peek_u32(req, 2);
                /* Key calculation algorithm: key = seed ^ 0xA5A5A5A5 */
                uint32_t expected_key = server->active_seed ^ 0xA5A5A5A5U;
                if (key == expected_key) {
                    server->security_state = SYN_UDS_SECURITY_UNLOCKED;
                    server->security_level = sub / 2U;
                    server->security_error_count = 0U;
                    resp_buf[0] = sid + 0x40U;
                    resp_buf[1] = sub;
                    *resp_len = 2U;
                    success = true;
                } else {
                    server->security_state = SYN_UDS_SECURITY_LOCKED;
                    server->security_level = 0U;
                    server->security_error_count++;
                    if (server->security_error_count >= SYN_UDS_SECURITY_MAX_ATTEMPTS) {
                        server->security_delay_timer_ms = SYN_UDS_SECURITY_DELAY_MS;
                        return make_negative_response(sid, SYN_UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS,
                                                      resp_buf, resp_len, addr_mode);
                    }
                    return make_negative_response(sid, SYN_UDS_NRC_INVALID_KEY, resp_buf, resp_len,
                                                  addr_mode);
                }
            }
        } else {
            /* LCOV_EXCL_START: UDS negative response NRC return */
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
            /* LCOV_EXCL_STOP */
        }
        break;
    }

    case SYN_UDS_SID_COMMUNICATION_CONTROL: {
        if (req_len < 3U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if (sub > 0x05U) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t comm_type = req[2];
        if (comm_type == 0U) {
            return make_negative_response(sid, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len,
                                          addr_mode);
        }

        if (server->comm_control_cb != NULL) {
            if (!server->comm_control_cb((SYN_UDS_CommControlType)sub, comm_type,
                                         server->comm_control_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
        }

        server->comm_control_state = (SYN_UDS_CommControlType)sub;
        server->comm_type = comm_type;

        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = sub;
        *resp_len = 2U;
        success = true;
        break;
    }

    case SYN_UDS_SID_READ_DATA_BY_IDENTIFIER: {
        if (req_len < 3U || ((req_len - 1U) % 2U) != 0U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }

        resp_buf[0] = sid + 0x40U;
        size_t out_pos = 1U;

        for (size_t req_pos = 1U; req_pos < req_len; req_pos += 2U) {
            uint16_t target_did = syn_peek_u16(req, req_pos);
            SYN_UDS_DIDEntry *matched_entry = NULL;
            for (uint8_t i = 0U; i < server->did_count; i++) {
                if (server->did_table[i].did == target_did) {
                    matched_entry = &server->did_table[i];
                    break;
                }
            }
            if (matched_entry == NULL) {
                return make_negative_response(sid, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp_buf,
                                              resp_len, addr_mode);
            }
            if ((matched_entry->session_mask & session_to_mask(server->session)) == 0U) {
                return make_negative_response(
                    sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION, resp_buf,
                    resp_len, addr_mode);
            }
            if ((get_active_security_mask(server) & matched_entry->security_mask) == 0U) {
                return make_negative_response(sid, SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp_buf,
                                              resp_len, addr_mode);
            }
            if (out_pos + 2U + matched_entry->len > max_resp_len) {
                return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                              resp_len, addr_mode);
            }

            syn_poke_u16(target_did, resp_buf, out_pos);
            out_pos += 2U;
            memcpy(&resp_buf[out_pos], matched_entry->data, matched_entry->len);
            out_pos += matched_entry->len;
        }

        *resp_len = out_pos;
        success = true;
        break;
    }

    case SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER: {
        if (req_len < 4U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint16_t target_did = syn_peek_u16(req, 1);
        SYN_UDS_DIDEntry *matched_entry = NULL;
        for (uint8_t i = 0U; i < server->did_count; i++) {
            if (server->did_table[i].did == target_did) {
                matched_entry = &server->did_table[i];
                break;
            }
        }
        if (matched_entry == NULL) {
            return make_negative_response(sid, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len,
                                          addr_mode);
        }
        if ((matched_entry->session_mask & session_to_mask(server->session)) == 0U) {
            return make_negative_response(sid,
                                          SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION,
                                          resp_buf, resp_len, addr_mode);
        }
        if (!matched_entry->writable) {
            return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                          resp_len, addr_mode);
        }
        if ((server->security_state != SYN_UDS_SECURITY_UNLOCKED &&
             matched_entry->security_mask == SYN_UDS_SECURITY_MASK_ALL) ||
            ((get_active_security_mask(server) & matched_entry->security_mask) == 0U)) {
            return make_negative_response(sid, SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp_buf,
                                          resp_len, addr_mode);
        }
        uint16_t data_len = req_len - 3U;
        if (data_len > matched_entry->len) {
            data_len = matched_entry->len;
        }
        memcpy(matched_entry->data, &req[3], data_len);
        resp_buf[0] = sid + 0x40U;
        syn_poke_u16(target_did, resp_buf, 1);
        *resp_len = 3U;
        success = true;
        break;
    }

    case SYN_UDS_SID_ROUTINE_CONTROL: {
        if (req_len < 4U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if (sub < 0x01U || sub > 0x03U) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        uint16_t routine_id = syn_peek_u16(req, 2);
        if (server->routine_cb != NULL) {
            uint16_t out_payload_len = 0U;
            if (!server->routine_cb(sub, routine_id, &req[4], req_len - 4U, &resp_buf[4],
                                    max_resp_len - 4U, &out_payload_len, server->routine_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            syn_poke_u16(routine_id, resp_buf, 2);
            *resp_len = 4U + out_payload_len;
        } else {
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            syn_poke_u16(routine_id, resp_buf, 2);
            *resp_len = 4U;
        }
        success = true;
        break;
    }

    case SYN_UDS_SID_TESTER_PRESENT: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if (sub != 0x00U) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = sub;
        *resp_len = 2U;
        success = true;
        break;
    }

    case SYN_UDS_SID_ACCESS_TIMING_PARAMETER: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        switch (sub) {
        case SYN_UDS_TIMING_READ_EXTENDED: {
            if (req_len != 2U) {
                return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                              resp_len, addr_mode);
            }
            if (max_resp_len < 6U) {
                return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                              resp_len, addr_mode);
            }
            uint16_t p2 = server->p2_max_ms;
            uint16_t p2_star = server->p2_star_max_10ms;
            if (server->timing_cb != NULL) {
                if (!server->timing_cb(SYN_UDS_TIMING_READ_EXTENDED, &p2, &p2_star,
                                       server->timing_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
            }
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            syn_poke_u16(p2, resp_buf, 2);
            syn_poke_u16(p2_star, resp_buf, 4);
            *resp_len = 6U;
            success = true;
            break;
        }

        case SYN_UDS_TIMING_SET_TO_DEFAULT: {
            if (req_len != 2U) {
                return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                              resp_len, addr_mode);
            }
            uint16_t p2 = server->p2_max_ms;
            uint16_t p2_star = server->p2_star_max_10ms;
            if (server->timing_cb != NULL) {
                if (!server->timing_cb(SYN_UDS_TIMING_SET_TO_DEFAULT, &p2, &p2_star,
                                       server->timing_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
            }
            server->active_p2_max_ms = p2;
            server->active_p2_star_max_10ms = p2_star;
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            *resp_len = 2U;
            success = true;
            break;
        }

        case SYN_UDS_TIMING_READ_ACTIVE: {
            if (req_len != 2U) {
                return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                              resp_len, addr_mode);
            }
            if (max_resp_len < 6U) {
                return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                              resp_len, addr_mode);
            }
            uint16_t p2 = server->active_p2_max_ms;
            uint16_t p2_star = server->active_p2_star_max_10ms;
            if (server->timing_cb != NULL) {
                if (!server->timing_cb(SYN_UDS_TIMING_READ_ACTIVE, &p2, &p2_star,
                                       server->timing_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
            }
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            syn_poke_u16(p2, resp_buf, 2);
            syn_poke_u16(p2_star, resp_buf, 4);
            *resp_len = 6U;
            success = true;
            break;
        }

        case SYN_UDS_TIMING_SET_TO_GIVEN: {
            if (req_len != 6U) {
                return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                              resp_len, addr_mode);
            }
            uint16_t given_p2 = syn_peek_u16(req, 2);
            uint16_t given_p2_star = syn_peek_u16(req, 4);
            if (server->timing_cb != NULL) {
                if (!server->timing_cb(SYN_UDS_TIMING_SET_TO_GIVEN, &given_p2, &given_p2_star,
                                       server->timing_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp_buf,
                                                  resp_len, addr_mode);
                }
            }
            server->active_p2_max_ms = given_p2;
            server->active_p2_star_max_10ms = given_p2_star;
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            *resp_len = 2U;
            success = true;
            break;
        }

        default: {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        }
        break;
    }

    case SYN_UDS_SID_SECURED_DATA_TRANSMISSION: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        if (server->security_state != SYN_UDS_SECURITY_UNLOCKED) {
            return make_negative_response(sid, SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp_buf,
                                          resp_len, addr_mode);
        }
        uint16_t sec_in_len = req_len - 1U;
        const uint8_t *sec_in_data = &req[1];
        uint16_t sec_out_len = 0U;

        if (server->secured_data_cb != NULL) {
            if (!server->secured_data_cb(sec_in_data, sec_in_len, &resp_buf[1], max_resp_len - 1U,
                                         &sec_out_len, server->secured_data_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
        } else {
            if (sec_in_len > (max_resp_len - 1U)) {
                /* LCOV_EXCL_START: UDS negative response NRC return */
                return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                              resp_len, addr_mode);
                /* LCOV_EXCL_STOP */
            }
            memcpy(&resp_buf[1], sec_in_data, sec_in_len);
            sec_out_len = sec_in_len;
        }

        resp_buf[0] = sid + 0x40U;
        *resp_len = 1U + sec_out_len;
        success = true;
        break;
    }

    case SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION: {
        if (req_len != 4U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint32_t group_of_dtc =
            ((uint32_t)req[1] << 16U) | ((uint32_t)req[2] << 8U) | (uint32_t)req[3];

        bool is_all = (group_of_dtc == SYN_UDS_DTC_GROUP_ALL);
        bool is_emissions = (group_of_dtc == SYN_UDS_DTC_GROUP_EMISSIONS);
        bool is_powertrain = (group_of_dtc == SYN_UDS_DTC_GROUP_POWERTRAIN);
        bool is_chassis = (group_of_dtc == SYN_UDS_DTC_GROUP_CHASSIS);
        bool is_body = (group_of_dtc == SYN_UDS_DTC_GROUP_BODY);
        bool is_network = (group_of_dtc == SYN_UDS_DTC_GROUP_NETWORK);

        bool matches_exact_dtc = false;
        for (uint8_t i = 0U; i < server->dtc_count; i++) {
            if (server->dtc_table[i].dtc == group_of_dtc) {
                matches_exact_dtc = true;
                break;
            }
        }

        if (!is_all && !is_emissions && !is_powertrain && !is_chassis && !is_body && !is_network &&
            !matches_exact_dtc) {
            return make_negative_response(sid, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len,
                                          addr_mode);
        }

        uint8_t write_idx = 0U;
        for (uint8_t i = 0U; i < server->dtc_count; i++) {
            uint32_t dtc = server->dtc_table[i].dtc;
            bool clear_dtc = false;

            if (is_all || (dtc == group_of_dtc)) {
                clear_dtc = true;
            } else if (is_emissions && (dtc <= 0x0FFFFFU)) {
                clear_dtc = true;
            } else if (is_powertrain && (dtc >= 0x100000U && dtc <= 0x3FFFFFU)) {
                clear_dtc = true;
            } else if (is_chassis && (dtc >= 0x400000U && dtc <= 0x7FFFFFU)) {
                clear_dtc = true;
            } else if (is_body && (dtc >= 0x800000U && dtc <= 0xBFFFFFU)) {
                clear_dtc = true;
            } else if (is_network && (dtc >= 0xC00000U && dtc <= 0xFEFFFFU)) {
                clear_dtc = true;
            }

            if (!clear_dtc) {
                if (write_idx != i) {
                    server->dtc_table[write_idx] = server->dtc_table[i];
                }
                write_idx++;
            }
        }
        server->dtc_count = write_idx;

        resp_buf[0] = sid + 0x40U;
        *resp_len = 1U;
        success = true;
        break;
    }

    case SYN_UDS_SID_READ_DTC_INFORMATION: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        switch (sub) {
        /* ISO 14229-1 Table 246: reportNumberOfDTCByStatusMask (0x01) */
        case SYN_UDS_DTC_REPORT_NUMBER_BY_STATUS_MASK:
        /* ISO 14229-1 Table 248: reportNumberOfMirrorMemoryDTCByStatusMask (0x11) */
        case SYN_UDS_DTC_REPORT_NUMBER_MIRROR_MEMORY_BY_STATUS_MASK:
        /* ISO 14229-1 Table 250: reportNumberOfEmissionsOBDDTCByStatusMask (0x12) */
        case SYN_UDS_DTC_REPORT_NUMBER_EMISSIONS_OBD_BY_STATUS_MASK: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 3U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                if (max_resp_len < 6U) {
                    return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                                  resp_len, addr_mode);
                }
                uint8_t mask = req[2];
                uint16_t match_cnt = 0U;
                for (uint8_t i = 0U; i < server->dtc_count; i++) {
                    if ((server->dtc_table[i].status & mask) != 0U) {
                        match_cnt++;
                    }
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                resp_buf[3] = SYN_UDS_DTC_FORMAT_ISO14229_1;
                syn_poke_u16(match_cnt, resp_buf, 4);
                *resp_len = 6U;
            }
            success = true;
            break;
        }

        /* ISO 14229-1 Table 252: reportDTCByStatusMask (0x02) */
        case SYN_UDS_DTC_REPORT_BY_STATUS_MASK:
        /* ISO 14229-1 Table 254: reportMirrorMemoryDTCByStatusMask (0x0F) */
        case SYN_UDS_DTC_REPORT_MIRROR_MEMORY_BY_STATUS_MASK:
        /* ISO 14229-1 Table 253: reportEmissionsOBDDTCByStatusMask (0x13) */
        case SYN_UDS_DTC_REPORT_EMISSIONS_OBD_BY_STATUS_MASK: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 3U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                uint8_t mask = req[2];
                uint16_t pos = 3U;
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                for (uint8_t i = 0U; i < server->dtc_count; i++) {
                    if ((server->dtc_table[i].status & mask) != 0U) {
                        if (pos + 4U > max_resp_len) {
                            return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG,
                                                          resp_buf, resp_len, addr_mode);
                        }
                        resp_buf[pos] = (uint8_t)(server->dtc_table[i].dtc >> 16U);
                        resp_buf[pos + 1U] = (uint8_t)(server->dtc_table[i].dtc >> 8U);
                        resp_buf[pos + 2U] = (uint8_t)(server->dtc_table[i].dtc);
                        resp_buf[pos + 3U] = server->dtc_table[i].status;
                        pos += 4U;
                    }
                }
                *resp_len = pos;
            }
            success = true;
            break;
        }

        /* ISO 14229-1 Table 255: reportSupportedDTC (0x0A) */
        case SYN_UDS_DTC_REPORT_SUPPORTED:
        /* ISO 14229-1 Table 255: reportDTCWithPermanentStatus (0x15) */
        case SYN_UDS_DTC_REPORT_WITH_PERMANENT_STATUS: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 2U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                uint16_t pos = 3U;
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                for (uint8_t i = 0U; i < server->dtc_count; i++) {
                    if (pos + 4U > max_resp_len) {
                        return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                                      resp_len, addr_mode);
                    }
                    resp_buf[pos] = (uint8_t)(server->dtc_table[i].dtc >> 16U);
                    resp_buf[pos + 1U] = (uint8_t)(server->dtc_table[i].dtc >> 8U);
                    resp_buf[pos + 2U] = (uint8_t)(server->dtc_table[i].dtc);
                    resp_buf[pos + 3U] = server->dtc_table[i].status;
                    pos += 4U;
                }
                *resp_len = pos;
            }
            success = true;
            break;
        }

        case SYN_UDS_DTC_REPORT_WWH_OBD_BY_MASK_RECORD: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 8U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                resp_buf[3] = (req_len >= 7U) ? req[6] : 0x01U; /* Format Identifier */
                *resp_len = 4U;
            }
            success = true;
            break;
        }

        /* ISO 14229-1 Table 256: reportFirstTestFailedDTC (0x0B) */
        case SYN_UDS_DTC_REPORT_FIRST_TEST_FAILED:
        /* ISO 14229-1 Table 256: reportMostRecentTestFailedDTC (0x0D) */
        case SYN_UDS_DTC_REPORT_MOST_RECENT_TEST_FAILED:
        /* ISO 14229-1 Table 256: reportFirstConfirmedDTC (0x0C) */
        case SYN_UDS_DTC_REPORT_FIRST_CONFIRMED:
        /* ISO 14229-1 Table 256: reportMostRecentConfirmedDTC (0x0E) */
        case SYN_UDS_DTC_REPORT_MOST_RECENT_CONFIRMED: {
            if (req_len != 2U) {
                return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                              resp_len, addr_mode);
            }
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
            uint8_t match_idx = 0xFFU;
            uint8_t target_mask = (sub == SYN_UDS_DTC_REPORT_FIRST_TEST_FAILED ||
                                   sub == SYN_UDS_DTC_REPORT_MOST_RECENT_TEST_FAILED)
                                      ? 0x01U
                                      : 0x08U;

            for (uint8_t i = 0U; i < server->dtc_count; i++) {
                if ((server->dtc_table[i].status & target_mask) != 0U) {
                    match_idx = i;
                    if (sub == SYN_UDS_DTC_REPORT_FIRST_TEST_FAILED ||
                        sub == SYN_UDS_DTC_REPORT_FIRST_CONFIRMED) {
                        break;
                    }
                }
            }

            if (match_idx != 0xFFU) {
                if (max_resp_len < 7U) {
                    return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[3] = (uint8_t)(server->dtc_table[match_idx].dtc >> 16U);
                resp_buf[4] = (uint8_t)(server->dtc_table[match_idx].dtc >> 8U);
                resp_buf[5] = (uint8_t)(server->dtc_table[match_idx].dtc);
                resp_buf[6] = server->dtc_table[match_idx].status;
                *resp_len = 7U;
            } else {
                *resp_len = 3U;
            }
            success = true;
            break;
        }

        case SYN_UDS_DTC_REPORT_NUMBER_BY_SEVERITY_MASK: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 4U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                uint8_t sev_mask = req[2];
                uint8_t stat_mask = req[3];
                uint16_t match_cnt = 0U;
                for (uint8_t i = 0U; i < server->dtc_count; i++) {
                    if (((server->dtc_table[i].severity & sev_mask) != 0U) &&
                        ((server->dtc_table[i].status & stat_mask) != 0U)) {
                        match_cnt++;
                    }
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                resp_buf[3] = SYN_UDS_DTC_FORMAT_ISO14229_1;
                syn_poke_u16(match_cnt, resp_buf, 4);
                *resp_len = 6U;
            }
            success = true;
            break;
        }

        case SYN_UDS_DTC_REPORT_BY_SEVERITY_MASK: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 4U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                uint8_t sev_mask = req[2];
                uint8_t stat_mask = req[3];
                uint16_t pos = 3U;
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                for (uint8_t i = 0U; i < server->dtc_count; i++) {
                    if (((server->dtc_table[i].severity & sev_mask) != 0U) &&
                        ((server->dtc_table[i].status & stat_mask) != 0U)) {
                        if (pos + 6U > max_resp_len) {
                            return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG,
                                                          resp_buf, resp_len, addr_mode);
                        }
                        resp_buf[pos] = server->dtc_table[i].severity;
                        resp_buf[pos + 1U] = 0x00U;
                        resp_buf[pos + 2U] = (uint8_t)(server->dtc_table[i].dtc >> 16U);
                        resp_buf[pos + 3U] = (uint8_t)(server->dtc_table[i].dtc >> 8U);
                        resp_buf[pos + 4U] = (uint8_t)(server->dtc_table[i].dtc);
                        resp_buf[pos + 5U] = server->dtc_table[i].status;
                        pos += 6U;
                    }
                }
                *resp_len = pos;
            }
            success = true;
            break;
        }

        case SYN_UDS_DTC_REPORT_SEVERITY_INFO: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 5U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                uint32_t req_dtc =
                    ((uint32_t)req[2] << 16U) | ((uint32_t)req[3] << 8U) | (uint32_t)req[4];
                uint8_t match_idx = 0xFFU;
                for (uint8_t i = 0U; i < server->dtc_count; i++) {
                    if (server->dtc_table[i].dtc == req_dtc) {
                        match_idx = i;
                        break;
                    }
                }
                if (match_idx != 0xFFU) {
                    if (max_resp_len < 8U) {
                        return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                                      resp_len, addr_mode);
                    }
                    resp_buf[0] = sid + 0x40U;
                    resp_buf[1] = sub;
                    resp_buf[2] = server->dtc_table[match_idx].severity;
                    resp_buf[3] = 0x00U; /* Functional unit */
                    resp_buf[4] = req[2];
                    resp_buf[5] = req[3];
                    resp_buf[6] = req[4];
                    resp_buf[7] = server->dtc_table[match_idx].status;
                    *resp_len = 8U;
                } else {
                    resp_buf[0] = sid + 0x40U;
                    resp_buf[1] = sub;
                    *resp_len = 2U;
                }
            }
            success = true;
            break;
        }

        case SYN_UDS_DTC_REPORT_FAULT_DETECTION_COUNTER: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 2U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                uint16_t pos = 2U;
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                for (uint8_t i = 0U; i < server->dtc_count; i++) {
                    if (pos + 4U > max_resp_len) {
                        return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                                      resp_len, addr_mode);
                    }
                    resp_buf[pos] = (uint8_t)(server->dtc_table[i].dtc >> 16U);
                    resp_buf[pos + 1U] = (uint8_t)(server->dtc_table[i].dtc >> 8U);
                    resp_buf[pos + 2U] = (uint8_t)(server->dtc_table[i].dtc);
                    resp_buf[pos + 3U] = (uint8_t)server->dtc_table[i].fault_cnt;
                    pos += 4U;
                }
                *resp_len = pos;
            }
            success = true;
            break;
        }

        case SYN_UDS_DTC_REPORT_WWH_OBD_WITH_PERMANENT_STATUS: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 4U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                uint8_t mask = req[3];
                uint16_t pos = 3U;
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                for (uint8_t i = 0U; i < server->dtc_count; i++) {
                    if ((server->dtc_table[i].status & mask) != 0U) {
                        if (pos + 4U > max_resp_len) {
                            return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG,
                                                          resp_buf, resp_len, addr_mode);
                        }
                        resp_buf[pos] = (uint8_t)(server->dtc_table[i].dtc >> 16U);
                        resp_buf[pos + 1U] = (uint8_t)(server->dtc_table[i].dtc >> 8U);
                        resp_buf[pos + 2U] = (uint8_t)(server->dtc_table[i].dtc);
                        resp_buf[pos + 3U] = server->dtc_table[i].status;
                        pos += 4U;
                    }
                }
                *resp_len = pos;
            }
            success = true;
            break;
        }

        /* ISO 14229-1 Table 257: reportDTCSnapshotRecordByDTCNumber (0x04) */
        case SYN_UDS_DTC_REPORT_SNAPSHOT_RECORD_BY_DTC: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 6U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                if (max_resp_len < 6U) {
                    return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = req[2];
                resp_buf[3] = req[3];
                resp_buf[4] = req[4];
                resp_buf[5] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                *resp_len = 6U;
            }
            success = true;
            break;
        }

        /* ISO 14229-1 Table 259: reportDTCExtDataRecordByDTCNumber (0x06),
         * reportMirrorMemoryDTCExtDataRecordByDTCNumber (0x10) */
        case SYN_UDS_DTC_REPORT_EXT_DATA_RECORD_BY_DTC:
        case SYN_UDS_DTC_REPORT_MIRROR_MEMORY_EXT_DATA: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 6U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                if (max_resp_len < 6U) {
                    return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = req[2];
                resp_buf[3] = req[3];
                resp_buf[4] = req[4];
                resp_buf[5] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                *resp_len = 6U;
            }
            success = true;
            break;
        }

        case SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_SNAPSHOT_BY_DTC:
        case SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_EXT_DATA_BY_DTC: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 7U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                if (max_resp_len < 7U) {
                    return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = req[6]; /* Memory Selection */
                resp_buf[3] = req[2]; /* DTC high */
                resp_buf[4] = req[3]; /* DTC mid */
                resp_buf[5] = req[4]; /* DTC low */
                resp_buf[6] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                *resp_len = 7U;
            }
            success = true;
            break;
        }

        case SYN_UDS_DTC_REPORT_SNAPSHOT_IDENTIFICATION: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 2U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U;
            }
            success = true;
            break;
        }

        case SYN_UDS_DTC_REPORT_STORED_DATA_BY_RECORD_NUM:
        case SYN_UDS_DTC_REPORT_EXT_DATA_RECORD_BY_RECORD_NUM: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 3U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = req[2]; /* Record Number */
                *resp_len = 3U;
            }
            success = true;
            break;
        }

        case SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_BY_STATUS_MASK: {
            if (server->dtc_cb != NULL) {
                uint16_t cb_out_len = 0U;
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
            } else {
                if (req_len != 4U) {
                    return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                                  resp_buf, resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                resp_buf[2] = req[3]; /* Memory Selection */
                resp_buf[3] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
                *resp_len = 4U;
            }
            success = true;
            break;
        }

        default: {
            uint16_t cb_out_len = 0U;
            if (server->dtc_cb != NULL) {
                if (!server->dtc_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                    &cb_out_len, server->dtc_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                                  resp_len, addr_mode);
                }
                resp_buf[0] = sid + 0x40U;
                resp_buf[1] = sub;
                *resp_len = 2U + cb_out_len;
                success = true;
            } else {
                return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                              resp_len, addr_mode);
            }
            break;
        }
        }
        break;
    }

    case SYN_UDS_SID_CONTROL_DTC_SETTING: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if (sub != 0x01U && sub != 0x02U) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = sub;
        *resp_len = 2U;
        success = true;
        break;
    }

    case SYN_UDS_SID_RESPONSE_ON_EVENT: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if (sub > 0x07U) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        if (server->roe_cb != NULL) {
            uint16_t out_payload_len = 0U;
            if (!server->roe_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                &out_payload_len, server->roe_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            *resp_len = 2U + out_payload_len;
        } else {
            if (max_resp_len < 4U) {
                /* clang-format off */
                return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf, resp_len, addr_mode);
                /* clang-format on */
            }
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            if (sub == 0x04U) {
                resp_buf[2] = 0x00U;
                *resp_len = 3U;
            } else {
                resp_buf[2] = 0x00U;
                resp_buf[3] = 0x02U;
                *resp_len = 4U;
            }
        }
        success = true;
        break;
    }

    case SYN_UDS_SID_REQUEST_DOWNLOAD:
    case SYN_UDS_SID_REQUEST_UPLOAD: {
        if (req_len < 3U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t alfid = req[2];
        uint8_t addr_len = alfid & 0x0FU;
        uint8_t size_len = (alfid >> 4U) & 0x0FU;
        if (req_len < (3U + addr_len + size_len)) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        if (server->security_state != SYN_UDS_SECURITY_UNLOCKED) {
            return make_negative_response(sid, SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp_buf,
                                          resp_len, addr_mode);
        }

        uint32_t addr = 0U;
        for (uint8_t i = 0U; i < addr_len; i++) {
            addr = (addr << 8U) | req[3U + i];
        }

        uint32_t size = 0U;
        for (uint8_t i = 0U; i < size_len; i++) {
            size = (size << 8U) | req[3U + addr_len + i];
        }

        server->transfer_state = (sid == SYN_UDS_SID_REQUEST_DOWNLOAD) ? SYN_UDS_TRANSFER_DOWNLOAD
                                                                       : SYN_UDS_TRANSFER_UPLOAD;
        server->transfer_address = addr;
        server->transfer_size = size;
        server->transfer_bytes_processed = 0U;
        server->expected_block_seq = 1U;

        if (max_resp_len < 4U) {
            return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf, resp_len,
                                          addr_mode);
        }

        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = 0x20U;
        syn_poke_u16(0x0400U, resp_buf, 2);
        *resp_len = 4U;
        success = true;
        break;
    }

    case SYN_UDS_SID_TRANSFER_DATA: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        if (server->transfer_state == SYN_UDS_TRANSFER_IDLE) {
            return make_negative_response(sid, SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR, resp_buf,
                                          resp_len, addr_mode);
        }

        uint8_t block_seq = req[1];
        if (block_seq == (uint8_t)(server->expected_block_seq - 1U)) {
            /* Repeated block sequence counter -> echo previous response */
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = block_seq;
            *resp_len = 2U;
            success = true;
            break;
        }

        if (block_seq != server->expected_block_seq) {
            return make_negative_response(sid, SYN_UDS_NRC_WRONG_BLOCK_SEQUENCE_COUNTER, resp_buf,
                                          resp_len, addr_mode);
        }

        if (server->transfer_state == SYN_UDS_TRANSFER_DOWNLOAD) {
            uint16_t payload_len = req_len - 2U;
            if (server->transfer_size > 0U &&
                (server->transfer_bytes_processed + payload_len > server->transfer_size)) {
                return make_negative_response(sid, SYN_UDS_NRC_TRANSFER_DATA_SUSPENDED, resp_buf,
                                              resp_len, addr_mode);
            }
            if (server->memory_cb != NULL) {
                if (!server->memory_cb(true,
                                       server->transfer_address + server->transfer_bytes_processed,
                                       payload_len, (uint8_t *)&req[2], server->memory_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_GENERAL_PROGRAMMING_FAILURE,
                                                  resp_buf, resp_len, addr_mode);
                }
            }
            server->transfer_bytes_processed += payload_len;
            server->expected_block_seq++;
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = block_seq;
            *resp_len = 2U;
            success = true;
        } else {
            uint16_t chunk_len = 0x03FEU;
            if (server->transfer_size > 0U) {
                uint32_t remaining = server->transfer_size - server->transfer_bytes_processed;
                if (remaining < chunk_len) {
                    chunk_len = (uint16_t)remaining;
                }
            }
            if ((uint32_t)2U + chunk_len > max_resp_len) {
                /* LCOV_EXCL_START: UDS negative response NRC return */
                return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf,
                                              resp_len, addr_mode);
                /* LCOV_EXCL_STOP */
            }
            if (server->memory_cb != NULL) {
                if (!server->memory_cb(false,
                                       server->transfer_address + server->transfer_bytes_processed,
                                       chunk_len, &resp_buf[2], server->memory_ctx)) {
                    return make_negative_response(sid, SYN_UDS_NRC_GENERAL_PROGRAMMING_FAILURE,
                                                  resp_buf, resp_len, addr_mode);
                }
            }
            server->transfer_bytes_processed += chunk_len;
            server->expected_block_seq++;
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = block_seq;
            *resp_len = 2U + chunk_len;
            success = true;
        }
        break;
    }

    case SYN_UDS_SID_REQUEST_TRANSFER_EXIT: {
        if (server->transfer_state == SYN_UDS_TRANSFER_IDLE) {
            return make_negative_response(sid, SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR, resp_buf,
                                          resp_len, addr_mode);
        }
        server->transfer_state = SYN_UDS_TRANSFER_IDLE;
        resp_buf[0] = sid + 0x40U;
        *resp_len = 1U;
        success = true;
        break;
    }

    case SYN_UDS_SID_READ_MEMORY_BY_ADDRESS: {
        if (req_len < 3U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        if (server->security_state != SYN_UDS_SECURITY_UNLOCKED) {
            return make_negative_response(sid, SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t alfid = req[1];
        uint8_t addr_len = alfid & 0x0FU;
        uint8_t size_len = (alfid >> 4U) & 0x0FU;
        if (req_len < (2U + addr_len + size_len)) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint32_t address = 0U;
        for (uint8_t i = 0U; i < addr_len && i < 4U; i++) {
            address = (address << 8U) | req[2U + i];
        }
        uint32_t size = 0U;
        for (uint8_t i = 0U; i < size_len && i < 4U; i++) {
            size = (size << 8U) | req[2U + addr_len + i];
        }
        if (max_resp_len < 1U || size > (uint32_t)(max_resp_len - 1U)) {
            return make_negative_response(sid, SYN_UDS_NRC_RESPONSE_TOO_LONG, resp_buf, resp_len,
                                          addr_mode);
        }
        if (server->memory_cb != NULL) {
            if (!server->memory_cb(false, address, size, &resp_buf[1], server->memory_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
        } else {
            memset(&resp_buf[1], 0, size);
        }
        resp_buf[0] = sid + 0x40U;
        *resp_len = 1U + (uint16_t)size;
        success = true;
        break;
    }

    case SYN_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER: {
        if (req_len < 3U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint16_t target_did = syn_peek_u16(req, 1);
        int did_idx = find_did_index(server, target_did);
        if (did_idx < 0 && server->scaling_cb == NULL) {
            return make_negative_response(sid, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len,
                                          addr_mode);
        }
        resp_buf[0] = sid + 0x40U;
        syn_poke_u16(target_did, resp_buf, 1);
        if (server->scaling_cb != NULL) {
            uint16_t out_payload_len = 0U;
            if (!server->scaling_cb(target_did, &resp_buf[3], max_resp_len - 3U, &out_payload_len,
                                    server->scaling_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
            *resp_len = 3U + out_payload_len;
        } else {
            resp_buf[3] = 0x01U;
            *resp_len = 4U;
        }
        success = true;
        break;
    }

    case SYN_UDS_SID_AUTHENTICATION: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if (sub > 0x08U) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        uint16_t out_payload_len = 0U;
        if (server->auth_cb != NULL) {
            if (!server->auth_cb(sub, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                 &out_payload_len, server->auth_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
        }
        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = sub;
        *resp_len = 2U + out_payload_len;
        success = true;
        break;
    }

    case SYN_UDS_SID_READ_DATA_BY_PERIODIC_IDENTIFIER: {
        if (req_len < 3U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t mode = req[1];
        if (mode < 0x01U || mode > 0x04U) {
            return make_negative_response(sid, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len,
                                          addr_mode);
        }
        if (server->periodic_cb != NULL) {
            uint16_t out_payload_len = 0U;
            if (!server->periodic_cb(mode, &req[2], req_len - 2U, &resp_buf[2], max_resp_len - 2U,
                                     &out_payload_len, server->periodic_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = req[2];
            *resp_len = 2U + out_payload_len;
        } else {
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = req[2];
            *resp_len = 2U;
        }
        success = true;
        break;
    }

    case SYN_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER: {
        if (req_len < 4U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if (sub < 0x01U || sub > 0x03U) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        uint16_t dyn_did = syn_peek_u16(req, 2);
        if (server->dynamic_did_cb != NULL) {
            uint16_t out_payload_len = 0U;
            if (!server->dynamic_did_cb(sub, dyn_did, &req[4], req_len - 4U, &resp_buf[4],
                                        max_resp_len - 4U, &out_payload_len,
                                        server->dynamic_did_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            syn_poke_u16(dyn_did, resp_buf, 2);
            *resp_len = 4U + out_payload_len;
        } else {
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            syn_poke_u16(dyn_did, resp_buf, 2);
            *resp_len = 4U;
        }
        success = true;
        break;
    }

    case SYN_UDS_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER: {
        if (req_len < 4U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint16_t target_did = syn_peek_u16(req, 1);
        uint8_t control_opt = req[3];
        if (control_opt > 0x03U) {
            return make_negative_response(sid, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len,
                                          addr_mode);
        }
        int did_idx = find_did_index(server, target_did);
        if (did_idx < 0 && server->io_control_cb == NULL) {
            return make_negative_response(sid, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len,
                                          addr_mode);
        }
        if (server->io_control_cb != NULL) {
            uint16_t out_payload_len = 0U;
            if (!server->io_control_cb(target_did, control_opt, &req[4], req_len - 4U, &resp_buf[4],
                                       max_resp_len - 4U, &out_payload_len,
                                       server->io_control_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
            resp_buf[0] = sid + 0x40U;
            syn_poke_u16(target_did, resp_buf, 1);
            resp_buf[3] = control_opt;
            *resp_len = 4U + out_payload_len;
        } else {
            resp_buf[0] = sid + 0x40U;
            syn_poke_u16(target_did, resp_buf, 1);
            resp_buf[3] = control_opt;
            *resp_len = 4U;
        }
        success = true;
        break;
    }

    case SYN_UDS_SID_REQUEST_FILE_TRANSFER: {
        if (req_len < 3U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t mode = req[1];
        if (mode < 0x01U || mode > 0x05U) {
            return make_negative_response(sid, SYN_UDS_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len,
                                          addr_mode);
        }
        uint16_t out_len = 0U;
        if (server->file_transfer_cb != NULL) {
            uint16_t path_len = req_len - 2U;
            if (!server->file_transfer_cb(mode, (const char *)&req[2], path_len, &resp_buf[2],
                                          max_resp_len - 2U, &out_len, server->file_transfer_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
        } else {
            resp_buf[2] = 0x20U;
            syn_poke_u16(0x0400U, resp_buf, 3);
            out_len = 3U;
        }
        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = mode;
        *resp_len = 2U + out_len;
        success = true;
        break;
    }

    case SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS: {
        if (req_len < 3U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        if (server->security_state != SYN_UDS_SECURITY_UNLOCKED) {
            return make_negative_response(sid, SYN_UDS_NRC_SECURITY_ACCESS_DENIED, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t alfid = req[1];
        uint8_t addr_len = alfid & 0x0FU;
        uint8_t size_len = (alfid >> 4U) & 0x0FU;
        if (req_len < (2U + addr_len + size_len)) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint32_t address = 0U;
        for (uint8_t i = 0U; i < addr_len && i < 4U; i++) {
            address = (address << 8U) | req[2U + i];
        }
        uint32_t size = 0U;
        for (uint8_t i = 0U; i < size_len && i < 4U; i++) {
            size = (size << 8U) | req[2U + addr_len + i];
        }
        uint16_t header_len = 2U + addr_len + size_len;
        if (req_len < header_len || size > (uint32_t)(req_len - header_len)) {
            /* LCOV_EXCL_START: UDS negative response NRC return */
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
            /* LCOV_EXCL_STOP */
        }
        if (server->memory_cb != NULL) {
            if (!server->memory_cb(true, address, size, (uint8_t *)&req[header_len],
                                   server->memory_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
        }
        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = alfid;
        memcpy(&resp_buf[2], &req[2], addr_len + size_len);
        *resp_len = 2U + addr_len + size_len;
        success = true;
        break;
    }

    case SYN_UDS_SID_LINK_CONTROL: {
        if (req_len < 2U) {
            return make_negative_response(sid, SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf,
                                          resp_len, addr_mode);
        }
        uint8_t sub = req[1] & 0x7FU;
        if (sub < 0x01U || sub > 0x03U) {
            return make_negative_response(sid, SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf,
                                          resp_len, addr_mode);
        }
        if (server->link_control_cb != NULL) {
            uint16_t out_payload_len = 0U;
            if (!server->link_control_cb(sub, &req[2], req_len - 2U, &resp_buf[2],
                                         max_resp_len - 2U, &out_payload_len,
                                         server->link_control_ctx)) {
                return make_negative_response(sid, SYN_UDS_NRC_CONDITIONS_NOT_CORRECT, resp_buf,
                                              resp_len, addr_mode);
            }
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            *resp_len = 2U + out_payload_len;
        } else {
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub;
            *resp_len = 2U;
        }
        success = true;
        break;
    }

    default: {
        return make_negative_response(sid, SYN_UDS_NRC_SERVICE_NOT_SUPPORTED, resp_buf, resp_len,
                                      addr_mode);
    }
    }

    if (success) {
        if (server->session != SYN_UDS_SESSION_DEFAULT) {
            server->s3_timer_ms = SYN_UDS_S3_TIMEOUT_MS;
        }

        /* ISO 14229-1: Suppress positive response if bit 7 (0x80) of sub-function is set */
        if (req_len >= 2U) {
            if ((sid == SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL || sid == SYN_UDS_SID_ECU_RESET ||
                 sid == SYN_UDS_SID_SECURITY_ACCESS || sid == SYN_UDS_SID_COMMUNICATION_CONTROL ||
                 sid == SYN_UDS_SID_ROUTINE_CONTROL || sid == SYN_UDS_SID_TESTER_PRESENT ||
                 sid == SYN_UDS_SID_ACCESS_TIMING_PARAMETER ||
                 sid == SYN_UDS_SID_CONTROL_DTC_SETTING || sid == SYN_UDS_SID_RESPONSE_ON_EVENT ||
                 sid == SYN_UDS_SID_LINK_CONTROL) &&
                ((req[1] & 0x80U) != 0U)) {
                *resp_len = 0U;
            }
        }
    }

    return true;
}

bool syn_uds_dtc_report_test_result(SYN_UDS_Server *server, uint32_t dtc, bool failed)
{
    if (server == NULL) {
        return false;
    }

    for (uint8_t i = 0U; i < server->dtc_count; i++) {
        if (server->dtc_table[i].dtc == dtc) {
            uint8_t status = server->dtc_table[i].status;

            if (failed) {
                status |= (uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED;
                status |= (uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE;
                status |= (uint8_t)SYN_UDS_DTC_STATUS_PENDING_DTC;
                status |= (uint8_t)SYN_UDS_DTC_STATUS_CONFIRMED_DTC;
                status |= (uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED_SINCE_LAST_CLEAR;
                status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
                status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;

                if (server->dtc_table[i].fault_cnt < 127) {
                    server->dtc_table[i].fault_cnt++;
                }
            } else {
                status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED;
                status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
                status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;

                if (server->dtc_table[i].fault_cnt > -128) {
                    server->dtc_table[i].fault_cnt--;
                }
            }

            server->dtc_table[i].status = status;
            return true;
        }
    }

    return false;
}

bool syn_uds_dtc_start_operation_cycle(SYN_UDS_Server *server)
{
    if (server == NULL) {
        return false;
    }

    for (uint8_t i = 0U; i < server->dtc_count; i++) {
        uint8_t status = server->dtc_table[i].status;
        status &= (uint8_t)~(uint8_t)SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE;
        status |= (uint8_t)SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE;
        server->dtc_table[i].status = status;
    }

    return true;
}

bool syn_uds_dtc_get_status(SYN_UDS_Server *server, uint32_t dtc, uint8_t *out_status)
{
    if (server == NULL || out_status == NULL) {
        return false;
    }

    for (uint8_t i = 0U; i < server->dtc_count; i++) {
        if (server->dtc_table[i].dtc == dtc) {
            *out_status = server->dtc_table[i].status;
            return true;
        }
    }

    return false;
}

uint8_t syn_uds_get_security_level(const SYN_UDS_Server *server)
{
    if (server == NULL || server->security_state != SYN_UDS_SECURITY_UNLOCKED) {
        return 0U;
    }
    return server->security_level;
}
```


