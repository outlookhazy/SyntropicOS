

# File syn\_mqttsn.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mqttsn.c**](syn__mqttsn_8c.md)

[Go to the documentation of this file](syn__mqttsn_8c.md)


```C++


#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_MQTTSN) || SYN_USE_MQTTSN

#include "../port/syn_port_system.h"
#include "../sched/syn_sched.h"
#include "syn_mqttsn.h"

#include <string.h>

/* ── Internal Framing Helpers ────────────────────────────────────────────── */

static size_t mqttsn_encode_header(uint8_t *buf, size_t max_buf, uint8_t msg_type,
                                   size_t payload_len)
{
    if (payload_len < 254U) {
        size_t total_len = payload_len + 2U;
        if (max_buf < total_len) {
            return 0U;
        }
        buf[0] = (uint8_t)total_len;
        buf[1] = msg_type;
        return 2U;
    }

    if ((payload_len > 65531U) || (max_buf < (payload_len + 4U))) {
        return 0U;
    }
    size_t total_len = payload_len + 4U;
    buf[0] = 0x01U;
    buf[1] = (uint8_t)((total_len >> 8U) & 0xFFU);
    buf[2] = (uint8_t)(total_len & 0xFFU);
    buf[3] = msg_type;
    return 4U;
}

static bool mqttsn_decode_header(const uint8_t *buf, size_t len, uint8_t *out_msg_type,
                                 size_t *out_payload_offset, size_t *out_payload_len)
{
    if (buf == NULL || len < 2U) {
        return false;
    }

    if (buf[0] != 0x01U) {
        size_t total_len = buf[0];
        if (total_len < 2U || total_len > len) {
            return false;
        }
        *out_msg_type = buf[1];
        *out_payload_offset = 2U;
        *out_payload_len = total_len - 2U;
        return true;
    }

    if (len < 4U) {
        return false;
    }
    size_t total_len = ((size_t)buf[1] << 8U) | buf[2];
    if (total_len < 4U || total_len > len) {
        return false;
    }
    *out_msg_type = buf[3];
    *out_payload_offset = 4U;
    *out_payload_len = total_len - 4U;
    return true;
}

static SYN_MQTTSN_TopicEntry *mqttsn_find_or_add_topic(SYN_MQTTSN_Client *client,
                                                       const char *topic_name)
{
    for (size_t i = 0U; i < client->topic_count; i++) {
        if (client->topics[i].active &&
            strncmp(client->topics[i].name, topic_name, SYN_MQTTSN_MAX_TOPIC_NAME) == 0) {
            return &client->topics[i];
        }
    }

    if (client->topic_count >= SYN_MQTTSN_MAX_REGISTRATIONS) {
        return NULL;
    }

    SYN_MQTTSN_TopicEntry *entry = &client->topics[client->topic_count++];
    (void)strncpy(entry->name, topic_name, sizeof(entry->name) - 1U);
    entry->name[sizeof(entry->name) - 1U] = '\0';
    entry->topic_id = 0U;
    entry->pending_msg_id = 0U;
    entry->active = true;
    return entry;
}

/* ── Public API Implementation ───────────────────────────────────────────── */

SYN_Status syn_mqttsn_client_init(SYN_MQTTSN_Client *client, const SYN_MQTTSN_Config *cfg)
{
    if (client == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cfg->transport == NULL || cfg->rx_buf == NULL || cfg->tx_buf == NULL ||
        cfg->rx_buf_size < 128U || cfg->tx_buf_size < 128U) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(client, 0, sizeof(*client));
    client->cfg = *cfg;
    if (client->cfg.duration_s == 0U) {
        client->cfg.duration_s = SYN_MQTTSN_DEFAULT_DURATION_S;
    }

    client->state = SYN_MQTTSN_STATE_DISCONNECTED;
    client->next_msg_id = 1U;
    PT_INIT(&client->pt);
    return SYN_OK;
}

SYN_Status syn_mqttsn_client_searchgw(SYN_MQTTSN_Client *client, uint8_t radius)
{
    if (client == NULL || client->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = client->cfg.tx_buf;
    size_t hdr_len = mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_SEARCHGW, 1U);
    if (hdr_len == 0U) {
        return SYN_ERROR;
    }

    tx[hdr_len] = radius;
    size_t total_len = hdr_len + 1U;

    if (!syn_transport_send(client->cfg.transport, tx, total_len)) {
        return SYN_ERROR;
    }

    client->state = SYN_MQTTSN_STATE_SEARCHING_GW;
    return SYN_OK;
}

SYN_Status syn_mqttsn_client_connect(SYN_MQTTSN_Client *client)
{
    if (client == NULL || client->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    const char *cid = (client->cfg.client_id != NULL) ? client->cfg.client_id : "SyntropicClient";
    size_t cid_len = strlen(cid);
    if (cid_len > SYN_MQTTSN_MAX_CLIENT_ID) {
        cid_len = SYN_MQTTSN_MAX_CLIENT_ID;
    }

    uint8_t flags = 0U;
    if (client->cfg.clean_session) {
        flags |= SYN_MQTTSN_FLAG_CLEAN_SESSION;
    }

    uint8_t *tx = client->cfg.tx_buf;
    size_t payload_len = 4U + cid_len;
    size_t hdr_len =
        mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_CONNECT, payload_len);
    if (hdr_len == 0U) {
        return SYN_ERROR;
    }

    tx[hdr_len] = flags;
    tx[hdr_len + 1U] = SYN_MQTTSN_PROTOCOL_ID;
    tx[hdr_len + 2U] = (uint8_t)((client->cfg.duration_s >> 8U) & 0xFFU);
    tx[hdr_len + 3U] = (uint8_t)(client->cfg.duration_s & 0xFFU);
    (void)memcpy(&tx[hdr_len + 4U], cid, cid_len);

    size_t total_len = hdr_len + payload_len;
    if (!syn_transport_send(client->cfg.transport, tx, total_len)) {
        return SYN_ERROR;
    }

    client->state = SYN_MQTTSN_STATE_CONNECTING;
    return SYN_OK;
}

SYN_Status syn_mqttsn_client_register_topic(SYN_MQTTSN_Client *client, const char *topic_name,
                                            uint16_t *out_msg_id)
{
    if (client == NULL || topic_name == NULL || client->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    size_t name_len = strlen(topic_name);
    if (name_len == 0U || name_len >= SYN_MQTTSN_MAX_TOPIC_NAME) {
        return SYN_INVALID_PARAM;
    }

    SYN_MQTTSN_TopicEntry *entry = mqttsn_find_or_add_topic(client, topic_name);
    if (entry == NULL) {
        return SYN_ERROR;
    }

    uint16_t msg_id = client->next_msg_id++;
    if (client->next_msg_id == 0U) {
        client->next_msg_id = 1U;
    }

    uint8_t *tx = client->cfg.tx_buf;
    size_t payload_len = 4U + name_len;
    size_t hdr_len =
        mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_REGISTER, payload_len);
    if (hdr_len == 0U) {
        return SYN_ERROR;
    }

    tx[hdr_len] = 0U; /* TopicId = 0 in request */
    tx[hdr_len + 1U] = 0U;
    tx[hdr_len + 2U] = (uint8_t)((msg_id >> 8U) & 0xFFU);
    tx[hdr_len + 3U] = (uint8_t)(msg_id & 0xFFU);
    (void)memcpy(&tx[hdr_len + 4U], topic_name, name_len);

    size_t total_len = hdr_len + payload_len;
    if (!syn_transport_send(client->cfg.transport, tx, total_len)) {
        return SYN_ERROR;
    }

    entry->pending_msg_id = msg_id;
    if (out_msg_id != NULL) {
        *out_msg_id = msg_id;
    }
    return SYN_OK;
}

static bool mqttsn_publish_args_valid(const SYN_MQTTSN_Client *client, int8_t qos,
                                      const uint8_t *payload, size_t len)
{
    if ((client == NULL) || (client->cfg.transport == NULL)) {
        return false;
    }
    if (((payload == NULL) && (len > 0U)) || (len > 65526U)) {
        return false;
    }
    return (qos >= -1) && (qos <= 1);
}

static bool mqttsn_topic_arg_valid(const char *topic_name)
{
    if (topic_name == NULL) {
        return true;
    }
    size_t len = strlen(topic_name);
    return (len > 0U) && (len < SYN_MQTTSN_MAX_TOPIC_NAME);
}

static SYN_Status mqttsn_publish_internal(SYN_MQTTSN_Client *client, uint16_t topic_id,
                                          uint8_t topic_type_flag, int8_t qos, bool retain,
                                          const uint8_t *payload, size_t len)
{
    if (!mqttsn_publish_args_valid(client, qos, payload, len)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t flags = topic_type_flag & SYN_MQTTSN_FLAG_TOPIC_MASK;
    if (retain) {
        flags |= SYN_MQTTSN_FLAG_RETAIN;
    }

    uint16_t msg_id = 0U;
    if (qos == 1) {
        flags |= SYN_MQTTSN_FLAG_QOS_1;
        msg_id = client->next_msg_id++;
        if (client->next_msg_id == 0U) {
            client->next_msg_id = 1U;
        }
    } else if (qos == -1) {
        flags |= SYN_MQTTSN_FLAG_QOS_NEG1;
    }

    uint8_t *tx = client->cfg.tx_buf;
    size_t payload_len = 5U + len;
    size_t hdr_len =
        mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_PUBLISH, payload_len);
    if (hdr_len == 0U) {
        return SYN_ERROR;
    }

    tx[hdr_len] = flags;
    tx[hdr_len + 1U] = (uint8_t)((topic_id >> 8U) & 0xFFU);
    tx[hdr_len + 2U] = (uint8_t)(topic_id & 0xFFU);
    tx[hdr_len + 3U] = (uint8_t)((msg_id >> 8U) & 0xFFU);
    tx[hdr_len + 4U] = (uint8_t)(msg_id & 0xFFU);
    if (len > 0U) {
        (void)memcpy(&tx[hdr_len + 5U], payload, len);
    }

    size_t total_len = hdr_len + payload_len;
    if (!syn_transport_send(client->cfg.transport, tx, total_len)) {
        return SYN_ERROR;
    }
    return SYN_OK;
}

SYN_Status syn_mqttsn_client_publish(SYN_MQTTSN_Client *client, uint16_t topic_id, int8_t qos,
                                     bool retain, const uint8_t *payload, size_t len)
{
    return mqttsn_publish_internal(client, topic_id, SYN_MQTTSN_FLAG_TOPIC_NORMAL, qos, retain,
                                   payload, len);
}

SYN_Status syn_mqttsn_client_publish_short(SYN_MQTTSN_Client *client, const char short_topic[2],
                                           int8_t qos, bool retain, const uint8_t *payload,
                                           size_t len)
{
    if (short_topic == NULL) {
        return SYN_INVALID_PARAM;
    }
    uint16_t topic_id = ((uint16_t)(uint8_t)short_topic[0] << 8U) | (uint8_t)short_topic[1];
    return mqttsn_publish_internal(client, topic_id, SYN_MQTTSN_FLAG_TOPIC_SHORT, qos, retain,
                                   payload, len);
}

SYN_Status syn_mqttsn_client_publish_predefined(SYN_MQTTSN_Client *client, uint16_t predef_id,
                                                int8_t qos, bool retain, const uint8_t *payload,
                                                size_t len)
{
    return mqttsn_publish_internal(client, predef_id, SYN_MQTTSN_FLAG_TOPIC_PREDEF, qos, retain,
                                   payload, len);
}

SYN_Status syn_mqttsn_client_subscribe(SYN_MQTTSN_Client *client, const char *topic_name,
                                       uint16_t predef_id, int8_t qos)
{
    if ((client == NULL) || (client->cfg.transport == NULL) ||
        !mqttsn_topic_arg_valid(topic_name)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t flags = 0U;
    if (qos == 1) {
        flags |= SYN_MQTTSN_FLAG_QOS_1;
    } else if (qos == -1) {
        flags |= SYN_MQTTSN_FLAG_QOS_NEG1;
    }

    uint16_t msg_id = client->next_msg_id++;
    if (client->next_msg_id == 0U) {
        client->next_msg_id = 1U;
    }

    uint8_t *tx = client->cfg.tx_buf;
    size_t payload_len = 0U;

    if (topic_name != NULL) {
        size_t name_len = strlen(topic_name);
        if (name_len == 2U) {
            flags |= SYN_MQTTSN_FLAG_TOPIC_SHORT;
            payload_len = 5U;
        } else {
            flags |= SYN_MQTTSN_FLAG_TOPIC_NORMAL;
            payload_len = 3U + name_len;
        }

        size_t hdr_len = mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_SUBSCRIBE,
                                              payload_len);
        if (hdr_len == 0U) {
            return SYN_ERROR;
        }
        tx[hdr_len] = flags;
        tx[hdr_len + 1U] = (uint8_t)((msg_id >> 8U) & 0xFFU);
        tx[hdr_len + 2U] = (uint8_t)(msg_id & 0xFFU);
        (void)memcpy(&tx[hdr_len + 3U], topic_name, name_len);

        return syn_transport_send(client->cfg.transport, tx, hdr_len + payload_len) ? SYN_OK
                                                                                    : SYN_ERROR;
    }

    /* Subscribing by Predefined ID */
    flags |= SYN_MQTTSN_FLAG_TOPIC_PREDEF;
    payload_len = 5U;
    size_t hdr_len =
        mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_SUBSCRIBE, payload_len);
    if (hdr_len == 0U) {
        return SYN_ERROR;
    }

    tx[hdr_len] = flags;
    tx[hdr_len + 1U] = (uint8_t)((msg_id >> 8U) & 0xFFU);
    tx[hdr_len + 2U] = (uint8_t)(msg_id & 0xFFU);
    tx[hdr_len + 3U] = (uint8_t)((predef_id >> 8U) & 0xFFU);
    tx[hdr_len + 4U] = (uint8_t)(predef_id & 0xFFU);

    return syn_transport_send(client->cfg.transport, tx, hdr_len + payload_len) ? SYN_OK
                                                                                : SYN_ERROR;
}

SYN_Status syn_mqttsn_client_unsubscribe(SYN_MQTTSN_Client *client, const char *topic_name,
                                         uint16_t predef_id)
{
    if ((client == NULL) || (client->cfg.transport == NULL) ||
        !mqttsn_topic_arg_valid(topic_name)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t flags = 0U;
    uint16_t msg_id = client->next_msg_id++;
    if (client->next_msg_id == 0U) {
        client->next_msg_id = 1U;
    }

    uint8_t *tx = client->cfg.tx_buf;
    size_t payload_len = 0U;

    if (topic_name != NULL) {
        size_t name_len = strlen(topic_name);
        if (name_len == 2U) {
            flags |= SYN_MQTTSN_FLAG_TOPIC_SHORT;
            payload_len = 5U;
        } else {
            flags |= SYN_MQTTSN_FLAG_TOPIC_NORMAL;
            payload_len = 3U + name_len;
        }

        size_t hdr_len = mqttsn_encode_header(tx, client->cfg.tx_buf_size,
                                              SYN_MQTTSN_MSG_UNSUBSCRIBE, payload_len);
        if (hdr_len == 0U) {
            return SYN_ERROR;
        }
        tx[hdr_len] = flags;
        tx[hdr_len + 1U] = (uint8_t)((msg_id >> 8U) & 0xFFU);
        tx[hdr_len + 2U] = (uint8_t)(msg_id & 0xFFU);
        (void)memcpy(&tx[hdr_len + 3U], topic_name, name_len);

        return syn_transport_send(client->cfg.transport, tx, hdr_len + payload_len) ? SYN_OK
                                                                                    : SYN_ERROR;
    }

    flags |= SYN_MQTTSN_FLAG_TOPIC_PREDEF;
    payload_len = 5U;
    size_t hdr_len =
        mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_UNSUBSCRIBE, payload_len);
    if (hdr_len == 0U) {
        return SYN_ERROR;
    }

    tx[hdr_len] = flags;
    tx[hdr_len + 1U] = (uint8_t)((msg_id >> 8U) & 0xFFU);
    tx[hdr_len + 2U] = (uint8_t)(msg_id & 0xFFU);
    tx[hdr_len + 3U] = (uint8_t)((predef_id >> 8U) & 0xFFU);
    tx[hdr_len + 4U] = (uint8_t)(predef_id & 0xFFU);

    return syn_transport_send(client->cfg.transport, tx, hdr_len + payload_len) ? SYN_OK
                                                                                : SYN_ERROR;
}

SYN_Status syn_mqttsn_client_sleep(SYN_MQTTSN_Client *client, uint16_t sleep_duration_s)
{
    if (client == NULL || client->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = client->cfg.tx_buf;
    size_t payload_len = 2U;
    size_t hdr_len =
        mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_DISCONNECT, payload_len);
    if (hdr_len == 0U) {
        return SYN_ERROR;
    }

    tx[hdr_len] = (uint8_t)((sleep_duration_s >> 8U) & 0xFFU);
    tx[hdr_len + 1U] = (uint8_t)(sleep_duration_s & 0xFFU);

    if (!syn_transport_send(client->cfg.transport, tx, hdr_len + payload_len)) {
        return SYN_ERROR;
    }

    client->state = SYN_MQTTSN_STATE_ASLEEP;
    return SYN_OK;
}

SYN_Status syn_mqttsn_client_wake(SYN_MQTTSN_Client *client)
{
    if (client == NULL || client->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    const char *cid = (client->cfg.client_id != NULL) ? client->cfg.client_id : "SyntropicClient";
    size_t cid_len = strlen(cid);
    if (cid_len > SYN_MQTTSN_MAX_CLIENT_ID) {
        cid_len = SYN_MQTTSN_MAX_CLIENT_ID;
    }

    uint8_t *tx = client->cfg.tx_buf;
    size_t hdr_len =
        mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_PINGREQ, cid_len);
    if (hdr_len == 0U) {
        return SYN_ERROR;
    }

    if (cid_len > 0U) {
        (void)memcpy(&tx[hdr_len], cid, cid_len);
    }

    if (!syn_transport_send(client->cfg.transport, tx, hdr_len + cid_len)) {
        return SYN_ERROR;
    }

    client->state = SYN_MQTTSN_STATE_AWAKE;
    return SYN_OK;
}

SYN_Status syn_mqttsn_client_disconnect(SYN_MQTTSN_Client *client)
{
    if (client == NULL || client->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t *tx = client->cfg.tx_buf;
    size_t hdr_len =
        mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_DISCONNECT, 0U);
    if (hdr_len == 0U) {
        return SYN_ERROR;
    }

    (void)syn_transport_send(client->cfg.transport, tx, hdr_len);
    client->state = SYN_MQTTSN_STATE_DISCONNECTED;
    return SYN_OK;
}

static void mqttsn_handle_regack(SYN_MQTTSN_Client *client, const uint8_t *payload,
                                 size_t payload_len)
{
    if (payload_len >= 5U && payload[4] == SYN_MQTTSN_RC_ACCEPTED) {
        uint16_t topic_id = ((uint16_t)payload[0] << 8U) | payload[1];
        uint16_t msg_id = ((uint16_t)payload[2] << 8U) | payload[3];
        if (msg_id != 0U) {
            for (size_t i = 0U; i < client->topic_count; i++) {
                if (client->topics[i].active && client->topics[i].pending_msg_id == msg_id) {
                    client->topics[i].topic_id = topic_id;
                    client->topics[i].pending_msg_id = 0U;
                    break;
                }
            }
        }
    }
}

static void mqttsn_handle_connack(SYN_MQTTSN_Client *client, const uint8_t *payload,
                                  size_t payload_len)
{
    if (payload_len >= 1U) {
        if (payload[0] == SYN_MQTTSN_RC_ACCEPTED) {
            client->state = SYN_MQTTSN_STATE_CONNECTED;
        } else if (client->state == SYN_MQTTSN_STATE_CONNECTING) {
            client->state = SYN_MQTTSN_STATE_DISCONNECTED;
        }
    }
}

static void mqttsn_handle_rx_publish(SYN_MQTTSN_Client *client, const uint8_t *payload,
                                     size_t payload_len)
{
    if (payload_len < 5U) {
        return;
    }
    uint8_t flags = payload[0];
    uint8_t topic_type = flags & SYN_MQTTSN_FLAG_TOPIC_MASK;
    uint16_t topic_id = ((uint16_t)payload[1] << 8U) | payload[2];
    uint16_t msg_id = ((uint16_t)payload[3] << 8U) | payload[4];

    if (client->cfg.on_message != NULL) {
        client->cfg.on_message(client, topic_id, topic_type, &payload[5], payload_len - 5U,
                               client->cfg.user_data);
    }

    if ((flags & SYN_MQTTSN_FLAG_QOS_MASK) == SYN_MQTTSN_FLAG_QOS_1) {
        uint8_t *tx = client->cfg.tx_buf;
        size_t hdr_len =
            mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_PUBACK, 5U);
        if (hdr_len > 0U) {
            tx[hdr_len] = (uint8_t)((topic_id >> 8U) & 0xFFU);
            tx[hdr_len + 1U] = (uint8_t)(topic_id & 0xFFU);
            tx[hdr_len + 2U] = (uint8_t)((msg_id >> 8U) & 0xFFU);
            tx[hdr_len + 3U] = (uint8_t)(msg_id & 0xFFU);
            tx[hdr_len + 4U] = SYN_MQTTSN_RC_ACCEPTED;
            (void)syn_transport_send(client->cfg.transport, tx, hdr_len + 5U);
        }
    }
}

static void mqttsn_process_rx_frame(SYN_MQTTSN_Client *client, uint8_t msg_type,
                                    const uint8_t *payload, size_t payload_len)
{
    switch (msg_type) {
    case SYN_MQTTSN_MSG_ADVERTISE:
    case SYN_MQTTSN_MSG_GWINFO:
        if (payload_len >= 1U) {
            client->gateway_id = payload[0];
            if (client->state == SYN_MQTTSN_STATE_SEARCHING_GW) {
                (void)syn_mqttsn_client_connect(client);
            }
        }
        break;

    case SYN_MQTTSN_MSG_CONNACK:
        mqttsn_handle_connack(client, payload, payload_len);
        break;

    case SYN_MQTTSN_MSG_REGACK:
        mqttsn_handle_regack(client, payload, payload_len);
        break;

    case SYN_MQTTSN_MSG_PUBLISH:
        mqttsn_handle_rx_publish(client, payload, payload_len);
        break;

    case SYN_MQTTSN_MSG_PINGRESP:
        if (client->state == SYN_MQTTSN_STATE_AWAKE) {
            client->state = SYN_MQTTSN_STATE_ASLEEP;
        }
        break;

    case SYN_MQTTSN_MSG_DISCONNECT:
        client->state = SYN_MQTTSN_STATE_DISCONNECTED;
        break;

    default:
        break;
    }
}

static SYN_Status mqttsn_client_step_ex(SYN_MQTTSN_Client *client, uint32_t now_ms,
                                        bool *out_rx_consumed)
{
    if (out_rx_consumed != NULL) {
        *out_rx_consumed = false;
    }
    if (client == NULL || client->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    size_t rx_len = 0U;
    if (syn_transport_recv(client->cfg.transport, client->cfg.rx_buf, client->cfg.rx_buf_size,
                           &rx_len) &&
        rx_len > 0U) {
        if (out_rx_consumed != NULL) {
            *out_rx_consumed = true;
        }
        client->last_activity_ms = now_ms;
        uint8_t msg_type = 0U;
        size_t payload_offset = 0U;
        size_t payload_len = 0U;

        if (mqttsn_decode_header(client->cfg.rx_buf, rx_len, &msg_type, &payload_offset,
                                 &payload_len)) {
            mqttsn_process_rx_frame(client, msg_type, &client->cfg.rx_buf[payload_offset],
                                    payload_len);
        }
    }

    /* Keep-alive ping */
    if (client->state == SYN_MQTTSN_STATE_CONNECTED) {
        uint32_t period_ms = (uint32_t)client->cfg.duration_s * 1000U;
        if (period_ms > 0U && (now_ms - client->last_activity_ms >= period_ms)) {
            client->last_activity_ms = now_ms;
            uint8_t *tx = client->cfg.tx_buf;
            size_t hdr_len =
                mqttsn_encode_header(tx, client->cfg.tx_buf_size, SYN_MQTTSN_MSG_PINGREQ, 0U);
            if (hdr_len > 0U) {
                (void)syn_transport_send(client->cfg.transport, tx, hdr_len);
            }
        }
    }

    return SYN_OK;
}

SYN_Status syn_mqttsn_client_step(SYN_MQTTSN_Client *client, uint32_t now_ms)
{
    return mqttsn_client_step_ex(client, now_ms, NULL);
}

SYN_PT_Status syn_mqttsn_client_pt(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }
    SYN_MQTTSN_Client *client = (SYN_MQTTSN_Client *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        bool rx_consumed = false;
        uint32_t now = syn_port_get_tick_ms();
        if (mqttsn_client_step_ex(client, now, &rx_consumed) != SYN_OK) {
            break;
        }
        if (rx_consumed && client->cfg.transport != NULL &&
            client->cfg.transport->has_data != NULL &&
            syn_transport_has_data(client->cfg.transport)) {
            PT_CONTINUE_IF_NEEDED(pt, task);
        } else {
            PT_PASS(pt);
        }
    }

    PT_END(pt);
}

#endif /* !defined(SYN_USE_MQTTSN) || SYN_USE_MQTTSN */
```


