/**
 * @file syn_coap.h
 * @brief Constrained Application Protocol (CoAP - RFC 7252) implementation.
 * @ingroup syn_net
 */

#ifndef SYN_COAP_H
#define SYN_COAP_H

#include "../common/syn_defs.h"
#include "syn_dtls.h"

#if !defined(SYN_USE_COAP) || SYN_USE_COAP

#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"
#include "../util/syn_backoff.h"
#include "syn_transport.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COAP_VERSION 1 /**< CoAP protocol version */

/** @defgroup coap_types CoAP Message Types
 *  @{ */
#define COAP_TYPE_CON 0 /**< Confirmable        */
#define COAP_TYPE_NON 1 /**< Non-confirmable    */
#define COAP_TYPE_ACK 2 /**< Acknowledgement    */
#define COAP_TYPE_RST 3 /**< Reset              */
/** @} */

/** @defgroup coap_codes CoAP Request Codes
 *  @{ */
#define COAP_CODE_GET 1    /**< GET method         */
#define COAP_CODE_POST 2   /**< POST method        */
#define COAP_CODE_PUT 3    /**< PUT method         */
#define COAP_CODE_DELETE 4 /**< DELETE method      */
/** @} */

/** @defgroup coap_resp CoAP Response Codes
 *  Encoding: (class << 5) | detail.
 *  @{ */
#define COAP_RESP_CREATED 65    /**< 2.01 Created      */
#define COAP_RESP_DELETED 66    /**< 2.02 Deleted      */
#define COAP_RESP_VALID 67      /**< 2.03 Valid        */
#define COAP_RESP_CHANGED 68    /**< 2.04 Changed      */
#define COAP_RESP_CONTENT 69    /**< 2.05 Content      */
#define COAP_RESP_BAD_REQ 128   /**< 4.00 Bad Request  */
#define COAP_RESP_UNAUTH 129    /**< 4.01 Unauthorized */
#define COAP_RESP_BAD_OPT 130   /**< 4.02 Bad Option   */
#define COAP_RESP_FORBIDDEN 131 /**< 4.03 Forbidden    */
#define COAP_RESP_NOT_FOUND 132 /**< 4.04 Not Found    */
#define COAP_RESP_METHOD_NA 133 /**< 4.05 Method Not Allowed */
#define COAP_RESP_NOT_ACC 134   /**< 4.06 Not Acceptable */
#define COAP_RESP_PRECOND_F 140 /**< 4.12 Precondition Failed */
#define COAP_RESP_REQ_TOO_L 141 /**< 4.13 Request Entity Too Large */
#define COAP_RESP_UNSUP_MED 143 /**< 4.15 Unsupported Content-Format */
#define COAP_RESP_INTERNAL 160  /**< 5.00 Internal Server Error */
#define COAP_RESP_NOT_IMPL 161  /**< 5.01 Not Implemented */
#define COAP_RESP_BAD_GATE 162  /**< 5.02 Bad Gateway  */
#define COAP_RESP_SVC_UNAV 163  /**< 5.03 Service Unavailable */
#define COAP_RESP_GATE_TO 164   /**< 5.04 Gateway Timeout */
#define COAP_RESP_PROXY_NA 165  /**< 5.05 Proxying Not Supported */
/** @} */

/** @defgroup coap_opts CoAP Option Numbers
 *  @{ */
#define COAP_OPT_URI_HOST 3        /**< Uri-Host          */
#define COAP_OPT_ETAG 4            /**< ETag              */
#define COAP_OPT_OBSERVE 6         /**< Observe           */
#define COAP_OPT_URI_PORT 7        /**< Uri-Port          */
#define COAP_OPT_LOCATION_PATH 8   /**< Location-Path     */
#define COAP_OPT_URI_PATH 11       /**< Uri-Path          */
#define COAP_OPT_CONTENT_FORMAT 12 /**< Content-Format   */
#define COAP_OPT_MAX_AGE 14        /**< Max-Age           */
#define COAP_OPT_URI_QUERY 15      /**< Uri-Query         */
#define COAP_OPT_ACCEPT 17         /**< Accept            */
#define COAP_OPT_LOCATION_QUERY 20 /**< Location-Query   */
#define COAP_OPT_BLOCK2 23         /**< Block2 (Response Blockwise - RFC 7959) */
#define COAP_OPT_BLOCK1 27         /**< Block1 (Request Blockwise - RFC 7959)  */
#define COAP_OPT_PROXY_URI 35      /**< Proxy-Uri         */
#define COAP_OPT_PROXY_SCHEME 39   /**< Proxy-Scheme      */
#define COAP_OPT_SIZE1 60          /**< Size1             */
/** @} */

/**
 * @brief CoAP Blockwise Transfer Parameter (RFC 7959).
 */
typedef struct {
    uint32_t num; /**< Block sequence number (0..1048575) */
    bool more;    /**< More blocks follow (true/false)     */
    uint8_t szx;  /**< Size exponent (0=16B, 1=32B, 2=64B, 3=128B, 4=256B, 5=512B, 6=1024B) */
} SYN_CoapBlock;

/**
 * @brief Single CoAP option (number + opaque value).
 */
typedef struct {
    uint16_t num;       /**< Option number (e.g. COAP_OPT_URI_PATH) */
    const uint8_t *val; /**< Pointer to option value bytes          */
    size_t len;         /**< Length of option value in bytes         */
} SYN_CoapOption;

/**
 * @brief Encode a CoAP Blockwise Option (Block1 / Block2) value (RFC 7959).
 *
 * @param block  Block parameters.
 * @param buf    [out] Output buffer for encoded uint (up to 3 bytes).
 * @return Number of bytes encoded (1, 2, or 3 bytes).
 */
size_t syn_coap_encode_block_opt(const SYN_CoapBlock *block, uint8_t buf[3]);

/**
 * @brief Decode a CoAP Blockwise Option (Block1 / Block2) value (RFC 7959).
 *
 * @param opt_val Pointer to option value bytes.
 * @param opt_len Option length in bytes (1, 2, or 3).
 * @param block   [out] Parsed block parameters.
 * @return true on success, false on invalid length or format.
 */
bool syn_coap_decode_block_opt(const uint8_t *opt_val, size_t opt_len, SYN_CoapBlock *block);

/**
 * @brief Parsed or to-be-serialized CoAP message header.
 */
typedef struct {
    uint8_t type;           /**< Message type (CON/NON/ACK/RST)        */
    uint8_t code;           /**< Request method or response code       */
    uint16_t msg_id;        /**< Message ID for deduplication          */
    uint8_t token_len;      /**< Token length (0–8)                    */
    uint8_t token[8];       /**< Token bytes                           */
    const uint8_t *payload; /**< Pointer to payload (NULL if none)     */
    size_t payload_len;     /**< Payload length in bytes               */
} SYN_CoapMsg;

/* Parser & Builder API */

/**
 * @brief Parse a raw CoAP packet into a message structure.
 * @param msg          [out] Parsed message header.
 * @param options      [out] Array to receive parsed options.
 * @param max_options  Capacity of the options array.
 * @param option_count [out] Number of options actually parsed.
 * @param buf          Raw packet buffer.
 * @param buf_len      Length of the raw packet in bytes.
 * @return SYN_OK on success, SYN_ERROR on malformed input.
 */
SYN_Status syn_coap_parse(SYN_CoapMsg *msg, SYN_CoapOption *options, size_t max_options,
                          size_t *option_count, const uint8_t *buf, size_t buf_len);

/**
 * @brief Serialize a CoAP message into a byte buffer.
 * @param msg          Message header to serialize.
 * @param options      Options array (will be sorted by option number).
 * @param option_count Number of options.
 * @param buf          [out] Destination buffer.
 * @param max_buf_len  Capacity of the destination buffer.
 * @return Number of bytes written, or 0 on error (buffer too small).
 */
size_t syn_coap_serialize(const SYN_CoapMsg *msg, const SYN_CoapOption *options,
                          size_t option_count, uint8_t *buf, size_t max_buf_len);

/**
 * @brief CoAP client request context.
 *
 * Populate the input fields, then register as a scheduler task using
 * syn_coap_request_task. On completion, check @c status and read
 * the response from @c resp_msg.
 */
typedef struct {
    /* ── Input (caller fills before launching task) ── */
    SYN_SockAddr server_addr;          /**< Destination UDP address           */
    const SYN_CoapMsg *req_msg;        /**< Request message to send           */
    const SYN_CoapOption *req_options; /**< Request options array             */
    size_t req_option_count;           /**< Number of request options          */

    /* ── Output (filled by task on completion) ── */
    SYN_CoapMsg resp_msg;           /**< Parsed response message               */
    SYN_CoapOption resp_options[8]; /**< Parsed response options               */
    size_t resp_option_count;       /**< Number of parsed response options      */
    uint8_t resp_buf[256];          /**< Raw response packet buffer             */
    size_t resp_len;                /**< Raw response length in bytes           */

    /* ── Execution state (must survive across yields) ── */
    SYN_Socket sock;        /**< UDP socket handle                     */
    uint8_t tx_buf[256];    /**< Serialized request (persists across retries) */
    size_t tx_len;          /**< Length of serialized request           */
    uint32_t start_ms;      /**< Tick at start of current send attempt */
    uint32_t attempt_delay; /**< Timeout duration for current attempt (ms) */
    SYN_Backoff backoff;    /**< Retransmit backoff state              */
    SYN_Status status;      /**< Final result (SYN_OK / SYN_TIMEOUT / SYN_ERROR) */
} SYN_CoapRequest;

/**
 * @brief Cooperative protothread task to execute a CoAP client request.
 * @param pt   Protothread state.
 * @param task Scheduler task (user_data must point to a SYN_CoapRequest).
 * @return Protothread status.
 */
SYN_PT_Status syn_coap_request_task(SYN_PT *pt, SYN_Task *task);

/**
 * @brief Initialize a CoAP request context.
 * @param r             Request context.
 * @param server_addr   Target server address.
 * @param msg           Request header/token/payload.
 * @param timeout_ms    Initial retransmit timeout (ms).
 * @param retries       Maximum number of retransmissions.
 */
void syn_coap_request_init(SYN_CoapRequest *r, const SYN_SockAddr *server_addr,
                           const SYN_CoapMsg *msg, uint32_t timeout_ms, uint8_t retries);

/* ── CoAP over Generic Transport (SYN_Transport) ────────────────────────── */

/**
 * @brief CoAP request context over generic SYN_Transport (e.g. DTLS 1.3 / UDP / Serial).
 */
typedef struct {
    /* ── Input ── */
    SYN_Transport *transport;          /**< Transport interface */
    const SYN_CoapMsg *req_msg;        /**< Request message to send */
    const SYN_CoapOption *req_options; /**< Request options array */
    size_t req_option_count;           /**< Number of request options */

    /* ── Output ── */
    SYN_CoapMsg resp_msg;           /**< Parsed response message */
    SYN_CoapOption resp_options[8]; /**< Parsed response options */
    size_t resp_option_count;       /**< Number of parsed response options */
    uint8_t resp_buf[256];          /**< Raw response packet buffer */
    size_t resp_len;                /**< Raw response length in bytes */

    /* ── Execution state ── */
    uint8_t tx_buf[256];    /**< Serialized request buffer */
    size_t tx_len;          /**< Serialized request length */
    uint32_t start_ms;      /**< Timestamp of current attempt */
    uint32_t attempt_delay; /**< Timeout duration for current attempt (ms) */
    SYN_Backoff backoff;    /**< Retransmit backoff state */
    SYN_Status status;      /**< Final request status */
} SYN_CoapTransportRequest;

/**
 * @brief Initialize a CoAP request context over a SYN_Transport.
 * @param r          Request context.
 * @param transport  Transport instance.
 * @param msg        Request message header.
 * @param timeout_ms Initial retransmit timeout (ms).
 * @param retries    Maximum number of retransmissions.
 */
void syn_coap_transport_request_init(SYN_CoapTransportRequest *r, SYN_Transport *transport,
                                     const SYN_CoapMsg *msg, uint32_t timeout_ms, uint8_t retries);

/**
 * @brief Cooperative protothread task for executing a CoAP request over SYN_Transport.
 * @param pt   Protothread context.
 * @param task Scheduler task (user_data pointing to SYN_CoapTransportRequest).
 * @return Protothread status.
 */
SYN_PT_Status syn_coap_transport_request_task(SYN_PT *pt, SYN_Task *task);

/**
 * @brief Send a CoAP request and await matching response over SYN_Transport.
 * @param transport     Transport instance.
 * @param req           Request message.
 * @param req_opts      Request options.
 * @param req_opt_cnt   Request option count.
 * @param resp          [out] Parsed response message.
 * @param resp_opts     [out] Parsed response options array.
 * @param max_resp_opts Capacity of response options array.
 * @param resp_opt_cnt  [out] Number of parsed response options.
 * @param resp_buf      [out] Buffer for raw received packet.
 * @param resp_buf_sz   Capacity of response buffer.
 * @return SYN_OK on success, error code otherwise.
 */
SYN_Status syn_coap_transport_send_request(SYN_Transport *transport, const SYN_CoapMsg *req,
                                           const SYN_CoapOption *req_opts, size_t req_opt_cnt,
                                           SYN_CoapMsg *resp, SYN_CoapOption *resp_opts,
                                           size_t max_resp_opts, size_t *resp_opt_cnt,
                                           uint8_t *resp_buf, size_t resp_buf_sz);

/* ── CoAP over DTLS 1.3 (coaps://) ───────────────────────────────────────── */

/** @brief Forward declaration for CoAP over DTLS 1.3 client context. */
typedef struct SYN_CoapsClient SYN_CoapsClient;

#if SYN_DTLS_ENABLED

/**
 * @brief CoAP over DTLS 1.3 Client Session (RFC 7252 Section 12.1.2 & RFC 9147).
 */
struct SYN_CoapsClient {
    SYN_DTLS_Context dtls;        /**< Underlying DTLS 1.3 protocol engine */
    SYN_Transport dtls_transport; /**< DTLS transport adapter */
};

/**
 * @brief Initialize a CoAP over DTLS 1.3 Client.
 * @param client               CoAPS client context.
 * @param config               DTLS 1.3 configuration.
 * @param underlying_transport Underlying datagram transport (e.g. UDP socket).
 * @param rx_buf               Caller-owned RX record buffer.
 * @param rx_buf_size          Capacity of RX record buffer.
 * @param tx_buf               Caller-owned TX record buffer.
 * @param tx_buf_size          Capacity of TX record buffer.
 * @return true on success, false on invalid parameters.
 */
bool syn_coaps_client_init(SYN_CoapsClient *client, const SYN_DTLS_Config *config,
                           SYN_Transport *underlying_transport, uint8_t *rx_buf, size_t rx_buf_size,
                           uint8_t *tx_buf, size_t tx_buf_size);

/**
 * @brief Advance or complete DTLS 1.3 handshake for CoAPS client.
 * @param client CoAPS client context.
 * @return true on handshake success, false on error.
 */
bool syn_coaps_client_handshake(SYN_CoapsClient *client);

/**
 * @brief Send a CoAPS request and receive response via DTLS 1.3 datagram channel.
 * @param client        CoAPS client context.
 * @param req           Request message.
 * @param req_opts      Request options.
 * @param req_opt_cnt   Request option count.
 * @param resp          [out] Parsed response message.
 * @param resp_opts     [out] Parsed response options array.
 * @param max_resp_opts Capacity of response options array.
 * @param resp_opt_cnt  [out] Number of parsed response options.
 * @param resp_buf      [out] Buffer for decrypted response payload.
 * @param resp_buf_sz   Capacity of response buffer.
 * @return SYN_OK on success, error code otherwise.
 */
SYN_Status syn_coaps_client_send_request(SYN_CoapsClient *client, const SYN_CoapMsg *req,
                                         const SYN_CoapOption *req_opts, size_t req_opt_cnt,
                                         SYN_CoapMsg *resp, SYN_CoapOption *resp_opts,
                                         size_t max_resp_opts, size_t *resp_opt_cnt,
                                         uint8_t *resp_buf, size_t resp_buf_sz);

#endif /* SYN_DTLS_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_COAP */

#endif /* SYN_COAP_H */
