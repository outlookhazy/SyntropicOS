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
 * @brief MQTT Protocol Version.
 */
typedef enum {
    SYN_MQTT_VERSION_3_1_1 = 4, /**< MQTT 3.1.1 */
    SYN_MQTT_VERSION_5_0 = 5    /**< MQTT 5.0   */
} SYN_MqttVersion;

/**
 * @name MQTT v5.0 Property Identifiers
 * @{
 */
#define SYN_MQTT5_PROP_PAYLOAD_FORMAT_INDICATOR 0x01U /**< Payload Format Indicator */
#define SYN_MQTT5_PROP_MESSAGE_EXPIRY_INTERVAL 0x02U  /**< Message Expiry Interval */
#define SYN_MQTT5_PROP_CONTENT_TYPE 0x03U             /**< Content Type */
#define SYN_MQTT5_PROP_RESPONSE_TOPIC 0x08U           /**< Response Topic */
#define SYN_MQTT5_PROP_CORRELATION_DATA 0x09U         /**< Correlation Data */
#define SYN_MQTT5_PROP_SUBSCRIPTION_IDENTIFIER 0x0BU  /**< Subscription Identifier */
#define SYN_MQTT5_PROP_SESSION_EXPIRY_INTERVAL 0x11U  /**< Session Expiry Interval */
#define SYN_MQTT5_PROP_ASSIGNED_CLIENT_ID 0x12U       /**< Assigned Client Identifier */
#define SYN_MQTT5_PROP_SERVER_KEEP_ALIVE 0x13U        /**< Server Keep Alive */
#define SYN_MQTT5_PROP_AUTH_METHOD 0x15U              /**< Authentication Method */
#define SYN_MQTT5_PROP_AUTH_DATA 0x16U                /**< Authentication Data */
#define SYN_MQTT5_PROP_REQ_PROBLEM_INFO 0x17U         /**< Request Problem Information */
#define SYN_MQTT5_PROP_WILL_DELAY_INTERVAL 0x18U      /**< Will Delay Interval */
#define SYN_MQTT5_PROP_REQ_RESPONSE_INFO 0x19U        /**< Request Response Information */
#define SYN_MQTT5_PROP_RESPONSE_INFO 0x1AU            /**< Response Information */
#define SYN_MQTT5_PROP_SERVER_REFERENCE 0x1CU         /**< Server Reference */
#define SYN_MQTT5_PROP_REASON_STRING 0x1FU            /**< Reason String */
#define SYN_MQTT5_PROP_RECEIVE_MAXIMUM 0x21U          /**< Receive Maximum */
#define SYN_MQTT5_PROP_TOPIC_ALIAS_MAXIMUM 0x22U      /**< Topic Alias Maximum */
#define SYN_MQTT5_PROP_TOPIC_ALIAS 0x23U              /**< Topic Alias */
#define SYN_MQTT5_PROP_MAXIMUM_QOS 0x24U              /**< Maximum QoS */
#define SYN_MQTT5_PROP_RETAIN_AVAILABLE 0x25U         /**< Retain Available */
#define SYN_MQTT5_PROP_USER_PROPERTY 0x26U            /**< User Property */
#define SYN_MQTT5_PROP_MAX_PACKET_SIZE 0x27U          /**< Maximum Packet Size */
#define SYN_MQTT5_PROP_WILDCARD_SUB_AVAIL 0x28U       /**< Wildcard Subscription Available */
#define SYN_MQTT5_PROP_SUB_ID_AVAIL 0x29U             /**< Subscription Identifiers Available */
#define SYN_MQTT5_PROP_SHARED_SUB_AVAIL 0x2AU         /**< Shared Subscription Available */
/** @} */

/**
 * @brief MQTT v5.0 Key-Value User Property.
 */
typedef struct {
    const char *key; /**< Property Name/Key   */
    const char *val; /**< Property Value      */
} SYN_Mqtt5_UserProp;

/**
 * @brief MQTT client connection states.
 */
typedef enum {
    SYN_MQTT_DISCONNECTED, /**< Connection is down, client inactive */
    SYN_MQTT_CONNECTING,   /**< Actively opening TCP socket and sending MQTT CONNECT */
    SYN_MQTT_CONNECTED,    /**< Connected and authenticated, ready to sub/pub */
} SYN_MqttState;

/**
 * @brief Non-blocking packet reception states.
 */
typedef enum {
    SYN_MQTT_RX_IDLE,          /**< Waiting for fixed header opcode byte */
    SYN_MQTT_RX_REMAINING_LEN, /**< Decoding variable-length packet size */
    SYN_MQTT_RX_PAYLOAD,       /**< Accumulating packet payload into rx_buf */
    SYN_MQTT_RX_DISCARD,       /**< Discarding bytes of an oversized packet */
} SYN_MqttRxPhase;

/**
 * @brief MQTT client context structure.
 */
typedef struct {
    SYN_Socket sock;       /**< TCP socket connection to broker */
    SYN_MqttState state;   /**< Connection state machine status */
    const char *host;      /**< Hostname or IP of the broker */
    uint16_t port;         /**< Broker Port (typically 1883) */
    const char *client_id; /**< Client identifier string (must be unique) */
    const char *username;  /**< Optional login username, or NULL */
    const char *password;  /**< Optional login password, or NULL */
    uint16_t keep_alive_s; /**< Keep alive ping interval in seconds */

    /**
     * @brief User callback for incoming publications.
     *
     * @note Inbound topic strings passed to @p topic are truncated to at most
     *       63 bytes (64-byte stack buffer including NUL terminator), while
     *       payload offset calculation advances past the full wire topic length.
     *
     * @param topic Topic name string (NUL-terminated, max 63 bytes).
     * @param payload Received data payload buffer.
     * @param len Size of payload in bytes.
     * @param ctx User-defined context pointer.
     */
    void (*on_message)(const char *topic, const uint8_t *payload, size_t len, void *ctx);
    void *ctx; /**< Context pointer for on_message callback */

    uint8_t *rx_buf;    /**< Receive packet formatting buffer */
    size_t rx_buf_size; /**< Capacity of rx_buf */
    uint8_t *tx_buf;    /**< Transmit packet formatting buffer */
    size_t tx_buf_size; /**< Capacity of tx_buf */

    uint32_t last_activity_ms; /**< Timestamp of last transmitted or received packet */
    uint16_t next_packet_id;   /**< Sequence counter for packet identifiers */

    uint16_t pending_puback_id;  /**< Awaiting QoS 1 puback confirmation packet ID */
    uint32_t pending_puback_ms;  /**< Timeout timer for pending puback confirmation */
    uint8_t retransmit_buf[128]; /**< Buffer for storing unacknowledged QoS 1 packet */
    size_t retransmit_len;       /**< Length of packet in retransmit_buf */
    bool ping_pending;           /**< True while awaiting PINGRESP for an in-flight PINGREQ */
    uint32_t ping_sent_ms;       /**< Timestamp when in-flight PINGREQ was transmitted */

    /* ── Non-blocking RX State Machine ─────────────────────────────────── */
    SYN_MqttRxPhase rx_phase; /**< Current RX state machine phase */
    uint8_t rx_header;        /**< Opcode byte of current packet */
    uint32_t rx_rem_len;      /**< Total remaining length of current packet */
    uint32_t rx_mult;         /**< Multiplier for decoding varint remaining len */
    size_t rx_pos;            /**< Bytes read into rx_buf or discarded so far */
    uint32_t rx_deadline;     /**< Tick deadline for incomplete packet RX */
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
                         uint16_t keep_alive_s, uint8_t *rx_buf, size_t rx_buf_size,
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
SYN_Status syn_mqtt_publish(SYN_MqttClient *client, const char *topic, const void *payload,
                            size_t len, uint8_t qos, bool retain);

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
 * @brief Transmit an explicit MQTT PINGREQ packet.
 *
 * Note: PINGREQ packets are sent automatically by syn_mqtt_task based on
 * the configured keep_alive_s interval. This function allows manual pinging on demand.
 *
 * @param client       Pointer to client context.
 * @return SYN_OK on success, SYN_ERROR if not connected or transmit failed.
 */
SYN_Status syn_mqtt_ping(SYN_MqttClient *client);

/**
 * @brief Disconnect the MQTT client and close underlying TCP socket.
 *
 * Sends an MQTT DISCONNECT packet if currently connected, then closes
 * the socket and transitions client state to DISCONNECTED.
 *
 * @param client Pointer to client context.
 */
void syn_mqtt_disconnect(SYN_MqttClient *client);

/**
 * @brief Cooperative task for driving the MQTT client.
 *
 * Yields during connection, socket polling, keep-alive pinging, and packet
 * parsing loops. Runs within the cooperative scheduler context.
 *
 * @param pt   Cooperative protothread handle.
 * @param task Corresponding task control block.
 * @return PT_IDLE while waiting, PT_ENDED when complete, or PT_EXITED on abort.
 */
SYN_PT_Status syn_mqtt_task(SYN_PT *pt, SYN_Task *task);

/**
 * @brief Encode a Variable Byte Integer (MQTT 3.1.1 & MQTT 5.0).
 *
 * @param val Value to encode (0..268435455).
 * @param buf [out] Output buffer (must have at least 4 bytes capacity).
 * @return Number of bytes written (1..4).
 */
size_t syn_mqtt_encode_varint(uint32_t val, uint8_t buf[4]);

/**
 * @brief Decode a Variable Byte Integer (MQTT 3.1.1 & MQTT 5.0).
 *
 * @param buf        Buffer containing varint bytes.
 * @param buf_len    Available bytes in buffer.
 * @param val        [out] Parsed integer value.
 * @param bytes_read [out] Number of bytes consumed (1..4).
 * @return true on success, false if incomplete or malformed (> 4 bytes).
 */
bool syn_mqtt_decode_varint(const uint8_t *buf, size_t buf_len, uint32_t *val, size_t *bytes_read);

/**
 * @brief Encode an MQTT 5.0 User Property (Key-Value string pair).
 *
 * @param key         Property key string.
 * @param val         Property value string.
 * @param buf         [out] Output buffer.
 * @param max_buf_len Capacity of output buffer.
 * @return Number of bytes written, or 0 on error/overflow.
 */
size_t syn_mqtt5_encode_user_prop(const char *key, const char *val, uint8_t *buf,
                                  size_t max_buf_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MQTT_H */
