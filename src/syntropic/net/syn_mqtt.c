#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_MQTT) || SYN_USE_MQTT

/**
 * @file syn_mqtt.c
 * @brief Lightweight MQTT 3.1.1 client implementation — fully non-blocking.
 */

#include "syn_mqtt.h"
#include "../util/syn_assert.h"
#include "../port/syn_port_system.h"
#include <string.h>

#define MQTT_RECV_TIMEOUT_MS  5000  /**< Timeout for incomplete packet reception (ms). */
#define MQTT_ACK_TIMEOUT_MS   5000  /**< Timeout for ACK responses (ms). */

/* ── Remaining Length Helper ────────────────────────────────────────────── */

/**
 * @brief Encode MQTT remaining length into variable-length bytes.
 * @param buf  Output buffer (at least 4 bytes).
 * @param len  Length value to encode.
 * @return Number of bytes written.
 */
static size_t encode_remaining_len(uint8_t *buf, uint32_t len)
{
    size_t bytes = 0;
    do {
        uint8_t d = (uint8_t)(len % 128);
        len /= 128;
        if (len > 0) d |= 128;
        buf[bytes++] = d;
    } while (len > 0);
    return bytes;
}

/* ── Connection Packets ─────────────────────────────────────────────────── */

/**
 * @brief Build and send an MQTT CONNECT packet.
 * @param c  MQTT client.
 * @return true on success.
 */
static bool send_mqtt_connect(SYN_MqttClient *c)
{
    uint8_t *tx = c->tx_buf;
    
    uint32_t rem_len = 2 + 4 + 1 + 1 + 2; /* protocol name, level, flags, keep_alive */
    rem_len += 2 + strlen(c->client_id);
    if (c->username != NULL) rem_len += 2 + strlen(c->username);
    if (c->password != NULL) rem_len += 2 + strlen(c->password);

    if (1 + 4 + rem_len > c->tx_buf_size) return false;

    tx[0] = 0x10; /* CONNECT */
    size_t pos = 1;
    pos += encode_remaining_len(tx + pos, rem_len);

    tx[pos++] = 0x00; tx[pos++] = 0x04;
    tx[pos++] = 'M'; tx[pos++] = 'Q'; tx[pos++] = 'T'; tx[pos++] = 'T';
    tx[pos++] = 0x04; /* Level 3.1.1 */

    uint8_t flags = 0x02; /* Clean session */
    if (c->username != NULL) flags |= 0x80;
    if (c->password != NULL) flags |= 0x40;
    tx[pos++] = flags;

    tx[pos++] = (uint8_t)(c->keep_alive_s >> 8);
    tx[pos++] = (uint8_t)(c->keep_alive_s & 0xFF);

    uint16_t cid_len = (uint16_t)strlen(c->client_id);
    tx[pos++] = (uint8_t)(cid_len >> 8); tx[pos++] = (uint8_t)(cid_len & 255);
    memcpy(tx + pos, c->client_id, cid_len); pos += cid_len;

    if (c->username != NULL) {
        uint16_t u_len = (uint16_t)strlen(c->username);
        tx[pos++] = (uint8_t)(u_len >> 8); tx[pos++] = (uint8_t)(u_len & 255);
        memcpy(tx + pos, c->username, u_len); pos += u_len;
    }

    if (c->password != NULL) {
        uint16_t p_len = (uint16_t)strlen(c->password);
        tx[pos++] = (uint8_t)(p_len >> 8); tx[pos++] = (uint8_t)(p_len & 255);
        memcpy(tx + pos, c->password, p_len); pos += p_len;
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
    static const uint8_t ping[] = { 0xC0, 0x00 };
    return syn_port_sock_send_all(c->sock, ping, 2) == 2;
}

/* ── Handlers ───────────────────────────────────────────────────────────── */

/**
 * @brief Handle an incoming PUBLISH packet.
 * @param c         MQTT client.
 * @param payload   Raw payload (after fixed header).
 * @param len       Payload length.
 * @param qos_bits  QoS flags from the fixed header.
 */
static void handle_publish(SYN_MqttClient *c, const uint8_t *payload, uint32_t len, uint8_t qos_bits)
{
    if (len < 2) return;
    
    uint16_t topic_len = (uint16_t)(((uint16_t)payload[0] << 8) | payload[1]);
    if ((uint32_t)(2 + topic_len) > len) return;

    /* Extract topic string dynamically */
    char topic[64];
    size_t to_copy = topic_len < sizeof(topic) - 1 ? topic_len : sizeof(topic) - 1;
    memcpy(topic, payload + 2, to_copy);
    topic[to_copy] = '\0';

    size_t payload_offset = 2 + topic_len;
    uint16_t packet_id = 0;
    uint8_t qos = (qos_bits >> 1) & 0x03;

    if (qos > 0) {
        if (payload_offset + 2 > len) return;
        packet_id = (uint16_t)(((uint16_t)payload[payload_offset] << 8) | payload[payload_offset+1]);
        payload_offset += 2;
    }

    if (c->on_message != NULL) {
        c->on_message(topic, payload + payload_offset, len - payload_offset, c->ctx);
    }

    /* QoS 1 ACK (PUBACK) */
    if (qos == 1) {
        const uint8_t puback[] = { 0x40, 0x02, (uint8_t)(packet_id >> 8), (uint8_t)(packet_id & 255) };
        syn_port_sock_send_all(c->sock, puback, 4);
    }
}

/**
 * @brief Dispatch a fully received MQTT packet.
 * @param c  MQTT client context.
 */
static void process_packet(SYN_MqttClient *c)
{
    uint8_t type = c->rx_header & 0xF0;
    uint32_t rem_len = c->rx_rem_len;

    if (type == 0x20) {
        /* CONNACK */
        if (rem_len >= 2 && c->rx_buf[1] == 0) {
            c->state = SYN_MQTT_CONNECTED;
        } else {
            syn_port_sock_close(c->sock);
            c->sock = SYN_SOCKET_INVALID;
            c->state = SYN_MQTT_DISCONNECTED;
            c->rx_phase = SYN_MQTT_RX_IDLE;
        }
    } else if (type == 0x30) {
        /* PUBLISH */
        handle_publish(c, c->rx_buf, rem_len, c->rx_header & 0x0F);
    } else if (type == 0x40) {
        /* PUBACK */
        if (rem_len >= 2) {
            uint16_t pid = (uint16_t)(((uint16_t)c->rx_buf[0] << 8) | c->rx_buf[1]);
            if (pid == c->pending_puback_id) {
                c->pending_puback_id = 0;
            }
        }
    } else if (type == 0xD0) {
        /* PINGRESP — reset keep alive activity timer */
    }
}

/**
 * @brief Non-blocking receive state machine poll.
 *
 * Reads available bytes with timeout=0. Advances through RX phases:
 *   IDLE → REMAINING_LEN → PAYLOAD / DISCARD → IDLE
 *
 * @param c  MQTT client.
 */
static void poll_rx(SYN_MqttClient *c)
{
    if (c->sock == SYN_SOCKET_INVALID) return;

    for (;;) {
        /* Non-blocking deadline check for mid-packet stalling */
        if (c->rx_phase != SYN_MQTT_RX_IDLE) {
            if ((int32_t)(syn_port_get_tick_ms() - c->rx_deadline) >= 0) {
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_MQTT_DISCONNECTED;
                c->rx_phase = SYN_MQTT_RX_IDLE;
                return;
            }
        }

        switch (c->rx_phase) {
        case SYN_MQTT_RX_IDLE: {
            uint8_t header;
            int n = syn_port_sock_recv(c->sock, &header, 1, 0);
            if (n < 0) return; /* No data ready */
            if (n == 0) {
                /* Connection closed by server */
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_MQTT_DISCONNECTED;
                return;
            }
            c->last_activity_ms = syn_port_get_tick_ms();
            c->rx_header  = header;
            c->rx_rem_len = 0;
            c->rx_mult    = 1;
            c->rx_phase   = SYN_MQTT_RX_REMAINING_LEN;
            c->rx_deadline= syn_port_get_tick_ms() + MQTT_RECV_TIMEOUT_MS;
            break;
        }

        case SYN_MQTT_RX_REMAINING_LEN: {
            uint8_t b;
            int n = syn_port_sock_recv(c->sock, &b, 1, 0);
            if (n < 0) return; /* No data ready */
            if (n == 0) {
                /* Connection closed during packet header */
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_MQTT_DISCONNECTED;
                c->rx_phase = SYN_MQTT_RX_IDLE;
                return;
            }
            c->last_activity_ms = syn_port_get_tick_ms();
            c->rx_rem_len += (uint32_t)(b & 127) * c->rx_mult;
            if (c->rx_mult > 128UL * 128UL * 128UL) {
                /* Multiplier overflow error */
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_MQTT_DISCONNECTED;
                c->rx_phase = SYN_MQTT_RX_IDLE;
                return;
            }
            c->rx_mult *= 128;

            if ((b & 128) == 0) {
                /* Varint remaining length decoded */
                c->rx_pos = 0;
                if (c->rx_rem_len == 0) {
                    process_packet(c);
                    c->rx_phase = SYN_MQTT_RX_IDLE;
                } else if (c->rx_rem_len <= c->rx_buf_size) {
                    c->rx_phase = SYN_MQTT_RX_PAYLOAD;
                } else {
                    /* Oversized packet — discard payload */
                    c->rx_phase = SYN_MQTT_RX_DISCARD;
                }
            }
            break;
        }

        case SYN_MQTT_RX_PAYLOAD: {
            size_t space = c->rx_rem_len - c->rx_pos;
            int n = syn_port_sock_recv(c->sock, c->rx_buf + c->rx_pos, space, 0);
            if (n < 0) return; /* No data ready */
            if (n == 0) {
                /* Connection closed during payload */
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_MQTT_DISCONNECTED;
                c->rx_phase = SYN_MQTT_RX_IDLE;
                return;
            }
            c->last_activity_ms = syn_port_get_tick_ms();
            c->rx_pos += (size_t)n;
            if (c->rx_pos == c->rx_rem_len) {
                process_packet(c);
                c->rx_phase = SYN_MQTT_RX_IDLE;
            }
            break;
        }

        case SYN_MQTT_RX_DISCARD: {
            uint8_t drop_buf[64];
            size_t space = c->rx_rem_len - c->rx_pos;
            if (space > sizeof(drop_buf)) space = sizeof(drop_buf);

            int n = syn_port_sock_recv(c->sock, drop_buf, space, 0);
            if (n < 0) return; /* No data ready */
            if (n == 0) {
                /* Connection closed during discard */
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_MQTT_DISCONNECTED;
                c->rx_phase = SYN_MQTT_RX_IDLE;
                return;
            }
            c->last_activity_ms = syn_port_get_tick_ms();
            c->rx_pos += (size_t)n;
            if (c->rx_pos == c->rx_rem_len) {
                c->rx_phase = SYN_MQTT_RX_IDLE;
            }
            break;
        }
        }
    }
}

/* ── Public API ─────────────────────────────────────────────────────────── */

SYN_Status syn_mqtt_init(SYN_MqttClient *client, const char *host, uint16_t port,
                         const char *client_id, const char *username, const char *password,
                         uint16_t keep_alive_s,
                         uint8_t *rx_buf, size_t rx_buf_size,
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

    return SYN_OK;
}

SYN_Status syn_mqtt_publish(SYN_MqttClient *client, const char *topic,
                            const void *payload, size_t len, uint8_t qos, bool retain)
{
    SYN_ASSERT(client != NULL);
    SYN_ASSERT(topic != NULL);
    if (client->state != SYN_MQTT_CONNECTED) return SYN_ERROR;

    uint16_t pkt_id = 0;
    if (qos > 0) {
        pkt_id = ++client->next_packet_id;
        if (pkt_id == 0) pkt_id = 1;
        client->pending_puback_id = pkt_id;
        client->pending_puback_ms = syn_port_get_tick_ms();
    }

    uint32_t rem_len = 2u + (uint32_t)strlen(topic) + (qos > 0 ? 2u : 0u) + (uint32_t)len;
    uint8_t *tx = client->tx_buf;

    tx[0] = (uint8_t)(0x30u | ((uint32_t)qos << 1) | (retain ? 0x01u : 0u));
    size_t pos = 1;
    pos += encode_remaining_len(tx + pos, rem_len);

    uint16_t t_len = (uint16_t)strlen(topic);
    tx[pos++] = (uint8_t)(t_len >> 8); tx[pos++] = (uint8_t)(t_len & 255);
    memcpy(tx + pos, topic, t_len); pos += t_len;

    if (qos > 0) {
        tx[pos++] = (uint8_t)(pkt_id >> 8); tx[pos++] = (uint8_t)(pkt_id & 255);
    }
    if (len > 0 && payload != NULL) {
        memcpy(tx + pos, payload, len); pos += len;
    }

    if (qos > 0) {
        if (pos <= sizeof(client->retransmit_buf)) {
            memcpy(client->retransmit_buf, tx, pos);
            client->retransmit_len = pos;
        } else {
            client->retransmit_len = 0;
        }
    }

    int sent = syn_port_sock_send_all(client->sock, tx, pos);
    return (sent == (int)pos) ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_mqtt_subscribe(SYN_MqttClient *client, const char *topic, uint8_t qos)
{
    SYN_ASSERT(client != NULL);
    SYN_ASSERT(topic != NULL);
    if (client->state != SYN_MQTT_CONNECTED) return SYN_ERROR;

    uint16_t pkt_id = ++client->next_packet_id;
    if (pkt_id == 0) pkt_id = 1;

    uint32_t rem_len = 2u + 2u + (uint32_t)strlen(topic) + 1u; /* packet id, topic len, topic, qos */
    uint8_t *tx = client->tx_buf;

    tx[0] = 0x82; /* SUBSCRIBE */
    size_t pos = 1;
    pos += encode_remaining_len(tx + pos, rem_len);

    tx[pos++] = (uint8_t)(pkt_id >> 8); tx[pos++] = (uint8_t)(pkt_id & 255);

    uint16_t t_len = (uint16_t)strlen(topic);
    tx[pos++] = (uint8_t)(t_len >> 8); tx[pos++] = (uint8_t)(t_len & 255);
    memcpy(tx + pos, topic, t_len); pos += t_len;

    tx[pos++] = qos;

    if (qos > 0) {
        if (pos <= sizeof(client->retransmit_buf)) {
            memcpy(client->retransmit_buf, tx, pos);
            client->retransmit_len = pos;
        } else {
            client->retransmit_len = 0;
        }
    }

    int sent = syn_port_sock_send_all(client->sock, tx, pos);
    return (sent == (int)pos) ? SYN_OK : SYN_ERROR;
}

SYN_PT_Status syn_mqtt_task(SYN_PT *pt, SYN_Task *task)
{
    SYN_MqttClient *c = (SYN_MqttClient *)task->user_data;
    SYN_ASSERT(c != NULL);

    PT_BEGIN(pt);

    for (;;) {
        if (c->state == SYN_MQTT_DISCONNECTED) {
            c->sock = syn_port_sock_connect_host(c->host, c->port);
            if (c->sock == SYN_SOCKET_INVALID) {
                PT_TASK_DELAY_MS(pt, task, 5000);
                continue;
            }
            c->state = SYN_MQTT_CONNECTING;
            c->rx_phase = SYN_MQTT_RX_IDLE;
            if (!send_mqtt_connect(c)) {
                syn_port_sock_close(c->sock);
                c->sock = SYN_SOCKET_INVALID;
                c->state = SYN_MQTT_DISCONNECTED;
                PT_TASK_DELAY_MS(pt, task, 5000);
                continue;
            }
            c->last_activity_ms = syn_port_get_tick_ms();
        }

        if (c->state == SYN_MQTT_CONNECTING || c->state == SYN_MQTT_CONNECTED) {
            /* Check QoS 1 Puback timeout */
            if (c->pending_puback_id != 0) {
                uint32_t now = syn_port_get_tick_ms();
                if ((now - c->pending_puback_ms) >= MQTT_ACK_TIMEOUT_MS) {
                    /* Retransmit */
                    c->pending_puback_ms = now;
                    if (c->retransmit_len > 0) {
                        /* Set DUP flag (0x08) */
                        c->retransmit_buf[0] |= 0x08;
                        syn_port_sock_send_all(c->sock, c->retransmit_buf, c->retransmit_len);
                    }
                }
            }

            /* Poll socket for incoming packets (non-blocking state machine) */
            poll_rx(c);

            /* Keep alive timer */
            uint32_t now = syn_port_get_tick_ms();
            if (c->state == SYN_MQTT_CONNECTED && c->keep_alive_s > 0) {
                if ((now - c->last_activity_ms) >= (uint32_t)c->keep_alive_s * 1000) {
                    if (send_mqtt_ping(c)) {
                        c->last_activity_ms = now;
                    } else {
                        syn_port_sock_close(c->sock);
                        c->sock = SYN_SOCKET_INVALID;
                        c->state = SYN_MQTT_DISCONNECTED;
                        c->rx_phase = SYN_MQTT_RX_IDLE;
                    }
                }
            }
        }
        PT_DEFER(pt, task);
    }

    PT_END(pt);
}

#endif /* SYN_USE_MQTT */
