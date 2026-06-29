/**
 * @file syn_mqtt.h
 * @brief Lightweight MQTT 3.1.1 client.
 * @ingroup syn_net
 */

#ifndef SYN_MQTT_H
#define SYN_MQTT_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MQTT client connection states.
 */
typedef enum {
    SYN_MQTT_DISCONNECTED,           /**< Connection is down, client inactive */
    SYN_MQTT_CONNECTING,             /**< Actively opening TCP socket and sending MQTT CONNECT */
    SYN_MQTT_CONNECTED,              /**< Connected and authenticated, ready to sub/pub */
} SYN_MqttState;

/**
 * @brief MQTT client context structure.
 */
typedef struct {
    SYN_Socket       sock;           /**< TCP socket connection to broker */
    SYN_MqttState    state;          /**< Connection state machine status */
    const char      *host;           /**< Hostname or IP of the broker */
    uint16_t         port;           /**< Broker Port (typically 1883) */
    const char      *client_id;      /**< Client identifier string (must be unique) */
    const char      *username;       /**< Optional login username, or NULL */
    const char      *password;       /**< Optional login password, or NULL */
    uint16_t         keep_alive_s;   /**< Keep alive ping interval in seconds */
    
    /**
     * @brief User callback for incoming publications.
     * @param topic Topic name string.
     * @param payload Received data payload buffer.
     * @param len Size of payload in bytes.
     * @param ctx User-defined context pointer.
     */
    void (*on_message)(const char *topic, const uint8_t *payload, size_t len, void *ctx);
    void            *ctx;            /**< Context pointer for on_message callback */

    uint8_t         *rx_buf;         /**< Receive packet formatting buffer */
    size_t           rx_buf_size;    /**< Capacity of rx_buf */
    uint8_t         *tx_buf;         /**< Transmit packet formatting buffer */
    size_t           tx_buf_size;    /**< Capacity of tx_buf */

    uint32_t         last_activity_ms; /**< Timestamp of last transmitted or received packet */
    uint16_t         next_packet_id;  /**< Sequence counter for packet identifiers */
    
    uint16_t         pending_puback_id; /**< Awaiting QoS 1 puback confirmation packet ID */
    uint32_t         pending_puback_ms; /**< Timeout timer for pending puback confirmation */
    uint8_t          retransmit_buf[128]; /**< Buffer for storing unacknowledged QoS 1 packet */
    size_t           retransmit_len;    /**< Length of packet in retransmit_buf */
} SYN_MqttClient;

/**
 * @brief Initialize the MQTT client.
 *
 * Configures broker destination, client ID, authentication credentials,
 * keep-alive timing parameters, and network packet buffers.
 *
 * @param client       Pointer to client context.
 * @param host         Broker network address string.
 * @param port         Broker port number.
 * @param client_id    MQTT client identity string.
 * @param username     Authentication username (or NULL).
 * @param password     Authentication password (or NULL).
 * @param keep_alive_s Keep-alive timeout parameter in seconds.
 * @param rx_buf       Receive buffer storage.
 * @param rx_buf_size  Receive buffer capacity.
 * @param tx_buf       Transmit buffer storage.
 * @param tx_buf_size  Transmit buffer capacity.
 * @return SYN_OK on successful configuration, or error parameter code.
 */
SYN_Status syn_mqtt_init(SYN_MqttClient *client, const char *host, uint16_t port,
                         const char *client_id, const char *username, const char *password,
                         uint16_t keep_alive_s,
                         uint8_t *rx_buf, size_t rx_buf_size,
                         uint8_t *tx_buf, size_t tx_buf_size);

/**
 * @brief Publish a message to a topic.
 *
 * Non-blocking publish command. For QoS 0, queued directly. For QoS 1,
 * tracks acknowledgement state.
 *
 * @param client       Pointer to client context.
 * @param topic        Topic name to target.
 * @param payload      Data payload to send.
 * @param len          Payload size in bytes.
 * @param qos          Quality of service level (0 or 1).
 * @param retain       Retain flag on broker.
 * @return SYN_OK on queued, or error status if payload bounds exceeded.
 */
SYN_Status syn_mqtt_publish(SYN_MqttClient *client, const char *topic,
                            const void *payload, size_t len, uint8_t qos, bool retain);

/**
 * @brief Subscribe to a topic.
 *
 * Formats and queues a subscription request for transmission.
 *
 * @param client       Pointer to client context.
 * @param topic        Topic filter string.
 * @param qos          Requested quality of service.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqtt_subscribe(SYN_MqttClient *client, const char *topic, uint8_t qos);

/**
 * @brief Cooperative task for driving the MQTT client.
 *
 * Yields during connection, socket polling, keep-alive pinging, and packet
 * parsing loops. Runs within the cooperative scheduler context.
 *
 * @param pt   Cooperative protothread handle.
 * @param task Corresponding task control block.
 * @return PT_WAITING or PT_EXITED status.
 */
SYN_PT_Status syn_mqtt_task(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MQTT_H */
