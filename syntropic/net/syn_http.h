/**
 * @file syn_http.h
 * @brief Cooperative HTTP/1.1 client — streaming, zero-alloc, non-blocking.
 *
 * Performs HTTP GET and POST requests over a TCP socket as a cooperative
 * protothread task. Response bodies are delivered via streaming callback.
 * @ingroup syn_net
 */

#ifndef SYN_HTTP_H
#define SYN_HTTP_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Response info ──────────────────────────────────────────────────────── */

/**
 * @brief HTTP response metadata.
 */
typedef struct {
    int          status_code;     /**< HTTP status (200, 404, etc.)        */
    uint32_t     content_length;  /**< Content-Length value, 0 if absent   */
    bool         chunked;         /**< Transfer-Encoding: chunked?         */
    bool         connection_close; /**< Connection: close?                 */
    char         location[128];   /**< Location header for redirects       */
} SYN_HttpResponse;

/* ── Request headers (optional) ─────────────────────────────────────────── */

/**
 * @brief Key-value pair representing an HTTP header.
 */
typedef struct {
    const char *name;    /**< Header name (e.g., "Authorization")      */
    const char *value;   /**< Header value                              */
} SYN_HttpHeader;

/* ── Body callback ──────────────────────────────────────────────────────── */

/**
 * @brief Callback invoked to stream chunks of the HTTP response body.
 *
 * @param data Pointer to the received chunk of body data.
 * @param len Length of the data chunk in bytes.
 * @param ctx User-defined context pointer passed to syn_http_client_init().
 * @return true to continue reading body, false to abort.
 */
typedef bool (*SYN_HttpBodyCallback)(const uint8_t *data, size_t len,
                                     void *ctx);

/* ── Cooperative Task API ────────────────────────────────────────────────── */

/**
 * @brief States for the HTTP client cooperative state machine.
 */
typedef enum {
    SYN_HTTP_STATE_IDLE,             /**< Client is idle, waiting for initialization */
    SYN_HTTP_STATE_CONNECTING,       /**< Actively opening TCP socket connection to host */
    SYN_HTTP_STATE_SENDING_REQUEST,  /**< Formatting and transmitting HTTP request lines */
    SYN_HTTP_STATE_READING_HEADERS,  /**< Receiving and parsing HTTP response header blocks */
    SYN_HTTP_STATE_READING_BODY,     /**< Streaming response body content back via callback */
    SYN_HTTP_STATE_DONE,             /**< Request succeeded, connection finalized cleanly */
    SYN_HTTP_STATE_ERROR             /**< Connection failed, timed out, or parse error occurred */
} SYN_HttpState;

/**
 * @brief HTTP client context structure.
 */
typedef struct {
    SYN_HttpState state;             /**< Current client state machine state */
    SYN_Socket    sock;              /**< Client TCP network socket handle */
    const char   *host;              /**< Hostname to query */
    uint16_t      port;              /**< Port to connect to (typically 80) */
    const char   *path;              /**< Resource path on the server (e.g. "/index.html") */
    const char   *method;            /**< HTTP method (e.g. "GET", "POST") */
    const char   *content_type;      /**< Content-Type for POST payload, or NULL */
    const uint8_t *body;             /**< Request body buffer for POST, or NULL */
    size_t        body_len;          /**< Request body length, or 0 */
    const SYN_HttpHeader *headers;   /**< Optional array of custom request headers */
    uint8_t       header_count;      /**< Number of custom request headers */
    
    SYN_HttpResponse resp;           /**< Parsed response status and headers */
    SYN_HttpBodyCallback body_cb;    /**< Callback function to stream response body */
    void         *cb_ctx;            /**< User context for the body callback */
    
    uint8_t      *work_buf;          /**< Work buffer for socket transmission and parsing */
    size_t        work_buf_size;     /**< Capacity of the work buffer */
    
    /* Internal tracking */
    int           hops;              /**< Redirect hop counter to prevent crash loops */
    char          cur_host[64];      /**< Cached current redirect hostname */
    char          cur_path[128];     /**< Cached current redirect path */
    uint16_t      cur_port;          /**< Cached current redirect port */
    
    size_t        buf_used;          /**< Active bytes stored in work_buf */
    size_t        buf_pos;           /**< Read cursor within the work_buf */
    size_t        body_start;        /**< Index in work_buf where body data begins */
    
    /* Header reading */
    size_t        line_len;          /**< Length of the current parsed line */
    uint32_t      header_timeout_ms; /**< Header receive timeout threshold */
    
    /* Body reading */
    uint32_t      body_remaining;    /**< Number of expected body bytes remaining */
    bool          known_length;      /**< True if Content-Length was provided */
    uint32_t      body_timeout_ms;   /**< Body receive timeout threshold */
    
    /* Chunked body reading */
    uint32_t      chunk_remaining;   /**< Current chunk bytes remaining to read */
    uint8_t       chunk_state;       /**< Internal parser state (0: size line, 1: data, 2: trailing CRLF) */
    char          chunk_line[32];    /**< Parser buffer for reading chunk length text */
    size_t        chunk_line_pos;    /**< Write cursor in chunk_line */
    
    SYN_Status    status;            /**< Final transaction execution status */
} SYN_HttpClient;

/**
 * @brief Initialize the HTTP client struct.
 *
 * Configures the request metadata, target destination, custom headers, 
 * streaming callback, and temporary work buffer.
 *
 * @param client         Pointer to the client context to initialize.
 * @param method         HTTP method to perform (e.g. "GET" or "POST").
 * @param host           Destination hostname or IP address string.
 * @param port           Destination TCP port (e.g. 80).
 * @param path           Resource URL path (e.g. "/api/v1/update").
 * @param content_type   Type of content if body is present (e.g. "application/json").
 * @param body           Pointer to binary data to transmit as request body (or NULL).
 * @param body_len       Length of the request body in bytes.
 * @param headers        Optional array of custom HTTP headers to append.
 * @param header_count   Number of custom headers in the array.
 * @param body_cb        Callback function for streaming response chunks.
 * @param cb_ctx         User context passed through to the body callback.
 * @param work_buf       Working buffer for socket buffering and parsing.
 * @param work_buf_size  Size of the working buffer in bytes.
 * @return SYN_OK on success, or an error code on invalid parameters.
 */
SYN_Status syn_http_client_init(SYN_HttpClient *client,
                                const char *method,
                                const char *host, uint16_t port,
                                const char *path,
                                const char *content_type,
                                const uint8_t *body, size_t body_len,
                                const SYN_HttpHeader *headers, uint8_t header_count,
                                SYN_HttpBodyCallback body_cb, void *cb_ctx,
                                uint8_t *work_buf, size_t work_buf_size);

/**
 * @brief Cooperative task to drive the HTTP client.
 *
 * Yields while resolving, connecting, transmitting requests, and streaming
 * the response body chunks. Must be run inside the cooperative scheduler.
 *
 * @param pt             Pointer to the cooperative protothread structure.
 * @param task           Pointer to the corresponding task control block.
 * @return PT_WAITING, PT_EXITED, or another protothread status code.
 */
SYN_PT_Status syn_http_client_task(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_HTTP_H */
