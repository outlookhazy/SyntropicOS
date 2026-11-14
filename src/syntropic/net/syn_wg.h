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
 *   syn_task_create(&tasks[1], "wg", syn_wg_task, SYN_PRIO_HIGH, &wg);
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
#include "../crypto/syn_x25519.h"
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
#define SYN_WG_MTU 1420U
#endif

/** WireGuard transport overhead: type(4) + receiver(4) + counter(8) + tag(16) = 32 */
#define SYN_WG_TRANSPORT_OVERHEAD 32U

/** WireGuard protocol timers (seconds). */
#define SYN_WG_REKEY_AFTER_TIME 120U  /**< Initiate rekey after this many seconds */
#define SYN_WG_REJECT_AFTER_TIME 180U /**< Drop session after this many seconds   */
#define SYN_WG_REKEY_TIMEOUT 5U       /**< Retry handshake if no response (s)     */
#define SYN_WG_KEEPALIVE_TIMEOUT 10U  /**< Send keepalive if no outbound (s)      */

/** WireGuard message counter limits (whitepaper Section 6.2). */
#define SYN_WG_REKEY_AFTER_MESSAGES (1ULL << 60U)         /**< Initiate rekey at 2^60 messages */
#define SYN_WG_REJECT_AFTER_MESSAGES (UINT64_MAX - 16ULL) /**< Reject transport at 2^64 - 17 */

/** WireGuard message types. */
#define SYN_WG_MSG_INITIATION 1U /**< Handshake initiation message */
#define SYN_WG_MSG_RESPONSE 2U   /**< Handshake response message   */
#define SYN_WG_MSG_COOKIE 3U     /**< Cookie reply message        */
#define SYN_WG_MSG_TRANSPORT 4U  /**< Encrypted transport message */

/** Message sizes. */
#define SYN_WG_INITIATION_SIZE 148U /**< Size of initiation message */
#define SYN_WG_RESPONSE_SIZE 92U    /**< Size of response message   */

/* ── States ─────────────────────────────────────────────────────────────── */

/**
 * @brief WireGuard client connection state.
 */
typedef enum {
    SYN_WG_DISCONNECTED,   /**< No session, no handshake in progress   */
    SYN_WG_HANDSHAKE_INIT, /**< Sent initiation, waiting for response  */
    SYN_WG_ESTABLISHED,    /**< Session active, transport data flowing */
} SYN_WgState;

/* ── Configuration ──────────────────────────────────────────────────────── */

/**
 * @brief WireGuard peer configuration — set once at init.
 */
typedef struct {
    uint8_t private_key[32];       /**< Our Curve25519 private key      */
    uint8_t peer_public_key[32];   /**< Server's public key             */
    uint8_t preshared_key[32];     /**< Optional PSK (zero if unused)   */
    SYN_SockAddr endpoint;         /**< Server IP:port                  */
    uint16_t keepalive_interval_s; /**< Persistent keepalive (0=off)    */
} SYN_WgConfig;

/* ── Session keys ───────────────────────────────────────────────────────── */

/**
 * @brief Active session derived from a completed handshake.
 */
typedef struct {
    uint8_t send_key[32];    /**< ChaCha20-Poly1305 key for outgoing      */
    uint8_t recv_key[32];    /**< ChaCha20-Poly1305 key for incoming      */
    uint64_t send_counter;   /**< Outgoing nonce counter                   */
    uint64_t recv_counter;   /**< Highest received nonce                   */
    uint64_t recv_bitmap;    /**< Anti-replay sliding window (64 bits)     */
    uint32_t sender_index;   /**< Our sender index (in transport headers)  */
    uint32_t receiver_index; /**< Peer's sender index                      */
    uint32_t established_ms; /**< Tick when session was established        */
} SYN_WgSession;

/**
 * @brief Staged handshake crypto scratchpad for cooperative X25519 yielding.
 */
typedef struct {
    SYN_X25519_Ctx x25519;  /**< Active cooperative X25519 scalar-mul context */
    uint32_t expected_idx;  /**< Initiator sender index (staged or expected)  */
    uint32_t peer_idx;      /**< Responder sender index from response         */
    SYN_PT hs_pt;           /**< Active handshake protothread continuation    */
    SYN_PT crypto_pt;       /**< Child protothread for syn_x25519_pt          */
    uint8_t eph_priv[32];   /**< Staged ephemeral private key                 */
    uint8_t eph_pub[32];    /**< Staged local or peer ephemeral public key    */
    uint8_t ck[32];         /**< Staged Noise chaining key                    */
    uint8_t h[32];          /**< Staged Noise handshake hash                  */
    uint8_t tmp_key[32];    /**< Staged DH shared secret or AEAD key          */
    uint8_t enc_static[48]; /**< Encrypted static key (48B) or resp tag (16B) */
    uint8_t enc_ts[28];     /**< Encrypted TAI64N timestamp + AEAD tag        */
    uint8_t dh_ok;          /**< Constant-time non-zero DH accumulator        */
    bool active;            /**< True while handshake protothread is running  */
    bool is_resp;           /**< True for response consumption, false init    */
    bool is_rekey;          /**< True when staged initiation is for rekey     */
    bool ok;                /**< True when handshake PT completed cleanly     */
} SYN_WgHsWork;

/* ── Client context ─────────────────────────────────────────────────────── */

/**
 * @brief WireGuard client context — caller-owned.
 *
 * Contains config, session state, handshake scratch, timers, and
 * buffer pointers. Approximately ~980 bytes + buffer pointers.
 */
typedef struct {
    SYN_WgConfig config; /**< Peer configuration                  */
    SYN_WgState state;   /**< Connection state                    */
    SYN_Socket udp_sock; /**< UDP socket to the server            */

    /* Time source */
    SYN_SNTP *sntp; /**< NTP time source (for TAI64N)        */

    /* Derived keys (computed once at init from config) */
    uint8_t public_key[32]; /**< Our public key (from private)       */

    /* Active session */
    SYN_WgSession session; /**< Currently active transport session */

    /* Handshake state (scratch — only valid during handshake) */
    uint8_t hs_ephemeral_priv[32]; /**< Ephemeral private key                        */
    uint8_t hs_chaining_key[32];   /**< Noise chaining key (CK)                      */
    uint8_t hs_hash[32];           /**< Noise handshake hash (H)                     */
    SYN_WgHsWork hs_work;          /**< Cooperative per-X25519 stage scratchpad      */
    uint32_t hs_sender_index;      /**< Pending handshake initiator sender index     */
    uint8_t rx_turn;               /**< Fair RX/rekey alternation toggle             */
    bool hs_valid;                 /**< True when hs_* holds an in-flight initiation */
    bool rekey_in_progress;        /**< True while negotiating hitless rekey session */

    /* Cumulative tunnel payload counters */
    uint64_t tx_bytes; /**< Total encrypted tunnel bytes transmitted     */
    uint64_t rx_bytes; /**< Total decrypted tunnel bytes received        */

    /* Timers */
    uint32_t last_sent_ms;      /**< Tick of last data sent             */
    uint32_t last_recv_ms;      /**< Tick of last data received         */
    uint32_t last_handshake_ms; /**< Tick of last handshake initiation  */

    /* Caller-owned I/O buffers */
    uint8_t *rx_buf;    /**< Receive buffer                     */
    size_t rx_buf_size; /**< Receive buffer capacity            */
    uint8_t *tx_buf;    /**< Transmit buffer                    */
    size_t tx_buf_size; /**< Transmit buffer capacity           */

    /**
     * @brief Callback for decrypted IP packets received from the tunnel.
     *
     * @param ip_packet  Raw IP packet data.
     * @param len        Packet length.
     * @param ctx        User context pointer.
     */
    void (*on_recv)(const uint8_t *ip_packet, size_t len, void *ctx);
    void *user_ctx; /**< User context for on_recv           */
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
void syn_wg_init(SYN_WG *wg, const SYN_WgConfig *config, SYN_SNTP *sntp, uint8_t *rx_buf,
                 size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size);

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
    return (wg != NULL) && (wg->state == SYN_WG_ESTABLISHED);
}

/** @brief WireGuard tunnel statistics structure. */
typedef struct {
    uint64_t tx_bytes;          /**< Total encrypted bytes transmitted */
    uint64_t rx_bytes;          /**< Total encrypted bytes received */
    uint32_t handshake_age_sec; /**< Seconds since last successful handshake */
    bool is_established;        /**< Tunnel connection status */
} SYN_WgStats;

/**
 * @brief Disconnect the WireGuard tunnel and close socket.
 * @param wg Client context.
 */
void syn_wg_disconnect(SYN_WG *wg);

/**
 * @brief Retrieve WireGuard tunnel metrics and statistics.
 * @param wg    Client context.
 * @param stats [out] Output statistics structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_wg_get_stats(const SYN_WG *wg, SYN_WgStats *stats);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_WG */

#endif /* SYN_WG_H */
