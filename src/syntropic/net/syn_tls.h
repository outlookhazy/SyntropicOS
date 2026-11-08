/**
 * @file syn_tls.h
 * @brief Native Zero-Heap TLS 1.3 Protocol Engine & Transport Adapter (RFC 8446).
 *
 * Implements TLS 1.3 Client & Server handshakes and encrypted record layer.
 * Supported modes:
 * - Pre-Shared Key (PSK / PSK-DHE)
 * - Raw Public Key (RPK) - RFC 7250
 * - X.509 Server Authentication & Mutual TLS (mTLS)
 *
 * Plugs into standard SYN_Transport interface.
 * @ingroup syn_net
 */

#ifndef SYN_TLS_H
#define SYN_TLS_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/crypto/syn_hkdf.h"
#include "syntropic/crypto/syn_x509.h"

#if (!defined(SYN_USE_TLS) || SYN_USE_TLS) && SYN_HKDF_SHA512_ENABLED && SYN_X509_ENABLED && \
    (!defined(SYN_USE_AES) || SYN_USE_AES) &&                                                \
    (!defined(SYN_USE_CHACHA20POLY1305) || SYN_USE_CHACHA20POLY1305) &&                      \
    (!defined(SYN_USE_X25519) || SYN_USE_X25519) && (!defined(SYN_USE_PT) || SYN_USE_PT) &&  \
    (!defined(SYN_USE_SCHED) || SYN_USE_SCHED)
/** @brief Non-zero when TLS 1.3 and required crypto/scheduler dependencies are enabled. */
#define SYN_TLS_ENABLED 1
#else
/** @brief Non-zero when TLS 1.3 and required crypto/scheduler dependencies are enabled. */
#define SYN_TLS_ENABLED 0
#endif

#if SYN_TLS_ENABLED

#include "syntropic/crypto/syn_aes.h"
#include "syntropic/crypto/syn_chacha20poly1305.h"
#include "syntropic/crypto/syn_sha256.h"
#include "syntropic/crypto/syn_sha512.h"
#include "syntropic/crypto/syn_x25519.h"
#include "syntropic/net/syn_transport.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum TLS record payload size in bytes (2048). */
#define SYN_TLS_RECORD_MAX_PAYLOAD 2048U
/** @brief Length of TLS 1.3 secret keys in bytes (48 for SHA-384 / SHA-256 capacity). */
#define SYN_TLS_SECRET_LEN 48U

/** TLS 1.3 Handshake State Machine States */
typedef enum {
    SYN_TLS_STATE_UNINITIALIZED = 0,
    SYN_TLS_STATE_CLIENT_HELLO_SENT,
    SYN_TLS_STATE_SERVER_HELLO_RECEIVED,
    SYN_TLS_STATE_HANDSHAKE_KEYS_DERIVED,
    SYN_TLS_STATE_CERTIFICATE_VERIFIED,
    SYN_TLS_STATE_FINISHED_SENT,
    SYN_TLS_STATE_ESTABLISHED,
    SYN_TLS_STATE_ERROR
} SYN_TLS_State;

/** TLS 1.3 Authentication Mode */
typedef enum {
    SYN_TLS_AUTH_MODE_PSK = 0,
    SYN_TLS_AUTH_MODE_RAW_PUBKEY,
    SYN_TLS_AUTH_MODE_X509_SERVER,
    SYN_TLS_AUTH_MODE_MTLS
} SYN_TLS_AuthMode;

/** TLS 1.3 Cipher Suite */
typedef enum {
    SYN_TLS_CIPHER_SUITE_CHACHA20_POLY1305_SHA256 = 0, /**< TLS_CHACHA20_POLY1305_SHA256 (0x1303) */
    SYN_TLS_CIPHER_SUITE_AES_128_GCM_SHA256 = 1,       /**< TLS_AES_128_GCM_SHA256 (0x1301) */
    SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384 = 2,       /**< TLS_AES_256_GCM_SHA384 (0x1302) */
    SYN_TLS_CIPHER_SUITE_AES_128_CCM_SHA256 = 3,       /**< TLS_AES_128_CCM_SHA256 (0x1304) */
    SYN_TLS_CIPHER_SUITE_AES_128_CCM_8_SHA256 = 4      /**< TLS_AES_128_CCM_8_SHA256 (0x1305) */
} SYN_TLS_CipherSuite;

/** TLS 1.3 Engine Configuration */
typedef struct {
    SYN_TLS_AuthMode mode;            /**< PSK, RPK, X.509, or mTLS */
    SYN_TLS_CipherSuite cipher_suite; /**< Selected TLS 1.3 cipher suite */
    bool is_server;                   /**< true for server endpoint, false for client (default) */
    const char *server_name;          /**< SNI hostname (optional) */

    /* PSK configuration */
    const uint8_t *psk_identity; /**< PSK identity bytes */
    size_t psk_identity_len;     /**< PSK identity length */
    const uint8_t *psk_secret;   /**< 32-byte pre-shared secret */
    size_t psk_secret_len;       /**< Secret length */

    /* Raw Public Key configuration */
    const uint8_t *peer_pubkey; /**< 32-byte raw peer X25519/Ed25519 public key */
    size_t peer_pubkey_len;     /**< Raw peer public key length */

    /* X.509 & mTLS configuration */
    const SYN_X509_Cert *root_ca; /**< Trusted Root CA cert for peer validation */
    const uint8_t *peer_cert_der; /**< Peer X.509 certificate DER bytes */
    size_t peer_cert_len;         /**< Peer X.509 certificate DER length */
    const uint8_t
        *client_cert_der;          /**< Local certificate DER bytes (mTLS client or X.509 server) */
    size_t client_cert_len;        /**< Local certificate DER length */
    const uint8_t *client_privkey; /**< Local private key (32 bytes) */
} SYN_TLS_Config;

/** TLS 1.3 Engine Context (Caller-owned, zero-heap). */
typedef struct {
    SYN_TLS_State state;                 /**< Current handshake state */
    SYN_TLS_Config config;               /**< Engine configuration copy */
    SYN_Transport *underlying_transport; /**< Wire transport (TCP, UDP, Serial) */

    /* Caller-owned I/O buffers */
    uint8_t *rx_buf;    /**< Caller-owned RX record buffer */
    size_t rx_buf_size; /**< RX record buffer capacity */
    uint8_t *tx_buf;    /**< Caller-owned TX record buffer */
    size_t tx_buf_size; /**< TX record buffer capacity */

    uint8_t my_privkey[SYN_TLS_SECRET_LEN];  /**< Ephemeral X25519 private key */
    uint8_t my_pubkey[SYN_TLS_SECRET_LEN];   /**< Ephemeral X25519 public key */
    uint8_t peer_pubkey[SYN_TLS_SECRET_LEN]; /**< Peer ephemeral X25519 public key */

    SYN_SHA256 transcript_hash;   /**< Running SHA-256 handshake transcript hash */
    SYN_SHA512 transcript_sha384; /**< Running SHA-384 handshake transcript hash */

    uint8_t client_handshake_secret[SYN_TLS_SECRET_LEN]; /**< Client handshake traffic secret */
    uint8_t server_handshake_secret[SYN_TLS_SECRET_LEN]; /**< Server handshake traffic secret */
    uint8_t client_app_secret[SYN_TLS_SECRET_LEN];       /**< Client application traffic secret */
    uint8_t server_app_secret[SYN_TLS_SECRET_LEN];       /**< Server application traffic secret */
    uint8_t master_secret[SYN_TLS_SECRET_LEN];           /**< Master secret */

    uint8_t client_app_key[SYN_TLS_SECRET_LEN]; /**< Cached client AEAD key */
    uint8_t client_app_iv[12];                  /**< Cached client AEAD base IV */
    uint8_t server_app_key[SYN_TLS_SECRET_LEN]; /**< Cached server AEAD key */
    uint8_t server_app_iv[12];                  /**< Cached server AEAD base IV */

    uint64_t client_seq_num; /**< Encryption record sequence counter */
    uint64_t server_seq_num; /**< Decryption record sequence counter */

    size_t rx_pos;               /**< Bytes accumulated in rx_buf */
    size_t rx_record_len;        /**< Expected record payload length */
    uint8_t rx_content_type;     /**< Current record content type */
    uint32_t handshake_start_ms; /**< Handshake start timestamp */

    uint8_t app_rx_buf[SYN_TLS_RECORD_MAX_PAYLOAD]; /**< Decrypted app data ring buffer */
    size_t app_rx_head;                             /**< App data head index */
    size_t app_rx_tail;                             /**< App data tail index */

    /** @brief Transient cryptographic scratchpad keeping stack frames within 256B. */
    union {
        SYN_AES_GCM_Context gcm; /**< AES-GCM scratch context */
        SYN_AES_Context aes;     /**< AES-CCM scratch context */
        SYN_X509_Cert cert;      /**< Parsed peer X.509 certificate scratch */
    } crypto_scratch;            /**< Shared cipher/X.509 scratch storage */
} SYN_TLS_Context;

/**
 * @brief Initialize a TLS 1.3 context.
 *
 * @param ctx         Context to initialize.
 * @param config      Configuration.
 * @param transport   Wire transport instance.
 * @param rx_buf      Caller-owned RX record buffer.
 * @param rx_buf_size RX buffer capacity.
 * @param tx_buf      Caller-owned TX record buffer.
 * @param tx_buf_size TX buffer capacity.
 * @return true on success.
 */
bool syn_tls_init(SYN_TLS_Context *ctx, const SYN_TLS_Config *config, SYN_Transport *transport,
                  uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size);

/**
 * @brief Perform a non-blocking TLS 1.3 handshake step.
 *
 * @param ctx TLS 1.3 context.
 * @return true if handshake completed or progressing, false on fatal error.
 */
bool syn_tls_handshake(SYN_TLS_Context *ctx);

/**
 * @brief Cooperative protothread task — drives the TLS 1.3 client handshake & session.
 *
 * @param pt   Protothread handle.
 * @param task Task descriptor.
 * @return PT status.
 */
SYN_PT_Status syn_tls_task(SYN_PT *pt, SYN_Task *task);

/**
 * @brief Encrypt and transmit application data payload over TLS 1.3.
 *
 * @param ctx  TLS 1.3 context.
 * @param data Application data buffer.
 * @param len  Application data length.
 * @return true if record encrypted and sent successfully.
 */
bool syn_tls_send(SYN_TLS_Context *ctx, const uint8_t *data, size_t len);

/**
 * @brief Receive and decrypt TLS 1.3 record payload into caller buffer.
 *
 * @param ctx     TLS 1.3 context.
 * @param data    [out] Output buffer for decrypted data.
 * @param max_len Capacity of output buffer.
 * @param out_len [out] Actual decrypted bytes written.
 * @return true if a complete record decrypted.
 */
bool syn_tls_recv(SYN_TLS_Context *ctx, uint8_t *data, size_t max_len, size_t *out_len);

/**
 * @brief Wrap a TLS 1.3 context into a standard SYN_Transport abstraction.
 *
 * @param tls_ctx  Initialized TLS 1.3 context.
 * @param tr_out   [out] Output transport instance.
 */
void syn_tls_bind_transport(SYN_TLS_Context *tls_ctx, SYN_Transport *tr_out);

/**
 * @brief Check if the TLS 1.3 session is established and ready for app data.
 *
 * @param ctx TLS 1.3 context.
 * @return true if established.
 */
static inline bool syn_tls_is_established(const SYN_TLS_Context *ctx)
{
    return (ctx != NULL && ctx->state == SYN_TLS_STATE_ESTABLISHED);
}

/**
 * @brief Retrieve current TLS 1.3 state machine state.
 *
 * @param ctx TLS 1.3 context.
 * @return Current SYN_TLS_State.
 */
static inline SYN_TLS_State syn_tls_get_state(const SYN_TLS_Context *ctx)
{
    return ctx ? ctx->state : SYN_TLS_STATE_UNINITIALIZED;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_TLS_ENABLED */

#endif /* SYN_TLS_H */
