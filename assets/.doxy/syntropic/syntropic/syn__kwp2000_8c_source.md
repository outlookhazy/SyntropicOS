

# File syn\_kwp2000.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_kwp2000.c**](syn__kwp2000_8c.md)

[Go to the documentation of this file](syn__kwp2000_8c.md)


```C++

#include "syn_kwp2000.h"

#include <string.h>

static void make_nrc(uint8_t sid, uint8_t nrc, uint8_t *resp_buf, uint16_t *resp_len)
{
    if (resp_buf != NULL && resp_len != NULL) {
        resp_buf[0] = SYN_KWP2000_SID_NEGATIVE_RESPONSE;
        resp_buf[1] = sid;
        resp_buf[2] = nrc;
        *resp_len = 3U;
    }
}

SYN_Status syn_kwp2000_init(SYN_KWP2000_Server *server)
{
    if (server == NULL)
        return SYN_INVALID_PARAM;
    memset(server, 0, sizeof(*server));
    server->current_session = SYN_KWP2000_SESSION_DEFAULT;
    return SYN_OK;
}

SYN_Status syn_kwp2000_register_lid(SYN_KWP2000_Server *server, uint8_t lid, const void *data,
                                    uint16_t len, bool writable)
{
    if (server == NULL || data == NULL || len == 0U)
        return SYN_INVALID_PARAM;

    for (uint16_t i = 0U; i < server->lid_count; i++) {
        if (server->lids[i].lid == lid) {
            server->lids[i].data = data;
            server->lids[i].len = len;
            server->lids[i].writable = writable;
            return SYN_OK;
        }
    }

    if (server->lid_count >= SYN_KWP2000_MAX_LIDS)
        return SYN_ERROR;

    server->lids[server->lid_count].lid = lid;
    server->lids[server->lid_count].data = data;
    server->lids[server->lid_count].len = len;
    server->lids[server->lid_count].writable = writable;
    server->lid_count++;
    return SYN_OK;
}

SYN_Status syn_kwp2000_register_cid(SYN_KWP2000_Server *server, uint16_t cid, const void *data,
                                    uint16_t len, bool writable)
{
    if (server == NULL || data == NULL || len == 0U)
        return SYN_INVALID_PARAM;

    for (uint16_t i = 0U; i < server->cid_count; i++) {
        if (server->cids[i].cid == cid) {
            server->cids[i].data = data;
            server->cids[i].len = len;
            server->cids[i].writable = writable;
            return SYN_OK;
        }
    }

    if (server->cid_count >= SYN_KWP2000_MAX_CIDS)
        return SYN_ERROR;

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
    if (server == NULL)
        return SYN_INVALID_PARAM;
    server->session_cb = cb;
    server->session_ctx = user_ctx;
    return SYN_OK;
}

SYN_Status syn_kwp2000_set_reset_handler(SYN_KWP2000_Server *server, SYN_KWP2000_ResetHandler cb,
                                         void *user_ctx)
{
    if (server == NULL)
        return SYN_INVALID_PARAM;
    server->reset_cb = cb;
    server->reset_ctx = user_ctx;
    return SYN_OK;
}

SYN_Status syn_kwp2000_set_security_handler(SYN_KWP2000_Server *server,
                                            SYN_KWP2000_SeedHandler seed_cb,
                                            SYN_KWP2000_KeyHandler key_cb, void *user_ctx)
{
    if (server == NULL)
        return SYN_INVALID_PARAM;
    server->seed_cb = seed_cb;
    server->key_cb = key_cb;
    server->security_ctx = user_ctx;
    return SYN_OK;
}

SYN_Status syn_kwp2000_set_routine_handler(SYN_KWP2000_Server *server,
                                           SYN_KWP2000_RoutineHandler cb, void *user_ctx)
{
    if (server == NULL)
        return SYN_INVALID_PARAM;
    server->routine_cb = cb;
    server->routine_ctx = user_ctx;
    return SYN_OK;
}

SYN_Status syn_kwp2000_set_memory_handler(SYN_KWP2000_Server *server, SYN_KWP2000_MemoryHandler cb,
                                          void *user_ctx)
{
    if (server == NULL)
        return SYN_INVALID_PARAM;
    server->memory_cb = cb;
    server->memory_ctx = user_ctx;
    return SYN_OK;
}

/* ── Service Processing Implementation ─────────────────────────────── */

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

    uint8_t sid = req_buf[0];

    switch (sid) {
    /* 0x10 StartDiagnosticSession */
    case SYN_KWP2000_SID_START_DIAGNOSTIC_SESSION: {
        if (req_len < 2U) {
            make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
            return SYN_OK;
        }
        uint8_t target_session = req_buf[1];
        if (target_session != SYN_KWP2000_SESSION_DEFAULT &&
            target_session != SYN_KWP2000_SESSION_PROGRAMMING &&
            target_session != SYN_KWP2000_SESSION_EXTENDED) {
            make_nrc(sid, SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf, resp_len);
            return SYN_OK;
        }

        if (server->session_cb != NULL) {
            if (!server->session_cb(server->current_session, target_session, server->session_ctx)) {
                make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
                return SYN_OK;
            }
        }

        server->current_session = target_session;
        server->s3_timer_ms = 0U;
        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = target_session;
        *resp_len = 2U;
        return SYN_OK;
    }

    /* 0x11 ECUReset */
    case SYN_KWP2000_SID_ECU_RESET: {
        if (req_len < 2U) {
            make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
            return SYN_OK;
        }
        uint8_t reset_type = req_buf[1];
        if (reset_type != 0x01U && reset_type != 0x02U) {
            make_nrc(sid, SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED, resp_buf, resp_len);
            return SYN_OK;
        }

        server->pending_reset_type = reset_type;
        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = reset_type;
        *resp_len = 2U;
        return SYN_OK;
    }

    /* 0x21 ReadDataByLocalIdentifier (LID) */
    case SYN_KWP2000_SID_READ_DATA_BY_LOCAL_IDENTIFIER: {
        if (req_len < 2U) {
            make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
            return SYN_OK;
        }
        uint8_t target_lid = req_buf[1];
        const SYN_KWP2000_LID *found_lid = NULL;

        for (uint16_t i = 0U; i < server->lid_count; i++) {
            if (server->lids[i].lid == target_lid) {
                found_lid = &server->lids[i];
                break;
            }
        }

        if (found_lid == NULL) {
            make_nrc(sid, SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len);
            return SYN_OK;
        }

        if (2U + found_lid->len > max_resp_len) {
            make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
            return SYN_OK;
        }

        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = target_lid;
        memcpy(&resp_buf[2], found_lid->data, found_lid->len);
        *resp_len = 2U + found_lid->len;
        return SYN_OK;
    }

    /* 0x22 ReadDataByCommonIdentifier (CID) */
    case SYN_KWP2000_SID_READ_DATA_BY_COMMON_IDENTIFIER: {
        if (req_len < 3U) {
            make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
            return SYN_OK;
        }
        uint16_t target_cid = ((uint16_t)req_buf[1] << 8) | req_buf[2];
        const SYN_KWP2000_CID *found_cid = NULL;

        for (uint16_t i = 0U; i < server->cid_count; i++) {
            if (server->cids[i].cid == target_cid) {
                found_cid = &server->cids[i];
                break;
            }
        }

        if (found_cid == NULL) {
            make_nrc(sid, SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE, resp_buf, resp_len);
            return SYN_OK;
        }

        if (3U + found_cid->len > max_resp_len) {
            make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
            return SYN_OK;
        }

        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = (uint8_t)(target_cid >> 8);
        resp_buf[2] = (uint8_t)(target_cid & 0xFFU);
        memcpy(&resp_buf[3], found_cid->data, found_cid->len);
        *resp_len = 3U + found_cid->len;
        return SYN_OK;
    }

    /* 0x27 SecurityAccess */
    case SYN_KWP2000_SID_SECURITY_ACCESS: {
        if (req_len < 2U) {
            make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
            return SYN_OK;
        }
        uint8_t sub_func = req_buf[1];

        /* Request Seed (Odd subfunctions e.g. 0x01) */
        if ((sub_func % 2U) != 0U) {
            uint8_t seed_buf[16];
            uint16_t seed_len = sizeof(seed_buf);
            if (server->seed_cb != NULL) {
                if (!server->seed_cb(sub_func, seed_buf, &seed_len, server->security_ctx)) {
                    make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
                    return SYN_OK;
                }
            } else {
                /* Default seed */
                seed_buf[0] = 0x36U;
                seed_buf[1] = 0x37U;
                seed_len = 2U;
            }

            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub_func;
            if (2U + seed_len > max_resp_len) {
                make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
                return SYN_OK;
            }
            memcpy(&resp_buf[2], seed_buf, seed_len);
            *resp_len = 2U + seed_len;
            return SYN_OK;
        }
        /* Send Key (Even subfunctions e.g. 0x02) */
        else {
            if (req_len < 3U) {
                make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
                return SYN_OK;
            }
            const uint8_t *key = &req_buf[2];
            uint16_t key_len = req_len - 2U;

            bool key_ok = false;
            if (server->key_cb != NULL) {
                key_ok = server->key_cb(sub_func, key, key_len, server->security_ctx);
            } else {
                key_ok = (key_len >= 2U && key[0] == 0xC5U && key[1] == 0xC9U);
            }

            if (!key_ok) {
                make_nrc(sid, SYN_KWP2000_NRC_INVALID_KEY, resp_buf, resp_len);
                return SYN_OK;
            }

            server->security_unlocked = true;
            server->security_level = sub_func;
            resp_buf[0] = sid + 0x40U;
            resp_buf[1] = sub_func;
            *resp_len = 2U;
            return SYN_OK;
        }
    }

    /* 0x31 StartRoutineByLocalIdentifier */
    case SYN_KWP2000_SID_START_ROUTINE_BY_LOCAL_IDENTIFIER: {
        if (req_len < 2U) {
            make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
            return SYN_OK;
        }
        uint8_t routine_id = req_buf[1];
        const uint8_t *in_data = (req_len > 2U) ? &req_buf[2] : NULL;
        uint16_t in_len = (req_len > 2U) ? (req_len - 2U) : 0U;

        uint8_t routine_out[64];
        uint16_t out_len = 0U;
        if (server->routine_cb != NULL) {
            if (!server->routine_cb(routine_id, in_data, in_len, routine_out, sizeof(routine_out),
                                    &out_len, server->routine_ctx)) {
                make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
                return SYN_OK;
            }
        }

        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = routine_id;
        if (out_len > 0U) {
            if (2U + out_len > max_resp_len) {
                make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
                return SYN_OK;
            }
            memcpy(&resp_buf[2], routine_out, out_len);
        }
        *resp_len = 2U + out_len;
        return SYN_OK;
    }

    /* 0x34 RequestDownload */
    case SYN_KWP2000_SID_REQUEST_DOWNLOAD: {
        if (req_len < 5U) {
            make_nrc(sid, SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH, resp_buf, resp_len);
            return SYN_OK;
        }
        uint32_t addr = ((uint32_t)req_buf[1] << 24) | ((uint32_t)req_buf[2] << 16) |
                        ((uint32_t)req_buf[3] << 8) | req_buf[4];
        uint32_t size = (req_len >= 9U)
                            ? (((uint32_t)req_buf[5] << 24) | ((uint32_t)req_buf[6] << 16) |
                               ((uint32_t)req_buf[7] << 8) | req_buf[8])
                            : 0U;

        server->is_downloading = true;
        server->download_addr = addr;
        server->download_size = size;
        server->download_received = 0U;

        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = 0x20U; /* Max block length format identifier */
        *resp_len = 2U;
        return SYN_OK;
    }

    /* 0x36 TransferData */
    case SYN_KWP2000_SID_TRANSFER_DATA: {
        if (!server->is_downloading || req_len < 2U) {
            make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
            return SYN_OK;
        }
        uint8_t block_num = req_buf[1];
        const uint8_t *data = &req_buf[2];
        uint16_t data_len = req_len - 2U;

        uint32_t target_addr = server->download_addr + server->download_received;
        if (server->memory_cb != NULL) {
            if (!server->memory_cb(true, target_addr, data_len, (uint8_t *)data,
                                   server->memory_ctx)) {
                make_nrc(sid, SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT, resp_buf, resp_len);
                return SYN_OK;
            }
        }

        server->download_received += data_len;
        resp_buf[0] = sid + 0x40U;
        resp_buf[1] = block_num;
        *resp_len = 2U;
        return SYN_OK;
    }

    /* 0x37 RequestTransferExit */
    case SYN_KWP2000_SID_REQUEST_TRANSFER_EXIT: {
        server->is_downloading = false;
        resp_buf[0] = sid + 0x40U;
        *resp_len = 1U;
        return SYN_OK;
    }

    /* 0x3E TesterPresent */
    case SYN_KWP2000_SID_TESTER_PRESENT: {
        server->s3_timer_ms = 0U;
        resp_buf[0] = sid + 0x40U;
        *resp_len = 1U;
        return SYN_OK;
    }

    default:
        make_nrc(sid, SYN_KWP2000_NRC_SERVICE_NOT_SUPPORTED, resp_buf, resp_len);
        return SYN_OK;
    }
}

void syn_kwp2000_tick(SYN_KWP2000_Server *server, uint32_t dt_ms)
{
    if (server == NULL)
        return;
    server->s3_timer_ms += dt_ms;

    /* S3 session inactivity timeout (ISO 14230-3 §5.3) */
    if (server->current_session != SYN_KWP2000_SESSION_DEFAULT) {
        if (server->s3_timer_ms >= SYN_KWP2000_S3_TIMEOUT_MS) {
            server->current_session = SYN_KWP2000_SESSION_DEFAULT;
            server->security_unlocked = false;
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
```


