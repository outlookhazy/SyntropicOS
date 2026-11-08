/**
 * @file syn_lwm2m.h
 * @brief Zero-Heap OMA Lightweight M2M (LwM2M v1.1/v1.2) Core Client Engine.
 * @ingroup syn_proto
 *
 * Implements the OMA LwM2M client specification over CoAP/CoAPS, supporting:
 * - Data formats: Plain Text, Opaque, TLV (Type-Length-Value Content-Format 11542), SenML CBOR
 * (Content-Format 112).
 * - Standard Object & Resource tree routing (/object/instance/resource).
 * - Registration, Registration Update, and Deregistration workflows.
 * - Device Management: Read, Write, Execute, Discover, Observe & Notify (RFC 7641).
 * - Standard Objects: Device (Obj 3), Firmware Update (Obj 5), IPSO Temperature (Obj 3303).
 */

#ifndef SYN_LWM2M_H
#define SYN_LWM2M_H

#include "syntropic/common/syn_defs.h"

#if (!defined(SYN_USE_LWM2M) || SYN_USE_LWM2M) && (!defined(SYN_USE_COAP) || SYN_USE_COAP)
/** @brief Non-zero when LwM2M and CoAP dependencies are enabled. */
#define SYN_LWM2M_ENABLED 1
#else
/** @brief Non-zero when LwM2M and CoAP dependencies are enabled. */
#define SYN_LWM2M_ENABLED 0
#endif

#if SYN_LWM2M_ENABLED

#include "syntropic/net/syn_coap.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Standard OMA LwM2M Object IDs ───────────────────────────────────────── */

#define SYN_LWM2M_OBJ_SECURITY 0U                /**< LwM2M Security Object */
#define SYN_LWM2M_OBJ_SERVER 1U                  /**< LwM2M Server Object */
#define SYN_LWM2M_OBJ_ACCESS_CONTROL 2U          /**< Access Control Object */
#define SYN_LWM2M_OBJ_DEVICE 3U                  /**< Device Object */
#define SYN_LWM2M_OBJ_CONNECTIVITY_MONITORING 4U /**< Connectivity Monitoring */
#define SYN_LWM2M_OBJ_FIRMWARE_UPDATE 5U         /**< Firmware Update Object */
#define SYN_LWM2M_OBJ_LOCATION 6U                /**< Location Object */
#define SYN_LWM2M_OBJ_CONNECTIVITY_STATS 7U      /**< Connectivity Statistics */
#define SYN_LWM2M_OBJ_GENERIC_SENSOR 3300U       /**< IPSO Generic Sensor */
#define SYN_LWM2M_OBJ_TEMPERATURE 3303U          /**< IPSO Temperature Sensor */
#define SYN_LWM2M_OBJ_HUMIDITY 3304U             /**< IPSO Humidity Sensor */
#define SYN_LWM2M_OBJ_ACTUATION 3306U            /**< IPSO Actuation / Switch */

/* ── Standard OMA Content-Formats ────────────────────────────────────────── */

#define SYN_LWM2M_FORMAT_TEXT 0U         /**< text/plain */
#define SYN_LWM2M_FORMAT_LINK_FORMAT 40U /**< application/link-format */
#define SYN_LWM2M_FORMAT_OPAQUE 42U      /**< application/octet-stream */
#define SYN_LWM2M_FORMAT_SENML_CBOR 112U /**< application/senml+cbor */
#define SYN_LWM2M_FORMAT_TLV 11542U      /**< application/vnd.oma.lwm2m+tlv */
#define SYN_LWM2M_FORMAT_JSON 11543U     /**< application/vnd.oma.lwm2m+json */
#define SYN_LWM2M_FORMAT_CBOR 11544U     /**< application/vnd.oma.lwm2m+cbor */

/* ── Limits & Constants ─────────────────────────────────────────────────── */

#define SYN_LWM2M_MAX_OBJECTS 16U      /**< Maximum registered objects per client */
#define SYN_LWM2M_MAX_OBSERVERS 8U     /**< Maximum active resource observations */
#define SYN_LWM2M_MAX_URI_PATH_LEN 64U /**< Max URI path buffer size */

/* ── Resource Operations Bitmask ─────────────────────────────────────────── */

#define SYN_LWM2M_OP_NONE 0x00U                           /**< No access permitted */
#define SYN_LWM2M_OP_R 0x01U                              /**< Read operation allowed */
#define SYN_LWM2M_OP_W 0x02U                              /**< Write operation allowed */
#define SYN_LWM2M_OP_E 0x04U                              /**< Execute operation allowed */
#define SYN_LWM2M_OP_RW (SYN_LWM2M_OP_R | SYN_LWM2M_OP_W) /**< Read/Write allowed */

/* ── Resource Value Types ────────────────────────────────────────────────── */

/**
 * @brief LwM2M primitive resource value types.
 */
typedef enum {
    SYN_LWM2M_TYPE_NONE = 0, /**< Empty or invalid */
    SYN_LWM2M_TYPE_STRING,   /**< UTF-8 String */
    SYN_LWM2M_TYPE_INT,      /**< 64-bit Signed Integer */
    SYN_LWM2M_TYPE_FLOAT,    /**< Double-precision Float */
    SYN_LWM2M_TYPE_BOOL,     /**< Boolean */
    SYN_LWM2M_TYPE_OPAQUE,   /**< Opaque binary data */
    SYN_LWM2M_TYPE_TIME,     /**< Unix timestamp */
    SYN_LWM2M_TYPE_OBJLNK,   /**< Object link (object_id:instance_id) */
} SYN_LwM2M_ValType;

/**
 * @brief LwM2M Generic Value Container.
 */
typedef struct {
    SYN_LwM2M_ValType type; /**< Value type tag */
    union {
        int64_t integer; /**< Integer value */
        double floating; /**< Float value */
        bool boolean;    /**< Boolean value */
        const char *str; /**< Pointer to string */
        struct {
            const uint8_t *data; /**< Pointer to binary data */
            size_t len;          /**< Data length in bytes */
        } opaque;                /**< Opaque binary data */
        struct {
            uint16_t obj_id;  /**< Target Object ID */
            uint16_t inst_id; /**< Target Instance ID */
        } objlnk;             /**< Object link value */
    } val;                    /**< Value storage union */
} SYN_LwM2M_Value;

/* ── TLV (Type-Length-Value) Binary Format ────────────────────────────────── */

/**
 * @brief LwM2M TLV Type identifier (Bits 7-6 of TLV header byte).
 */
typedef enum {
    SYN_LWM2M_TLV_OBJECT_INSTANCE = 0x00U,   /**< Object Instance TLV */
    SYN_LWM2M_TLV_RESOURCE_INSTANCE = 0x40U, /**< Resource Instance TLV for multiple resource */
    SYN_LWM2M_TLV_MULTIPLE_RESOURCE = 0x80U, /**< Multiple Resource TLV */
    SYN_LWM2M_TLV_RESOURCE = 0xC0U,          /**< Single Resource TLV */
} SYN_LwM2M_TLV_Type;

/**
 * @brief Decoded / to-be-encoded LwM2M TLV item.
 */
typedef struct {
    SYN_LwM2M_TLV_Type type; /**< Item type */
    uint16_t id;             /**< Identifier (Object/Instance/Resource ID) */
    const uint8_t *val;      /**< Pointer to value bytes */
    size_t len;              /**< Length of value in bytes */
} SYN_LwM2M_TLV;

/* ── Object & Resource Descriptors ───────────────────────────────────────── */

/**
 * @brief Resource descriptor defining ID, permissions, and type.
 */
typedef struct {
    uint16_t id;            /**< Resource ID */
    uint8_t operations;     /**< Bitmask of allowed operations (SYN_LWM2M_OP_*) */
    SYN_LwM2M_ValType type; /**< Data type */
} SYN_LwM2M_ResourceDesc;

/**
 * @brief Forward declaration of LwM2M object descriptor.
 */
typedef struct SYN_LwM2M_Object SYN_LwM2M_Object;

/**
 * @brief Read callback: retrieves current resource value.
 */
typedef SYN_Status (*SYN_LwM2M_ReadCb)(uint16_t instance_id, uint16_t resource_id,
                                       SYN_LwM2M_Value *out_val, void *user_ctx);

/**
 * @brief Write callback: writes new resource value.
 */
typedef SYN_Status (*SYN_LwM2M_WriteCb)(uint16_t instance_id, uint16_t resource_id,
                                        const SYN_LwM2M_Value *in_val, void *user_ctx);

/**
 * @brief Execute callback: executes an executable resource with optional arguments.
 */
typedef SYN_Status (*SYN_LwM2M_ExecCb)(uint16_t instance_id, uint16_t resource_id,
                                       const uint8_t *args, size_t args_len, void *user_ctx);

/**
 * @brief LwM2M Object Definition.
 */
struct SYN_LwM2M_Object {
    uint16_t id;                             /**< Object ID (e.g. 3 for Device) */
    uint16_t instance_count;                 /**< Number of instances supported */
    const uint16_t *instances;               /**< Array of valid instance IDs */
    size_t resource_count;                   /**< Number of resources in descriptor */
    const SYN_LwM2M_ResourceDesc *resources; /**< Array of resource descriptors */
    SYN_LwM2M_ReadCb read;                   /**< Read callback */
    SYN_LwM2M_WriteCb write;                 /**< Write callback */
    SYN_LwM2M_ExecCb exec;                   /**< Execute callback */
    void *user_ctx;                          /**< User context pointer */
};

/* ── Observation Table ───────────────────────────────────────────────────── */

/**
 * @brief Active resource or instance observation (RFC 7641).
 */
typedef struct {
    bool active;             /**< Slot is currently active */
    uint8_t token_len;       /**< CoAP observation token length */
    uint8_t token[8];        /**< CoAP observation token */
    uint16_t obj_id;         /**< Target Object ID */
    uint16_t inst_id;        /**< Target Instance ID */
    int32_t res_id;          /**< Target Resource ID (-1 if observing whole instance) */
    uint32_t pmin;           /**< Minimum observation period in seconds */
    uint32_t pmax;           /**< Maximum observation period in seconds */
    uint32_t last_notify_ms; /**< Timestamp of last notification in ms */
    uint32_t seq;            /**< Notification sequence counter */
} SYN_LwM2M_Observation;

/* ── LwM2M Client State Machine ──────────────────────────────────────────── */

/**
 * @brief LwM2M client registration states.
 */
typedef enum {
    SYN_LWM2M_STATE_DEREGISTERED = 0, /**< Client is not registered */
    SYN_LWM2M_STATE_REGISTERING,      /**< Registration request in progress */
    SYN_LWM2M_STATE_REGISTERED,       /**< Client is registered with LwM2M server */
    SYN_LWM2M_STATE_UPDATING,         /**< Registration update in progress */
} SYN_LwM2M_ClientState;

/**
 * @brief LwM2M Client Context.
 */
typedef struct {
    const char *endpoint_name; /**< Client endpoint name (e.g. "urn:imei:12345") */
    uint32_t lifetime_s;       /**< Registration lifetime in seconds */
    const char *binding;       /**< Transport binding ("U" for UDP) */
    SYN_Transport *transport;  /**< Underlying generic or DTLS transport */

    SYN_LwM2M_ClientState state;                    /**< Registration state */
    char location_path[SYN_LWM2M_MAX_URI_PATH_LEN]; /**< Server-assigned location path (e.g.
                                                       "rd/42") */
    uint32_t last_update_ms; /**< Timestamp of last registration/update in ms */

    const SYN_LwM2M_Object *objects[SYN_LWM2M_MAX_OBJECTS]; /**< Registered objects */
    size_t object_count;                                    /**< Count of registered objects */

    SYN_LwM2M_Observation observations[SYN_LWM2M_MAX_OBSERVERS]; /**< Observation slots */
    char query_ep[64];      /**< Per-instance Uri-Query endpoint scratch buffer */
    char query_lt[32];      /**< Per-instance Uri-Query lifetime scratch buffer */
    uint8_t obs_seq_buf[3]; /**< Per-instance Observe sequence scratch buffer */
} SYN_LwM2M_Client;

/* ── TLV Codec API ───────────────────────────────────────────────────────── */

/**
 * @brief Encode a raw TLV header and value into a buffer.
 *
 * @param tlv     TLV item to encode.
 * @param buf     [out] Target buffer.
 * @param max_len Capacity of target buffer.
 * @return Number of bytes written, or 0 on buffer overflow.
 */
size_t syn_lwm2m_tlv_encode(const SYN_LwM2M_TLV *tlv, uint8_t *buf, size_t max_len);

/**
 * @brief Encode a 64-bit integer resource as TLV.
 *
 * @param res_id  Resource ID.
 * @param val     Integer value to encode.
 * @param buf     [out] Target buffer.
 * @param max_len Capacity of target buffer.
 * @return Number of bytes written, or 0 on overflow.
 */
size_t syn_lwm2m_tlv_encode_int(uint16_t res_id, int64_t val, uint8_t *buf, size_t max_len);

/**
 * @brief Encode a float/double resource as TLV (Float32 or Float64).
 *
 * @param res_id  Resource ID.
 * @param val     Floating-point value.
 * @param buf     [out] Target buffer.
 * @param max_len Capacity of target buffer.
 * @return Number of bytes written, or 0 on overflow.
 */
size_t syn_lwm2m_tlv_encode_float(uint16_t res_id, double val, uint8_t *buf, size_t max_len);

/**
 * @brief Encode a boolean resource as TLV (1-byte int 0 or 1).
 *
 * @param res_id  Resource ID.
 * @param val     Boolean value.
 * @param buf     [out] Target buffer.
 * @param max_len Capacity of target buffer.
 * @return Number of bytes written, or 0 on overflow.
 */
size_t syn_lwm2m_tlv_encode_bool(uint16_t res_id, bool val, uint8_t *buf, size_t max_len);

/**
 * @brief Encode a string resource as TLV.
 *
 * @param res_id  Resource ID.
 * @param str     Null-terminated string.
 * @param buf     [out] Target buffer.
 * @param max_len Capacity of target buffer.
 * @return Number of bytes written, or 0 on overflow.
 */
size_t syn_lwm2m_tlv_encode_string(uint16_t res_id, const char *str, uint8_t *buf, size_t max_len);

/**
 * @brief Encode opaque binary data resource as TLV.
 *
 * @param res_id  Resource ID.
 * @param data    Binary buffer.
 * @param len     Length of binary data.
 * @param buf     [out] Target buffer.
 * @param max_len Capacity of target buffer.
 * @return Number of bytes written, or 0 on overflow.
 */
size_t syn_lwm2m_tlv_encode_opaque(uint16_t res_id, const uint8_t *data, size_t len, uint8_t *buf,
                                   size_t max_len);

/**
 * @brief Encode a generic SYN_LwM2M_Value as TLV.
 *
 * @param res_id  Resource ID.
 * @param val     Typed resource value container.
 * @param buf     [out] Target buffer.
 * @param max_len Capacity of target buffer.
 * @return Number of bytes written, or 0 on overflow.
 */
size_t syn_lwm2m_tlv_encode_value(uint16_t res_id, const SYN_LwM2M_Value *val, uint8_t *buf,
                                  size_t max_len);

/**
 * @brief Wrap nested resource TLVs inside an Object Instance TLV header.
 *
 * @param inst_id     Object instance ID.
 * @param tlv_payload Serialized resource TLVs buffer.
 * @param payload_len Length of resource TLVs.
 * @param buf         [out] Target buffer.
 * @param max_len     Capacity of target buffer.
 * @return Number of bytes written, or 0 on overflow.
 */
size_t syn_lwm2m_tlv_encode_instance(uint16_t inst_id, const uint8_t *tlv_payload,
                                     size_t payload_len, uint8_t *buf, size_t max_len);

/**
 * @brief Decode a single TLV item from a byte stream.
 *
 * @param buf       Input buffer.
 * @param buf_len   Available bytes in buffer.
 * @param tlv       [out] Decoded TLV header and value pointer.
 * @param consumed  [out] Total bytes consumed for this TLV.
 * @return true on success, false on format/length error.
 */
bool syn_lwm2m_tlv_decode(const uint8_t *buf, size_t buf_len, SYN_LwM2M_TLV *tlv, size_t *consumed);

/**
 * @brief Decode a signed integer from TLV value bytes.
 *
 * @param tlv     Decoded TLV descriptor.
 * @param out_val [out] Decoded 64-bit integer.
 * @return true on success, false on length error or null parameters.
 */
bool syn_lwm2m_tlv_decode_int(const SYN_LwM2M_TLV *tlv, int64_t *out_val);

/**
 * @brief Decode a float/double from TLV value bytes.
 *
 * @param tlv     Decoded TLV descriptor.
 * @param out_val [out] Decoded double.
 * @return true on success, false on length error or null parameters.
 */
bool syn_lwm2m_tlv_decode_float(const SYN_LwM2M_TLV *tlv, double *out_val);

/**
 * @brief Decode a boolean from TLV value bytes.
 *
 * @param tlv     Decoded TLV descriptor.
 * @param out_val [out] Decoded boolean.
 * @return true on success, false on length error or null parameters.
 */
bool syn_lwm2m_tlv_decode_bool(const SYN_LwM2M_TLV *tlv, bool *out_val);

/**
 * @brief Decode a string from TLV value bytes into a null-terminated buffer.
 *
 * @param tlv     Decoded TLV descriptor.
 * @param out_str [out] Target buffer for string.
 * @param max_len Capacity of target buffer.
 * @return true on success, false on overflow or null parameters.
 */
bool syn_lwm2m_tlv_decode_string(const SYN_LwM2M_TLV *tlv, char *out_str, size_t max_len);

/**
 * @brief Decode a TLV into a typed SYN_LwM2M_Value container.
 *
 * @param tlv           Decoded TLV descriptor.
 * @param expected_type Expected value type.
 * @param out_val       [out] Decoded value container.
 * @return true on success, false on decoding failure.
 */
bool syn_lwm2m_tlv_decode_value(const SYN_LwM2M_TLV *tlv, SYN_LwM2M_ValType expected_type,
                                SYN_LwM2M_Value *out_val);

/* ── Core Link Format & Registration API ─────────────────────────────────── */

/**
 * @brief Generate CoRE Link Format string listing all registered object instances.
 *
 * Example output: `</1/0>, </3/0>, </5/0>, </3303/0>`
 *
 * @param client  LwM2M client context.
 * @param buf     [out] Buffer to receive link format string.
 * @param max_len Capacity of link format buffer.
 * @return Number of characters written (excluding null terminator), or 0 on overflow.
 */
size_t syn_lwm2m_generate_link_format(const SYN_LwM2M_Client *client, char *buf, size_t max_len);

/**
 * @brief Initialize a zero-heap LwM2M client context.
 *
 * @param client        Client context to initialize.
 * @param endpoint_name Client endpoint name (e.g. "urn:imei:862415039201923").
 * @param lifetime_s    Registration lifetime in seconds (e.g. 300).
 * @param transport     Underlying datagram transport.
 * @return true on success, false on invalid parameters.
 */
bool syn_lwm2m_client_init(SYN_LwM2M_Client *client, const char *endpoint_name, uint32_t lifetime_s,
                           SYN_Transport *transport);

/**
 * @brief Register an object definition into the client object registry.
 *
 * @param client LwM2M client context.
 * @param obj    Object definition descriptor.
 * @return true on success, false if registry is full or invalid parameters.
 */
bool syn_lwm2m_register_object(SYN_LwM2M_Client *client, const SYN_LwM2M_Object *obj);

/**
 * @brief Build a CoAP Registration POST request (`/rd?ep=...&lt=...`).
 *
 * @param client          LwM2M client context.
 * @param msg_id          CoAP message ID.
 * @param token           CoAP token bytes.
 * @param token_len       Length of CoAP token.
 * @param req             [out] Populated CoAP request header.
 * @param req_opts        [out] Populated CoAP options array.
 * @param max_req_opts    Capacity of options array.
 * @param req_opt_cnt     [out] Number of populated options.
 * @param payload_buf     [out] Buffer for link format payload.
 * @param payload_buf_sz  Capacity of payload buffer.
 * @return Length of serialized payload in bytes, or 0 on error.
 */
size_t syn_lwm2m_build_register_request(SYN_LwM2M_Client *client, uint16_t msg_id,
                                        const uint8_t *token, uint8_t token_len, SYN_CoapMsg *req,
                                        SYN_CoapOption *req_opts, size_t max_req_opts,
                                        size_t *req_opt_cnt, uint8_t *payload_buf,
                                        size_t payload_buf_sz);

/**
 * @brief Build a CoAP Registration Update POST request (`/rd/<location>`).
 *
 * @param client          LwM2M client context.
 * @param msg_id          CoAP message ID.
 * @param token           CoAP token bytes.
 * @param token_len       Length of CoAP token.
 * @param lifetime_s      Updated registration lifetime in seconds (0 to keep current).
 * @param req             [out] Populated CoAP request header.
 * @param req_opts        [out] Populated CoAP options array.
 * @param max_req_opts    Capacity of options array.
 * @param req_opt_cnt     [out] Number of populated options.
 * @param payload_buf     [out] Optional buffer for updated link format.
 * @param payload_buf_sz  Capacity of payload buffer.
 * @return Length of payload in bytes (0 if lifetime update only; req->code is COAP_CODE_POST on
 *         success or 0 on error).
 */
size_t syn_lwm2m_build_update_request(SYN_LwM2M_Client *client, uint16_t msg_id,
                                      const uint8_t *token, uint8_t token_len, uint32_t lifetime_s,
                                      SYN_CoapMsg *req, SYN_CoapOption *req_opts,
                                      size_t max_req_opts, size_t *req_opt_cnt,
                                      uint8_t *payload_buf, size_t payload_buf_sz);

/**
 * @brief Build a CoAP Deregistration DELETE request (`/rd/<location>`).
 *
 * @param client          LwM2M client context.
 * @param msg_id          CoAP message ID.
 * @param token           CoAP token bytes.
 * @param token_len       Length of CoAP token.
 * @param req             [out] Populated CoAP request header.
 * @param req_opts        [out] Populated CoAP options array.
 * @param max_req_opts    Capacity of options array.
 * @param req_opt_cnt     [out] Number of populated options.
 * @return 0 on success.
 */
size_t syn_lwm2m_build_deregister_request(const SYN_LwM2M_Client *client, uint16_t msg_id,
                                          const uint8_t *token, uint8_t token_len, SYN_CoapMsg *req,
                                          SYN_CoapOption *req_opts, size_t max_req_opts,
                                          size_t *req_opt_cnt);

/**
 * @brief Handle server response to Registration request (parses Location-Path).
 *
 * @param client       LwM2M client context.
 * @param resp         Received CoAP response.
 * @param resp_opts    Received response options.
 * @param resp_opt_cnt Number of response options.
 * @return true if registration succeeded (2.01 Created), false otherwise.
 */
bool syn_lwm2m_handle_register_response(SYN_LwM2M_Client *client, const SYN_CoapMsg *resp,
                                        const SYN_CoapOption *resp_opts, size_t resp_opt_cnt);

/* ── Request Processing & Device Management ──────────────────────────────── */

/**
 * @brief Dispatch and process an incoming CoAP request against registered LwM2M objects.
 *
 * Handles GET (Read / Discover), PUT/POST (Write), POST (Execute), DELETE.
 *
 * @param client       LwM2M client context.
 * @param req          Incoming CoAP request.
 * @param req_opts     Request options (Uri-Path, Content-Format, Observe, etc.).
 * @param req_opt_cnt  Number of request options.
 * @param resp         [out] Prepared CoAP response.
 * @param resp_opts    [out] Output response options array.
 * @param max_resp_opts Capacity of response options array.
 * @param resp_opt_cnt [out] Number of populated response options.
 * @param resp_buf     [out] Buffer for response payload.
 * @param resp_buf_sz  Capacity of response buffer.
 * @return SYN_OK on successful request handling, error code otherwise.
 */
SYN_Status syn_lwm2m_process_request(SYN_LwM2M_Client *client, const SYN_CoapMsg *req,
                                     const SYN_CoapOption *req_opts, size_t req_opt_cnt,
                                     SYN_CoapMsg *resp, SYN_CoapOption *resp_opts,
                                     size_t max_resp_opts, size_t *resp_opt_cnt, uint8_t *resp_buf,
                                     size_t resp_buf_sz);

/* ── Observation & Notification API (RFC 7641) ───────────────────────────── */

/**
 * @brief Add or update an observation for an object instance or resource.
 *
 * @param client    LwM2M client context.
 * @param obj_id    Object ID.
 * @param inst_id   Instance ID.
 * @param res_id    Resource ID (-1 for entire instance).
 * @param token     CoAP observation token.
 * @param token_len Length of token.
 * @param pmin      Minimum period in seconds (0 = immediate).
 * @param pmax      Maximum period in seconds (0 = none).
 * @return true on success, false if table full.
 */
bool syn_lwm2m_observe_add(SYN_LwM2M_Client *client, uint16_t obj_id, uint16_t inst_id,
                           int32_t res_id, const uint8_t *token, uint8_t token_len, uint32_t pmin,
                           uint32_t pmax);

/**
 * @brief Remove an active observation by matching token.
 *
 * @param client    LwM2M client context.
 * @param token     CoAP token bytes.
 * @param token_len Length of token.
 * @return true if an entry was found and removed, false otherwise.
 */
bool syn_lwm2m_observe_remove(SYN_LwM2M_Client *client, const uint8_t *token, uint8_t token_len);

/**
 * @brief Build a CoAP Notification response for an active observation.
 *
 * @param client       LwM2M client context.
 * @param obj_id       Object ID.
 * @param inst_id      Instance ID.
 * @param res_id       Resource ID (-1 for entire instance).
 * @param msg_id       CoAP message ID.
 * @param resp         [out] Populated CoAP notification response header.
 * @param resp_opts    [out] Output response options array.
 * @param max_resp_opts Capacity of response options array.
 * @param resp_opt_cnt [out] Number of populated response options.
 * @param resp_buf     [out] Buffer for notification payload.
 * @param resp_buf_sz  Capacity of notification buffer.
 * @return Length of encoded payload in bytes, or 0 on error.
 */
size_t syn_lwm2m_build_notify(SYN_LwM2M_Client *client, uint16_t obj_id, uint16_t inst_id,
                              int32_t res_id, uint16_t msg_id, SYN_CoapMsg *resp,
                              SYN_CoapOption *resp_opts, size_t max_resp_opts, size_t *resp_opt_cnt,
                              uint8_t *resp_buf, size_t resp_buf_sz);

/* ── Standard Standard Object Constructors ───────────────────────────────── */

/**
 * @brief Device Object (Object ID 3) state and context.
 */
typedef struct {
    const char *manufacturer;     /**< Res 0: Manufacturer */
    const char *model_number;     /**< Res 1: Model Number */
    const char *serial_number;    /**< Res 2: Serial Number */
    const char *firmware_ver;     /**< Res 3: Firmware Version */
    int64_t battery_level;        /**< Res 9: Battery Level (0..100) */
    int64_t memory_free_kb;       /**< Res 10: Free RAM in KB */
    int64_t error_code;           /**< Res 11: Error Code */
    int64_t current_time;         /**< Res 13: Current Time (Unix Epoch) */
    const char *utc_offset;       /**< Res 14: UTC Offset */
    const char *timezone;         /**< Res 15: Timezone */
    bool reboot_requested;        /**< Flag set when Res 4 (Reboot) executed */
    bool factory_reset_requested; /**< Flag set when Res 5 (Factory Reset) executed */
} SYN_LwM2M_DeviceContext;

/**
 * @brief Create an Object 3 (Device) descriptor bound to @p ctx.
 *
 * @param ctx Device object context state.
 * @return Initialized LwM2M object descriptor.
 */
SYN_LwM2M_Object syn_lwm2m_make_device_object(SYN_LwM2M_DeviceContext *ctx);

/**
 * @brief Firmware Update Object (Object ID 5) states per OMA LwM2M spec.
 */
typedef enum {
    SYN_LWM2M_FW_STATE_IDLE = 0,    /**< 0: Idle */
    SYN_LWM2M_FW_STATE_DOWNLOADING, /**< 1: Downloading */
    SYN_LWM2M_FW_STATE_DOWNLOADED,  /**< 2: Downloaded */
    SYN_LWM2M_FW_STATE_UPDATING,    /**< 3: Updating */
} SYN_LwM2M_FirmwareState;

/**
 * @brief Firmware Update Object (Object ID 5) update results.
 */
typedef enum {
    SYN_LWM2M_FW_RESULT_DEFAULT = 0,    /**< 0: Initial default */
    SYN_LWM2M_FW_RESULT_SUCCESS,        /**< 1: Firmware updated successfully */
    SYN_LWM2M_FW_RESULT_NO_FLASH,       /**< 2: Not enough flash storage */
    SYN_LWM2M_FW_RESULT_OUT_OF_RAM,     /**< 3: Out of RAM */
    SYN_LWM2M_FW_RESULT_CONN_LOST,      /**< 4: Connection lost during download */
    SYN_LWM2M_FW_RESULT_INTEGRITY_FAIL, /**< 5: Signature / hash verification failed */
    SYN_LWM2M_FW_RESULT_BAD_PKG_TYPE,   /**< 6: Unsupported package type */
    SYN_LWM2M_FW_RESULT_INVALID_URI,    /**< 7: Invalid package URI */
} SYN_LwM2M_FirmwareResult;

/**
 * @brief Firmware Update Object (Object ID 5) context.
 */
typedef struct {
    SYN_LwM2M_FirmwareState state;   /**< Res 3: State */
    SYN_LwM2M_FirmwareResult result; /**< Res 5: Update Result */
    char package_uri[128];           /**< Res 1: Package URI */
    char pkg_name[32];               /**< Res 6: Package Name */
    char pkg_version[32];            /**< Res 7: Package Version */
    bool update_requested;           /**< Flag set when Res 2 (Update) executed */
} SYN_LwM2M_FirmwareContext;

/**
 * @brief Create an Object 5 (Firmware Update) descriptor bound to @p ctx.
 *
 * @param ctx Firmware update object context state.
 * @return Initialized LwM2M object descriptor.
 */
SYN_LwM2M_Object syn_lwm2m_make_firmware_object(SYN_LwM2M_FirmwareContext *ctx);

/**
 * @brief Generic Sensor / Temperature Object (Object ID 3303) context.
 */
typedef struct {
    double sensor_value;     /**< Res 5700: Sensor Value */
    const char *unit;        /**< Res 5701: Sensor Unit (e.g. "Cel") */
    double min_measured_val; /**< Res 5601: Min Measured Value */
    double max_measured_val; /**< Res 5602: Max Measured Value */
} SYN_LwM2M_SensorContext;

/**
 * @brief Create an Object 3303 (Temperature Sensor) descriptor bound to @p ctx.
 *
 * @param ctx Temperature sensor object context state.
 * @return Initialized LwM2M object descriptor.
 */
SYN_LwM2M_Object syn_lwm2m_make_temperature_object(SYN_LwM2M_SensorContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_LWM2M_ENABLED */

#endif /* SYN_LWM2M_H */
