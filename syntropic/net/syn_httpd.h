/**
 * @file syn_httpd.h
 * @brief Minimal HTTP/1.1 server — route-based, zero-alloc.
 *
 * Designed to run as a cooperative protothread task within the
 * SyntropicOS scheduler. Yields between accepts so other tasks
 * can run. Handles one request per scheduler tick.
 *
 * @par Usage
 * @code
 *   static void handle_status(const SYN_HttpdRequest *req,
 *                              SYN_HttpdResponse *resp, void *ctx) {
 *       syn_httpd_status(resp, 200, "OK");
 *       syn_httpd_header(resp, "Content-Type", "application/json");
 *       syn_httpd_body_str(resp, "{\"status\":\"ok\"}");
 *   }
 *
 *   static const SYN_HttpdRoute routes[] = {
 *       { SYN_HTTP_GET, "/api/status", handle_status, NULL },
 *   };
 *
 *   // In your task array:
 *   static uint8_t httpd_buf[1024];
 *   static SYN_Httpd httpd;
 *   syn_httpd_init(&httpd, 80, routes, 1, httpd_buf, sizeof(httpd_buf));
 *   syn_task_create(&tasks[N], "httpd", syn_httpd_task, 2, &httpd);
 * @endcode
 * @ingroup syn_net
 */

#ifndef SYN_HTTPD_H
#define SYN_HTTPD_H

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

/* ── HTTP methods ──────────────────────────────────────────────────────── */

/**
 * @brief Supported HTTP request methods.
 */
typedef enum {
    SYN_HTTP_GET    = 0,             /**< Retrieve a resource */
    SYN_HTTP_POST   = 1,             /**< Submit data to be processed */
    SYN_HTTP_PUT    = 2,             /**< Upload or replace a resource */
    SYN_HTTP_DELETE = 3,             /**< Delete a resource */
} SYN_HttpMethod;

/* ── Request (parsed, presented to handler) ────────────────────────────── */

/**
 * @brief Parsed HTTP request container presented to route handlers.
 */
typedef struct {
    SYN_HttpMethod  method;          /**< GET, POST, etc.                  */
    const char     *path;            /**< Request path (in work_buf)       */
    const char     *query;           /**< Query string after '?', or NULL  */
    uint32_t        content_length;  /**< Content-Length, 0 if absent      */
    const char     *content_type;    /**< Content-Type, or NULL            */
    const char     *headers;         /**< Pointer to start of headers      */
    SYN_Socket      client_sock;     /**< Client socket (for body reads)   */
    size_t          body_consumed;   /**< Bytes of body already consumed   */
    size_t          body_buffered_offset; /**< Offset in work_buf to buffered body */
    size_t          body_buffered_len;    /**< Length of buffered body bytes    */
} SYN_HttpdRequest;

/* ── Response writer ───────────────────────────────────────────────────── */

/**
 * @brief HTTP response formatting state.
 */
typedef struct {
    SYN_Socket  sock;               /**< Client socket                    */
    uint8_t    *buf;                 /**< Shared work buffer               */
    size_t      buf_size;            /**< Buffer capacity                  */
    bool        headers_sent;        /**< Have headers been finalized?     */
    bool        upgraded;            /**< Has connection been upgraded?    */
} SYN_HttpdResponse;

/* ── Route handler ─────────────────────────────────────────────────────── */

/**
 * @brief Route handler function.
 *
 * Called when a request matches the route. The handler sends the
 * response using syn_httpd_status(), syn_httpd_header(), syn_httpd_body().
 */
typedef void (*SYN_HttpdHandler)(const SYN_HttpdRequest *req,
                                  SYN_HttpdResponse *resp,
                                  void *ctx);

/* ── Route entry ───────────────────────────────────────────────────────── */

/**
 * @brief An HTTP routing entry configuration.
 */
typedef struct {
    SYN_HttpMethod    method;        /**< HTTP method to match             */
    const char       *path;          /**< Path to match (or prefix + '*')  */
    SYN_HttpdHandler  handler;       /**< Handler function                 */
    void             *ctx;           /**< User context for handler         */
} SYN_HttpdRoute;

/* ── Server instance ───────────────────────────────────────────────────── */

/**
 * @brief HTTP server context structure.
 */
typedef struct {
    const SYN_HttpdRoute *routes;    /**< Array of registered route entries */
    size_t                route_count; /**< Number of routes in array */
    uint8_t              *work_buf;  /**< Buffer for request processing */
    size_t                work_buf_size; /**< Size of work buffer in bytes */
    uint16_t              port;      /**< Listening TCP port number */
    SYN_Socket            listener;  /**< Bound listener socket handle */
    bool                  running;   /**< Server state active flag */
} SYN_Httpd;

/* ── Server API ────────────────────────────────────────────────────────── */

/**
 * @brief Initialize and start the HTTP server.
 *
 * Creates a listening socket on the given port.
 *
 * @param srv            Server instance to initialize.
 * @param port           Port to listen on (typically 80).
 * @param routes         Route table (must outlive the server).
 * @param route_count    Number of routes.
 * @param work_buf       Shared work buffer for request parsing + responses.
 * @param work_buf_size  Buffer size (512+ recommended).
 * @return SYN_OK on success, SYN_ERROR if bind/listen fails.
 */
SYN_Status syn_httpd_init(SYN_Httpd *srv, uint16_t port,
                           const SYN_HttpdRoute *routes, size_t route_count,
                           uint8_t *work_buf, size_t work_buf_size);

/**
 * @brief Protothread task function for the HTTP server.
 *
 * Register this as a SYN_TaskFunc with user_data pointing to a
 * SYN_Httpd instance. The task accepts a connection, handles one
 * request, then yields back to the scheduler.
 *
 * @param pt   Pointer to the cooperative protothread context.
 * @param task Pointer to the scheduler task structure.
 * @return PT_WAITING or PT_EXITED status.
 */
SYN_PT_Status syn_httpd_task(SYN_PT *pt, SYN_Task *task);

/**
 * @brief Handle one incoming request (non-protothread version).
 *
 * Accepts one connection, parses the request, dispatches to the
 * matching route handler, and closes the connection. Use this if
 * you prefer manual control over the server loop.
 *
 * @param srv  Server instance.
 * @return SYN_OK if a request was handled, SYN_TIMEOUT if no client
 *         connected, SYN_ERROR on parse failure.
 */
SYN_Status syn_httpd_step(SYN_Httpd *srv);

/**
 * @brief Stop the server and close the listener socket.
 *
 * @param srv Server instance to stop.
 */
void syn_httpd_stop(SYN_Httpd *srv);

/* ── Response helpers (used from handlers) ─────────────────────────────── */

/**
 * @brief Begin the response with a status line.
 *
 * Must be called first. E.g., syn_httpd_status(resp, 200, "OK").
 *
 * @param resp   HTTP response state to write to.
 * @param code   HTTP numeric status code (e.g. 200).
 * @param reason Human readable status message (e.g. "OK").
 */
void syn_httpd_status(const SYN_HttpdResponse *resp, int code, const char *reason);

/**
 * @brief Add a response header.
 *
 * Must be called after syn_httpd_status() and before syn_httpd_body().
 *
 * @param resp   HTTP response state to write to.
 * @param name   Header key string (e.g. "Content-Type").
 * @param value  Header value string (e.g. "text/plain").
 */
void syn_httpd_header(const SYN_HttpdResponse *resp,
                       const char *name, const char *value);

/**
 * @brief Send response body data.
 *
 * Automatically finalizes headers on first call. Can be called multiple
 * times for streaming responses.
 *
 * @param resp HTTP response state to write to.
 * @param data Pointer to binary data to transmit.
 * @param len  Length of data in bytes.
 */
void syn_httpd_body(SYN_HttpdResponse *resp, const void *data, size_t len);

/**
 * @brief Send a string as the response body.
 *
 * Convenience wrapper for syn_httpd_body().
 *
 * @param resp HTTP response state to write to.
 * @param str  Null-terminated string to send.
 */
void syn_httpd_body_str(SYN_HttpdResponse *resp, const char *str);

/**
 * @brief Read request body data (for POST/PUT).
 *
 * @param req      Request info.
 * @param resp     Response writer context.
 * @param buf      Buffer to read into.
 * @param max_len  Buffer capacity in bytes.
 * @return Bytes read, 0 at end of body, -1 on error.
 */
int syn_httpd_read_body(const SYN_HttpdRequest *req,
                         const SYN_HttpdResponse *resp,
                         void *buf, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_HTTPD_H */
