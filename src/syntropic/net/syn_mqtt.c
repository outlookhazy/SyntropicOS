#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_MQTT) || SYN_USE_MQTT

/**
 * @file syn_mqtt.c
 * @brief Lightweight MQTT 3.1.1 client implementation — fully non-blocking.
 */

#include "../port/syn_port_system.h"
#include "../sched/syn_sched.h"
#include "../util/syn_assert.h"
#include "../util/syn_metrics.h"
#include "../util/syn_pack.h"
#include "syn_mqtt.h"

#include <string.h>

#define MQTT_RECV_TIMEOUT_MS 5000 /**< Timeout for incomplete packet reception (ms). */
#define MQTT_ACK_TIMEOUT_MS 5000  /**< Timeout for ACK responses (ms). */

#if SYN_USE_METRICS
SYN_METRIC_DECLARE(mqtt_tx_msgs, "mqtt_tx_msgs", "Total MQTT messages published",
                   SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(mqtt_rx_msgs, "mqtt_rx_msgs", "Total MQTT messages received",
                   SYN_METRIC_TYPE_COUNTER);
SYN_METRIC_DECLARE(mqtt_errors, "mqtt_errors", "Total MQTT client errors", SYN_METRIC_TYPE_COUNTER);
#endif

/* ── Remaining Length & Varint Helpers ──────────────────────────────────── */

size_t syn_mqtt_encode_varint(uint32_t val, uint8_t buf[4])
{
    if (buf == NULL || val > 268435455U) {
        return 0;
    }
    size_t bytes = 0;
    do {
        uint8_t d = (uint8_t)(val % 128U);
        val /= 128U;
        if (val > 0U) {
            d |= 0x80U;
        }
        buf[bytes++] = d;
    } while (val > 0U && bytes < 4U);
    return bytes;
}

bool syn_mqtt_decode_varint(const uint8_t *buf, size_t buf_len, uint32_t *val, size_t *bytes_read)
{
    if (buf == NULL || val == NULL || bytes_read == NULL || buf_len == 0U) {
        return false;
    }
    uint32_t multiplier = 1U;
    uint32_t value = 0U;
    size_t pos = 0U;
    uint8_t encoded_byte = 0U;

    do {
        if (pos >= buf_len || pos >= 4U) {
            return false;
        }
        encoded_byte = buf[pos++];
        value += (uint32_t)(encoded_byte & 0x7FU) * multiplier;
        if ((encoded_byte & 0x80U) != 0U) {
            multiplier *= 128U;
        }
    } while ((encoded_byte & 0x80U) != 0U);

    *val = value;
    *bytes_read = pos;
    return true;
}

size_t syn_mqtt5_encode_user_prop(const char *key, const char *val, uint8_t *buf,
                                  size_t max_buf_len)
{
    if (key == NULL || val == NULL || buf == NULL) {
        return 0U;
    }
    size_t klen = strlen(key);
    size_t vlen = strlen(val);
    if (klen > UINT16_MAX || vlen > UINT16_MAX) {
        return 0U;
    }
    size_t total = 5U + klen + vlen; /* Identifier (1) + Key (2+klen) + Val (2+vlen) */
    if (max_buf_len < total) {
        return 0U;
    }

    size_t pos = 0U;
    buf[pos++] = SYN_MQTT5_PROP_USER_PROPERTY;
    buf[pos++] = (uint8_t)(klen >> 8U);
    buf[pos++] = (uint8_t)(klen & 0xFFU);
    memcpy(&buf[pos], key, klen);
    pos += klen;

    buf[pos++] = (uint8_t)(vlen >> 8U);
    buf[pos++] = (uint8_t)(vlen & 0xFFU);
    memcpy(&buf[pos], val, vlen);
    pos += vlen;

    return pos;
}

/**
 * @brief Encode remaining length field.
 * @param[out] buf Destination buffer.
 * @param[in] len Length value to encode.
 * @return Number of bytes written.
 */
static size_t encode_remaining_len(uint8_t *buf, uint32_t len)
{
    return syn_mqtt_encode_varint(len, buf);
}

/**
 * @brief Close any open socket and reset transient MQTT session and QoS 1 retransmit state.
 * @param c MQTT client context.
 */
static void mqtt_reset_session(SYN_MqttClient *c)
{
    if (c->sock != SYN_SOCKET_INVALID) {
        syn_port_sock_close(c->sock);
        c->sock = SYN_SOCKET_INVALID;
    }
    c->state = SYN_MQTT_DISCONNECTED;
    c->rx_phase = SYN_MQTT_RX_IDLE;
    c->pending_puback_id = 0U;
    c->retransmit_len = 0U;
    c->ping_pending = false;
    c->ping_sent_ms = 0U;
}

/**
 * @brief Transmit an MQTT packet on an active session, updating activity or resetting on error.
 * @param c   MQTT client context.
 * @param buf Packet bytes to transmit.
 * @param len Number of bytes to transmit.
 * @return true if all bytes sent, false if transport failed and session was reset.
 */
static bool mqtt_send_tracked(SYN_MqttClient *c, const uint8_t *buf, size_t len)
{
    if (syn_port_sock_send_all(c->sock, buf, len) != (int)len) {
        mqtt_reset_session(c);
        return false;
    }
    c->last_activity_ms = syn_port_get_tick_ms();
    return true;
}

/**
 * @brief Append a 16-bit length-prefixed UTF-8 string into an MQTT packet buffer.
 * @param tx  Destination buffer.
 * @param pos [in,out] Write cursor.
 * @param str Source string bytes.
 * @param len String length (<= UINT16_MAX).
 */
static void mqtt_put_str(uint8_t *tx, size_t *pos, const char *str, uint16_t len)
{
    syn_poke_u16(len, tx, *pos);
    *pos += 2U;
    if (len > 0U) {
        memcpy(tx + *pos, str, len);
        *pos += len;
    }
}

/* ── Connection Packets ─────────────────────────────────────────────────── */

/**
 * @brief Compute validated MQTT 3.1.1 CONNECT credential flags and lengths.
 * @param c        MQTT client context.
 * @param has_user [out] Set true when username is present.
 * @param has_pass [out] Set true when both username and password are present.
 * @param u_len    [out] Username byte length.
 * @param p_len    [out] Password byte length.
 * @return true if credential lengths fit in 16-bit MQTT strings, false otherwise.
 */
static bool mqtt_connect_creds_len(const SYN_MqttClient *c, bool *has_user, bool *has_pass,
                                   size_t *u_len, size_t *p_len)
{
    *has_user = (c->username != NULL);
    *has_pass = *has_user && (c->password != NULL);
    *u_len = *has_user ? strlen(c->username) : 0U;
    *p_len = *has_pass ? strlen(c->password) : 0U;
    return (*u_len <= UINT16_MAX) && (*p_len <= UINT16_MAX);
}

/**
 * @brief Build and send an MQTT CONNECT packet.
 * @param c  MQTT client.
 * @return true on success.
 */
static bool send_mqtt_connect(SYN_MqttClient *c)
{
    size_t cid_len = strlen(c->client_id);
    bool has_user = false;
    bool has_pass = false;
    size_t u_len = 0U;
    size_t p_len = 0U;
    if (cid_len > UINT16_MAX || !mqtt_connect_creds_len(c, &has_user, &has_pass, &u_len, &p_len)) {
        return false;
    }

    uint32_t rem_len = (uint32_t)(12U + cid_len);
    if (has_user) {
        rem_len += (uint32_t)(2U + u_len);
    }
    if (has_pass) {
        rem_len += (uint32_t)(2U + p_len);
    }

    uint8_t varint_buf[4];
    size_t varint_len = encode_remaining_len(varint_buf, rem_len);
    if (c->tx_buf_size < 1U + varint_len || (size_t)rem_len > c->tx_buf_size - 1U - varint_len) {
        return false;
    }

    uint8_t *tx = c->tx_buf;
    tx[0] = 0x10U; /* CONNECT */
    memcpy(tx + 1U, varint_buf, varint_len);
    size_t pos = 1U + varint_len;

    mqtt_put_str(tx, &pos, "MQTT", 4U);
    tx[pos++] = 0x04U; /* Level 3.1.1 */

    uint8_t flags = 0x02U; /* Clean session */
    if (has_user) {
        flags |= 0x80U;
    }
    if (has_pass) {
        flags |= 0x40U;
    }
    tx[pos++] = flags;

    syn_poke_u16(c->keep_alive_s, tx, pos);
    pos += 2U;

    mqtt_put_str(tx, &pos, c->client_id, (uint16_t)cid_len);
    if (has_user) {
        mqtt_put_str(tx, &pos, c->username, (uint16_t)u_len);
    }
    if (has_pass) {
        mqtt_put_str(tx, &pos, c->password, (uint16_t)p_len);
    }

    return syn_port_sock_send_all(c->sock, tx, pos) == (int)pos;
}

/**
 * @brief Send an MQTT PINGREQ packet.
 * @param c  MQTT client.
 * @return true on success.
 */
static bool send_mqtt_ping(const SYN_MqttClient *c)
{
    static const uint8_t ping[] = {0xC0, 0x00};
    return syn_port_sock_send_all(c->sock, ping, 2) == 2;
}

SYN_Status syn_mqtt_ping(SYN_MqttClient *client)
{
    if (client == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (client->state != SYN_MQTT_CONNECTED) {
        return SYN_ERROR;
    }
    if (send_mqtt_ping(client)) {
        uint32_t now = syn_port_get_tick_ms();
        client->last_activity_ms = now;
        client->ping_pending = true;
        client->ping_sent_ms = now;
        return SYN_OK;
    }
    return SYN_ERROR;
}

/* ── Main Task ──────────────────────────────────────────────────────────── */

/**
 * @brief Handle an incoming PUBLISH packet.
 * @param c         MQTT client.
 * @param payload   Raw payload (after fixed header).
 * @param len       Payload length.
 * @param qos_bits  QoS flags from the fixed header.
 */
static void handle_publish(SYN_MqttClient *c, const uint8_t *payload, uint32_t len,
                           uint8_t qos_bits)
{
    uint8_t qos = (qos_bits >> 1U) & 0x03U;
    if ((qos == 0U) && ((qos_bits & 0x08U) != 0U)) {
        mqtt_reset_session(c);
        return;
    }
    if (len < 2U) {
        return;
    }

    uint16_t topic_len = syn_peek_u16(payload, 0U);
    if ((2U + (uint32_t)topic_len) > len) {
        return;
    }

    /* Extract topic string dynamically */
    char topic[64];
    size_t to_copy = (topic_len < (sizeof(topic) - 1U)) ? topic_len : (sizeof(topic) - 1U);
    memcpy(topic, payload + 2U, to_copy);
    topic[to_copy] = '\0';

    size_t payload_offset = 2U + topic_len;
    uint16_t packet_id = 0U;

    if (qos > 0U) {
        if ((payload_offset + 2U) > len) {
            return;
        }
        packet_id = syn_peek_u16(payload, payload_offset);
        payload_offset += 2U;
    }

    if (c->on_message != NULL) {
        c->on_message(topic, payload + payload_offset, len - payload_offset, c->ctx);
    }

    /* QoS 1 ACK (PUBACK) */
    if (qos == 1U) {
        uint8_t puback[4];
        puback[0] = 0x40U;
        puback[1] = 0x02U;
        syn_poke_u16(packet_id, puback, 2U);
        (void)syn_port_sock_send_all(c->sock, puback, 4U);
    }
}

/**
 * @brief Validate an inbound MQTT 3.1.1 CONNACK packet.
 * @param c MQTT client context with complete packet in rx_buf.
 * @return true if CONNACK is well-formed and accepted while CONNECTING, false otherwise.
 */
static bool mqtt_connack_valid(const SYN_MqttClient *c)
{
    if ((c->state != SYN_MQTT_CONNECTING) || (c->rx_header != 0x20U) || (c->rx_rem_len != 2U)) {
        return false;
    }
    return ((c->rx_buf[0] & 0xFEU) == 0U) && (c->rx_buf[1] == 0U);
}

/**
 * @brief Dispatch a fully received MQTT packet.
 * @param c  MQTT client context.
 */
static void process_packet(SYN_MqttClient *c)
{
    uint8_t type = c->rx_header & 0xF0U;
    uint32_t rem_len = c->rx_rem_len;

    if (type == 0x20U) {
        if (mqtt_connack_valid(c)) {
            c->state = SYN_MQTT_CONNECTED;
        } else {
            mqtt_reset_session(c);
        }
    } else if (type == 0x30U) {
        handle_publish(c, c->rx_buf, rem_len, c->rx_header & 0x0FU);
    } else if (type == 0x40U) {
        if (rem_len >= 2U) {
            uint16_t pid = syn_peek_u16(c->rx_buf, 0U);
            if (pid == c->pending_puback_id) {
                c->pending_puback_id = 0U;
                c->retransmit_len = 0U;
            }
        }
    } else if (type == 0x90U) {
        /* SUBACK */
    } else if (type == 0xD0U) {
        c->ping_pending = false;
    }
}

/**
 * @brief Non-blocking socket read that resets session on EOF or readable-error.
 * @param c   MQTT client context.
 * @param buf Destination byte buffer.
 * @param len Maximum bytes to read.
 * @return Bytes read (> 0), or -1 on no data / session closed.
 */
static int mqtt_recv_checked(SYN_MqttClient *c, uint8_t *buf, size_t len)
{
    bool was_readable = syn_port_sock_readable(c->sock);
    int n = syn_port_sock_recv(c->sock, buf, len, 0);
    if (n == 0 || (n < 0 && was_readable)) {
        mqtt_reset_session(c);
        return -1;
    }
    if (n > 0) {
        c->last_activity_ms = syn_port_get_tick_ms();
    }
    return n;
}

/**
 * @brief Read and process the 1-byte fixed header opcode in SYN_MQTT_RX_IDLE.
 * @param c MQTT client context.
 * @return true if a byte was consumed.
 */
static bool mqtt_rx_idle(SYN_MqttClient *c)
{
    uint8_t hdr = 0U;
    if (mqtt_recv_checked(c, &hdr, 1U) <= 0) {
        return false;
    }
    c->rx_header = hdr;
    c->rx_rem_len = 0U;
    c->rx_mult = 1U;
    c->rx_phase = SYN_MQTT_RX_REMAINING_LEN;
    c->rx_deadline = syn_port_get_tick_ms() + MQTT_RECV_TIMEOUT_MS;
    return true;
}

/**
 * @brief Read and decode one remaining-length varint byte.
 * @param c MQTT client context.
 * @return true if a valid byte was consumed without resetting the session.
 */
static bool mqtt_rx_rem_len(SYN_MqttClient *c)
{
    uint8_t b = 0U;
    if (mqtt_recv_checked(c, &b, 1U) <= 0) {
        return false;
    }
    c->rx_rem_len += (uint32_t)(b & 0x7FU) * c->rx_mult;
    if ((b & 0x80U) != 0U) {
        if (c->rx_mult >= 2097152U) {
            mqtt_reset_session(c);
            return false;
        }
        c->rx_mult *= 128U;
        return true;
    }

    c->rx_pos = 0U;
    if (c->rx_rem_len == 0U) {
        process_packet(c);
        c->rx_phase = SYN_MQTT_RX_IDLE;
    } else if (c->rx_rem_len <= c->rx_buf_size) {
        c->rx_phase = SYN_MQTT_RX_PAYLOAD;
    } else {
        c->rx_phase = SYN_MQTT_RX_DISCARD;
    }
    return true;
}

/**
 * @brief Accumulate payload bytes into c->rx_buf and dispatch when complete.
 * @param c MQTT client context.
 * @return true if payload bytes were consumed.
 */
static bool mqtt_rx_payload(SYN_MqttClient *c)
{
    size_t space = (size_t)c->rx_rem_len - c->rx_pos;
    int n = mqtt_recv_checked(c, c->rx_buf + c->rx_pos, space);
    if (n <= 0) {
        return false;
    }
    c->rx_pos += (size_t)n;
    if (c->rx_pos == c->rx_rem_len) {
        process_packet(c);
        c->rx_phase = SYN_MQTT_RX_IDLE;
    }
    return true;
}

/**
 * @brief Drain oversized packet payload bytes without buffering.
 * @param c MQTT client context.
 * @return true if discard bytes were consumed.
 */
static bool mqtt_rx_discard(SYN_MqttClient *c)
{
    uint8_t drop_buf[64];
    size_t space = (size_t)c->rx_rem_len - c->rx_pos;
    if (space > sizeof(drop_buf)) {
        space = sizeof(drop_buf);
    }

    int n = mqtt_recv_checked(c, drop_buf, space);
    if (n <= 0) {
        return false;
    }
    c->rx_pos += (size_t)n;
    if (c->rx_pos == c->rx_rem_len) {
        c->rx_phase = SYN_MQTT_RX_IDLE;
    }
    return true;
}

/**
 * @brief Non-blocking receive state machine poll (up to 32 steps per slice).
 * @param c MQTT client context.
 * @return true if at least one socket read consumed >= 1 byte.
 */
static bool poll_rx(SYN_MqttClient *c)
{
    if (c->sock == SYN_SOCKET_INVALID) {
        return false;
    }

    bool progressed = false;
    for (size_t steps = 0U; steps < 32U && c->sock != SYN_SOCKET_INVALID; steps++) {
        if (c->rx_phase != SYN_MQTT_RX_IDLE &&
            (int32_t)(syn_port_get_tick_ms() - c->rx_deadline) >= 0) {
            mqtt_reset_session(c);
            return progressed;
        }

        bool step_ok = false;
        if (c->rx_phase == SYN_MQTT_RX_IDLE) {
            step_ok = mqtt_rx_idle(c);
        } else if (c->rx_phase == SYN_MQTT_RX_REMAINING_LEN) {
            step_ok = mqtt_rx_rem_len(c);
        } else if (c->rx_phase == SYN_MQTT_RX_PAYLOAD) {
            step_ok = mqtt_rx_payload(c);
        } else {
            step_ok = mqtt_rx_discard(c);
        }

        if (!step_ok) {
            break;
        }
        progressed = true;
    }
    return progressed;
}

/**
 * @brief Allocate the next non-zero 16-bit MQTT packet identifier.
 * @param c MQTT client context.
 * @return Non-zero packet identifier (1..65535).
 */
static uint16_t mqtt_next_packet_id(SYN_MqttClient *c)
{
    c->next_packet_id++;
    if (c->next_packet_id == 0U) {
        c->next_packet_id = 1U;
    }
    return c->next_packet_id;
}

/**
 * @brief Cache an encoded QoS 1 PUBLISH packet in the bounded retransmit buffer.
 * @param c   MQTT client context.
 * @param tx  Encoded PUBLISH frame.
 * @param len Total frame length.
 */
static void mqtt_store_retransmit(SYN_MqttClient *c, const uint8_t *tx, size_t len)
{
    memcpy(c->retransmit_buf, tx, len);
    c->retransmit_len = len;
}

/* ── Public API ─────────────────────────────────────────────────────────── */

SYN_Status syn_mqtt_init(SYN_MqttClient *client, const char *host, uint16_t port,
                         const char *client_id, const char *username, const char *password,
                         uint16_t keep_alive_s, uint8_t *rx_buf, size_t rx_buf_size,
                         uint8_t *tx_buf, size_t tx_buf_size)
{
    SYN_ASSERT(client != NULL);
    SYN_ASSERT(host != NULL);
    SYN_ASSERT(client_id != NULL);
    SYN_ASSERT(rx_buf != NULL);
    SYN_ASSERT(tx_buf != NULL);

    memset(client, 0, sizeof(*client));
    client->host = host;
    client->port = port;
    client->client_id = client_id;
    client->username = username;
    client->password = password;
    client->keep_alive_s = keep_alive_s;
    client->rx_buf = rx_buf;
    client->rx_buf_size = rx_buf_size;
    client->tx_buf = tx_buf;
    client->tx_buf_size = tx_buf_size;
    client->sock = SYN_SOCKET_INVALID;
    client->state = SYN_MQTT_DISCONNECTED;
    client->rx_phase = SYN_MQTT_RX_IDLE;

    SYN_METRIC_REGISTER(mqtt_tx_msgs);
    SYN_METRIC_REGISTER(mqtt_rx_msgs);
    SYN_METRIC_REGISTER(mqtt_errors);

    return SYN_OK;
}

/**
 * @brief Validate arguments and client state for syn_mqtt_publish.
 * @param client  MQTT client context.
 * @param topic   Topic string.
 * @param payload Payload pointer.
 * @param len     Payload length.
 * @param qos     Requested QoS level (0 or 1).
 * @return true if valid and client is connected, false otherwise.
 */
static bool mqtt_publish_args_ok(const SYN_MqttClient *client, const char *topic,
                                 const void *payload, size_t len, uint8_t qos)
{
    if (client == NULL || topic == NULL || client->state != SYN_MQTT_CONNECTED || qos > 1U) {
        return false;
    }
    if (qos == 1U && client->pending_puback_id != 0U) {
        return false;
    }
    return (len == 0U) || (payload != NULL);
}

SYN_Status syn_mqtt_publish(SYN_MqttClient *client, const char *topic, const void *payload,
                            size_t len, uint8_t qos, bool retain)
{
    if (!mqtt_publish_args_ok(client, topic, payload, len, qos)) {
        return SYN_ERROR;
    }

    size_t topic_len = strlen(topic);
    size_t hdr_rem = 2U + topic_len + ((qos > 0U) ? 2U : 0U);
    if (topic_len > UINT16_MAX || len > 268435455U - hdr_rem) {
        return SYN_ERROR;
    }

    uint32_t rem_len = (uint32_t)(hdr_rem + len);
    uint8_t varint_buf[4];
    size_t varint_len = encode_remaining_len(varint_buf, rem_len);
    size_t total_len = 1U + varint_len + (size_t)rem_len;
    if (client->tx_buf == NULL || total_len > client->tx_buf_size ||
        (qos > 0U && total_len > sizeof(client->retransmit_buf))) {
        return SYN_ERROR;
    }

    uint8_t *tx = client->tx_buf;
    tx[0] = (uint8_t)(0x30U | ((uint32_t)qos << 1U) | (retain ? 0x01U : 0U));
    memcpy(tx + 1U, varint_buf, varint_len);
    size_t pos = 1U + varint_len;

    mqtt_put_str(tx, &pos, topic, (uint16_t)topic_len);
    uint16_t pkt_id = 0U;
    if (qos > 0U) {
        pkt_id = mqtt_next_packet_id(client);
        syn_poke_u16(pkt_id, tx, pos);
        pos += 2U;
    }
    if (len > 0U) {
        memcpy(tx + pos, payload, len);
        pos += len;
    }

    if (!mqtt_send_tracked(client, tx, pos)) {
        return SYN_ERROR;
    }
    if (qos > 0U) {
        client->pending_puback_id = pkt_id;
        client->pending_puback_ms = client->last_activity_ms;
        mqtt_store_retransmit(client, tx, pos);
    }
    return SYN_OK;
}

SYN_Status syn_mqtt_subscribe(SYN_MqttClient *client, const char *topic, uint8_t qos)
{
    if (client == NULL || topic == NULL || client->state != SYN_MQTT_CONNECTED) {
        return SYN_ERROR;
    }
    if (qos > 1U) {
        return SYN_INVALID_PARAM;
    }

    size_t topic_len = strlen(topic);
    if (topic_len > UINT16_MAX) {
        return SYN_ERROR;
    }

    uint32_t rem_len = (uint32_t)(5U + topic_len);
    uint8_t varint_buf[4];
    size_t varint_len = encode_remaining_len(varint_buf, rem_len);
    size_t total_len = 1U + varint_len + (size_t)rem_len;
    if (client->tx_buf == NULL || total_len > client->tx_buf_size) {
        return SYN_ERROR;
    }

    uint16_t pkt_id = mqtt_next_packet_id(client);
    uint8_t *tx = client->tx_buf;
    tx[0] = 0x82U; /* SUBSCRIBE */
    memcpy(tx + 1U, varint_buf, varint_len);
    size_t pos = 1U + varint_len;

    syn_poke_u16(pkt_id, tx, pos);
    pos += 2U;
    mqtt_put_str(tx, &pos, topic, (uint16_t)topic_len);
    tx[pos++] = qos;

    return mqtt_send_tracked(client, tx, pos) ? SYN_OK : SYN_ERROR;
}

void syn_mqtt_disconnect(SYN_MqttClient *client)
{
    if (client == NULL) {
        return;
    }

    if (client->state == SYN_MQTT_CONNECTED && client->sock != SYN_SOCKET_INVALID) {
        static const uint8_t disc[] = {0xE0U, 0x00U};
        (void)syn_port_sock_send_all(client->sock, disc, 2U);
    }
    mqtt_reset_session(client);
}

/**
 * @brief Check whether QoS 1 retransmit or keep-alive timers have elapsed while connected.
 * @param c   MQTT client context.
 * @param now Current millisecond tick.
 * @return true if a connected-state timer is due.
 */
static bool mqtt_connected_timers_due(const SYN_MqttClient *c, uint32_t now)
{
    if (c->state != SYN_MQTT_CONNECTED) {
        return false;
    }
    if (c->ping_pending && (now - c->ping_sent_ms) >= MQTT_ACK_TIMEOUT_MS) {
        return true;
    }
    if (c->pending_puback_id != 0U && (now - c->pending_puback_ms) >= MQTT_ACK_TIMEOUT_MS) {
        return true;
    }
    if (!c->ping_pending && c->keep_alive_s > 0U &&
        (now - c->last_activity_ms) >= (uint32_t)c->keep_alive_s * 1000U) {
        return true;
    }
    return false;
}

/**
 * @brief Check if MQTT client task has actionable work.
 * @param c Pointer to MQTT client instance.
 * @return true if work pending, false otherwise.
 */
static bool mqtt_has_work(const SYN_MqttClient *c)
{
    if (c->state == SYN_MQTT_DISCONNECTED) {
        return true;
    }
    if (c->sock != SYN_SOCKET_INVALID && syn_port_sock_readable(c->sock)) {
        return true;
    }

    uint32_t now = syn_port_get_tick_ms();
    if (c->rx_phase != SYN_MQTT_RX_IDLE && (int32_t)(now - c->rx_deadline) >= 0) {
        return true;
    }
    if (c->state == SYN_MQTT_CONNECTING && (now - c->last_activity_ms) >= MQTT_ACK_TIMEOUT_MS) {
        return true;
    }
    return mqtt_connected_timers_due(c, now);
}

/**
 * @brief Open a fresh TCP socket and send MQTT CONNECT.
 * @param c MQTT client context.
 * @return true if socket connected and CONNECT frame was sent.
 */
static bool mqtt_try_connect(SYN_MqttClient *c)
{
    mqtt_reset_session(c);
    c->sock = syn_port_sock_connect_host(c->host, c->port);
    if (c->sock == SYN_SOCKET_INVALID) {
        return false;
    }
    c->state = SYN_MQTT_CONNECTING;
    c->rx_phase = SYN_MQTT_RX_IDLE;
    if (!send_mqtt_connect(c)) {
        mqtt_reset_session(c);
        return false;
    }
    c->last_activity_ms = syn_port_get_tick_ms();
    return true;
}

/**
 * @brief Enforce PINGRESP timeout or transmit due keep-alive PINGREQ while connected.
 * @param c   MQTT client context.
 * @param now Current millisecond tick.
 * @return true if session remains connected, false if session was reset.
 */
static bool mqtt_step_keepalive(SYN_MqttClient *c, uint32_t now)
{
    if (c->ping_pending) {
        if ((now - c->ping_sent_ms) >= MQTT_ACK_TIMEOUT_MS) {
            mqtt_reset_session(c);
            return false;
        }
        return true;
    }
    if (c->keep_alive_s > 0U && (now - c->last_activity_ms) >= (uint32_t)c->keep_alive_s * 1000U) {
        if (!send_mqtt_ping(c)) {
            mqtt_reset_session(c);
            return false;
        }
        c->last_activity_ms = now;
        c->ping_pending = true;
        c->ping_sent_ms = now;
    }
    return true;
}

/**
 * @brief Advance an active (CONNECTING or CONNECTED) MQTT session by one non-blocking slice.
 * @param c MQTT client context.
 * @return true if RX polling consumed at least one byte from the socket.
 */
static bool mqtt_step_active(SYN_MqttClient *c)
{
    if (c->state == SYN_MQTT_CONNECTING &&
        (syn_port_get_tick_ms() - c->last_activity_ms) >= MQTT_ACK_TIMEOUT_MS) {
        mqtt_reset_session(c);
        return false;
    }

    bool progressed = poll_rx(c);
    if (c->state != SYN_MQTT_CONNECTED) {
        return progressed;
    }

    uint32_t now = syn_port_get_tick_ms();
    if (!mqtt_step_keepalive(c, now)) {
        return false;
    }

    now = syn_port_get_tick_ms();
    if ((c->pending_puback_id != 0U) && (c->retransmit_len > 0U) &&
        ((now - c->pending_puback_ms) >= MQTT_ACK_TIMEOUT_MS)) {
        c->retransmit_buf[0] |= 0x08U;
        if (!mqtt_send_tracked(c, c->retransmit_buf, c->retransmit_len)) {
            return false;
        }
        c->pending_puback_ms = c->last_activity_ms;
    }
    return progressed;
}

/**
 * @brief Check whether sustained RX progress left additional readable bytes on an active socket.
 * @param c          MQTT client context.
 * @param progressed Whether the preceding poll slice consumed bytes.
 * @return true if another cooperative RX slice should run immediately.
 */
static bool mqtt_more_rx_ready(const SYN_MqttClient *c, bool progressed)
{
    return progressed && (c->sock != SYN_SOCKET_INVALID) && syn_port_sock_readable(c->sock);
}

/* ── Protothread task ──────────────────────────────────────────────────── */

SYN_PT_Status syn_mqtt_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }

    SYN_MqttClient *c = (SYN_MqttClient *)task->user_data;

    PT_BEGIN(pt);

    if (c->state != SYN_MQTT_CONNECTED) {
        mqtt_reset_session(c);
    }

    for (;;) {
        if (c->state == SYN_MQTT_DISCONNECTED) {
            if (!mqtt_try_connect(c)) {
                PT_TASK_DELAY_MS(pt, task, 5000);
                continue;
            }
        }

        bool progressed = mqtt_step_active(c);
        if (c->state == SYN_MQTT_DISCONNECTED) {
            PT_TASK_DELAY_MS(pt, task, 5000);
            continue;
        }

        if (mqtt_more_rx_ready(c, progressed)) {
            PT_CONTINUE_IF_NEEDED(pt, task);
            continue;
        }

        PT_WAIT_UNTIL(pt, mqtt_has_work(c));
    }

    PT_END(pt);
}

#endif /* SYN_USE_MQTT */
