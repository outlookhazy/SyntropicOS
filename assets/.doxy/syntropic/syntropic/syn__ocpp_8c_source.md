

# File syn\_ocpp.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ocpp.c**](syn__ocpp_8c.md)

[Go to the documentation of this file](syn__ocpp_8c.md)


```C++


#include "syn_ocpp.h"

#include <stdio.h>
#include <string.h>

static const char *status_to_str(SYN_OCPP_ChargePointStatus status)
{
    switch (status) {
    case SYN_OCPP_STATUS_AVAILABLE:
        return "Available";
    case SYN_OCPP_STATUS_PREPARING:
        return "Preparing";
    case SYN_OCPP_STATUS_CHARGING:
        return "Charging";
    case SYN_OCPP_STATUS_SUSPENDED_EV:
        return "SuspendedEV";
    case SYN_OCPP_STATUS_SUSPENDED_EVSE:
        return "SuspendedEVSE";
    case SYN_OCPP_STATUS_FINISHING:
        return "Finishing";
    case SYN_OCPP_STATUS_RESERVED:
        return "Reserved";
    case SYN_OCPP_STATUS_UNAVAILABLE:
        return "Unavailable";
    case SYN_OCPP_STATUS_FAULTED:
        return "Faulted";
    default:
        return "Unavailable";
    }
}


static const char *ocpp_find_sub(const char *hay, size_t hay_len, const char *needle)
{
    size_t nlen = strlen(needle);
    if (hay_len < nlen) {
        return NULL;
    }
    size_t limit = hay_len - nlen;
    for (size_t i = 0U; i <= limit; i++) {
        if (memcmp(&hay[i], needle, nlen) == 0) {
            return &hay[i];
        }
    }
    return NULL;
}

static bool ocpp_extract_msg_id(const char *buf, size_t len, char *out_id, size_t out_cap)
{
    out_id[0] = '\0';
    size_t pos = 0U;
    while (pos < len && buf[pos] != '"') {
        pos++;
    }
    if (pos >= len) {
        return false;
    }
    size_t start = pos + 1U;
    pos = start;
    while (pos < len && buf[pos] != '"') {
        pos++;
    }
    size_t id_len = pos - start;
    if (pos >= len || id_len == 0U || id_len >= out_cap) {
        return false;
    }
    memcpy(out_id, &buf[start], id_len);
    out_id[id_len] = '\0';
    return true;
}

static bool ocpp_seek_json_value(const char *buf, size_t len, const char *key, size_t *out_pos)
{
    const char *hit = ocpp_find_sub(buf, len, key);
    if (hit == NULL) {
        return false;
    }
    size_t pos = (size_t)(hit - buf) + strlen(key);
    while ((pos < len) && (buf[pos] != ':')) {
        pos++;
    }
    pos++;
    while ((pos < len) && (buf[pos] == ' ')) {
        pos++;
    }
    *out_pos = pos;
    return true;
}

static bool ocpp_extract_json_str(const char *buf, size_t len, const char *key, char *out_val,
                                  size_t out_cap)
{
    out_val[0] = '\0';
    size_t pos = 0U;
    if (!ocpp_seek_json_value(buf, len, key, &pos)) {
        return false;
    }
    if ((pos >= len) || (buf[pos] != '"')) {
        return false;
    }
    size_t start = pos + 1U;
    pos = start;
    while ((pos < len) && (buf[pos] != '"')) {
        pos++;
    }
    if (pos >= len) {
        return false;
    }
    size_t copy_len = pos - start;
    if (copy_len >= out_cap) {
        copy_len = out_cap - 1U;
    }
    memcpy(out_val, &buf[start], copy_len);
    out_val[copy_len] = '\0';
    return true;
}

static bool ocpp_extract_json_i32(const char *buf, size_t len, const char *key, int32_t *out_val)
{
    size_t pos = 0U;
    if (!ocpp_seek_json_value(buf, len, key, &pos)) {
        return false;
    }
    bool neg = false;
    if ((pos < len) && (buf[pos] == '-')) {
        neg = true;
        pos++;
    }
    uint64_t acc = 0ULL;
    size_t digits = 0U;
    const uint64_t limit = neg ? 2147483648ULL : 2147483647ULL;
    while ((pos < len) && (buf[pos] >= '0') && (buf[pos] <= '9')) {
        acc = (acc * 10ULL) + (uint64_t)(uint8_t)(buf[pos] - '0');
        if (acc > limit) {
            return false;
        }
        pos++;
        digits++;
    }
    if (digits == 0U) {
        return false;
    }
    *out_val = neg ? (int32_t)(0LL - (int64_t)acc) : (int32_t)acc;
    return true;
}

static size_t ocpp_escape_char(uint8_t ch, char out_seq[6])
{
    static const char hex[16] = "0123456789abcdef";
    if ((ch == (uint8_t)'"') || (ch == (uint8_t)'\\')) {
        out_seq[0] = '\\';
        out_seq[1] = (char)ch;
        return 2U;
    }
    if (ch == (uint8_t)'\n') {
        out_seq[0] = '\\';
        out_seq[1] = 'n';
        return 2U;
    }
    if (ch == (uint8_t)'\r') {
        out_seq[0] = '\\';
        out_seq[1] = 'r';
        return 2U;
    }
    if (ch == (uint8_t)'\t') {
        out_seq[0] = '\\';
        out_seq[1] = 't';
        return 2U;
    }
    if (ch < 0x20U) {
        out_seq[0] = '\\';
        out_seq[1] = 'u';
        out_seq[2] = '0';
        out_seq[3] = '0';
        out_seq[4] = hex[(ch >> 4U) & 0x0FU];
        out_seq[5] = hex[ch & 0x0FU];
        return 6U;
    }
    out_seq[0] = (char)ch;
    return 1U;
}

static bool ocpp_json_escape(const char *src, const char *fallback, char *dst, size_t dst_cap)
{
    const char *in = (src != NULL) ? src : fallback;
    size_t pos = 0U;
    while (*in != '\0') {
        char seq[6];
        size_t seq_len = ocpp_escape_char((uint8_t)*in, seq);
        if (seq_len >= (dst_cap - pos)) {
            dst[pos] = '\0';
            return false;
        }
        (void)memcpy(&dst[pos], seq, seq_len);
        pos += seq_len;
        in++;
    }
    dst[pos] = '\0';
    return true;
}

static uint32_t ocpp_sec_to_ms_sat(uint32_t sec)
{
    uint64_t ms = (uint64_t)sec * 1000ULL;
    return (ms > (uint64_t)UINT32_MAX) ? UINT32_MAX : (uint32_t)ms;
}

static SYN_OCPP_RegistrationStatus ocpp_parse_reg_status(const char *s)
{
    if (strcmp(s, "Accepted") == 0) {
        return SYN_OCPP_REGISTRATION_ACCEPTED;
    }
    if (strcmp(s, "Pending") == 0) {
        return SYN_OCPP_REGISTRATION_PENDING;
    }
    return SYN_OCPP_REGISTRATION_REJECTED;
}

static const char *ocpp_reg_status_str(SYN_OCPP_RegistrationStatus st)
{
    if (st == SYN_OCPP_REGISTRATION_ACCEPTED) {
        return "Accepted";
    }
    if (st == SYN_OCPP_REGISTRATION_PENDING) {
        return "Pending";
    }
    return "Rejected";
}

static SYN_OCPP_AuthorizationStatus ocpp_parse_auth_status(const char *s)
{
    if (strcmp(s, "Accepted") == 0) {
        return SYN_OCPP_AUTH_ACCEPTED;
    }
    if (strcmp(s, "Blocked") == 0) {
        return SYN_OCPP_AUTH_BLOCKED;
    }
    if (strcmp(s, "Expired") == 0) {
        return SYN_OCPP_AUTH_EXPIRED;
    }
    return SYN_OCPP_AUTH_INVALID;
}

static const char *ocpp_auth_status_str(SYN_OCPP_AuthorizationStatus st)
{
    if (st == SYN_OCPP_AUTH_ACCEPTED) {
        return "Accepted";
    }
    if (st == SYN_OCPP_AUTH_BLOCKED) {
        return "Blocked";
    }
    if (st == SYN_OCPP_AUTH_EXPIRED) {
        return "Expired";
    }
    return "Invalid";
}

static void ocpp_set_pending(SYN_OCPP_Client *client, SYN_OCPP_PendingAction action)
{
    size_t slot = (size_t)(client->message_counter % SYN_OCPP_MAX_PENDING);
    client->pending_action = action;
    client->pending_calls[slot].action = action;
    (void)snprintf(client->pending_calls[slot].unique_id,
                   sizeof(client->pending_calls[slot].unique_id), "%u",
                   (unsigned int)client->message_counter);
    (void)snprintf(client->pending_unique_id, sizeof(client->pending_unique_id), "%s",
                   client->pending_calls[slot].unique_id);
}

static SYN_Status ocpp_finish_write(int written, size_t max_len, size_t *out_len)
{
    if (written < 0 || (size_t)written >= max_len) {
        return SYN_ERROR;
    }
    *out_len = (size_t)written;
    return SYN_OK;
}

/* ── EVSE Client Implementation ─────────────────────────────────────── */

SYN_Status syn_ocpp_init(SYN_OCPP_Client *client)
{
    if (client == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(client, 0, sizeof(*client));
    client->registration_status = SYN_OCPP_REGISTRATION_PENDING;
    client->heartbeat_interval_sec = 60U;
    client->heartbeat_timer_ms = 60000U;
    client->connector_id = 1U;
    client->connector_status = SYN_OCPP_STATUS_AVAILABLE;
    client->active_transaction_id = -1;
    client->pending_action = SYN_OCPP_ACTION_NONE;
    return SYN_OK;
}

SYN_Status syn_ocpp_set_callbacks(SYN_OCPP_Client *client, SYN_OCPP_RegistrationHandler reg_cb,
                                  SYN_OCPP_AuthorizationHandler auth_cb,
                                  SYN_OCPP_StartTxHandler start_tx_cb,
                                  SYN_OCPP_RemoteStartHandler remote_start_cb,
                                  SYN_OCPP_RemoteStopHandler remote_stop_cb, void *user_ctx)
{
    if (client == NULL) {
        return SYN_INVALID_PARAM;
    }
    client->reg_cb = reg_cb;
    client->auth_cb = auth_cb;
    client->start_tx_cb = start_tx_cb;
    client->remote_start_cb = remote_start_cb;
    client->remote_stop_cb = remote_stop_cb;
    client->user_ctx = user_ctx;
    return SYN_OK;
}

SYN_Status syn_ocpp_format_boot_notification(SYN_OCPP_Client *client,
                                             const SYN_OCPP_ChargePointInfo *info, char *out_buf,
                                             size_t max_len, size_t *out_len)
{
    if (client == NULL || info == NULL || out_buf == NULL || out_len == NULL || max_len < 64U) {
        return SYN_INVALID_PARAM;
    }

    client->message_counter++;
    char v_esc[96];
    char m_esc[96];
    char s_esc[96];
    char f_esc[96];
    bool esc_ok =
        ocpp_json_escape(info->charge_point_vendor, "SyntropicOS", v_esc, sizeof(v_esc)) &&
        ocpp_json_escape(info->charge_point_model, "EVSE-v1", m_esc, sizeof(m_esc)) &&
        ocpp_json_escape(info->serial_number, "SN-12345", s_esc, sizeof(s_esc)) &&
        ocpp_json_escape(info->firmware_version, "1.0.0", f_esc, sizeof(f_esc));
    int written = esc_ok
                      ? snprintf(out_buf, max_len,
                                 "[2,\"%u\",\"BootNotification\",{"
                                 "\"chargePointVendor\":\"%s\","
                                 "\"chargePointModel\":\"%s\","
                                 "\"chargePointSerialNumber\":\"%s\","
                                 "\"firmwareVersion\":\"%s\"}]",
                                 (unsigned int)client->message_counter, v_esc, m_esc, s_esc, f_esc)
                      : -1;

    if (ocpp_finish_write(written, max_len, out_len) != SYN_OK) {
        return SYN_ERROR;
    }
    ocpp_set_pending(client, SYN_OCPP_ACTION_BOOT_NOTIFICATION);
    return SYN_OK;
}

SYN_Status syn_ocpp_format_heartbeat(SYN_OCPP_Client *client, char *out_buf, size_t max_len,
                                     size_t *out_len)
{
    if (client == NULL || out_buf == NULL || out_len == NULL || max_len < 22U) {
        return SYN_INVALID_PARAM;
    }

    client->message_counter++;
    int written = snprintf(out_buf, max_len, "[2,\"%u\",\"Heartbeat\",{}]",
                           (unsigned int)client->message_counter);

    if (ocpp_finish_write(written, max_len, out_len) != SYN_OK) {
        return SYN_ERROR;
    }
    ocpp_set_pending(client, SYN_OCPP_ACTION_HEARTBEAT);
    return SYN_OK;
}

SYN_Status syn_ocpp_format_status_notification(SYN_OCPP_Client *client, uint32_t connector_id,
                                               SYN_OCPP_ChargePointStatus status,
                                               const char *error_code, char *out_buf,
                                               size_t max_len, size_t *out_len)
{
    if (client == NULL || out_buf == NULL || out_len == NULL || max_len < 64U) {
        return SYN_INVALID_PARAM;
    }

    client->connector_id = connector_id;
    client->connector_status = status;
    client->message_counter++;

    char err_esc[128];
    bool esc_ok = ocpp_json_escape(error_code, "NoError", err_esc, sizeof(err_esc));
    int written = esc_ok ? snprintf(out_buf, max_len,
                                    "[2,\"%u\",\"StatusNotification\",{"
                                    "\"connectorId\":%u,"
                                    "\"status\":\"%s\","
                                    "\"errorCode\":\"%s\"}]",
                                    (unsigned int)client->message_counter,
                                    (unsigned int)connector_id, status_to_str(status), err_esc)
                         : -1;

    if (ocpp_finish_write(written, max_len, out_len) != SYN_OK) {
        return SYN_ERROR;
    }
    ocpp_set_pending(client, SYN_OCPP_ACTION_STATUS_NOTIFY);
    return SYN_OK;
}

SYN_Status syn_ocpp_format_authorize(SYN_OCPP_Client *client, const char *id_tag, char *out_buf,
                                     size_t max_len, size_t *out_len)
{
    if (client == NULL || id_tag == NULL || out_buf == NULL || out_len == NULL || max_len < 48U) {
        return SYN_INVALID_PARAM;
    }

    (void)snprintf(client->active_id_tag, sizeof(client->active_id_tag), "%s", id_tag);
    client->message_counter++;
    char tag_esc[128];
    bool esc_ok = ocpp_json_escape(id_tag, "", tag_esc, sizeof(tag_esc));
    int written = esc_ok ? snprintf(out_buf, max_len, "[2,\"%u\",\"Authorize\",{\"idTag\":\"%s\"}]",
                                    (unsigned int)client->message_counter, tag_esc)
                         : -1;

    if (ocpp_finish_write(written, max_len, out_len) != SYN_OK) {
        return SYN_ERROR;
    }
    ocpp_set_pending(client, SYN_OCPP_ACTION_AUTHORIZE);
    return SYN_OK;
}

SYN_Status syn_ocpp_format_start_transaction(SYN_OCPP_Client *client, uint32_t connector_id,
                                             const char *id_tag, uint32_t meter_start_wh,
                                             char *out_buf, size_t max_len, size_t *out_len)
{
    if (client == NULL || id_tag == NULL || out_buf == NULL || out_len == NULL || max_len < 96U) {
        return SYN_INVALID_PARAM;
    }

    client->tx_start_energy_wh = meter_start_wh;
    (void)snprintf(client->active_id_tag, sizeof(client->active_id_tag), "%s", id_tag);
    client->message_counter++;

    char tag_esc[128];
    bool esc_ok = ocpp_json_escape(id_tag, "", tag_esc, sizeof(tag_esc));
    int written = esc_ok
                      ? snprintf(out_buf, max_len,
                                 "[2,\"%u\",\"StartTransaction\",{"
                                 "\"connectorId\":%u,"
                                 "\"idTag\":\"%s\","
                                 "\"meterStart\":%u,"
                                 "\"timestamp\":\"2026-08-06T12:00:00Z\"}]",
                                 (unsigned int)client->message_counter, (unsigned int)connector_id,
                                 tag_esc, (unsigned int)meter_start_wh)
                      : -1;

    if (ocpp_finish_write(written, max_len, out_len) != SYN_OK) {
        return SYN_ERROR;
    }
    ocpp_set_pending(client, SYN_OCPP_ACTION_START_TRANSACTION);
    return SYN_OK;
}

SYN_Status syn_ocpp_format_stop_transaction(SYN_OCPP_Client *client, int32_t transaction_id,
                                            uint32_t meter_stop_wh, const char *reason,
                                            char *out_buf, size_t max_len, size_t *out_len)
{
    if (client == NULL || out_buf == NULL || out_len == NULL || max_len < 96U) {
        return SYN_INVALID_PARAM;
    }

    client->message_counter++;
    char rsn_esc[128];
    bool esc_ok = ocpp_json_escape(reason, "Local", rsn_esc, sizeof(rsn_esc));
    int written = esc_ok ? snprintf(out_buf, max_len,
                                    "[2,\"%u\",\"StopTransaction\",{"
                                    "\"transactionId\":%d,"
                                    "\"meterStop\":%u,"
                                    "\"timestamp\":\"2026-08-06T12:30:00Z\","
                                    "\"reason\":\"%s\"}]",
                                    (unsigned int)client->message_counter, (int)transaction_id,
                                    (unsigned int)meter_stop_wh, rsn_esc)
                         : -1;

    if (ocpp_finish_write(written, max_len, out_len) != SYN_OK) {
        return SYN_ERROR;
    }
    client->active_transaction_id = -1;
    ocpp_set_pending(client, SYN_OCPP_ACTION_STOP_TRANSACTION);
    return SYN_OK;
}

SYN_Status syn_ocpp_format_meter_values(SYN_OCPP_Client *client, uint32_t connector_id,
                                        const SYN_OCPP_MeterValues *values, char *out_buf,
                                        size_t max_len, size_t *out_len)
{
    if (client == NULL || values == NULL || out_buf == NULL || out_len == NULL || max_len < 128U) {
        return SYN_INVALID_PARAM;
    }

    client->message_counter++;
    int written =
        snprintf(out_buf, max_len,
                 "[2,\"%u\",\"MeterValues\",{"
                 "\"connectorId\":%u,"
                 "\"transactionId\":%d,"
                 "\"meterValue\":[{"
                 "\"timestamp\":\"2026-08-06T12:15:00Z\","
                 "\"sampledValue\":["
                 "{\"value\":\"%u\",\"unit\":\"Wh\"},"
                 "{\"value\":\"%u\",\"unit\":\"V\"},"
                 "{\"value\":\"%u.%u\",\"unit\":\"A\"},"
                 "{\"value\":\"%u\",\"unit\":\"Percent\"}]}]}]",
                 (unsigned int)client->message_counter, (unsigned int)connector_id,
                 (int)client->active_transaction_id, (unsigned int)values->energy_wh,
                 (unsigned int)values->voltage_v, (unsigned int)(values->current_a / 10U),
                 (unsigned int)(values->current_a % 10U), (unsigned int)values->soc_percent);

    if (ocpp_finish_write(written, max_len, out_len) != SYN_OK) {
        return SYN_ERROR;
    }
    ocpp_set_pending(client, SYN_OCPP_ACTION_METER_VALUES);
    return SYN_OK;
}

static void ocpp_dispatch_call_result(SYN_OCPP_Client *client, SYN_OCPP_PendingAction action,
                                      const char *in_buf, size_t in_len)
{
    char status_str[24];
    (void)ocpp_extract_json_str(in_buf, in_len, "\"status\"", status_str, sizeof(status_str));

    if (action == SYN_OCPP_ACTION_BOOT_NOTIFICATION) {
        SYN_OCPP_RegistrationStatus reg_st = ocpp_parse_reg_status(status_str);
        int32_t iv = 0;
        if (ocpp_extract_json_i32(in_buf, in_len, "\"interval\"", &iv) && iv > 0) {
            client->heartbeat_interval_sec = (uint32_t)iv;
            client->heartbeat_timer_ms = ocpp_sec_to_ms_sat((uint32_t)iv);
        }
        client->registration_status = reg_st;
        if (client->reg_cb != NULL) {
            client->reg_cb(reg_st, client->heartbeat_interval_sec, client->user_ctx);
        }
        return;
    }

    if (action == SYN_OCPP_ACTION_AUTHORIZE) {
        SYN_OCPP_AuthorizationStatus auth_st = ocpp_parse_auth_status(status_str);
        if (client->auth_cb != NULL) {
            client->auth_cb(client->active_id_tag, auth_st, client->user_ctx);
        }
        return;
    }

    if (action == SYN_OCPP_ACTION_START_TRANSACTION) {
        SYN_OCPP_AuthorizationStatus auth_st = ocpp_parse_auth_status(status_str);
        int32_t tx_id = -1;
        (void)ocpp_extract_json_i32(in_buf, in_len, "\"transactionId\"", &tx_id);
        if (auth_st == SYN_OCPP_AUTH_ACCEPTED && tx_id > 0) {
            client->active_transaction_id = tx_id;
        }
        if (client->start_tx_cb != NULL) {
            client->start_tx_cb(tx_id, auth_st, client->user_ctx);
        }
    }
}

static SYN_Status ocpp_client_handle_call(SYN_OCPP_Client *client, const char *in_buf,
                                          size_t in_len, char *out_resp, size_t max_resp_len,
                                          size_t *out_resp_len)
{
    char msg_id[32];
    if (!ocpp_extract_msg_id(in_buf, in_len, msg_id, sizeof(msg_id))) {
        (void)snprintf(msg_id, sizeof(msg_id), "1");
    }
    char id_esc[192];
    (void)ocpp_json_escape(msg_id, "1", id_esc, sizeof(id_esc));

    if (ocpp_find_sub(in_buf, in_len, "\"RemoteStartTransaction\"") != NULL) {
        char id_tag[32];
        bool has_tag = ocpp_extract_json_str(in_buf, in_len, "\"idTag\"", id_tag, sizeof(id_tag));
        int32_t conn_id = (int32_t)client->connector_id;
        (void)ocpp_extract_json_i32(in_buf, in_len, "\"connectorId\"", &conn_id);

        bool ok = false;
        if (has_tag && id_tag[0] != '\0' && conn_id >= 0) {
            ok = (client->remote_start_cb != NULL)
                     ? client->remote_start_cb((uint32_t)conn_id, id_tag, client->user_ctx)
                     : true;
        }
        int written = snprintf(out_resp, max_resp_len, "[3,\"%s\",{\"status\":\"%s\"}]", id_esc,
                               ok ? "Accepted" : "Rejected");
        return ocpp_finish_write(written, max_resp_len, out_resp_len);
    }

    if (ocpp_find_sub(in_buf, in_len, "\"RemoteStopTransaction\"") != NULL) {
        int32_t req_tx_id = -1;
        bool has_tx = ocpp_extract_json_i32(in_buf, in_len, "\"transactionId\"", &req_tx_id);
        bool ok = false;
        if (has_tx) {
            ok = (client->remote_stop_cb != NULL)
                     ? client->remote_stop_cb(req_tx_id, client->user_ctx)
                     : true;
        }
        int written = snprintf(out_resp, max_resp_len, "[3,\"%s\",{\"status\":\"%s\"}]", id_esc,
                               ok ? "Accepted" : "Rejected");
        return ocpp_finish_write(written, max_resp_len, out_resp_len);
    }

    int written = snprintf(out_resp, max_resp_len, "[3,\"%s\",{}]", id_esc);
    return ocpp_finish_write(written, max_resp_len, out_resp_len);
}

static void ocpp_sync_pending_summary(SYN_OCPP_Client *client)
{
    client->pending_action = SYN_OCPP_ACTION_NONE;
    client->pending_unique_id[0] = '\0';
    for (size_t i = 0U; i < SYN_OCPP_MAX_PENDING; i++) {
        if (client->pending_calls[i].action != SYN_OCPP_ACTION_NONE) {
            client->pending_action = client->pending_calls[i].action;
            (void)snprintf(client->pending_unique_id, sizeof(client->pending_unique_id), "%s",
                           client->pending_calls[i].unique_id);
            return;
        }
    }
}

static void ocpp_client_handle_result(SYN_OCPP_Client *client, uint8_t msg_type, const char *in_buf,
                                      size_t in_len)
{
    char msg_id[32];
    if (!ocpp_extract_msg_id(in_buf, in_len, msg_id, sizeof(msg_id))) {
        return;
    }
    for (size_t i = 0U; i < SYN_OCPP_MAX_PENDING; i++) {
        if (client->pending_calls[i].action != SYN_OCPP_ACTION_NONE &&
            strcmp(msg_id, client->pending_calls[i].unique_id) == 0) {
            SYN_OCPP_PendingAction action = client->pending_calls[i].action;
            client->pending_calls[i].action = SYN_OCPP_ACTION_NONE;
            client->pending_calls[i].unique_id[0] = '\0';
            if (strcmp(msg_id, client->pending_unique_id) == 0) {
                ocpp_sync_pending_summary(client);
            }
            if (msg_type == SYN_OCPP_MSG_TYPE_CALLRESULT) {
                ocpp_dispatch_call_result(client, action, in_buf, in_len);
            }
            return;
        }
    }
}

SYN_Status syn_ocpp_process_message(SYN_OCPP_Client *client, const char *in_buf, size_t in_len,
                                    char *out_resp, size_t max_resp_len, size_t *out_resp_len)
{
    if (client == NULL || in_buf == NULL || in_len < 5U) {
        return SYN_INVALID_PARAM;
    }

    if (out_resp_len != NULL) {
        *out_resp_len = 0U;
    }
    if (in_buf[0] != '[') {
        return SYN_ERROR;
    }

    uint8_t msg_type = (uint8_t)(in_buf[1] - '0');

    if (msg_type == SYN_OCPP_MSG_TYPE_CALLRESULT || msg_type == SYN_OCPP_MSG_TYPE_CALLERROR) {
        ocpp_client_handle_result(client, msg_type, in_buf, in_len);
        return SYN_OK;
    }

    if (msg_type == SYN_OCPP_MSG_TYPE_CALL && out_resp != NULL && out_resp_len != NULL &&
        max_resp_len >= 32U) {
        return ocpp_client_handle_call(client, in_buf, in_len, out_resp, max_resp_len,
                                       out_resp_len);
    }

    return SYN_OK;
}

void syn_ocpp_tick(SYN_OCPP_Client *client, uint32_t dt_ms, char *out_hb_buf, size_t max_len,
                   size_t *out_len)
{
    if (client == NULL) {
        return;
    }
    if (out_len != NULL) {
        *out_len = 0U;
    }

    if (client->heartbeat_timer_ms > dt_ms) {
        client->heartbeat_timer_ms -= dt_ms;
    } else {
        client->heartbeat_timer_ms = (client->heartbeat_interval_sec > 0U)
                                         ? ocpp_sec_to_ms_sat(client->heartbeat_interval_sec)
                                         : 60000U;

        if (out_hb_buf != NULL && out_len != NULL && max_len >= 32U) {
            (void)syn_ocpp_format_heartbeat(client, out_hb_buf, max_len, out_len);
        }
    }
}

/* ── CSMS Server Implementation ─────────────────────────────────────── */

SYN_Status syn_ocpp_server_init(SYN_OCPP_Server *server)
{
    if (server == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(server, 0, sizeof(*server));
    server->next_transaction_id = 1000;
    return SYN_OK;
}

SYN_Status syn_ocpp_server_set_callbacks(SYN_OCPP_Server *server,
                                         SYN_OCPP_ServerBootHandler boot_cb,
                                         SYN_OCPP_ServerAuthorizeHandler auth_cb,
                                         SYN_OCPP_ServerStartTxHandler start_tx_cb, void *user_ctx)
{
    if (server == NULL) {
        return SYN_INVALID_PARAM;
    }
    server->boot_cb = boot_cb;
    server->auth_cb = auth_cb;
    server->start_tx_cb = start_tx_cb;
    server->user_ctx = user_ctx;
    return SYN_OK;
}

SYN_Status syn_ocpp_server_format_remote_start(SYN_OCPP_Server *server, uint32_t connector_id,
                                               const char *id_tag, char *out_buf, size_t max_len,
                                               size_t *out_len)
{
    if (server == NULL || id_tag == NULL || out_buf == NULL || out_len == NULL || max_len < 64U) {
        return SYN_INVALID_PARAM;
    }

    server->message_counter++;
    char tag_esc[128];
    bool esc_ok = ocpp_json_escape(id_tag, "", tag_esc, sizeof(tag_esc));
    int written = esc_ok ? snprintf(out_buf, max_len,
                                    "[2,\"%u\",\"RemoteStartTransaction\",{"
                                    "\"connectorId\":%u,"
                                    "\"idTag\":\"%s\"}]",
                                    (unsigned int)server->message_counter,
                                    (unsigned int)connector_id, tag_esc)
                         : -1;

    return ocpp_finish_write(written, max_len, out_len);
}

SYN_Status syn_ocpp_server_format_remote_stop(SYN_OCPP_Server *server, int32_t transaction_id,
                                              char *out_buf, size_t max_len, size_t *out_len)
{
    if (server == NULL || out_buf == NULL || out_len == NULL || max_len < 48U) {
        return SYN_INVALID_PARAM;
    }

    server->message_counter++;
    int written = snprintf(out_buf, max_len,
                           "[2,\"%u\",\"RemoteStopTransaction\",{"
                           "\"transactionId\":%d}]",
                           (unsigned int)server->message_counter, (int)transaction_id);

    return ocpp_finish_write(written, max_len, out_len);
}

static SYN_Status ocpp_server_handle_boot(SYN_OCPP_Server *server, const char *msg_id,
                                          const char *in_buf, size_t in_len, char *out_resp,
                                          size_t max_resp_len, size_t *out_resp_len)
{
    char vendor[32];
    char model[32];
    char serial[32];
    char fw[32];
    (void)ocpp_extract_json_str(in_buf, in_len, "\"chargePointVendor\"", vendor, sizeof(vendor));
    (void)ocpp_extract_json_str(in_buf, in_len, "\"chargePointModel\"", model, sizeof(model));
    (void)ocpp_extract_json_str(in_buf, in_len, "\"chargePointSerialNumber\"", serial,
                                sizeof(serial));
    (void)ocpp_extract_json_str(in_buf, in_len, "\"firmwareVersion\"", fw, sizeof(fw));

    SYN_OCPP_RegistrationStatus st = SYN_OCPP_REGISTRATION_ACCEPTED;
    uint32_t interval = 60U;
    if (server->boot_cb != NULL) {
        SYN_OCPP_ChargePointInfo info = {vendor, model, serial, fw};
        st = server->boot_cb(&info, &interval, server->user_ctx);
    }
    int written = snprintf(out_resp, max_resp_len,
                           "[3,\"%s\",{"
                           "\"status\":\"%s\","
                           "\"currentTime\":\"2026-08-06T12:00:00Z\","
                           "\"interval\":%u}]",
                           msg_id, ocpp_reg_status_str(st), (unsigned int)interval);
    return ocpp_finish_write(written, max_resp_len, out_resp_len);
}

static SYN_Status ocpp_server_handle_start_tx(SYN_OCPP_Server *server, const char *msg_id,
                                              const char *in_buf, size_t in_len, char *out_resp,
                                              size_t max_resp_len, size_t *out_resp_len)
{
    char id_tag[32];
    bool has_tag = ocpp_extract_json_str(in_buf, in_len, "\"idTag\"", id_tag, sizeof(id_tag));
    int32_t conn_id = 1;
    int32_t meter_start = 0;
    (void)ocpp_extract_json_i32(in_buf, in_len, "\"connectorId\"", &conn_id);
    (void)ocpp_extract_json_i32(in_buf, in_len, "\"meterStart\"", &meter_start);

    if (!has_tag || id_tag[0] == '\0' || conn_id < 0 || meter_start < 0) {
        int written = snprintf(out_resp, max_resp_len,
                               "[3,\"%s\",{\"transactionId\":0,"
                               "\"idTagInfo\":{\"status\":\"Invalid\"}}]",
                               msg_id);
        return ocpp_finish_write(written, max_resp_len, out_resp_len);
    }

    int32_t tx_id = server->next_transaction_id++;
    if (server->start_tx_cb != NULL) {
        int32_t user_tx =
            server->start_tx_cb((uint32_t)conn_id, id_tag, (uint32_t)meter_start, server->user_ctx);
        if (user_tx > 0) {
            tx_id = user_tx;
        }
    }
    int written = snprintf(out_resp, max_resp_len,
                           "[3,\"%s\",{"
                           "\"transactionId\":%d,"
                           "\"idTagInfo\":{\"status\":\"Accepted\"}}]",
                           msg_id, (int)tx_id);
    return ocpp_finish_write(written, max_resp_len, out_resp_len);
}

static SYN_Status ocpp_server_handle_auth(SYN_OCPP_Server *server, const char *msg_id,
                                          const char *in_buf, size_t in_len, char *out_resp,
                                          size_t max_resp_len, size_t *out_resp_len)
{
    char id_tag[32];
    bool has_tag = ocpp_extract_json_str(in_buf, in_len, "\"idTag\"", id_tag, sizeof(id_tag));
    SYN_OCPP_AuthorizationStatus auth_st = SYN_OCPP_AUTH_INVALID;
    if (has_tag && id_tag[0] != '\0') {
        auth_st = (server->auth_cb != NULL) ? server->auth_cb(id_tag, server->user_ctx)
                                            : SYN_OCPP_AUTH_ACCEPTED;
    }
    int written = snprintf(out_resp, max_resp_len, "[3,\"%s\",{\"idTagInfo\":{\"status\":\"%s\"}}]",
                           msg_id, ocpp_auth_status_str(auth_st));
    return ocpp_finish_write(written, max_resp_len, out_resp_len);
}

SYN_Status syn_ocpp_server_process_message(SYN_OCPP_Server *server, const char *in_buf,
                                           size_t in_len, char *out_resp, size_t max_resp_len,
                                           size_t *out_resp_len)
{
    if (server == NULL || in_buf == NULL || out_resp == NULL || out_resp_len == NULL ||
        in_len < 5U || max_resp_len < 64U) {
        return SYN_INVALID_PARAM;
    }

    *out_resp_len = 0U;
    if (in_buf[0] != '[') {
        return SYN_ERROR;
    }

    uint8_t msg_type = (uint8_t)(in_buf[1] - '0');
    if (msg_type != SYN_OCPP_MSG_TYPE_CALL) {
        return SYN_OK;
    }

    char msg_id[32];
    if (!ocpp_extract_msg_id(in_buf, in_len, msg_id, sizeof(msg_id))) {
        (void)snprintf(msg_id, sizeof(msg_id), "1");
    }
    char id_esc[192];
    (void)ocpp_json_escape(msg_id, "1", id_esc, sizeof(id_esc));

    if (ocpp_find_sub(in_buf, in_len, "\"BootNotification\"") != NULL) {
        return ocpp_server_handle_boot(server, id_esc, in_buf, in_len, out_resp, max_resp_len,
                                       out_resp_len);
    }

    if (ocpp_find_sub(in_buf, in_len, "\"Authorize\"") != NULL) {
        return ocpp_server_handle_auth(server, id_esc, in_buf, in_len, out_resp, max_resp_len,
                                       out_resp_len);
    }

    if (ocpp_find_sub(in_buf, in_len, "\"StartTransaction\"") != NULL) {
        return ocpp_server_handle_start_tx(server, id_esc, in_buf, in_len, out_resp, max_resp_len,
                                           out_resp_len);
    }

    int written = snprintf(out_resp, max_resp_len, "[3,\"%s\",{}]", id_esc);
    return ocpp_finish_write(written, max_resp_len, out_resp_len);
}

SYN_Status syn_ocpp_format_display_message(SYN_OCPP_Client *client,
                                           const SYN_OCPP_DisplayMessage *msg, char *out_buf,
                                           size_t max_len, size_t *out_len)
{
    if (client == NULL || msg == NULL || out_buf == NULL || out_len == NULL || max_len < 32U) {
        return SYN_INVALID_PARAM;
    }

    uint32_t msg_id = ++client->message_counter;
    char hdr_esc[192];
    char cnt_esc[192];
    bool esc_ok = ocpp_json_escape(msg->header, "", hdr_esc, sizeof(hdr_esc)) &&
                  ocpp_json_escape(msg->content, "", cnt_esc, sizeof(cnt_esc));
    int written =
        esc_ok ? snprintf(out_buf, max_len,
                          "[2,\"%u\",\"DisplayMessage\",{\"message\":{\"id\":%u,\"header\":\"%s\","
                          "\"content\":\"%s\",\"displayTimeSec\":%u}}]",
                          (unsigned int)msg_id, (unsigned int)msg->message_id, hdr_esc, cnt_esc,
                          (unsigned int)msg->display_time_s)
               : -1;

    if (ocpp_finish_write(written, max_len, out_len) != SYN_OK) {
        return SYN_ERROR;
    }
    ocpp_set_pending(client, SYN_OCPP_ACTION_DISPLAY_MESSAGE);
    return SYN_OK;
}

SYN_Status syn_ocpp_format_v2g_energy_transfer(SYN_OCPP_Client *client, SYN_OCPP_V2GMode mode,
                                               int32_t power_limit_w, char *out_buf, size_t max_len,
                                               size_t *out_len)
{
    if (client == NULL || out_buf == NULL || out_len == NULL || max_len < 32U) {
        return SYN_INVALID_PARAM;
    }

    client->v2g_mode = mode;
    uint32_t msg_id = ++client->message_counter;
    const char *mode_str = (mode == SYN_OCPP_V2G_SCHEDULED) ? "Scheduled" : "Dynamic";

    int written = snprintf(out_buf, max_len,
                           "[2,\"%u\",\"V2GEnergyTransfer\",{\"v2gMode\":\"%s\","
                           "\"powerLimitW\":%d}]",
                           (unsigned int)msg_id, mode_str, (int)power_limit_w);

    if (ocpp_finish_write(written, max_len, out_len) != SYN_OK) {
        return SYN_ERROR;
    }
    ocpp_set_pending(client, SYN_OCPP_ACTION_V2G_TRANSFER);
    return SYN_OK;
}

SYN_Status syn_ocpp_format_call_error(const char *msg_id, const char *error_code,
                                      const char *error_description, char *out_buf, size_t max_len,
                                      size_t *out_len)
{
    if (msg_id == NULL || error_code == NULL || out_buf == NULL || out_len == NULL ||
        max_len < 32U) {
        return SYN_INVALID_PARAM;
    }

    char id_esc[128];
    char code_esc[128];
    char desc_esc[192];
    bool esc_ok = ocpp_json_escape(msg_id, "", id_esc, sizeof(id_esc)) &&
                  ocpp_json_escape(error_code, "", code_esc, sizeof(code_esc)) &&
                  ocpp_json_escape(error_description, "", desc_esc, sizeof(desc_esc));
    int written = esc_ok ? snprintf(out_buf, max_len, "[4,\"%s\",\"%s\",\"%s\",{}]", id_esc,
                                    code_esc, desc_esc)
                         : -1;

    return ocpp_finish_write(written, max_len, out_len);
}

```


