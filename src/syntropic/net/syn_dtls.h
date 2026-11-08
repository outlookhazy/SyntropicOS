/**
 * @file syn_dtls.h
 * @brief Native Zero-Heap DTLS 1.3 Datagram Protocol Engine (RFC 9147).
 *
 * Implements Datagram Transport Layer Security (DTLS) version 1.3:
 * - Unified record header format (RFC 9147 Section 4)
 * - 64-packet sliding window anti-replay protection (RFC 9147 Section 4.5.2)
 * - Epoch-based key scheduling & AEAD record encryption/decryption
 * - Pre-Shared Key (PSK), Raw Public Key (RPK), and X.509 modes
 * - Full pluggability into SYN_Transport (UDP, wireless, serial, etc.)
 *
 * @ingroup syn_net
 */

#ifndef SYN_DTLS_H
#define SYN_DTLS_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_tls.h"

#if (!defined(SYN_USE_DTLS) || SYN_USE_DTLS) && SYN_TLS_ENABLED
/** @brief Non-zero when DTLS 1.3 and required TLS/crypto dependencies are enabled. */
#define SYN_DTLS_ENABLED 1
#else
/** @brief Non-zero when DTLS 1.3 and required TLS/crypto dependencies are enabled. */
#define SYN_DTLS_ENABLED 0
#endif

#if SYN_DTLS_ENABLED

#include "syntropic/crypto/syn_aes.h"
#include "syntropic/crypto/syn_chacha20poly1305.h"
#include "syntropic/crypto/syn_hkdf.h"
#include "syntropic/crypto/syn_sha256.h"
#include "syntropic/crypto/syn_sha512.h"
#include "syntropic/crypto/syn_x25519.h"
#include "syntropic/crypto/syn_x509.h"
#include "syntropic/net/syn_transport.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum DTLS datagram record payload size in bytes (2048). */
#define SYN_DTLS_RECORD_MAX_PAYLOAD 2048U
/** @brief Length of DTLS 1.3 secret keys in bytes (48 for SHA-384 / SHA-256 capacity). */
#define SYN_DTLS_SECRET_LEN 48U
/** @brief Size of anti-replay sliding window in packets (64). */
#define SYN_DTLS_REPLAY_WINDOW_SIZE 64U

/** DTLS 1.3 Unified Header Flags */
#define SYN_DTLS_UNIFIED_FIXED_BIT 0x20U  /**< Bit 5: Must be 1 for DTLS 1.3 unified record */
#define SYN_DTLS_UNIFIED_CID_BIT 0x10U    /**< Bit 4: Connection ID present */
#define SYN_DTLS_UNIFIED_SEQ_16BIT 0x08U  /**< Bit 3: 16-bit sequence number (0 = 8-bit) */
#define SYN_DTLS_UNIFIED_LEN_BIT 0x04U    /**< Bit 2: Length field present */
#define SYN_DTLS_UNIFIED_EPOCH_MASK 0x03U /**< Bits 1..0: Epoch (0..3) */

/** DTLS 1.3 Epochs */
typedef enum {
    SYN_DTLS_EPOCH_PLAINTEXT = 0,  /**< Epoch 0: Unencrypted Initial/Alert */
    SYN_DTLS_EPOCH_EARLY_DATA = 1, /**< Epoch 1: 0-RTT Application Data */
    SYN_DTLS_EPOCH_HANDSHAKE = 2,  /**< Epoch 2: Encrypted Handshake */
    SYN_DTLS_EPOCH_APP_DATA = 3    /**< Epoch 3: 1-RTT Application Data */
} SYN_DTLS_Epoch;

/** DTLS 1.3 Handshake State */
typedef enum {
    SYN_DTLS_STATE_UNINITIALIZED = 0,
    SYN_DTLS_STATE_CLIENT_HELLO_SENT,
    SYN_DTLS_STATE_SERVER_HELLO_RECEIVED,
    SYN_DTLS_STATE_HANDSHAKE_KEYS_DERIVED,
    SYN_DTLS_STATE_CERTIFICATE_VERIFIED,
    SYN_DTLS_STATE_FINISHED_SENT,
    SYN_DTLS_STATE_ESTABLISHED,
    SYN_DTLS_STATE_ERROR
} SYN_DTLS_State;

/** DTLS 1.3 Authentication Mode */
typedef enum {
    SYN_DTLS_AUTH_MODE_PSK = 0,
    SYN_DTLS_AUTH_MODE_RAW_PUBKEY,
    SYN_DTLS_AUTH_MODE_X509_SERVER,
    SYN_DTLS_AUTH_MODE_MTLS
} SYN_DTLS_AuthMode;

/** DTLS 1.3 Cipher Suite */
typedef enum {
    SYN_DTLS_CIPHER_SUITE_CHACHA20_POLY1305_SHA256 =
        0,                                         /**< TLS_CHACHA20_POLY1305_SHA256 (0x1303) */
    SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256 = 1,  /**< TLS_AES_128_GCM_SHA256 (0x1301) */
    SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384 = 2,  /**< TLS_AES_256_GCM_SHA384 (0x1302) */
    SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256 = 3,  /**< TLS_AES_128_CCM_SHA256 (0x1304) */
    SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256 = 4 /**< TLS_AES_128_CCM_8_SHA256 (0x1305) */
} SYN_DTLS_CipherSuite;

/**
 * @brief 64-packet Sliding Window Anti-Replay Filter (RFC 9147 Section 4.5.2).
 */
typedef struct {
    uint64_t bitmap;  /**< 64-bit window bitmask */
    uint64_t max_seq; /**< Highest verified sequence number received */
    bool initialized; /**< True once the first valid packet is processed */
} SYN_DTLS_ReplayWindow;

/** DTLS 1.3 Engine Configuration */
typedef struct {
    SYN_DTLS_AuthMode mode;            /**< PSK, RPK, X.509, or mTLS */
    SYN_DTLS_CipherSuite cipher_suite; /**< Selected cipher suite */
    bool is_server;                    /**< true for server endpoint, false for client (default) */
    const char *server_name;           /**< SNI hostname (optional) */

    /* PSK configuration */
    const uint8_t *psk_identity; /**< PSK identity bytes */
    size_t psk_identity_len;     /**< PSK identity length */
    const uint8_t *psk_secret;   /**< Pre-shared secret bytes */
    size_t psk_secret_len;       /**< Secret length */

    /* Raw Public Key configuration */
    const uint8_t *peer_pubkey; /**< 32-byte raw peer public key */
    size_t peer_pubkey_len;     /**< Peer public key length */

    /* X.509 & mTLS configuration */
    const SYN_X509_Cert *root_ca;   /**< Trusted Root CA cert */
    const uint8_t *peer_cert_der;   /**< Peer certificate DER */
    size_t peer_cert_len;           /**< Peer certificate length */
    const uint8_t *client_cert_der; /**< Local certificate DER (mTLS client or X.509 server) */
    size_t client_cert_len;         /**< Local certificate length */
    const uint8_t *client_privkey;  /**< Local private key (32 bytes) */
} SYN_DTLS_Config;

/** DTLS 1.3 Engine Context (Caller-owned, zero-heap). */
typedef struct {
    SYN_DTLS_State state;                /**< Current handshake state */
    SYN_DTLS_Config config;              /**< Engine configuration copy */
    SYN_Transport *underlying_transport; /**< Wire transport (UDP, Serial, etc.) */

    /* Caller-owned I/O buffers */
    uint8_t *rx_buf;    /**< Caller-owned RX record buffer */
    size_t rx_buf_size; /**< RX buffer capacity */
    uint8_t *tx_buf;    /**< Caller-owned TX record buffer */
    size_t tx_buf_size; /**< TX buffer capacity */

    /* Protothread state */
    SYN_PT pt; /**< Non-blocking task state */

    /* Cryptographic secrets & keys */
    uint8_t client_handshake_secret[SYN_DTLS_SECRET_LEN]; /**< Client handshake secret */
    uint8_t server_handshake_secret[SYN_DTLS_SECRET_LEN]; /**< Server handshake secret */
    uint8_t master_secret[SYN_DTLS_SECRET_LEN];           /**< Derived DTLS 1.3 master secret */
    uint8_t client_app_secret[SYN_DTLS_SECRET_LEN];       /**< Client application traffic secret */
    uint8_t server_app_secret[SYN_DTLS_SECRET_LEN];       /**< Server application traffic secret */
    uint8_t transcript_digest[SYN_DTLS_SECRET_LEN];       /**< Chained handshake transcript hash */

    /* Cached record traffic keys & IVs */
    uint8_t client_app_key[SYN_DTLS_SECRET_LEN]; /**< Client record encryption key */
    uint8_t client_app_iv[12];                   /**< Client record base IV */
    uint8_t server_app_key[SYN_DTLS_SECRET_LEN]; /**< Server record encryption key */
    uint8_t server_app_iv[12];                   /**< Server record base IV */

    /* Epoch & Sequence Counters */
    uint64_t client_seq_num; /**< Outgoing 48-bit record sequence number */
    uint64_t server_seq_num; /**< Expected incoming sequence number */
    SYN_DTLS_Epoch epoch;    /**< Active DTLS record epoch */

    /* Anti-Replay Protection */
    SYN_DTLS_ReplayWindow replay_window; /**< Sliding window for incoming datagrams */

    /* Ephemeral Key Exchange */
    uint8_t ecdhe_priv[32];     /**< Local ephemeral ECDHE private key */
    uint8_t ecdhe_pub[32];      /**< Local ephemeral ECDHE public key */
    uint8_t peer_ecdhe_pub[32]; /**< Peer ephemeral ECDHE public key */
    uint8_t client_random[32];  /**< ClientHello 32-byte random */
    uint8_t server_random[32];  /**< ServerHello 32-byte random */

    /** @brief Transient cryptographic scratchpad keeping stack frames within 256B. */
    union {
        SYN_AES_GCM_Context gcm; /**< AES-GCM scratch context */
        SYN_AES_Context aes;     /**< AES-CCM scratch context */
        SYN_SHA512 sha512;       /**< SHA-384 transcript scratch */
        SYN_SHA256 sha256;       /**< SHA-256 transcript scratch */
        SYN_X509_Cert cert;      /**< Parsed peer X.509 certificate scratch */
    } crypto_scratch;            /**< Shared cipher/hash/X.509 scratch storage */
} SYN_DTLS_Context;

/* ── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief Check whether a sequence number is acceptable by the replay window.
 * @param win Pointer to replay window.
 * @param seq 64-bit reconstructed packet sequence number.
 * @return true if packet is not a duplicate and is within or ahead of window.
 */
bool syn_dtls_replay_check(const SYN_DTLS_ReplayWindow *win, uint64_t seq);

/**
 * @brief Commit a validated sequence number to the replay window.
 * @param win Pointer to replay window.
 * @param seq 64-bit packet sequence number.
 */
void syn_dtls_replay_update(SYN_DTLS_ReplayWindow *win, uint64_t seq);

/**
 * @brief Initialize DTLS 1.3 engine context with caller-allocated memory.
 *
 * @param ctx         Context to initialize.
 * @param config      Engine configuration.
 * @param transport   Underlying transport (UDP, Socket, Serial).
 * @param rx_buf      Caller-allocated buffer for incoming datagrams.
 * @param rx_buf_size Size of rx_buf in bytes.
 * @param tx_buf      Caller-allocated buffer for outgoing datagrams.
 * @param tx_buf_size Size of tx_buf in bytes.
 * @return true on success, false on invalid parameters.
 */
bool syn_dtls_init(SYN_DTLS_Context *ctx, const SYN_DTLS_Config *config, SYN_Transport *transport,
                   uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size);

/**
 * @brief Execute DTLS 1.3 handshake.
 * @param ctx Initialized DTLS context.
 * @return true on successful handshake completion.
 */
bool syn_dtls_handshake(SYN_DTLS_Context *ctx);

/**
 * @brief Send application data protected by DTLS 1.3 AEAD datagram record.
 * @param ctx  Established DTLS context.
 * @param data Application data buffer.
 * @param len  Length in bytes.
 * @return true if encrypted record was sent.
 */
bool syn_dtls_send(SYN_DTLS_Context *ctx, const uint8_t *data, size_t len);

/**
 * @brief Receive and decrypt a DTLS 1.3 datagram record.
 * @param ctx      Established DTLS context.
 * @param data     Output buffer for decrypted application data.
 * @param max_len  Capacity of output buffer.
 * @param out_len  [out] Actual decrypted payload length.
 * @return true if valid uncorrupted, non-replayed record received.
 */
bool syn_dtls_recv(SYN_DTLS_Context *ctx, uint8_t *data, size_t max_len, size_t *out_len);

/**
 * @brief Non-blocking DTLS background task (Protothread).
 * @param pt   Pointer to task protothread.
 * @param task Pointer to scheduler task context.
 * @return SYN_PT_Status.
 */
SYN_PT_Status syn_dtls_task(SYN_PT *pt, SYN_Task *task);

/**
 * @brief Bind DTLS 1.3 engine to abstract SYN_Transport interface.
 * @param dtls_ctx Initialized DTLS context.
 * @param tr_out   [out] Transport instance to populate.
 */
void syn_dtls_bind_transport(SYN_DTLS_Context *dtls_ctx, SYN_Transport *tr_out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DTLS_ENABLED */

#endif /* SYN_DTLS_H */
