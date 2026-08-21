

# File syn\_dtls.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_dtls.h**](syn__dtls_8h.md)

[Go to the documentation of this file](syn__dtls_8h.md)


```C++

#ifndef SYN_DTLS_H
#define SYN_DTLS_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_DTLS) || SYN_USE_DTLS

#include "syntropic/crypto/syn_aes.h"
#include "syntropic/crypto/syn_chacha20poly1305.h"
#include "syntropic/crypto/syn_hkdf.h"
#include "syntropic/crypto/syn_sha256.h"
#include "syntropic/crypto/syn_sha512.h"
#include "syntropic/crypto/syn_x25519.h"
#include "syntropic/crypto/syn_x509.h"
#include "syntropic/net/syn_tls.h"
#include "syntropic/net/syn_transport.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_DTLS_RECORD_MAX_PAYLOAD 2048U
#define SYN_DTLS_SECRET_LEN 48U
#define SYN_DTLS_REPLAY_WINDOW_SIZE 64U

#define SYN_DTLS_UNIFIED_FIXED_BIT 0x20U  
#define SYN_DTLS_UNIFIED_CID_BIT 0x10U    
#define SYN_DTLS_UNIFIED_SEQ_16BIT 0x08U  
#define SYN_DTLS_UNIFIED_LEN_BIT 0x04U    
#define SYN_DTLS_UNIFIED_EPOCH_MASK 0x03U 
typedef enum {
    SYN_DTLS_EPOCH_PLAINTEXT = 0,  
    SYN_DTLS_EPOCH_EARLY_DATA = 1, 
    SYN_DTLS_EPOCH_HANDSHAKE = 2,  
    SYN_DTLS_EPOCH_APP_DATA = 3    
} SYN_DTLS_Epoch;

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

typedef enum {
    SYN_DTLS_AUTH_MODE_PSK = 0,
    SYN_DTLS_AUTH_MODE_RAW_PUBKEY,
    SYN_DTLS_AUTH_MODE_X509_SERVER,
    SYN_DTLS_AUTH_MODE_MTLS
} SYN_DTLS_AuthMode;

typedef enum {
    SYN_DTLS_CIPHER_SUITE_CHACHA20_POLY1305_SHA256 =
        0,                                         
    SYN_DTLS_CIPHER_SUITE_AES_128_GCM_SHA256 = 1,  
    SYN_DTLS_CIPHER_SUITE_AES_256_GCM_SHA384 = 2,  
    SYN_DTLS_CIPHER_SUITE_AES_128_CCM_SHA256 = 3,  
    SYN_DTLS_CIPHER_SUITE_AES_128_CCM_8_SHA256 = 4 
} SYN_DTLS_CipherSuite;

typedef struct {
    uint64_t bitmap;  
    uint64_t max_seq; 
    bool initialized; 
} SYN_DTLS_ReplayWindow;

typedef struct {
    SYN_DTLS_AuthMode mode;            
    SYN_DTLS_CipherSuite cipher_suite; 
    const char *server_name;           
    /* PSK configuration */
    const uint8_t *psk_identity; 
    size_t psk_identity_len;     
    const uint8_t *psk_secret;   
    size_t psk_secret_len;       
    /* Raw Public Key configuration */
    const uint8_t *peer_pubkey; 
    size_t peer_pubkey_len;     
    /* X.509 & mTLS configuration */
    const SYN_X509_Cert *root_ca;   
    const uint8_t *client_cert_der; 
    size_t client_cert_len;         
    const uint8_t *client_privkey;  
} SYN_DTLS_Config;

typedef struct {
    SYN_DTLS_State state;                
    SYN_DTLS_Config config;              
    SYN_Transport *underlying_transport; 
    /* Caller-owned I/O buffers */
    uint8_t *rx_buf;    
    size_t rx_buf_size; 
    uint8_t *tx_buf;    
    size_t tx_buf_size; 
    /* Protothread state */
    SYN_PT pt; 
    /* Cryptographic secrets & keys */
    uint8_t master_secret[SYN_DTLS_SECRET_LEN];     
    uint8_t client_app_secret[SYN_DTLS_SECRET_LEN]; 
    uint8_t server_app_secret[SYN_DTLS_SECRET_LEN]; 
    /* Cached record traffic keys & IVs */
    uint8_t client_app_key[SYN_DTLS_SECRET_LEN]; 
    uint8_t client_app_iv[12];                   
    uint8_t server_app_key[SYN_DTLS_SECRET_LEN]; 
    uint8_t server_app_iv[12];                   
    /* Epoch & Sequence Counters */
    uint64_t client_seq_num; 
    uint64_t server_seq_num; 
    SYN_DTLS_Epoch epoch;    
    /* Anti-Replay Protection */
    SYN_DTLS_ReplayWindow replay_window; 
    /* Ephemeral Key Exchange */
    uint8_t ecdhe_priv[32];     
    uint8_t ecdhe_pub[32];      
    uint8_t peer_ecdhe_pub[32]; 
    /* Single-record RX buffer */
    uint8_t app_rx_buf[SYN_DTLS_RECORD_MAX_PAYLOAD]; 
    size_t app_rx_len;                               
} SYN_DTLS_Context;

/* ── API ────────────────────────────────────────────────────────────────── */

bool syn_dtls_replay_check(const SYN_DTLS_ReplayWindow *win, uint64_t seq);

void syn_dtls_replay_update(SYN_DTLS_ReplayWindow *win, uint64_t seq);

bool syn_dtls_init(SYN_DTLS_Context *ctx, const SYN_DTLS_Config *config, SYN_Transport *transport,
                   uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size);

bool syn_dtls_handshake(SYN_DTLS_Context *ctx);

bool syn_dtls_send(SYN_DTLS_Context *ctx, const uint8_t *data, size_t len);

bool syn_dtls_recv(SYN_DTLS_Context *ctx, uint8_t *data, size_t max_len, size_t *out_len);

SYN_PT_Status syn_dtls_task(SYN_PT *pt, SYN_Task *task);

void syn_dtls_bind_transport(SYN_DTLS_Context *dtls_ctx, SYN_Transport *tr_out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_DTLS */

#endif /* SYN_DTLS_H */
```


