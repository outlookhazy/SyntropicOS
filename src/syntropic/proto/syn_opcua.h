/**
 * @file syn_opcua.h
 * @brief Micro OPC UA (IEC 62541) Binary Protocol Engine & Embedded Server.
 * @ingroup syn_proto
 *
 * Implements a zero-heap, deterministic OPC UA Binary protocol engine:
 * - UACP (OPC UA Connection Protocol) framing (`HEL`, `ACK`, `ERR`).
 * - UASC (OPC UA Secure Conversation) framing (`OPN`, `CLO`, `MSG`) with Security Policy None &
 * Basic256Sha256.
 * - Static In-Memory Address Space: Object, Variable, and Method nodes with numeric & string
 * NodeIds.
 * - Core Services: Read, Write, Browse, CreateSession, ActivateSession, CreateSubscription,
 * Publish.
 * - Non-blocking poll step API and cooperative protothread continuation for `syn_sched`.
 */

#ifndef SYN_OPCUA_H
#define SYN_OPCUA_H

#include "../common/syn_defs.h"
#include "../net/syn_transport.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_sched.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_OPCUA) || SYN_USE_OPCUA

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Limits ─────────────────────────────────────────────────── */

#define SYN_OPCUA_DEFAULT_PORT 4840U /**< Standard OPC UA TCP Port */
#ifndef SYN_OPCUA_MAX_NODES
#define SYN_OPCUA_MAX_NODES 32U /**< Static Node Table Capacity */
#endif
#define SYN_OPCUA_MAX_REFERENCES 8U /**< Max References per Node */
#ifndef SYN_OPCUA_MAX_STRING_LEN
#define SYN_OPCUA_MAX_STRING_LEN 64U /**< Max Identifier & Name String Length */
#endif
#define SYN_OPCUA_PROTOCOL_VERSION 0U       /**< OPC UA Binary Protocol Version */
#define SYN_OPCUA_DEFAULT_BUFFER_SIZE 1024U /**< Default Chunk Buffer Size */

/* OPC UA Standard StatusCode constants */
#define SYN_OPCUA_STATUS_GOOD 0x00000000U               /**< Operation succeeded */
#define SYN_OPCUA_STATUS_UNCERTAIN 0x40000000U          /**< Value is uncertain */
#define SYN_OPCUA_STATUS_BAD_UNEXPECTED 0x80010000U     /**< Unexpected error */
#define SYN_OPCUA_STATUS_BAD_INTERNAL_ERROR 0x80020000U /**< Internal error occurred */
#define SYN_OPCUA_STATUS_BAD_DECODING_ERROR 0x80070000U /**< Binary decoding error */
#define SYN_OPCUA_STATUS_BAD_SECURITY_CHECKS_FAILED \
    0x80130000U /**< Security verification failed   \
                 */
#define SYN_OPCUA_STATUS_BAD_SECURE_CHANNEL_ID_INVALID \
    0x80220000U                                             /**< SecureChannelId is invalid */
#define SYN_OPCUA_STATUS_BAD_SESSION_ID_INVALID 0x80250000U /**< SessionId is invalid */
#define SYN_OPCUA_STATUS_BAD_SESSION_NOT_ACTIVATED \
    0x80270000U /**< Session has not been activated */
#define SYN_OPCUA_STATUS_BAD_NODE_ID_UNKNOWN            \
    0x80340000U /**< NodeId is unknown in address space \
                 */
#define SYN_OPCUA_STATUS_BAD_ATTRIBUTE_ID_INVALID 0x80350000U /**< AttributeId is invalid */
#define SYN_OPCUA_STATUS_BAD_INDEX_RANGE_INVALID 0x80360000U  /**< IndexRange is invalid */
#define SYN_OPCUA_STATUS_BAD_NOT_READABLE 0x803A0000U         /**< Node attribute is not readable */
#define SYN_OPCUA_STATUS_BAD_NOT_WRITABLE 0x803B0000U         /**< Node attribute is not writable */
#define SYN_OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED \
    0x80430000U /**< Requested service is unsupported */
#define SYN_OPCUA_STATUS_BAD_TYPE_MISMATCH \
    0x80740000U /**< Value data type does not match node type */

/* Standard OPC UA Attribute IDs */
#define SYN_OPCUA_ATTR_NODE_ID 1U       /**< NodeId attribute */
#define SYN_OPCUA_ATTR_NODE_CLASS 2U    /**< NodeClass attribute */
#define SYN_OPCUA_ATTR_BROWSE_NAME 3U   /**< BrowseName attribute */
#define SYN_OPCUA_ATTR_DISPLAY_NAME 4U  /**< DisplayName attribute */
#define SYN_OPCUA_ATTR_VALUE 13U        /**< Value attribute */
#define SYN_OPCUA_ATTR_DATA_TYPE 14U    /**< DataType attribute */
#define SYN_OPCUA_ATTR_VALUE_RANK 15U   /**< ValueRank attribute */
#define SYN_OPCUA_ATTR_ACCESS_LEVEL 17U /**< AccessLevel attribute */

/* Standard Node Types & Namespaces */
#define SYN_OPCUA_NS_STANDARD 0U /**< OPC UA standard namespace index */
#define SYN_OPCUA_NS_VENDOR 1U   /**< Vendor custom namespace index */

/* Standard Well-Known Node IDs */
#define SYN_OPCUA_NODEID_ROOT 84U                  /**< RootFolder NodeId */
#define SYN_OPCUA_NODEID_OBJECTS_FOLDER 85U        /**< ObjectsFolder NodeId */
#define SYN_OPCUA_NODEID_SERVER 2253U              /**< Server Object NodeId */
#define SYN_OPCUA_NODEID_SERVER_SERVERSTATUS 2256U /**< ServerStatus Variable NodeId */

/* ── Type Definitions ────────────────────────────────────────────────────── */

/**
 * @brief NodeClass enum.
 */
typedef enum {
    SYN_OPCUA_NODECLASS_UNSPECIFIED = 0,
    SYN_OPCUA_NODECLASS_OBJECT = 1,
    SYN_OPCUA_NODECLASS_VARIABLE = 2,
    SYN_OPCUA_NODECLASS_METHOD = 4
} SYN_OPCUA_NodeClass;

/**
 * @brief Built-in DataType enum (per Part 6).
 */
typedef enum {
    SYN_OPCUA_TYPE_NULL = 0,
    SYN_OPCUA_TYPE_BOOLEAN = 1,
    SYN_OPCUA_TYPE_SBYTE = 2,
    SYN_OPCUA_TYPE_BYTE = 3,
    SYN_OPCUA_TYPE_INT16 = 4,
    SYN_OPCUA_TYPE_UINT16 = 5,
    SYN_OPCUA_TYPE_INT32 = 6,
    SYN_OPCUA_TYPE_UINT32 = 7,
    SYN_OPCUA_TYPE_INT64 = 8,
    SYN_OPCUA_TYPE_UINT64 = 9,
    SYN_OPCUA_TYPE_FLOAT = 10,
    SYN_OPCUA_TYPE_DOUBLE = 11,
    SYN_OPCUA_TYPE_STRING = 12,
    SYN_OPCUA_TYPE_DATETIME = 13
} SYN_OPCUA_DataType;

/**
 * @brief NodeId identifier type.
 */
typedef enum { SYN_OPCUA_NODEID_NUMERIC = 0, SYN_OPCUA_NODEID_STRING = 1 } SYN_OPCUA_NodeIdType;

/**
 * @brief OPC UA NodeId.
 */
typedef struct {
    uint16_t ns_index;            /**< Namespace index */
    SYN_OPCUA_NodeIdType id_type; /**< Numeric or String ID type */
    union {
        uint32_t num;    /**< 32-bit numeric identifier */
        const char *str; /**< String identifier */
    } id;                /**< Identifier payload union */
} SYN_OPCUA_NodeId;

/**
 * @brief OPC UA Variant containing dynamic value.
 */
typedef struct {
    SYN_OPCUA_DataType type; /**< Value data type */
    union {
        bool boolean;       /**< Boolean value */
        int8_t sbyte;       /**< Signed 8-bit integer */
        uint8_t byte;       /**< Unsigned 8-bit integer */
        int16_t int16;      /**< Signed 16-bit integer */
        uint16_t uint16;    /**< Unsigned 16-bit integer */
        int32_t int32;      /**< Signed 32-bit integer */
        uint32_t uint32;    /**< Unsigned 32-bit integer */
        int64_t int64;      /**< Signed 64-bit integer */
        uint64_t uint64;    /**< Unsigned 64-bit integer */
        float float_val;    /**< 32-bit IEEE-754 single precision float */
        double double_val;  /**< 64-bit IEEE-754 double precision float */
        const char *string; /**< Null-terminated string */
    } val;                  /**< Value payload union */
} SYN_OPCUA_Variant;

/**
 * @brief DataValue with value, status code, and source timestamp.
 */
typedef struct {
    SYN_OPCUA_Variant value; /**< Actual variant value */
    uint32_t status_code;    /**< Quality status code */
    uint64_t source_ts_ms;   /**< Source timestamp in milliseconds */
} SYN_OPCUA_DataValue;

struct SYN_OPCUA_Server;
struct SYN_OPCUA_Node;

/**
 * @brief Dynamic callback invoked when a variable node is read.
 */
typedef SYN_Status (*SYN_OPCUA_ReadCb)(struct SYN_OPCUA_Server *srv,
                                       const struct SYN_OPCUA_Node *node,
                                       SYN_OPCUA_DataValue *out_val, void *user_data);

/**
 * @brief Dynamic callback invoked when a variable node is written.
 */
typedef SYN_Status (*SYN_OPCUA_WriteCb)(struct SYN_OPCUA_Server *srv,
                                        const struct SYN_OPCUA_Node *node,
                                        const SYN_OPCUA_DataValue *in_val, void *user_data);

/**
 * @brief In-memory OPC UA Node representation.
 */
typedef struct SYN_OPCUA_Node {
    SYN_OPCUA_NodeId node_id;       /**< Unique NodeId */
    SYN_OPCUA_NodeClass node_class; /**< NodeClass (Object, Variable, Method) */
    const char *browse_name;        /**< Qualified browse name */
    const char *display_name;       /**< Localized display name */
    SYN_OPCUA_DataType data_type;   /**< Data type (for Variables) */
    SYN_OPCUA_DataValue value;      /**< Current cached value */
    uint8_t access_level;           /**< 0x01 = Read, 0x02 = Write, 0x03 = Read/Write */

    /* Hierarchical parent linking */
    SYN_OPCUA_NodeId parent_id; /**< Parent NodeId */

    SYN_OPCUA_ReadCb on_read;                  /**< Optional dynamic read hook */
    SYN_OPCUA_WriteCb on_write;                /**< Optional dynamic write hook */
    void *user_data;                           /**< Context passed to callbacks */
    char string_buf[SYN_OPCUA_MAX_STRING_LEN]; /**< Inline storage for written string values */
} SYN_OPCUA_Node;

/**
 * @brief Configuration parameters for Micro OPC UA Server.
 */
typedef struct {
    const char *endpoint_url; /**< Server endpoint URL (e.g. "opc.tcp://0.0.0.0:4840") */
    const char *server_name;  /**< Human-readable server name */
    SYN_Transport *transport; /**< Underlying transport (TCP stream) */
    void *user_data;          /**< User context pointer */

    uint8_t *rx_buf;    /**< Caller-allocated RX scratch buffer */
    size_t rx_buf_size; /**< RX buffer size (>= 512 bytes) */
    uint8_t *tx_buf;    /**< Caller-allocated TX scratch buffer */
    size_t tx_buf_size; /**< TX buffer size (>= 512 bytes) */
} SYN_OPCUA_Config;

/**
 * @brief Server connection and session state.
 */
typedef enum {
    SYN_OPCUA_STATE_CLOSED = 0,      /**< Connection closed */
    SYN_OPCUA_STATE_HELLO_RECEIVED,  /**< Hello processed, Ack sent */
    SYN_OPCUA_STATE_SECURE_CHANNEL,  /**< Secure channel opened */
    SYN_OPCUA_STATE_SESSION_CREATED, /**< Session created, awaiting ActivateSession */
    SYN_OPCUA_STATE_SESSION_ACTIVE   /**< Session active and authenticated */
} SYN_OPCUA_ServerState;

/**
 * @brief Micro OPC UA Server Instance Context.
 */
typedef struct SYN_OPCUA_Server {
    SYN_OPCUA_Config cfg;        /**< Server configuration */
    SYN_OPCUA_ServerState state; /**< Active session state */
    SYN_PT pt;                   /**< Protothread state */

    uint32_t secure_channel_id; /**< Assigned secure channel ID */
    uint32_t security_token_id; /**< Assigned security token ID */
    uint32_t sequence_number;   /**< Outgoing sequence number */
    uint32_t request_id;        /**< Last processed request ID */

    SYN_OPCUA_Node nodes[SYN_OPCUA_MAX_NODES]; /**< Static address space table */
    size_t node_count;                         /**< Number of registered nodes */
    size_t rx_pending; /**< Buffered partial TCP frame byte count in cfg.rx_buf */
} SYN_OPCUA_Server;

/* ── Public API ─────────────────────────────────────────────────────────── */

/**
 * @brief Initialize Micro OPC UA Server.
 * @param srv Server instance to initialize.
 * @param cfg Server configuration.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid argument.
 */
SYN_Status syn_opcua_server_init(SYN_OPCUA_Server *srv, const SYN_OPCUA_Config *cfg);

/**
 * @brief Register a Node in the server address space.
 * @param srv  Server instance.
 * @param node Node descriptor to copy into address space.
 * @return SYN_OK on success, SYN_ERROR if address space is full.
 */
SYN_Status syn_opcua_server_register_node(SYN_OPCUA_Server *srv, const SYN_OPCUA_Node *node);

/**
 * @brief Find a node in the address space by numeric NodeId.
 * @param srv    Server instance.
 * @param ns     Namespace index.
 * @param num_id Numeric identifier.
 * @return Pointer to node in address space or NULL if not found.
 */
SYN_OPCUA_Node *syn_opcua_server_find_node_num(SYN_OPCUA_Server *srv, uint16_t ns, uint32_t num_id);

/**
 * @brief Update a variable node's value in the address space.
 * @param srv    Server instance.
 * @param ns     Namespace index.
 * @param num_id Numeric identifier.
 * @param val    New variant value.
 * @return SYN_OK on success, SYN_ERROR if not found.
 */
SYN_Status syn_opcua_server_write_variable(SYN_OPCUA_Server *srv, uint16_t ns, uint32_t num_id,
                                           const SYN_OPCUA_Variant *val);

/**
 * @brief Read a variable node's value from the address space.
 * @param srv     Server instance.
 * @param ns      Namespace index.
 * @param num_id  Numeric identifier.
 * @param out_val Output variant value pointer.
 * @return SYN_OK on success, SYN_ERROR if not found.
 */
SYN_Status syn_opcua_server_read_variable(const SYN_OPCUA_Server *srv, uint16_t ns, uint32_t num_id,
                                          SYN_OPCUA_Variant *out_val);

/**
 * @brief Process an incoming raw OPC UA frame and generate a response frame.
 * @param srv     Server instance.
 * @param rx      Inbound raw message frame.
 * @param rx_len  Inbound length in bytes.
 * @param tx      Outbound response frame buffer.
 * @param max_tx  Outbound buffer capacity.
 * @param out_len Pointer receiving generated response length.
 * @return SYN_OK on success.
 */
SYN_Status syn_opcua_server_process_message(SYN_OPCUA_Server *srv, const uint8_t *rx, size_t rx_len,
                                            uint8_t *tx, size_t max_tx, size_t *out_len);

/**
 * @brief Advance OPC UA server state machine by one step / tick.
 * @param srv    Server instance.
 * @param now_ms Current system timestamp in milliseconds.
 * @return SYN_OK on success.
 */
SYN_Status syn_opcua_server_step(SYN_OPCUA_Server *srv, uint32_t now_ms);

/**
 * @brief Protothread continuation function for `syn_sched`.
 * @param pt   Protothread continuation structure.
 * @param task Task handle containing SYN_OPCUA_Server in user_data.
 * @return SYN_PT_Status continuation status.
 */
SYN_PT_Status syn_opcua_server_pt(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_OPCUA */

#endif /* SYN_OPCUA_H */
