/**
 * @file syn_wg.h
 * @brief WireGuard VPN client — Noise IK, pure C99, cooperative.
 *
 * Implements a WireGuard client as a cooperative protothread task.
 * Handles the Noise_IKpsk2 handshake, ChaCha20-Poly1305 transport
 * encryption, keepalive timers, and rekeying.
 *
 * The client connects to a single WireGuard peer (server) over UDP.
 * Decrypted IP packets are delivered to user code via a callback.
 * User code sends packets into the tunnel via syn_wg_send().
 *
 * No TUN device, no host routing — all traffic is handled in
 * userspace. The host OS only sees a single UDP socket.
 *
 * Requires: BLAKE2S, CHACHA20POLY1305, X25519, SNTP.
 *
 * @par Usage
 * @code
 *   static SYN_WG wg;
 *   static uint8_t rx[1500], tx[1500];
 *
 *   SYN_WgConfig cfg = {
 *       .endpoint = { .ip = {1,2,3,4}, .port = 51820 },
 *       .keepalive_interval_s = 25,
 *   };
 *   memcpy(cfg.private_key, my_key, 32);
 *   memcpy(cfg.peer_public_key, server_key, 32);
 *
 *   syn_wg_init(&wg, &cfg, &sntp, rx, sizeof(rx), tx, sizeof(tx));
 *   wg.on_recv = my_handler;
 *
 *   syn_task_create(&tasks[1], "wg", syn_wg_task, 1, &wg);
 * @endcode
 * @ingroup syn_net
 */

#ifndef SYN_WG_H
#define SYN_WG_H

#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_WG) || SYN_USE_WG

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"
#include "syn_sntp.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ──────────────────────────────────────────────────────────── */

/** Default tunnel MTU (inner IP packet before encryption). */
#ifndef SYN_WG_MTU
#define SYN_WG_MTU  1420
#endif

/** WireGuard transport overhead: type(4) + receiver(4) + counter(8) + tag(16) = 32 */
#define SYN_WG_TRANSPORT_OVERHEAD  32

/** WireGuard protocol timers (seconds). */
#define SYN_WG_REKEY_AFTER_TIME       120   /**< Initiate rekey after this many seconds */
#define SYN_WG_REJECT_AFTER_TIME      180   /**< Drop session after this many seconds   */
#define SYN_WG_REKEY_TIMEOUT            5   /**< Retry handshake if no response (s)     */
#define SYN_WG_KEEPALIVE_TIMEOUT       10   /**< Send keepalive if no outbound (s)      */

/** WireGuard message types. */
#define SYN_WG_MSG_INITIATION   1  /**< Handshake initiation message */
#define SYN_WG_MSG_RESPONSE     2  /**< Handshake response message   */
#define SYN_WG_MSG_COOKIE       3  /**< Cookie reply message        */
#define SYN_WG_MSG_TRANSPORT    4  /**< Encrypted transport message */

/** Message sizes. */
#define SYN_WG_INITIATION_SIZE  148 /**< Size of initiation message */
#define SYN_WG_RESPONSE_SIZE     92 /**< Size of response message   */

/* ── States ─────────────────────────────────────────────────────────────── */

/**
 * @brief WireGuard client connection state.
 */
typedef enum {
    SYN_WG_DISCONNECTED,     /**< No session, no handshake in progress   */
    SYN_WG_HANDSHAKE_INIT,   /**< Sent initiation, waiting for response  */
    SYN_WG_ESTABLISHED,      /**< Session active, transport data flowing */
} SYN_WgState;

/* ── Configuration ──────────────────────────────────────────────────────── */

/**
 * @brief WireGuard peer configuration — set once at init.
 */
typedef struct {
    uint8_t      private_key[32];       /**< Our Curve25519 private key      */
    uint8_t      peer_public_key[32];   /**< Server's public key             */
    uint8_t      preshared_key[32];     /**< Optional PSK (zero if unused)   */
    SYN_SockAddr endpoint;              /**< Server IP:port                  */
    uint16_t     keepalive_interval_s;  /**< Persistent keepalive (0=off)    */
} SYN_WgConfig;

/* ── Session keys ───────────────────────────────────────────────────────── */

/**
 * @brief Active session derived from a completed handshake.
 */
typedef struct {
    uint8_t  send_key[32];    /**< ChaCha20-Poly1305 key for outgoing      */
    uint8_t  recv_key[32];    /**< ChaCha20-Poly1305 key for incoming      */
    uint64_t send_counter;    /**< Outgoing nonce counter                   */
    uint64_t recv_counter;    /**< Highest received nonce                   */
    uint32_t recv_bitmap;     /**< Anti-replay sliding window (32 bits)     */
    uint32_t sender_index;    /**< Our sender index (in transport headers)  */
    uint32_t receiver_index;  /**< Peer's sender index                      */
    uint32_t established_ms;  /**< Tick when session was established        */
} SYN_WgSession;

/* ── Client context ─────────────────────────────────────────────────────── */

/**
 * @brief WireGuard client context — caller-owned.
 *
 * Contains config, session state, handshake scratch, timers, and
 * buffer pointers. Approximately ~350 bytes + buffer pointers.
 */
typedef struct {
    SYN_WgConfig   config;          /**< Peer configuration                  */
    SYN_WgState    state;           /**< Connection state                    */
    SYN_Socket     udp_sock;        /**< UDP socket to the server            */

    /* Time source */
    SYN_SNTP      *sntp;            /**< NTP time source (for TAI64N)        */

    /* Derived keys (computed once at init from config) */
    uint8_t        public_key[32];  /**< Our public key (from private)       */

    /* Active session */
    SYN_WgSession  session;          /**< Currently active transport session */

    /* Handshake state (scratch — only valid during handshake) */
    uint8_t        hs_ephemeral_priv[32]; /**< Ephemeral private key         */
    uint8_t        hs_chaining_key[32];   /**< Noise chaining key (CK)       */
    uint8_t        hs_hash[32];           /**< Noise handshake hash (H)      */

    /* Timers */
    uint32_t       last_sent_ms;     /**< Tick of last data sent             */
    uint32_t       last_recv_ms;     /**< Tick of last data received         */
    uint32_t       last_handshake_ms;/**< Tick of last handshake initiation  */

    /* Caller-owned I/O buffers */
    uint8_t       *rx_buf;           /**< Receive buffer                     */
    size_t         rx_buf_size;      /**< Receive buffer capacity            */
    uint8_t       *tx_buf;           /**< Transmit buffer                    */
    size_t         tx_buf_size;      /**< Transmit buffer capacity           */

    /**
     * @brief Callback for decrypted IP packets received from the tunnel.
     *
     * @param ip_packet  Raw IP packet data.
     * @param len        Packet length.
     * @param ctx        User context pointer.
     */
    void (*on_recv)(const uint8_t *ip_packet, size_t len, void *ctx);
    void           *user_ctx;        /**< User context for on_recv           */
} SYN_WG;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Initialize the WireGuard client.
 *
 * Derives the public key from the private key and prepares state.
 * Does NOT open a socket or start a handshake — that happens when
 * the protothread task runs.
 *
 * @param wg          Client context.
 * @param config      Peer configuration (copied).
 * @param sntp        SNTP time source (must be initialized, may not be synced yet).
 * @param rx_buf      Receive buffer (at least SYN_WG_MTU + SYN_WG_TRANSPORT_OVERHEAD).
 * @param rx_buf_size Receive buffer capacity.
 * @param tx_buf      Transmit buffer (same sizing).
 * @param tx_buf_size Transmit buffer capacity.
 */
void syn_wg_init(SYN_WG *wg, const SYN_WgConfig *config,
                 SYN_SNTP *sntp,
                 uint8_t *rx_buf, size_t rx_buf_size,
                 uint8_t *tx_buf, size_t tx_buf_size);

/**
 * @brief Send an IP packet through the WireGuard tunnel.
 *
 * Encrypts the packet with the current session key and sends it
 * as a WireGuard transport message.
 *
 * @param wg          Client context.
 * @param ip_packet   Raw IP packet to send.
 * @param len         Packet length.
 * @return SYN_OK on success, SYN_ERROR if no session or send failed.
 */
SYN_Status syn_wg_send(SYN_WG *wg, const uint8_t *ip_packet, size_t len);

/**
 * @brief Cooperative protothread task — drives the WireGuard client.
 *
 * Handles: NTP sync wait, handshake, incoming packet processing,
 * keepalive, and rekeying. Pass the SYN_WG context via task->user_data.
 *
 * @param pt   Protothread.
 * @param task Task descriptor.
 * @return PT status.
 */
SYN_PT_Status syn_wg_task(SYN_PT *pt, SYN_Task *task);

/**
 * @brief Check if the tunnel is established and ready for data.
 *
 * @param wg  Client context.
 * @return true if the session is active.
 */
static inline bool syn_wg_is_established(const SYN_WG *wg)
{
    return wg->state == SYN_WG_ESTABLISHED;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_WG */

#endif /* SYN_WG_H */
