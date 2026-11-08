/**
 * @file syn_mqttsn.h
 * @brief MQTT for Sensor Networks (MQTT-SN v1.2) Protocol Client Engine.
 * @ingroup syn_proto
 *
 * Implements a zero-heap, deterministic MQTT-SN v1.2 client for low-power wireless networks:
 * - Operates over datagram transports (UDP, Zigbee, 802.15.4, Sub-GHz, BLE, RS485).
 * - Gateway Discovery (`SEARCHGW`, `GWINFO`, `ADVERTISE`).
 * - Short (2-char), Predefined, and Dynamically Registered Topic IDs (`REGISTER`, `REGACK`).
 * - Publish and Subscribe (QoS 0, QoS 1, QoS -1) with payload framing.
 * - Low-power Sleep/Awake mode transitions (`SLEEP`, `AWAKE` queue flush).
 * - Non-blocking poll step and cooperative protothread continuation for `syn_sched`.
 */

#ifndef SYN_MQTTSN_H
#define SYN_MQTTSN_H

#include "../common/syn_defs.h"
#include "../net/syn_transport.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_sched.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_MQTTSN) || SYN_USE_MQTTSN

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Limits ─────────────────────────────────────────────────── */

#define SYN_MQTTSN_PROTOCOL_ID 0x01U      /**< MQTT-SN Protocol ID v1.2 */
#define SYN_MQTTSN_DEFAULT_DURATION_S 60U /**< Default Keep-Alive duration in seconds */
#define SYN_MQTTSN_MAX_CLIENT_ID 24U      /**< Max Client Identifier string length */
#define SYN_MQTTSN_MAX_TOPIC_NAME 64U     /**< Max Topic name string length */
#define SYN_MQTTSN_MAX_REGISTRATIONS 16U  /**< Max tracked registered topics */

/* ── Message Types ──────────────────────────────────────────────────────── */

#define SYN_MQTTSN_MSG_ADVERTISE 0x00U     /**< Gateway advertise broadcast */
#define SYN_MQTTSN_MSG_SEARCHGW 0x01U      /**< Client search gateway broadcast */
#define SYN_MQTTSN_MSG_GWINFO 0x02U        /**< Gateway information response */
#define SYN_MQTTSN_MSG_CONNECT 0x04U       /**< Connect request */
#define SYN_MQTTSN_MSG_CONNACK 0x05U       /**< Connect acknowledgment */
#define SYN_MQTTSN_MSG_WILLTOPICREQ 0x06U  /**< Will topic request */
#define SYN_MQTTSN_MSG_WILLTOPIC 0x07U     /**< Will topic specification */
#define SYN_MQTTSN_MSG_WILLMSGREQ 0x08U    /**< Will message request */
#define SYN_MQTTSN_MSG_WILLMSG 0x09U       /**< Will message payload */
#define SYN_MQTTSN_MSG_REGISTER 0x0AU      /**< Topic registration request */
#define SYN_MQTTSN_MSG_REGACK 0x0BU        /**< Topic registration acknowledgment */
#define SYN_MQTTSN_MSG_PUBLISH 0x0CU       /**< Publish message */
#define SYN_MQTTSN_MSG_PUBACK 0x0DU        /**< Publish acknowledgment */
#define SYN_MQTTSN_MSG_PUBCOMP 0x0EU       /**< Publish complete (QoS 2) */
#define SYN_MQTTSN_MSG_PUBREC 0x0FU        /**< Publish received (QoS 2) */
#define SYN_MQTTSN_MSG_PUBREL 0x10U        /**< Publish release (QoS 2) */
#define SYN_MQTTSN_MSG_SUBSCRIBE 0x12U     /**< Subscribe request */
#define SYN_MQTTSN_MSG_SUBACK 0x13U        /**< Subscribe acknowledgment */
#define SYN_MQTTSN_MSG_UNSUBSCRIBE 0x14U   /**< Unsubscribe request */
#define SYN_MQTTSN_MSG_UNSUBACK 0x15U      /**< Unsubscribe acknowledgment */
#define SYN_MQTTSN_MSG_PINGREQ 0x16U       /**< Ping request / wake ping */
#define SYN_MQTTSN_MSG_PINGRESP 0x17U      /**< Ping response */
#define SYN_MQTTSN_MSG_DISCONNECT 0x18U    /**< Disconnect / sleep command */
#define SYN_MQTTSN_MSG_WILLTOPICUPD 0x1AU  /**< Will topic update */
#define SYN_MQTTSN_MSG_WILLTOPICRESP 0x1BU /**< Will topic update response */
#define SYN_MQTTSN_MSG_WILLMSGUPD 0x1CU    /**< Will message update */
#define SYN_MQTTSN_MSG_WILLMSGRESP 0x1DU   /**< Will message update response */

/* ── Return Codes ───────────────────────────────────────────────────────── */

#define SYN_MQTTSN_RC_ACCEPTED 0x00U             /**< Request accepted */
#define SYN_MQTTSN_RC_REJ_CONGESTION 0x01U       /**< Rejected: Congestion */
#define SYN_MQTTSN_RC_REJ_INVALID_TOPIC_ID 0x02U /**< Rejected: Invalid Topic ID */
#define SYN_MQTTSN_RC_REJ_NOT_SUPPORTED 0x03U    /**< Rejected: Not supported */

/* ── Flag Bitmasks ──────────────────────────────────────────────────────── */

#define SYN_MQTTSN_FLAG_DUP 0x80U           /**< Duplicate delivery flag */
#define SYN_MQTTSN_FLAG_QOS_MASK 0x60U      /**< QoS mask */
#define SYN_MQTTSN_FLAG_QOS_0 0x00U         /**< QoS level 0 */
#define SYN_MQTTSN_FLAG_QOS_1 0x20U         /**< QoS level 1 */
#define SYN_MQTTSN_FLAG_QOS_2 0x40U         /**< QoS level 2 */
#define SYN_MQTTSN_FLAG_QOS_NEG1 0x60U      /**< QoS level -1 (transparent) */
#define SYN_MQTTSN_FLAG_RETAIN 0x10U        /**< Retain flag */
#define SYN_MQTTSN_FLAG_WILL 0x08U          /**< Will flag */
#define SYN_MQTTSN_FLAG_CLEAN_SESSION 0x04U /**< Clean session flag */
#define SYN_MQTTSN_FLAG_TOPIC_MASK 0x03U    /**< Topic ID type mask */
#define SYN_MQTTSN_FLAG_TOPIC_NORMAL 0x00U  /**< Topic ID type: Normal registered ID */
#define SYN_MQTTSN_FLAG_TOPIC_PREDEF 0x01U  /**< Topic ID type: Predefined 16-bit ID */
#define SYN_MQTTSN_FLAG_TOPIC_SHORT 0x02U   /**< Topic ID type: Short 2-char string */

/* ── Client State Enum ──────────────────────────────────────────────────── */

/**
 * @brief MQTT-SN client connection state.
 */
typedef enum {
    SYN_MQTTSN_STATE_DISCONNECTED = 0, /**< Disconnected */
    SYN_MQTTSN_STATE_SEARCHING_GW,     /**< Broadcasting SEARCHGW */
    SYN_MQTTSN_STATE_CONNECTING,       /**< CONNECT frame sent, waiting CONNACK */
    SYN_MQTTSN_STATE_CONNECTED,        /**< Fully connected */
    SYN_MQTTSN_STATE_ASLEEP,           /**< Sleeping in low-power mode */
    SYN_MQTTSN_STATE_AWAKE             /**< Awoken to flush buffered messages */
} SYN_MQTTSN_State;

/* ── Callback Signatures ────────────────────────────────────────────────── */

struct SYN_MQTTSN_Client;

/**
 * @brief Incoming publish message handler callback.
 * @param client Client instance.
 * @param topic_id 16-bit Topic ID or 2-char short topic.
 * @param topic_type Type of topic (normal, predef, short).
 * @param payload Payload data pointer.
 * @param len Payload length in bytes.
 * @param user_data User context pointer.
 */
typedef void (*SYN_MQTTSN_MessageCb)(struct SYN_MQTTSN_Client *client, uint16_t topic_id,
                                     uint8_t topic_type, const uint8_t *payload, size_t len,
                                     void *user_data);

/* ── Topic Registry Entry ───────────────────────────────────────────────── */

/**
 * @brief Registered topic entry mapping string name to 16-bit Topic ID.
 */
typedef struct {
    char name[SYN_MQTTSN_MAX_TOPIC_NAME]; /**< Topic name string */
    uint16_t topic_id;                    /**< Gateway-assigned Topic ID */
    uint16_t pending_msg_id;              /**< Pending REGISTER message ID (0 if none) */
    bool active;                          /**< Entry active flag */
} SYN_MQTTSN_TopicEntry;

/* ── Configuration Descriptor ────────────────────────────────────────────── */

/**
 * @brief MQTT-SN client configuration descriptor.
 */
typedef struct {
    SYN_Transport *transport;        /**< Underlying datagram transport */
    const char *client_id;           /**< Client identifier string (<= 23 chars) */
    uint16_t duration_s;             /**< Keep-alive duration in seconds */
    bool clean_session;              /**< Clean session flag */
    SYN_MQTTSN_MessageCb on_message; /**< Incoming message callback */
    void *user_data;                 /**< User context pointer */

    uint8_t *rx_buf;    /**< Scratch RX buffer */
    size_t rx_buf_size; /**< Scratch RX buffer size (>= 128 bytes) */
    uint8_t *tx_buf;    /**< Scratch TX buffer */
    size_t tx_buf_size; /**< Scratch TX buffer size (>= 128 bytes) */
} SYN_MQTTSN_Config;

/* ── Client Context ─────────────────────────────────────────────────────── */

/**
 * @brief MQTT-SN Client instance structure.
 */
typedef struct SYN_MQTTSN_Client {
    SYN_MQTTSN_Config cfg;  /**< Client configuration */
    SYN_MQTTSN_State state; /**< Connection state */
    SYN_PT pt;              /**< Protothread state */

    uint8_t gateway_id;        /**< Discovered gateway ID */
    uint16_t next_msg_id;      /**< Next message sequence ID */
    uint32_t last_activity_ms; /**< Timestamp of last sent/received frame */

    SYN_MQTTSN_TopicEntry topics[SYN_MQTTSN_MAX_REGISTRATIONS]; /**< Topic table */
    size_t topic_count;                                         /**< Number of topics */
} SYN_MQTTSN_Client;

/* ── Public Client API ──────────────────────────────────────────────────── */

/**
 * @brief Initialize MQTT-SN client.
 * @param client Client instance pointer.
 * @param cfg Configuration descriptor.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid argument.
 */
SYN_Status syn_mqttsn_client_init(SYN_MQTTSN_Client *client, const SYN_MQTTSN_Config *cfg);

/**
 * @brief Broadcast SEARCHGW to discover active MQTT-SN gateways.
 * @param client Client instance.
 * @param radius Search radius hop count.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_searchgw(SYN_MQTTSN_Client *client, uint8_t radius);

/**
 * @brief Send CONNECT packet to MQTT-SN gateway.
 * @param client Client instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_connect(SYN_MQTTSN_Client *client);

/**
 * @brief Register a topic name with gateway to obtain a 16-bit Topic ID.
 * @param client Client instance.
 * @param topic_name String topic name.
 * @param out_msg_id Optional pointer to receive allocated MsgId.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_register_topic(SYN_MQTTSN_Client *client, const char *topic_name,
                                            uint16_t *out_msg_id);

/**
 * @brief Publish data payload using a registered 16-bit Topic ID.
 * @param client Client instance.
 * @param topic_id 16-bit Topic ID.
 * @param qos Quality of Service (0, 1, or -1).
 * @param retain Retain flag.
 * @param payload Payload buffer.
 * @param len Payload length.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_publish(SYN_MQTTSN_Client *client, uint16_t topic_id, int8_t qos,
                                     bool retain, const uint8_t *payload, size_t len);

/**
 * @brief Publish data payload using a short 2-character topic name.
 * @param client Client instance.
 * @param short_topic 2-character topic string.
 * @param qos Quality of Service (0, 1, or -1).
 * @param retain Retain flag.
 * @param payload Payload buffer.
 * @param len Payload length.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_publish_short(SYN_MQTTSN_Client *client, const char short_topic[2],
                                           int8_t qos, bool retain, const uint8_t *payload,
                                           size_t len);

/**
 * @brief Publish data payload using a predefined 16-bit Topic ID.
 * @param client Client instance.
 * @param predef_id Predefined 16-bit Topic ID.
 * @param qos Quality of Service (0, 1, or -1).
 * @param retain Retain flag.
 * @param payload Payload buffer.
 * @param len Payload length.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_publish_predefined(SYN_MQTTSN_Client *client, uint16_t predef_id,
                                                int8_t qos, bool retain, const uint8_t *payload,
                                                size_t len);

/**
 * @brief Subscribe to a topic name or predefined ID.
 * @param client Client instance.
 * @param topic_name Topic name string (or NULL if subscribing by predef_id).
 * @param predef_id Predefined Topic ID (if topic_name is NULL).
 * @param qos Requested QoS level.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_subscribe(SYN_MQTTSN_Client *client, const char *topic_name,
                                       uint16_t predef_id, int8_t qos);

/**
 * @brief Unsubscribe from a topic.
 * @param client Client instance.
 * @param topic_name Topic name string.
 * @param predef_id Predefined Topic ID.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_unsubscribe(SYN_MQTTSN_Client *client, const char *topic_name,
                                         uint16_t predef_id);

/**
 * @brief Put client into low-power SLEEP state with specified sleep duration.
 * @param client Client instance.
 * @param sleep_duration_s Sleep duration in seconds.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_sleep(SYN_MQTTSN_Client *client, uint16_t sleep_duration_s);

/**
 * @brief Wake up from sleep to flush messages or return to connected state.
 * @param client Client instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_wake(SYN_MQTTSN_Client *client);

/**
 * @brief Disconnect client from gateway.
 * @param client Client instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_disconnect(SYN_MQTTSN_Client *client);

/**
 * @brief Non-blocking poll step processing received frames and keep-alive ping timers.
 * @param client Client instance.
 * @param now_ms Current timestamp in milliseconds.
 * @return SYN_OK on success.
 */
SYN_Status syn_mqttsn_client_step(SYN_MQTTSN_Client *client, uint32_t now_ms);

/**
 * @brief Protothread continuation function for `syn_sched`.
 * @param pt   Protothread continuation structure.
 * @param task Task handle containing SYN_MQTTSN_Client in user_data.
 * @return SYN_PT_Status continuation status.
 */
SYN_PT_Status syn_mqttsn_client_pt(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* !defined(SYN_USE_MQTTSN) || SYN_USE_MQTTSN */

#endif /* SYN_MQTTSN_H */
