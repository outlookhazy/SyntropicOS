

# File syn\_ocpp.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ocpp.c**](syn__ocpp_8c.md)

[Go to the documentation of this file](syn__ocpp_8c.md)


```C++

#include "syn_ocpp.h"

#include <stdio.h>
#include <stdlib.h>
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

/* ── EVSE Client Implementation ─────────────────────────────────────── */

SYN_Status syn_ocpp_init(SYN_OCPP_Client *client)
{
    if (client == NULL)
        return SYN_INVALID_PARAM;
    memset(client, 0, sizeof(*client));
    client->registration_status = SYN_OCPP_REGISTRATION_PENDING;
    client->heartbeat_interval_sec = 60U;
    client->heartbeat_timer_ms = 60000U;
    client->connector_id = 1U;
    client->connector_status = SYN_OCPP_STATUS_AVAILABLE;
    client->active_transaction_id = -1;
    return SYN_OK;
}

SYN_Status syn_ocpp_set_callbacks(SYN_OCPP_Client *client, SYN_OCPP_RegistrationHandler reg_cb,
                                  SYN_OCPP_AuthorizationHandler auth_cb,
                                  SYN_OCPP_StartTxHandler start_tx_cb,
                                  SYN_OCPP_RemoteStartHandler remote_start_cb,
                                  SYN_OCPP_RemoteStopHandler remote_stop_cb, void *user_ctx)
{
    if (client == NULL)
        return SYN_INVALID_PARAM;
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
    int written = snprintf(out_buf, max_len,
                           "[2,\"%u\",\"BootNotification\",{"
                           "\"chargePointVendor\":\"%s\","
                           "\"chargePointModel\":\"%s\","
                           "\"chargePointSerialNumber\":\"%s\","
                           "\"firmwareVersion\":\"%s\"}]",
                           (unsigned int)client->message_counter,
                           info->charge_point_vendor ? info->charge_point_vendor : "SyntropicOS",
                           info->charge_point_model ? info->charge_point_model : "EVSE-v1",
                           info->serial_number ? info->serial_number : "SN-12345",
                           info->firmware_version ? info->firmware_version : "1.0.0");

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;
    *out_len = (size_t)written;
    return SYN_OK;
}

SYN_Status syn_ocpp_format_heartbeat(SYN_OCPP_Client *client, char *out_buf, size_t max_len,
                                     size_t *out_len)
{
    if (client == NULL || out_buf == NULL || out_len == NULL || max_len < 32U) {
        return SYN_INVALID_PARAM;
    }

    client->message_counter++;
    int written = snprintf(out_buf, max_len, "[2,\"%u\",\"Heartbeat\",{}]",
                           (unsigned int)client->message_counter);

    *out_len = (size_t)written;
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

    int written = snprintf(out_buf, max_len,
                           "[2,\"%u\",\"StatusNotification\",{"
                           "\"connectorId\":%u,"
                           "\"status\":\"%s\","
                           "\"errorCode\":\"%s\"}]",
                           (unsigned int)client->message_counter, (unsigned int)connector_id,
                           status_to_str(status), error_code ? error_code : "NoError");

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;
    *out_len = (size_t)written;
    return SYN_OK;
}

SYN_Status syn_ocpp_format_authorize(SYN_OCPP_Client *client, const char *id_tag, char *out_buf,
                                     size_t max_len, size_t *out_len)
{
    if (client == NULL || id_tag == NULL || out_buf == NULL || out_len == NULL || max_len < 48U) {
        return SYN_INVALID_PARAM;
    }

    client->message_counter++;
    int written = snprintf(out_buf, max_len, "[2,\"%u\",\"Authorize\",{\"idTag\":\"%s\"}]",
                           (unsigned int)client->message_counter, id_tag);

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;
    *out_len = (size_t)written;
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
    snprintf(client->active_id_tag, sizeof(client->active_id_tag), "%s", id_tag);
    client->message_counter++;

    int written = snprintf(out_buf, max_len,
                           "[2,\"%u\",\"StartTransaction\",{"
                           "\"connectorId\":%u,"
                           "\"idTag\":\"%s\","
                           "\"meterStart\":%u,"
                           "\"timestamp\":\"2026-08-06T12:00:00Z\"}]",
                           (unsigned int)client->message_counter, (unsigned int)connector_id,
                           id_tag, (unsigned int)meter_start_wh);

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;
    *out_len = (size_t)written;
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
    int written = snprintf(out_buf, max_len,
                           "[2,\"%u\",\"StopTransaction\",{"
                           "\"transactionId\":%d,"
                           "\"meterStop\":%u,"
                           "\"timestamp\":\"2026-08-06T12:30:00Z\","
                           "\"reason\":\"%s\"}]",
                           (unsigned int)client->message_counter, (int)transaction_id,
                           (unsigned int)meter_stop_wh, reason ? reason : "Local");

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;
    *out_len = (size_t)written;
    client->active_transaction_id = -1;
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
    int written = snprintf(out_buf, max_len,
                           "[2,\"%u\",\"MeterValues\",{"
                           "\"connectorId\":%u,"
                           "\"transactionId\":%d,"
                           "\"meterValue\":[{"
                           "\"timestamp\":\"2026-08-06T12:15:00Z\","
                           "\"sampledValue\":["
                           "{\"value\":\"%u\",\"unit\":\"Wh\"},"
                           "{\"value\":\"%u\",\"unit\":\"V\"},"
                           "{\"value\":\"%u\",\"unit\":\"A\"},"
                           "{\"value\":\"%u\",\"unit\":\"Percent\"}]}]}]",
                           (unsigned int)client->message_counter, (unsigned int)connector_id,
                           (int)client->active_transaction_id, (unsigned int)values->energy_wh,
                           (unsigned int)values->voltage_v, (unsigned int)values->current_a,
                           (unsigned int)values->soc_percent);

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;
    *out_len = (size_t)written;
    return SYN_OK;
}

SYN_Status syn_ocpp_process_message(SYN_OCPP_Client *client, const char *in_buf, size_t in_len,
                                    char *out_resp, size_t max_resp_len, size_t *out_resp_len)
{
    if (client == NULL || in_buf == NULL || in_len < 5U)
        return SYN_INVALID_PARAM;

    if (out_resp_len != NULL)
        *out_resp_len = 0U;
    if (in_buf[0] != '[')
        return SYN_ERROR;

    uint8_t msg_type = (uint8_t)(in_buf[1] - '0');

    if (msg_type == SYN_OCPP_MSG_TYPE_CALLRESULT) {
        if (strstr(in_buf, "\"Accepted\"") != NULL) {
            client->registration_status = SYN_OCPP_REGISTRATION_ACCEPTED;
            if (client->reg_cb != NULL) {
                client->reg_cb(SYN_OCPP_REGISTRATION_ACCEPTED, client->heartbeat_interval_sec,
                               client->user_ctx);
            }
            if (client->auth_cb != NULL && strstr(in_buf, "idTagInfo") != NULL) {
                client->auth_cb(client->active_id_tag, SYN_OCPP_AUTH_ACCEPTED, client->user_ctx);
            }
        }
        const char *tx_ptr = strstr(in_buf, "\"transactionId\":");
        if (tx_ptr != NULL) {
            int tx_id = atoi(tx_ptr + 16);
            if (tx_id > 0) {
                client->active_transaction_id = tx_id;
                if (client->start_tx_cb != NULL) {
                    client->start_tx_cb(tx_id, SYN_OCPP_AUTH_ACCEPTED, client->user_ctx);
                }
            }
        }
        return SYN_OK;
    }

    if (msg_type == SYN_OCPP_MSG_TYPE_CALL && out_resp != NULL && out_resp_len != NULL &&
        max_resp_len >= 32U) {
        char msg_id[32] = {0};
        const char *id_start = strchr(in_buf, '"');
        if (id_start != NULL) {
            const char *id_end = strchr(id_start + 1, '"');
            if (id_end != NULL && (size_t)(id_end - id_start - 1) < sizeof(msg_id)) {
                memcpy(msg_id, id_start + 1, id_end - id_start - 1);
            }
        }

        if (strstr(in_buf, "\"RemoteStartTransaction\"") != NULL) {
            bool ok = false;
            if (client->remote_start_cb != NULL) {
                ok = client->remote_start_cb(client->connector_id, "REMOTE_TAG", client->user_ctx);
            } else {
                ok = true;
            }
            int written = snprintf(out_resp, max_resp_len, "[3,\"%s\",{\"status\":\"%s\"}]",
                                   msg_id[0] ? msg_id : "1", ok ? "Accepted" : "Rejected");
            if (written > 0 && (size_t)written < max_resp_len)
                *out_resp_len = (size_t)written;
            return SYN_OK;
        }
        if (strstr(in_buf, "\"RemoteStopTransaction\"") != NULL) {
            bool ok = false;
            if (client->remote_stop_cb != NULL) {
                ok = client->remote_stop_cb(client->active_transaction_id, client->user_ctx);
            } else {
                ok = true;
            }
            int written = snprintf(out_resp, max_resp_len, "[3,\"%s\",{\"status\":\"%s\"}]",
                                   msg_id[0] ? msg_id : "1", ok ? "Accepted" : "Rejected");
            if (written > 0 && (size_t)written < max_resp_len)
                *out_resp_len = (size_t)written;
            return SYN_OK;
        }

        int written = snprintf(out_resp, max_resp_len, "[3,\"%s\",{}]", msg_id[0] ? msg_id : "1");
        if (written > 0 && (size_t)written < max_resp_len)
            *out_resp_len = (size_t)written;
        return SYN_OK;
    }

    return SYN_OK;
}

void syn_ocpp_tick(SYN_OCPP_Client *client, uint32_t dt_ms, char *out_hb_buf, size_t max_len,
                   size_t *out_len)
{
    if (client == NULL)
        return;
    if (out_len != NULL)
        *out_len = 0U;

    if (client->heartbeat_timer_ms > dt_ms) {
        client->heartbeat_timer_ms -= dt_ms;
    } else {
        client->heartbeat_timer_ms = (client->heartbeat_interval_sec > 0U)
                                         ? (client->heartbeat_interval_sec * 1000U)
                                         : 60000U;

        if (out_hb_buf != NULL && out_len != NULL && max_len >= 32U) {
            (void)syn_ocpp_format_heartbeat(client, out_hb_buf, max_len, out_len);
        }
    }
}

/* ── CSMS Server Implementation ─────────────────────────────────────── */

SYN_Status syn_ocpp_server_init(SYN_OCPP_Server *server)
{
    if (server == NULL)
        return SYN_INVALID_PARAM;
    memset(server, 0, sizeof(*server));
    server->next_transaction_id = 1000;
    return SYN_OK;
}

SYN_Status syn_ocpp_server_set_callbacks(SYN_OCPP_Server *server,
                                         SYN_OCPP_ServerBootHandler boot_cb,
                                         SYN_OCPP_ServerAuthorizeHandler auth_cb,
                                         SYN_OCPP_ServerStartTxHandler start_tx_cb, void *user_ctx)
{
    if (server == NULL)
        return SYN_INVALID_PARAM;
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
    int written =
        snprintf(out_buf, max_len,
                 "[2,\"%u\",\"RemoteStartTransaction\",{"
                 "\"connectorId\":%u,"
                 "\"idTag\":\"%s\"}]",
                 (unsigned int)server->message_counter, (unsigned int)connector_id, id_tag);

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;
    *out_len = (size_t)written;
    return SYN_OK;
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

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;
    *out_len = (size_t)written;
    return SYN_OK;
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
    if (in_buf[0] != '[')
        return SYN_ERROR;

    uint8_t msg_type = (uint8_t)(in_buf[1] - '0');
    if (msg_type != SYN_OCPP_MSG_TYPE_CALL)
        return SYN_OK;

    char msg_id[32] = {0};
    const char *id_start = strchr(in_buf, '"');
    if (id_start != NULL) {
        const char *id_end = strchr(id_start + 1, '"');
        if (id_end != NULL && (size_t)(id_end - id_start - 1) < sizeof(msg_id)) {
            memcpy(msg_id, id_start + 1, id_end - id_start - 1);
        }
    }

    /* 1. BootNotification */
    if (strstr(in_buf, "\"BootNotification\"") != NULL) {
        SYN_OCPP_RegistrationStatus st = SYN_OCPP_REGISTRATION_ACCEPTED;
        uint32_t interval = 60U;
        if (server->boot_cb != NULL) {
            SYN_OCPP_ChargePointInfo info = {"StationVendor", "StationModel", "SN-1", "v1.0"};
            st = server->boot_cb(&info, &interval, server->user_ctx);
        }
        int written = snprintf(out_resp, max_resp_len,
                               "[3,\"%s\",{"
                               "\"status\":\"%s\","
                               "\"currentTime\":\"2026-08-06T12:00:00Z\","
                               "\"interval\":%u}]",
                               msg_id[0] ? msg_id : "1",
                               (st == SYN_OCPP_REGISTRATION_ACCEPTED) ? "Accepted" : "Rejected",
                               (unsigned int)interval);
        if (written > 0 && (size_t)written < max_resp_len)
            *out_resp_len = (size_t)written;
        return SYN_OK;
    }

    /* 2. Authorize */
    if (strstr(in_buf, "\"Authorize\"") != NULL) {
        SYN_OCPP_AuthorizationStatus auth_st = SYN_OCPP_AUTH_ACCEPTED;
        if (server->auth_cb != NULL) {
            auth_st = server->auth_cb("TAG-1", server->user_ctx);
        }
        int written = snprintf(
            out_resp, max_resp_len, "[3,\"%s\",{\"idTagInfo\":{\"status\":\"%s\"}}]",
            msg_id[0] ? msg_id : "1", (auth_st == SYN_OCPP_AUTH_ACCEPTED) ? "Accepted" : "Invalid");
        if (written > 0 && (size_t)written < max_resp_len)
            *out_resp_len = (size_t)written;
        return SYN_OK;
    }

    /* 3. StartTransaction */
    if (strstr(in_buf, "\"StartTransaction\"") != NULL) {
        int32_t tx_id = server->next_transaction_id++;
        if (server->start_tx_cb != NULL) {
            int32_t user_tx = server->start_tx_cb(1, "TAG-1", 0, server->user_ctx);
            if (user_tx > 0)
                tx_id = user_tx;
        }
        int written = snprintf(out_resp, max_resp_len,
                               "[3,\"%s\",{"
                               "\"transactionId\":%d,"
                               "\"idTagInfo\":{\"status\":\"Accepted\"}}]",
                               msg_id[0] ? msg_id : "1", (int)tx_id);
        if (written > 0 && (size_t)written < max_resp_len)
            *out_resp_len = (size_t)written;
        return SYN_OK;
    }

    /* Generic response for Heartbeat / StatusNotification / MeterValues / StopTransaction */
    int written = snprintf(out_resp, max_resp_len, "[3,\"%s\",{}]", msg_id[0] ? msg_id : "1");
    if (written > 0 && (size_t)written < max_resp_len)
        *out_resp_len = (size_t)written;
    return SYN_OK;
}

SYN_Status syn_ocpp_format_display_message(SYN_OCPP_Client *client,
                                           const SYN_OCPP_DisplayMessage *msg, char *out_buf,
                                           size_t max_len, size_t *out_len)
{
    if (client == NULL || msg == NULL || out_buf == NULL || out_len == NULL || max_len < 32U)
        return SYN_INVALID_PARAM;

    uint32_t msg_id = ++client->message_counter;
    int written = snprintf(out_buf, max_len,
                           "[2,\"%u\",\"DisplayMessage\",{\"message\":{\"id\":%u,\"header\":\"%s\","
                           "\"content\":\"%s\",\"displayTimeSec\":%u}}]",
                           (unsigned int)msg_id, (unsigned int)msg->message_id,
                           msg->header ? msg->header : "", msg->content ? msg->content : "",
                           (unsigned int)msg->display_time_s);

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;

    *out_len = (size_t)written;
    return SYN_OK;
}

SYN_Status syn_ocpp_format_v2g_energy_transfer(SYN_OCPP_Client *client, SYN_OCPP_V2GMode mode,
                                               int32_t power_limit_w, char *out_buf, size_t max_len,
                                               size_t *out_len)
{
    if (client == NULL || out_buf == NULL || out_len == NULL || max_len < 32U)
        return SYN_INVALID_PARAM;

    client->v2g_mode = mode;
    uint32_t msg_id = ++client->message_counter;
    const char *mode_str = (mode == SYN_OCPP_V2G_SCHEDULED) ? "Scheduled" : "Dynamic";

    int written = snprintf(out_buf, max_len,
                           "[2,\"%u\",\"V2GEnergyTransfer\",{\"v2gMode\":\"%s\","
                           "\"powerLimitW\":%d}]",
                           (unsigned int)msg_id, mode_str, (int)power_limit_w);

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;

    *out_len = (size_t)written;
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

    int written = snprintf(out_buf, max_len, "[4,\"%s\",\"%s\",\"%s\",{}]", msg_id, error_code,
                           error_description ? error_description : "");

    if (written < 0 || (size_t)written >= max_len)
        return SYN_ERROR;

    *out_len = (size_t)written;
    return SYN_OK;
}

```


