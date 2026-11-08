/**
 * @file syn_xrce_dds.h
 * @brief Micro-ROS & eProsima Micro XRCE-DDS v1.2 Protocol Client Engine.
 * @ingroup syn_proto
 *
 * Implements a zero-heap, deterministic Micro XRCE-DDS client for ROS 2:
 * - Common Data Representation (CDR) serializer and deserializer with strict alignment.
 * - XRCE-DDS submessage framing (CREATE_CLIENT, CREATE, WRITE_DATA, READ_DATA, DATA, STATUS).
 * - Full ROS 2 entity tree management (Participant, Topic, Publisher, Subscriber, DataWriter,
 * DataReader).
 * - Non-blocking poll ticks and cooperative protothread continuation for `syn_sched`.
 */

#ifndef SYN_XRCE_DDS_H
#define SYN_XRCE_DDS_H

#include "../common/syn_defs.h"
#include "../net/syn_transport.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_sched.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_XRCE_DDS) || SYN_USE_XRCE_DDS

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Limits ─────────────────────────────────────────────────── */

#define SYN_XRCE_COOKIE 0x0100U                 /**< XRCE Header cookie */
#define SYN_XRCE_CLIENT_KEY_DEFAULT 0xCAFEBABEU /**< Default 32-bit client key */
#define SYN_XRCE_SESSION_ID_DEFAULT 0x81U       /**< Default client session ID */
#define SYN_XRCE_STREAM_BEST_EFFORT 0x01U       /**< Best-effort stream ID */
#define SYN_XRCE_STREAM_RELIABLE 0x80U          /**< Reliable stream ID */
#define SYN_XRCE_MAX_TOPIC_NAME 64U             /**< Maximum topic string length */
#define SYN_XRCE_MAX_TYPE_NAME 64U              /**< Maximum type string length */
#define SYN_XRCE_MAX_OBJECTS 16U                /**< Max simultaneously tracked entities */

/* XRCE-DDS Submessage Identifiers */
#define SYN_XRCE_SUBMSG_CREATE_CLIENT 0x00U /**< Submessage: CREATE_CLIENT */
#define SYN_XRCE_SUBMSG_CREATE 0x01U        /**< Submessage: CREATE */
#define SYN_XRCE_SUBMSG_DELETE 0x02U        /**< Submessage: DELETE */
#define SYN_XRCE_SUBMSG_WRITE_DATA 0x03U    /**< Submessage: WRITE_DATA */
#define SYN_XRCE_SUBMSG_READ_DATA 0x04U     /**< Submessage: READ_DATA */
#define SYN_XRCE_SUBMSG_DATA 0x05U          /**< Submessage: DATA */
#define SYN_XRCE_SUBMSG_STATUS 0x06U        /**< Submessage: STATUS */
#define SYN_XRCE_SUBMSG_HEARTBEAT 0x07U     /**< Submessage: HEARTBEAT */
#define SYN_XRCE_SUBMSG_ACKNACK 0x08U       /**< Submessage: ACKNACK */

/* XRCE-DDS Object Kinds */
#define SYN_XRCE_OBJKIND_PARTICIPANT 0x01U /**< Object kind: DomainParticipant */
#define SYN_XRCE_OBJKIND_TOPIC 0x02U       /**< Object kind: Topic */
#define SYN_XRCE_OBJKIND_PUBLISHER 0x03U   /**< Object kind: Publisher */
#define SYN_XRCE_OBJKIND_SUBSCRIBER 0x04U  /**< Object kind: Subscriber */
#define SYN_XRCE_OBJKIND_DATAWRITER 0x05U  /**< Object kind: DataWriter */
#define SYN_XRCE_OBJKIND_DATAREADER 0x06U  /**< Object kind: DataReader */

/* XRCE-DDS Status Codes */
#define SYN_XRCE_STATUS_OK 0x00U                 /**< Status: OK */
#define SYN_XRCE_STATUS_OK_MATCHED 0x01U         /**< Status: OK and matched */
#define SYN_XRCE_STATUS_ERR_ALREADY_EXISTS 0x11U /**< Status Error: Already exists */
#define SYN_XRCE_STATUS_ERR_UNKNOWN_REF 0x12U    /**< Status Error: Unknown reference */
#define SYN_XRCE_STATUS_ERR_INVALID_DATA 0x13U   /**< Status Error: Invalid data */
#define SYN_XRCE_STATUS_ERR_INCOMPATIBLE 0x14U   /**< Status Error: Incompatible */
#define SYN_XRCE_STATUS_ERR_RESOURCES 0x15U      /**< Status Error: Out of resources */

/* ── CDR Codec Contexts ──────────────────────────────────────────────────── */

/**
 * @brief CDR serialization output buffer context.
 */
typedef struct {
    uint8_t *data; /**< Output buffer */
    size_t size;   /**< Buffer capacity */
    size_t pos;    /**< Current write offset */
    bool error;    /**< Error flag on overflow */
} SYN_CDR_Writer;

/**
 * @brief CDR deserialization input buffer context.
 */
typedef struct {
    const uint8_t *data; /**< Input payload buffer */
    size_t size;         /**< Total payload length */
    size_t pos;          /**< Current read offset */
    bool error;          /**< Error flag on underflow/malformed */
} SYN_CDR_Reader;

/* ── Client Entity & State Types ─────────────────────────────────────────── */

/**
 * @brief XRCE client connection state.
 */
typedef enum {
    SYN_XRCE_STATE_DISCONNECTED = 0, /**< Not connected */
    SYN_XRCE_STATE_CONNECTING,       /**< Session handshake pending */
    SYN_XRCE_STATE_CONNECTED,        /**< Active session established */
    SYN_XRCE_STATE_ERROR             /**< Fatal error / disconnected */
} SYN_XRCE_ClientState;

/**
 * @brief Registered XRCE entity tracking record.
 */
typedef struct {
    uint16_t id;     /**< Entity Object ID */
    uint8_t kind;    /**< Entity Object Kind */
    bool configured; /**< Configuration state */
    uint16_t parent; /**< Parent object ID */
} SYN_XRCE_Object;

/**
 * @brief Callback for incoming topic data from an agent DataReader.
 * @param reader_id DataReader object ID.
 * @param payload   CDR serialized topic payload.
 * @param len       Payload length in bytes.
 * @param user_data User context pointer.
 */
typedef void (*SYN_XRCE_DataCb)(uint16_t reader_id, const uint8_t *payload, size_t len,
                                void *user_data);

/**
 * @brief Callback for status notifications (entity creation, ack, etc).
 * @param req_id    Request ID matching the command.
 * @param status    XRCE status code (SYN_XRCE_STATUS_OK, etc).
 * @param user_data User context pointer.
 */
typedef void (*SYN_XRCE_StatusCb)(uint16_t req_id, uint8_t status, void *user_data);

/**
 * @brief Micro XRCE-DDS Client configuration.
 */
typedef struct {
    uint32_t client_key;          /**< 32-bit unique client identifier */
    uint8_t session_id;           /**< Session ID (default 0x81) */
    SYN_Transport *transport;     /**< Transport adapter */
    uint32_t heartbeat_period_ms; /**< Keep-alive ping interval in ms */
    SYN_XRCE_DataCb on_data;      /**< Incoming message callback */
    SYN_XRCE_StatusCb on_status;  /**< Command status callback */
    void *user_data;              /**< Context pointer for callbacks */

    uint8_t *rx_buf;    /**< RX scratch buffer */
    size_t rx_buf_size; /**< RX buffer size (>= 128 bytes) */
    uint8_t *tx_buf;    /**< TX scratch buffer */
    size_t tx_buf_size; /**< TX buffer size (>= 128 bytes) */
} SYN_XRCE_Config;

/**
 * @brief Micro XRCE-DDS Client instance context.
 */
typedef struct {
    SYN_XRCE_Config cfg;        /**< Task and connection configuration */
    SYN_XRCE_ClientState state; /**< Active connection state */
    SYN_PT pt;                  /**< Protothread state */

    uint16_t stream_seq_num;    /**< Outgoing sequence number */
    uint16_t next_req_id;       /**< Request ID counter */
    uint32_t last_heartbeat_ms; /**< Timestamp of last transmitted heartbeat */
    bool heartbeat_sent;        /**< True after initial session heartbeat is sent */

    SYN_XRCE_Object objects[SYN_XRCE_MAX_OBJECTS]; /**< Tracked local entity table */
    size_t object_count;                           /**< Count of configured entities */
} SYN_XRCE_Client;

/* ── CDR Serialization API ───────────────────────────────────────────────── */

/**
 * @brief Initialize CDR writer.
 * @param w    Writer context.
 * @param buf  Target buffer.
 * @param size Buffer size.
 */
void syn_cdr_writer_init(SYN_CDR_Writer *w, uint8_t *buf, size_t size);

/**
 * @brief Write unsigned 8-bit integer.
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_u8(SYN_CDR_Writer *w, uint8_t v);

/**
 * @brief Write unsigned 16-bit integer (2-byte aligned).
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_u16(SYN_CDR_Writer *w, uint16_t v);

/**
 * @brief Write unsigned 32-bit integer (4-byte aligned).
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_u32(SYN_CDR_Writer *w, uint32_t v);

/**
 * @brief Write unsigned 64-bit integer (8-byte aligned).
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_u64(SYN_CDR_Writer *w, uint64_t v);

/**
 * @brief Write signed 8-bit integer.
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_i8(SYN_CDR_Writer *w, int8_t v);

/**
 * @brief Write signed 16-bit integer (2-byte aligned).
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_i16(SYN_CDR_Writer *w, int16_t v);

/**
 * @brief Write signed 32-bit integer (4-byte aligned).
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_i32(SYN_CDR_Writer *w, int32_t v);

/**
 * @brief Write signed 64-bit integer (8-byte aligned).
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_i64(SYN_CDR_Writer *w, int64_t v);

/**
 * @brief Write 32-bit float (4-byte aligned).
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_float(SYN_CDR_Writer *w, float v);

/**
 * @brief Write 64-bit double (8-byte aligned).
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_double(SYN_CDR_Writer *w, double v);

/**
 * @brief Write boolean.
 * @param w Writer context.
 * @param v Value.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_bool(SYN_CDR_Writer *w, bool v);

/**
 * @brief Write null-terminated string with 32-bit length prefix and null terminator.
 * @param w   Writer context.
 * @param str String pointer.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_string(SYN_CDR_Writer *w, const char *str);

/**
 * @brief Write raw byte buffer without alignment padding.
 * @param w     Writer context.
 * @param bytes Data buffer.
 * @param len   Number of bytes.
 * @return true on success, false on overflow.
 */
bool syn_cdr_write_bytes(SYN_CDR_Writer *w, const uint8_t *bytes, size_t len);

/* ── CDR Deserialization API ─────────────────────────────────────────────── */

/**
 * @brief Initialize CDR reader.
 * @param r    Reader context.
 * @param buf  Input buffer.
 * @param size Buffer size.
 */
void syn_cdr_reader_init(SYN_CDR_Reader *r, const uint8_t *buf, size_t size);

/**
 * @brief Read unsigned 8-bit integer.
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_u8(SYN_CDR_Reader *r, uint8_t *out);

/**
 * @brief Read unsigned 16-bit integer (2-byte aligned).
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_u16(SYN_CDR_Reader *r, uint16_t *out);

/**
 * @brief Read unsigned 32-bit integer (4-byte aligned).
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_u32(SYN_CDR_Reader *r, uint32_t *out);

/**
 * @brief Read unsigned 64-bit integer (8-byte aligned).
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_u64(SYN_CDR_Reader *r, uint64_t *out);

/**
 * @brief Read signed 8-bit integer.
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_i8(SYN_CDR_Reader *r, int8_t *out);

/**
 * @brief Read signed 16-bit integer (2-byte aligned).
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_i16(SYN_CDR_Reader *r, int16_t *out);

/**
 * @brief Read signed 32-bit integer (4-byte aligned).
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_i32(SYN_CDR_Reader *r, int32_t *out);

/**
 * @brief Read signed 64-bit integer (8-byte aligned).
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_i64(SYN_CDR_Reader *r, int64_t *out);

/**
 * @brief Read 32-bit float (4-byte aligned).
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_float(SYN_CDR_Reader *r, float *out);

/**
 * @brief Read 64-bit double (8-byte aligned).
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_double(SYN_CDR_Reader *r, double *out);

/**
 * @brief Read boolean.
 * @param r   Reader context.
 * @param out Output pointer.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_bool(SYN_CDR_Reader *r, bool *out);

/**
 * @brief Read null-terminated string with length prefix.
 * @param r       Reader context.
 * @param out_buf Output string buffer.
 * @param max_len Capacity of output string buffer.
 * @return true on success, false on underflow/truncation.
 */
bool syn_cdr_read_string(SYN_CDR_Reader *r, char *out_buf, size_t max_len);

/**
 * @brief Read raw byte buffer without alignment padding.
 * @param r       Reader context.
 * @param out_buf Output buffer.
 * @param len     Number of bytes to read.
 * @return true on success, false on underflow.
 */
bool syn_cdr_read_bytes(SYN_CDR_Reader *r, uint8_t *out_buf, size_t len);

/* ── XRCE-DDS Client Public API ──────────────────────────────────────────── */

/**
 * @brief Initialize Micro XRCE-DDS Client.
 * @param client Client context to initialize.
 * @param cfg    Configuration options and buffers (rx_buf_size/tx_buf_size >= 128 bytes).
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid argument.
 */
SYN_Status syn_xrce_client_init(SYN_XRCE_Client *client, const SYN_XRCE_Config *cfg);

/**
 * @brief Connect and establish session with Micro-ROS Agent.
 * @param client Client instance.
 * @return SYN_OK on success, SYN_ERROR on transport failure.
 */
SYN_Status syn_xrce_client_create_session(SYN_XRCE_Client *client);

/**
 * @brief Create a DDS DomainParticipant entity.
 * @param client         Client instance.
 * @param participant_id Object ID for the participant.
 * @return SYN_OK on success.
 */
SYN_Status syn_xrce_client_create_participant(SYN_XRCE_Client *client, uint16_t participant_id);

/**
 * @brief Create a DDS Topic entity.
 * @param client         Client instance.
 * @param topic_id       Object ID for the topic.
 * @param participant_id Parent participant ID.
 * @param topic_name     ROS 2 topic name (e.g. "sensors/imu").
 * @param type_name      ROS 2 type identifier (e.g. "sensor_msgs::msg::dds_::Imu_").
 * @return SYN_OK on success.
 */
SYN_Status syn_xrce_client_create_topic(SYN_XRCE_Client *client, uint16_t topic_id,
                                        uint16_t participant_id, const char *topic_name,
                                        const char *type_name);

/**
 * @brief Create a DDS Publisher entity.
 * @param client         Client instance.
 * @param publisher_id   Object ID for the publisher.
 * @param participant_id Parent participant ID.
 * @return SYN_OK on success.
 */
SYN_Status syn_xrce_client_create_publisher(SYN_XRCE_Client *client, uint16_t publisher_id,
                                            uint16_t participant_id);

/**
 * @brief Create a DDS Subscriber entity.
 * @param client         Client instance.
 * @param subscriber_id  Object ID for the subscriber.
 * @param participant_id Parent participant ID.
 * @return SYN_OK on success.
 */
SYN_Status syn_xrce_client_create_subscriber(SYN_XRCE_Client *client, uint16_t subscriber_id,
                                             uint16_t participant_id);

/**
 * @brief Create a DDS DataWriter entity.
 * @param client       Client instance.
 * @param writer_id    Object ID for the writer.
 * @param publisher_id Parent publisher ID.
 * @param topic_id     Associated topic ID.
 * @return SYN_OK on success.
 */
SYN_Status syn_xrce_client_create_datawriter(SYN_XRCE_Client *client, uint16_t writer_id,
                                             uint16_t publisher_id, uint16_t topic_id);

/**
 * @brief Create a DDS DataReader entity.
 * @param client        Client instance.
 * @param reader_id     Object ID for the reader.
 * @param subscriber_id Parent subscriber ID.
 * @param topic_id      Associated topic ID.
 * @return SYN_OK on success.
 */
SYN_Status syn_xrce_client_create_datareader(SYN_XRCE_Client *client, uint16_t reader_id,
                                             uint16_t subscriber_id, uint16_t topic_id);

/**
 * @brief Publish CDR serialized data payload to a DataWriter.
 * @param client    Client instance.
 * @param writer_id DataWriter entity ID.
 * @param data      CDR payload buffer.
 * @param len       Payload length.
 * @return SYN_OK on success.
 */
SYN_Status syn_xrce_client_write_data(SYN_XRCE_Client *client, uint16_t writer_id,
                                      const uint8_t *data, size_t len);

/**
 * @brief Request incoming topic samples from an active DataReader.
 * @param client    Client instance.
 * @param reader_id DataReader entity ID.
 * @return SYN_OK on success.
 */
SYN_Status syn_xrce_client_read_data(SYN_XRCE_Client *client, uint16_t reader_id);

/**
 * @brief Advance the XRCE-DDS client state machine by one step / tick.
 * @param client Client instance.
 * @param now_ms Current timestamp in milliseconds.
 * @return SYN_OK on success.
 */
SYN_Status syn_xrce_client_step(SYN_XRCE_Client *client, uint32_t now_ms);

/**
 * @brief Protothread continuation function for `syn_sched`.
 * @param pt   Protothread continuation structure.
 * @param task Task handle containing SYN_XRCE_Client in user_data.
 * @return SYN_PT_Status continuation status.
 */
SYN_PT_Status syn_xrce_client_pt(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_XRCE_DDS */

#endif /* SYN_XRCE_DDS_H */
