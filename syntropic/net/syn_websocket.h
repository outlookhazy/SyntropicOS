/**
 * @file syn_websocket.h
 * @brief WebSocket protocol support on httpd.
 * @ingroup syn_net
 */

#ifndef SYN_WEBSOCKET_H
#define SYN_WEBSOCKET_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"
#include "syn_httpd.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WebSocket connection states.
 */
typedef enum {
    SYN_WS_STATE_CLOSED,             /**< Connection is closed */
    SYN_WS_STATE_CONNECTED,          /**< Handshake complete, active socket */
} SYN_WebsocketState;

/**
 * @brief WebSocket session context.
 */
typedef struct {
    SYN_Socket         sock;         /**< Network socket handle */
    SYN_WebsocketState state;        /**< WebSocket connection state */
    uint8_t            rx_buf[128];  /**< Payload receive packet storage */
    uint8_t            rx_state;     /**< Frame parsing state: 0=header, 1=length, 2=mask, 3=payload */
    uint32_t           payload_len;  /**< Size of the current incoming frame payload */
    uint32_t           bytes_read;   /**< Accumulated payload bytes read so far */
    uint8_t            mask_key[4];  /**< Client-to-server frame masking key */
    bool               masked;       /**< True if the incoming frame is masked */
    uint8_t            opcode;       /**< WebSocket opcode (e.g. text, binary, close, ping) */
    
    /**
     * @brief User callback invoked when a complete frame is received.
     * @param payload Received frame data.
     * @param len     Frame length in bytes.
     * @param opcode  Frame type opcode.
     * @param ctx     User context pointer.
     */
    void (*on_message)(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx);
    void              *ctx;          /**< User context pointer for message callback */
} SYN_WebsocketSession;

/**
 * @brief Handle upgrading a connection in an httpd route handler to WebSocket.
 *
 * Performs handshake, sends 101 status, sets upgraded = true on the response.
 *
 * @param req        Incoming HTTP upgrade request.
 * @param resp       HTTP response interface to write upgrade headers to.
 * @param ws         WebSocket session context to initialize.
 * @param on_message Handled frame callback function.
 * @param ctx        User context pointer passed through to callback.
 * @return SYN_OK on successful handshake, SYN_ERROR on negotiation failure.
 */
SYN_Status syn_websocket_upgrade(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp,
                                 SYN_WebsocketSession *ws,
                                 void (*on_message)(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx),
                                 void *ctx);

/**
 * @brief Send a frame over WebSocket.
 *
 * Formats a WebSocket packet header and writes the payload to the socket.
 *
 * @param ws     WebSocket session context.
 * @param opcode Frame type opcode (e.g., text, binary, ping).
 * @param data   Payload data to send.
 * @param len    Length of payload in bytes.
 * @return SYN_OK on success, or socket error code.
 */
SYN_Status syn_websocket_send(SYN_WebsocketSession *ws, uint8_t opcode,
                              const void *data, size_t len);

/**
 * @brief Background task for polling active WebSockets.
 *
 * @param pt   Cooperative protothread state tracker.
 * @param task Scheduler task control block.
 * @return PT_WAITING or PT_EXITED status.
 */
SYN_PT_Status syn_websocket_task(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_WEBSOCKET_H */
