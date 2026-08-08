

# File syn\_tls.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_tls.h**](syn__tls_8h.md)

[Go to the documentation of this file](syn__tls_8h.md)


```C++

#ifndef SYN_TLS_H
#define SYN_TLS_H

#include "syntropic/crypto/syn_chacha20poly1305.h"
#include "syntropic/crypto/syn_hkdf.h"
#include "syntropic/crypto/syn_x25519.h"
#include "syntropic/crypto/syn_x509.h"
#include "syntropic/net/syn_transport.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_task.h"
#include "syntropic/util/syn_sha256.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_TLS_RECORD_MAX_PAYLOAD 2048U
#define SYN_TLS_SECRET_LEN 32U

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

typedef enum {
    SYN_TLS_AUTH_MODE_PSK = 0,
    SYN_TLS_AUTH_MODE_RAW_PUBKEY,
    SYN_TLS_AUTH_MODE_X509_SERVER,
    SYN_TLS_AUTH_MODE_MTLS
} SYN_TLS_AuthMode;

typedef struct {
    SYN_TLS_AuthMode mode;   
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
} SYN_TLS_Config;

typedef struct {
    SYN_TLS_State state;                 
    SYN_TLS_Config config;               
    SYN_Transport *underlying_transport; 
    /* Caller-owned I/O buffers */
    uint8_t *rx_buf;    
    size_t rx_buf_size; 
    uint8_t *tx_buf;    
    size_t tx_buf_size; 
    uint8_t my_privkey[SYN_TLS_SECRET_LEN];  
    uint8_t my_pubkey[SYN_TLS_SECRET_LEN];   
    uint8_t peer_pubkey[SYN_TLS_SECRET_LEN]; 
    SYN_SHA256 transcript_hash; 
    uint8_t client_handshake_secret[SYN_TLS_SECRET_LEN]; 
    uint8_t server_handshake_secret[SYN_TLS_SECRET_LEN]; 
    uint8_t client_app_secret[SYN_TLS_SECRET_LEN];       
    uint8_t server_app_secret[SYN_TLS_SECRET_LEN];       
    uint8_t master_secret[SYN_TLS_SECRET_LEN];           
    uint8_t client_app_key[SYN_TLS_SECRET_LEN]; 
    uint8_t client_app_iv[12];                  
    uint8_t server_app_key[SYN_TLS_SECRET_LEN]; 
    uint8_t server_app_iv[12];                  
    uint64_t client_seq_num; 
    uint64_t server_seq_num; 
    size_t rx_pos;               
    size_t rx_record_len;        
    uint8_t rx_content_type;     
    uint32_t handshake_start_ms; 
    uint8_t app_rx_buf[SYN_TLS_RECORD_MAX_PAYLOAD]; 
    size_t app_rx_head;                             
    size_t app_rx_tail;                             
} SYN_TLS_Context;

bool syn_tls_init(SYN_TLS_Context *ctx, const SYN_TLS_Config *config, SYN_Transport *transport,
                  uint8_t *rx_buf, size_t rx_buf_size, uint8_t *tx_buf, size_t tx_buf_size);

bool syn_tls_handshake(SYN_TLS_Context *ctx);

SYN_PT_Status syn_tls_task(SYN_PT *pt, SYN_Task *task);

bool syn_tls_send(SYN_TLS_Context *ctx, const uint8_t *data, size_t len);

bool syn_tls_recv(SYN_TLS_Context *ctx, uint8_t *data, size_t max_len, size_t *out_len);

void syn_tls_bind_transport(SYN_TLS_Context *tls_ctx, SYN_Transport *tr_out);

static inline bool syn_tls_is_established(const SYN_TLS_Context *ctx)
{
    return (ctx != NULL && ctx->state == SYN_TLS_STATE_ESTABLISHED);
}

static inline SYN_TLS_State syn_tls_get_state(const SYN_TLS_Context *ctx)
{
    return ctx ? ctx->state : SYN_TLS_STATE_UNINITIALIZED;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_TLS_H */
```


