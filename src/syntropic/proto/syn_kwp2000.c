/**
 * @file syn_kwp2000.c
 * @brief ISO 14230-3 Key Word Protocol 2000 (KWP2000) Server Engine Implementation.
 * @ingroup syn_proto
 */

#include "syn_kwp2000.h"

#include <string.h>

/**
 * @brief Helper to format a KWP2000 Negative Response Code (NRC).
 * @param sid Service Identifier.
 * @param nrc Negative Response Code.
 * @param resp_buf Response buffer.
 * @param resp_len Pointer to store response length.
 */
static void make_nrc(uint8_t sid, uint8_t nrc, uint8_t *resp_buf, uint16_t *resp_len)
{
    resp_buf[0] = SYN_KWP2000_SID_NEGATIVE_RESPONSE;
    resp_buf[1] = sid;
    resp_buf[2] = nrc;
    *resp_len = 3U;
}

SYN_Status syn_kwp2000_init(SYN_KWP2000_Server *server)
{
    if (server == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(server, 0, sizeof(*server));
    server->current_session = SYN_KWP2000_SESSION_DEFAULT;
    return SYN_OK;
}

SYN_Status syn_kwp2000_register_lid(SYN_KWP2000_Server *server, uint8_t lid, void *data,
                                    uint16_t len, bool writable)
{
    if (server == NULL || data == NULL || len == 0U) {
        return SYN_INVALID_PARAM;
    }

    for (uint16_t i = 0U; i < server->lid_count; i++) {
        if (server->lids[i].lid == lid) {
            server->lids[i].data = data;
            server->lids[i].len = len;
            server->lids[i].writable = writable;
            return SYN_OK;
        }
    }

    if (server->lid_count >= SYN_KWP2000_MAX_LIDS) {
        return SYN_ERROR;
    }

    server->lids[server->lid_count].lid = lid;
    server->lids[server->lid_count].data = data;
    server->lids[server->lid_count].len = len;
    server->lids[server->lid_count].writable = writable;
    server->lid_count++;
    return SYN_OK;
}

SYN_Status syn_kwp2000_register_cid(SYN_KWP2000_Server *server, uint16_t cid, void *data,
                                    uint16_t len, bool writable)
{
    if (server == NULL || data == NULL || len == 0U) {
        return SYN_INVALID_PARAM;
    }

    for (uint16_t i = 0U; i < server->cid_count; i++) {
        if (server->cids[i].cid == cid) {
            server->cids[i].data = data;
            server->cids[i].len = len;
            server->cids[i].writable = writable;
            return SYN_OK;
        }
    }

    if (server->cid_count >= SYN_KWP2000_MAX_CIDS) {
        return SYN_ERROR;
    }

    server->cids[server->cid_count].cid = cid;
    server->cids[server->cid_count].data = data;
    server->cids[server->cid_count].len = len;
    server->cids[server->cid_count].writable = writable;
    server->cid_count++;
    return SYN_OK;
}

SYN_Status syn_kwp2000_set_session_handler(SYN_KWP2000_Server *server,
                                           SYN_KWP2000_SessionHandler cb, void *user_ctx)
{
    if (server == NULL) {
        return SYN_INVALID_PARAM;
    }
    server->session_cb = cb;
    server->session_ctx = user_ctx;
    return SYN_OK;
}

SYN_Status syn_kwp2000_set_reset_handler(SYN_KWP2000_Server *server, SYN_KWP2000_ResetHandler cb,
                                         void *user_ctx)
{
    if (server == NULL) {
        return SYN_INVALID_PARAM;
    }
    server->reset_cb = cb;
    server->reset_ctx = user_ctx;
    return SYN_OK;
}

SYN_Status syn_kwp2000_set_security_handler(SYN_KWP2000_Server *server,
                                            SYN_KWP2000_SeedHandler seed_cb,
                                            SYN_KWP2000_KeyHandler key_cb, void *user_ctx)
{
    if (server == NULL) {
        return SYN_INVALID_PARAM;
    }
    server->seed_cb = seed_cb;
    server->key_cb = key_cb;
    server->security_ctx = user_ctx;
    return SYN_OK;
}

SYN_Status syn_kwp2000_set_routine_handler(SYN_KWP2000_Server *server,
                                           SYN_KWP2000_RoutineHandler cb, void *user_ctx)
{
    if (server == NULL) {
        return SYN_INVALID_PARAM;
    }
    server->routine_cb = cb;
    server->routine_ctx = user_ctx;
    return SYN_OK;
}

SYN_Status syn_kwp2000_set_memory_handler(SYN_KWP2000_Server *server, SYN_KWP2000_MemoryHandler cb,
                                          void *user_ctx)
{
    if (server == NULL) {
        return SYN_INVALID_PARAM;
    }
    server->memory_cb = cb;
    server->memory_ctx = user_ctx;
    return SYN_OK;
}

/* ── Service Processing Implementation ─────────────────────────────── */

/**
 * @brief Look up a registered Local Identifier (LID) entry in the KWP2000 server.
 * @param server KWP2000 server instance.
 * @param target_lid Local Identifier byte to match.
 * @return Pointer to matching LID entry, or NULL if not registered.
 */
static const SYN_KWP2000_LID *kwp_find_lid(const SYN_KWP2000_Server *server, uint8_t target_lid)
{
    for (uint16_t i = 0U; i < server->lid_count; i++) {
        if (server->lids[i].lid == target_lid) {
            return &server->lids[i];
        }
    }
    return NULL;
}

/**
 * @brief Look up a registered Common Identifier (CID) entry in the KWP2000 server.
 * @param server KWP2000 server instance.
 * @param target_cid 16-bit Common Identifier to match.
 * @return Pointer to matching CID entry, or NULL if not registered.
 */
static const SYN_KWP2000_CID *kwp_find_cid(const SYN_KWP2000_Server *server, uint16_t target_cid)
{
    for (uint16_t i = 0U; i < server->cid_count; i++) {
        if (server->cids[i].cid == target_cid) {
            return &server->cids[i];
        }
    }
    return NULL;
}

/**
 * @brief Handle StartDiagnosticSession (0x10) request.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_session(SYN_KWP2000_Server *server, const uint8_t *req_buf, uint16_t req_len,
                               uint8_t *resp_buf, uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    if (req_len < 2U) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    uint8_t target_session = req_buf[1];
    if (target_session != SYN_KWP2000_SESSION_DEFAULT &&
        target_session != SYN_KWP2000_SESSION_PROGRAMMING &&
        target_session != SYN_KWP2000_SESSION_EXTENDED) {
        make_nrc(sid, SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf, resp_len);
        return;
    }
    if (server->session_cb != NULL &&
        !server->session_cb(server->current_session, target_session, server->session_ctx)) {
        make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
        return;
    }

    server->current_session = target_session;
    server->security_unlocked = false;
    server->security_level = 0U;
    server->is_downloading = false;
    server->s3_timer_ms = 0U;
    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = target_session;
    *resp_len = 2U;
}

/**
 * @brief Handle ECUReset (0x11) request.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_reset(SYN_KWP2000_Server *server, const uint8_t *req_buf, uint16_t req_len,
                             uint8_t *resp_buf, uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    if (req_len < 2U) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    uint8_t reset_type = req_buf[1];
    if (reset_type != 0x01U && reset_type != 0x02U) {
        make_nrc(sid, SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf, resp_len);
        return;
    }
    server->pending_reset_type = reset_type;
    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = reset_type;
    *resp_len = 2U;
}

/**
 * @brief Handle ReadDataByLocalIdentifier (0x21) request.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param max_resp_len Capacity of resp_buf in bytes.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_read_lid(const SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                uint16_t req_len, uint8_t *resp_buf, uint16_t max_resp_len,
                                uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    if (req_len < 2U) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    uint8_t target_lid = req_buf[1];
    const SYN_KWP2000_LID *found_lid = kwp_find_lid(server, target_lid);
    if (found_lid == NULL) {
        make_nrc(sid, SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len);
        return;
    }
    if ((uint32_t)found_lid->len + 2U > (uint32_t)max_resp_len) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = target_lid;
    memcpy(&resp_buf[2], found_lid->data, found_lid->len);
    *resp_len = (uint16_t)(2U + found_lid->len);
}

/**
 * @brief Handle ReadDataByCommonIdentifier (0x22) request.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param max_resp_len Capacity of resp_buf in bytes.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_read_cid(const SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                uint16_t req_len, uint8_t *resp_buf, uint16_t max_resp_len,
                                uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    if (req_len < 3U) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    uint16_t target_cid = (uint16_t)(((uint16_t)req_buf[1] << 8U) | (uint16_t)req_buf[2]);
    const SYN_KWP2000_CID *found_cid = kwp_find_cid(server, target_cid);
    if (found_cid == NULL) {
        make_nrc(sid, SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len);
        return;
    }
    if ((uint32_t)found_cid->len + 3U > (uint32_t)max_resp_len) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = (uint8_t)(target_cid >> 8U);
    resp_buf[2] = (uint8_t)(target_cid & 0xFFU);
    memcpy(&resp_buf[3], found_cid->data, found_cid->len);
    *resp_len = (uint16_t)(3U + found_cid->len);
}

/**
 * @brief Handle WriteDataByLocalIdentifier (0x3B) request.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_write_lid(SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                 uint16_t req_len, uint8_t *resp_buf, uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    if (req_len < 3U) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    uint8_t target_lid = req_buf[1];
    const SYN_KWP2000_LID *found_lid = kwp_find_lid(server, target_lid);
    if (found_lid == NULL || !found_lid->writable) {
        make_nrc(sid, SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len);
        return;
    }
    if ((uint16_t)(req_len - 2U) != found_lid->len) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    if (!server->security_unlocked) {
        make_nrc(sid, SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED, resp_buf, resp_len);
        return;
    }
    memcpy(found_lid->data, &req_buf[2], found_lid->len);
    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = target_lid;
    *resp_len = 2U;
}

/**
 * @brief Handle WriteDataByCommonIdentifier (0x2E) request.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_write_cid(SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                 uint16_t req_len, uint8_t *resp_buf, uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    if (req_len < 4U) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    uint16_t target_cid = (uint16_t)(((uint16_t)req_buf[1] << 8U) | (uint16_t)req_buf[2]);
    const SYN_KWP2000_CID *found_cid = kwp_find_cid(server, target_cid);
    if (found_cid == NULL || !found_cid->writable) {
        make_nrc(sid, SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len);
        return;
    }
    if ((uint16_t)(req_len - 3U) != found_cid->len) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    if (!server->security_unlocked) {
        make_nrc(sid, SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED, resp_buf, resp_len);
        return;
    }
    memcpy(found_cid->data, &req_buf[3], found_cid->len);
    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = req_buf[1];
    resp_buf[2] = req_buf[2];
    *resp_len = 3U;
}

/**
 * @brief Handle SecurityAccess requestSeed (odd sub-function) step.
 * @param server KWP2000 server instance.
 * @param sid Request service identifier.
 * @param sub_func Odd SecurityAccess sub-function byte.
 * @param resp_buf Response PDU buffer.
 * @param max_resp_len Capacity of resp_buf in bytes.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_security_seed(SYN_KWP2000_Server *server, uint8_t sid, uint8_t sub_func,
                                     uint8_t *resp_buf, uint16_t max_resp_len, uint16_t *resp_len)
{
    uint8_t seed_buf[16];
    uint16_t seed_len = (uint16_t)sizeof(seed_buf);
    if (server->seed_cb != NULL) {
        if (!server->seed_cb(sub_func, seed_buf, &seed_len, server->security_ctx)) {
            make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
            return;
        }
    } else {
        seed_buf[0] = 0x36U;
        seed_buf[1] = 0x37U;
        seed_len = 2U;
    }

    if ((seed_len > (uint16_t)sizeof(seed_buf)) ||
        (((uint32_t)seed_len + 2U) > (uint32_t)max_resp_len)) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = sub_func;
    memcpy(&resp_buf[2], seed_buf, seed_len);
    *resp_len = (uint16_t)(2U + seed_len);
}

/**
 * @brief Handle SecurityAccess sendKey (even sub-function) step.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_security_key(SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                    uint16_t req_len, uint8_t *resp_buf, uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    uint8_t sub_func = req_buf[1];
    if (req_len < 3U) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    const uint8_t *key = &req_buf[2];
    uint16_t key_len = (uint16_t)(req_len - 2U);

    bool key_ok = false;
    if (server->key_cb != NULL) {
        key_ok = server->key_cb(sub_func, key, key_len, server->security_ctx);
    } else {
        key_ok = (key_len >= 2U && key[0] == 0xC5U && key[1] == 0xC9U);
    }

    if (!key_ok) {
        make_nrc(sid, SYN_KWP2000_NRC_INVALID_KEY, resp_buf, resp_len);
        return;
    }

    server->security_unlocked = true;
    server->security_level = sub_func;
    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = sub_func;
    *resp_len = 2U;
}

/**
 * @brief Handle StartRoutineByLocalIdentifier (0x31) request.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param max_resp_len Capacity of resp_buf in bytes.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_routine(SYN_KWP2000_Server *server, const uint8_t *req_buf, uint16_t req_len,
                               uint8_t *resp_buf, uint16_t max_resp_len, uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    if (req_len < 2U) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }

    uint8_t routine_id = req_buf[1];
    const uint8_t *in_data = (req_len > 2U) ? &req_buf[2] : NULL;
    uint16_t in_len = (req_len > 2U) ? (uint16_t)(req_len - 2U) : 0U;

    uint8_t routine_out[64];
    uint16_t out_len = 0U;
    if (server->routine_cb != NULL &&
        !server->routine_cb(routine_id, in_data, in_len, routine_out, (uint16_t)sizeof(routine_out),
                            &out_len, server->routine_ctx)) {
        uint8_t nrc = server->security_unlocked ? SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT
                                                : SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED;
        make_nrc(sid, nrc, resp_buf, resp_len);
        return;
    }

    if ((out_len > (uint16_t)sizeof(routine_out)) ||
        (((uint32_t)out_len + 2U) > (uint32_t)max_resp_len)) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = routine_id;
    if (out_len > 0U) {
        memcpy(&resp_buf[2], routine_out, out_len);
    }
    *resp_len = (uint16_t)(2U + out_len);
}

/**
 * @brief Handle RequestDownload (0x34) request.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_download(SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                uint16_t req_len, uint8_t *resp_buf, uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    if ((req_len < 5U) || ((req_len > 5U) && (req_len < 9U))) {
        make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return;
    }
    if (!server->security_unlocked) {
        make_nrc(sid, SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED, resp_buf, resp_len);
        return;
    }

    uint32_t addr = ((uint32_t)req_buf[1] << 24U) | ((uint32_t)req_buf[2] << 16U) |
                    ((uint32_t)req_buf[3] << 8U) | (uint32_t)req_buf[4];
    uint32_t size = (req_len >= 9U)
                        ? (((uint32_t)req_buf[5] << 24U) | ((uint32_t)req_buf[6] << 16U) |
                           ((uint32_t)req_buf[7] << 8U) | (uint32_t)req_buf[8])
                        : 0U;

    server->is_downloading = true;
    server->expected_block_seq = 1U;
    server->download_addr = addr;
    server->download_size = size;
    server->download_received = 0U;

    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = 0x20U;
    *resp_len = 2U;
}

/**
 * @brief Handle TransferData (0x36) block write request.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_transfer(SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                uint16_t req_len, uint8_t *resp_buf, uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    if (!server->is_downloading || req_len < 2U) {
        make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
        return;
    }

    uint8_t block_num = req_buf[1];
    uint16_t data_len = (uint16_t)(req_len - 2U);
    if (block_num != server->expected_block_seq) {
        make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
        return;
    }
    if (server->download_size > 0U &&
        (uint32_t)data_len > (server->download_size - server->download_received)) {
        make_nrc(sid, SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len);
        return;
    }

    uint32_t target_addr = server->download_addr + server->download_received;
    if (server->memory_cb != NULL &&
        !server->memory_cb(true, target_addr, data_len, (uint8_t *)(uintptr_t)&req_buf[2],
                           server->memory_ctx)) {
        make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
        return;
    }

    server->download_received += data_len;
    server->expected_block_seq = (uint8_t)(server->expected_block_seq + 1U);
    resp_buf[0] = (uint8_t)(sid + 0x40U);
    resp_buf[1] = block_num;
    *resp_len = 2U;
}

/**
 * @brief Dispatch routine, download, transfer, tester present, or unsupported KWP2000 services.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param max_resp_len Capacity of resp_buf in bytes.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_dispatch_transfer_or_misc(SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                          uint16_t req_len, uint8_t *resp_buf,
                                          uint16_t max_resp_len, uint16_t *resp_len)
{
    uint8_t sid = req_buf[0];
    switch (sid) {
    case SYN_KWP2000_SID_START_ROUTINE_BY_LOCAL_IDENTIFIER:
        kwp_handle_routine(server, req_buf, req_len, resp_buf, max_resp_len, resp_len);
        break;
    case SYN_KWP2000_SID_REQUEST_DOWNLOAD:
        kwp_handle_download(server, req_buf, req_len, resp_buf, resp_len);
        break;
    case SYN_KWP2000_SID_TRANSFER_DATA:
        kwp_handle_transfer(server, req_buf, req_len, resp_buf, resp_len);
        break;
    case SYN_KWP2000_SID_REQUEST_TRANSFER_EXIT:
        if (!server->is_downloading) {
            make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
        } else {
            server->is_downloading = false;
            resp_buf[0] = (uint8_t)(sid + 0x40U);
            *resp_len = 1U;
        }
        break;
    case SYN_KWP2000_SID_TESTER_PRESENT:
        server->s3_timer_ms = 0U;
        resp_buf[0] = (uint8_t)(sid + 0x40U);
        *resp_len = 1U;
        break;
    default:
        make_nrc(sid, SYN_KWP2000_NRC_SERVICE_NOT_SUPPORTED, resp_buf, resp_len);
        break;
    }
}

/**
 * @brief Handle SecurityAccess (0x27) request and dispatch seed or key step.
 * @param server KWP2000 server instance.
 * @param req_buf Request PDU buffer.
 * @param req_len Request PDU length in bytes.
 * @param resp_buf Response PDU buffer.
 * @param max_resp_len Capacity of resp_buf in bytes.
 * @param resp_len Output response PDU length in bytes.
 */
static void kwp_handle_security(SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                uint16_t req_len, uint8_t *resp_buf, uint16_t max_resp_len,
                                uint16_t *resp_len)
{
    if (req_len < 2U) {
        make_nrc(req_buf[0], SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
    } else if (req_buf[1] == 0x00U) {
        make_nrc(req_buf[0], SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf, resp_len);
    } else if ((req_buf[1] % 2U) != 0U) {
        kwp_handle_security_seed(server, req_buf[0], req_buf[1], resp_buf, max_resp_len, resp_len);
    } else {
        kwp_handle_security_key(server, req_buf, req_len, resp_buf, resp_len);
    }
}

SYN_Status syn_kwp2000_process_request(SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                       uint16_t req_len, uint8_t *resp_buf, uint16_t max_resp_len,
                                       uint16_t *resp_len)
{
    if (server == NULL || req_buf == NULL || resp_buf == NULL || resp_len == NULL ||
        max_resp_len < 3U) {
        return SYN_INVALID_PARAM;
    }

    if (req_len < 1U) {
        make_nrc(0x00U, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
        return SYN_OK;
    }

    if (server->current_session != SYN_KWP2000_SESSION_DEFAULT) {
        server->s3_timer_ms = 0U;
    }

    uint8_t sid = req_buf[0];
    switch (sid) {
    case SYN_KWP2000_SID_START_DIAGNOSTIC_SESSION:
        kwp_handle_session(server, req_buf, req_len, resp_buf, resp_len);
        break;
    case SYN_KWP2000_SID_ECU_RESET:
        kwp_handle_reset(server, req_buf, req_len, resp_buf, resp_len);
        break;
    case SYN_KWP2000_SID_READ_DATA_BY_LOCAL_IDENTIFIER:
        kwp_handle_read_lid(server, req_buf, req_len, resp_buf, max_resp_len, resp_len);
        break;
    case SYN_KWP2000_SID_READ_DATA_BY_COMMON_IDENTIFIER:
        kwp_handle_read_cid(server, req_buf, req_len, resp_buf, max_resp_len, resp_len);
        break;
    case SYN_KWP2000_SID_SECURITY_ACCESS:
        kwp_handle_security(server, req_buf, req_len, resp_buf, max_resp_len, resp_len);
        break;
    case SYN_KWP2000_SID_WRITE_DATA_BY_COMMON_IDENTIFIER:
        kwp_handle_write_cid(server, req_buf, req_len, resp_buf, resp_len);
        break;
    case SYN_KWP2000_SID_WRITE_DATA_BY_LOCAL_IDENTIFIER:
        kwp_handle_write_lid(server, req_buf, req_len, resp_buf, resp_len);
        break;
    default:
        kwp_dispatch_transfer_or_misc(server, req_buf, req_len, resp_buf, max_resp_len, resp_len);
        break;
    }
    return SYN_OK;
}

void syn_kwp2000_tick(SYN_KWP2000_Server *server, uint32_t dt_ms)
{
    if (server == NULL) {
        return;
    }
    if (dt_ms > (UINT32_MAX - server->s3_timer_ms)) {
        server->s3_timer_ms = UINT32_MAX;
    } else {
        server->s3_timer_ms += dt_ms;
    }

    /* S3 session inactivity timeout (ISO 14230-3 §5.3) */
    if (server->current_session != SYN_KWP2000_SESSION_DEFAULT) {
        if (server->s3_timer_ms >= SYN_KWP2000_S3_TIMEOUT_MS) {
            server->current_session = SYN_KWP2000_SESSION_DEFAULT;
            server->security_unlocked = false;
            server->security_level = 0U;
            server->is_downloading = false;
            server->s3_timer_ms = 0U;
        }
    } else {
        server->s3_timer_ms = 0U;
    }

    if (server->pending_reset_type != 0U) {
        if (server->reset_wait_ms > dt_ms) {
            server->reset_wait_ms -= dt_ms;
        } else {
            uint8_t rtype = server->pending_reset_type;
            server->pending_reset_type = 0U;
            if (server->reset_cb != NULL) {
                server->reset_cb(rtype, server->reset_ctx);
            }
        }
    }
}
