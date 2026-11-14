# Cryptography & Security Modules

SyntropicOS provides pure C99, constant-time cryptographic primitives and AEAD ciphers designed for resource-constrained microcontrollers.

---

## Technical Specifications

| Feature | Specification |
|---|---|
| **Constant-Time** | Branchless arithmetic operations to prevent side-channel timing attacks. |
| **Memory Allocation** | **100% Zero-Heap**. All operations use caller-owned stacks or buffers. |
| **Standards Compliance** | BLAKE2s (RFC 7693), ChaCha20-Poly1305 AEAD (RFC 8439), X25519 (RFC 7748). |

---

## 1. BLAKE2s Cryptographic Hash (`crypto/syn_blake2s.h`)

BLAKE2s-256 provides high-speed cryptographic hashing and keyed-MAC authentication (HMAC replacement without HMAC overhead).

```c
#include <syntropic/crypto/syn_blake2s.h>

void hash_payload(const uint8_t *msg, size_t len, uint8_t digest[32]) {
    // One-shot BLAKE2s-256 hash
    syn_blake2s(msg, len, digest, 32);
}

void compute_mac(const uint8_t *key, size_t key_len, const uint8_t *msg, size_t msg_len, uint8_t tag[16]) {
    SYN_BLAKE2s ctx;
    syn_blake2s_init_keyed(&ctx, key, key_len, 16);
    syn_blake2s_update(&ctx, msg, msg_len);
    syn_blake2s_final(&ctx, tag);
}
```

---

## 2. ChaCha20-Poly1305 AEAD Cipher (`crypto/syn_chacha20poly1305.h`)

Authenticated Encryption with Associated Data (AEAD) per RFC 8439.

```c
#include <syntropic/crypto/syn_chacha20poly1305.h>

void encrypt_sensor_telemetry(void) {
    uint8_t key[32] = { /* 256-bit symmetric key */ };
    uint8_t nonce[12] = { /* 96-bit unique nonce */ };
    
    uint8_t payload[64] = "Temperature=24.5C;Humidity=60%";
    uint8_t ciphertext[64];
    uint8_t auth_tag[16];

    // Encrypt and authenticate payload
    syn_aead_encrypt(key, nonce,
                     NULL, 0, // Additional Authenticated Data (AAD)
                     payload, sizeof(payload),
                     ciphertext, auth_tag);
}
```


---

## 3. X25519 ECDH Key Exchange (`crypto/syn_x25519.h`)

Elliptic Curve Diffie-Hellman (Curve25519 / RFC 7748) key exchange for secure peer-to-peer session key negotiation.

```c
#include <syntropic/crypto/syn_x25519.h>

void ecdh_key_exchange(const uint8_t peer_public[32], uint8_t my_public[32],
                       uint8_t shared_secret[32]) {
    uint8_t my_private[32] = { /* 32 CSPRNG bytes */ };

    // Clamp private scalar per RFC 7748 and derive public key: P = k * G
    syn_x25519_clamp(my_private);
    syn_x25519_pubkey(my_public, my_private);

    // Derive shared secret: S = k * Peer_P
    syn_x25519(shared_secret, my_private, peer_public);
}
```

---

## 4. SHA-256 Hash Engine (`crypto/syn_sha256.h`)

Standard NIST FIPS 180-4 SHA-256 digest computation for bootloader firmware verification and packet checksums.

```c
#include <syntropic/crypto/syn_sha256.h>

void sha256_demo(const uint8_t *data, size_t len, uint8_t hash[32]) {
    syn_sha256(data, len, hash);
}
```

---

## 5. NIST P-256 (secp256r1) Elliptic Curve Engine (`crypto/syn_p256.h`)

Full cleanroom implementation of NIST P-256 ECC arithmetic in Jacobian projective coordinates featuring:
- **Solinas Fast Modular Reduction** ($p = 2^{256} - 2^{224} + 2^{192} + 2^{96} - 1$ per FIPS 186-4 §D.2.1).
- **Constant-Time Scalar Multiplication** (`point_cmov` 4-bit fixed windowing) with zero side-channel branching.
- **ECDH Key Agreement** (RFC 5903).
- **ECDSA Sign & Verify** (RFC 6979 / ANSI X9.62).

```c
#include <syntropic/crypto/syn_p256.h>

void p256_ecdh_and_ecdsa_demo(void) {
    uint8_t priv_key[32] = { /* 256-bit scalar */ };
    uint8_t pub_x[32], pub_y[32];

    // Derive public key: Q = d * G
    syn_p256_base_mul(priv_key, pub_x, pub_y);

    // Compute shared ECDH secret: S = d * Peer_Q
    uint8_t peer_x[32], peer_y[32], shared_secret[32];
    syn_p256_ecdh(priv_key, peer_x, peer_y, shared_secret);

    // Sign SHA-256 digest with ECDSA
    uint8_t hash[32], nonce_k[32], r[32], s[32];
    syn_p256_sign_ecdsa(priv_key, nonce_k, hash, r, s);

    // Verify ECDSA signature
    bool valid = syn_p256_verify_ecdsa(hash, r, s, pub_x, pub_y);
}
```

---

## 6. TCG TPM 2.0 Command Marshaller & Measured Boot Engine (`crypto/syn_tpm2.h`)

Complete cleanroom TCG TPM 2.0 serializer and response parser for hardware security chips (SPI / I2C TPM):
- **Core Operations**: `TPM2_Startup`, `TPM2_SelfTest`, `TPM2_GetRandom`.
- **Platform Configuration Registers (PCRs)**: `TPM2_PCR_Read` and `TPM2_PCR_Extend` (SHA-256, SHA-384) for measured boot integrity chains.
- **Remote Attestation**: `TPM2_Quote` over selected PCR banks with nonce verification.
- **Hardware NVRAM**: Authenticated `TPM2_NV_Read` and `TPM2_NV_Write`.

```c
#include <syntropic/crypto/syn_tpm2.h>

static SYN_TPM2_Context g_tpm;
static uint8_t g_tpm_rx[512], g_tpm_tx[512];

void tpm2_setup(SYN_Transport *tpm_spi_transport) {
    SYN_TPM2_Config cfg = {
        .transport = tpm_spi_transport,
        .rx_buf = g_tpm_rx,
        .rx_buf_size = sizeof(g_tpm_rx),
        .tx_buf = g_tpm_tx,
        .tx_buf_size = sizeof(g_tpm_tx),
    };
    syn_tpm2_init(&g_tpm, &cfg);
    syn_tpm2_startup(&g_tpm, SYN_TPM2_SU_CLEAR);
}

void measure_stage2_firmware(const uint8_t *fw_sha256) {
    /* Extend PCR 0 with bootloader / kernel measurement */
    syn_tpm2_pcr_extend(&g_tpm, 0U, SYN_TPM2_ALG_SHA256, fw_sha256, 32U);
}
```

---

## 7. AES Block/AEAD Modes, SHA-512, HMAC/HKDF, DRBG, Ed25519 & X.509 PKI

| Module | Guard | Primary API | Summary |
|---|---|---|---|
| `crypto/syn_aes.h` | `SYN_USE_AES` | `syn_aes_init`, `syn_aes_cbc_encrypt/decrypt`, `syn_aes_ctr`, `syn_aes_gcm_encrypt/decrypt`, `syn_aes_ccm_encrypt/decrypt` | Constant-time AES-128/192/256 ECB, PKCS#7 CBC, CTR, AES-GCM AEAD, and AES-CCM AEAD via bitsliced Boyar–Peralta circuit on non-AVR (AVR retains single-cycle PROGMEM tables); optional `SYN_USE_PORT_AES` hardware offload |
| `crypto/syn_aes_cmac.h` | `SYN_USE_AES_CMAC` | `syn_aes_cmac`, `syn_aes_cmac_verify` | Constant-time 128-bit AES-CMAC (RFC 4493 / NIST SP 800-38B) |
| `crypto/syn_hmac.h` | `SYN_USE_SHA256` | `syn_hmac_sha256_init/update/final`, `syn_hmac_sha256` | Streaming and single-shot HMAC-SHA256 with stack state zeroization |
| `crypto/syn_sha512.h` | `SYN_USE_SHA512` | `syn_sha512`, `syn_sha384`, `syn_hmac_sha512`, `syn_hmac_sha384` | FIPS 180-4 SHA-512 / SHA-384 digests and RFC 4231 HMACs |
| `crypto/syn_hkdf.h` | `SYN_USE_HKDF` | `syn_hkdf_extract`, `syn_hkdf_expand`, `syn_hkdf`, `syn_hkdf_expand_label` | RFC 5869 HKDF and RFC 8446 TLS 1.3 `HKDF-Expand-Label` (SHA-256/384/512) |
| `crypto/syn_hmac_drbg.h` | `SYN_USE_HMAC_DRBG` | `syn_hmac_drbg_init`, `syn_hmac_drbg_reseed`, `syn_hmac_drbg_generate` | NIST SP 800-90A Rev. 1 HMAC-DRBG (SHA-256) CSPRNG |
| `crypto/syn_ed25519.h` | `SYN_USE_ED25519` | `syn_ed25519_create_keypair`, `syn_ed25519_sign`, `syn_ed25519_verify` | RFC 8032 Ed25519 deterministic digital signatures |
| `crypto/syn_asn1.h` | `SYN_USE_ASN1` | `syn_asn1_parse_element`, `syn_asn1_step`, `syn_asn1_enter_container`, `syn_asn1_match_oid` | Zero-copy ASN.1 DER TLV container, OID, integer, and bit-string parser |
| `crypto/syn_x509.h` | `SYN_USE_X509` | `syn_x509_parse`, `syn_x509_verify_signature`, `syn_x509_validate_chain` | DER X.509 v3 certificate parser and Ed25519 / ECDSA-P256 chain validator |

---

## 8. Cooperative Protothread Crypto API (`*_init` / `*_pt` / `*_wipe`)

Asymmetric scalar multiplication, signing, and certificate verification expose caller-owned, zero-heap contexts that run as `SYN_PT` child protothreads. Each slice executes bounded field, ladder, or hash steps and yields `PT_MORE` until returning `PT_ENDED`.

| Module | Context | Initializers | Child Protothreads | Cancellation Wipe |
|---|---|---|---|---|
| `crypto/syn_x25519.h` | `SYN_X25519_Ctx` | `syn_x25519_init`, `syn_x25519_pubkey_init` | `syn_x25519_pt` | `syn_x25519_wipe` |
| `crypto/syn_p256.h` | `SYN_P256_Ctx` | `syn_p256_base_mul_init`, `syn_p256_point_mul_init`, `syn_p256_sign_init`, `syn_p256_sign_deterministic_init`, `syn_p256_verify_init` | `syn_p256_mul_pt`, `syn_p256_sign_pt`, `syn_p256_verify_pt` | `syn_p256_wipe` |
| `crypto/syn_ed25519.h` | `SYN_Ed25519_Ctx` | `syn_ed25519_publickey_init`, `syn_ed25519_sign_init`, `syn_ed25519_verify_init`, `syn_ed25519_verify_hash_init` | `syn_ed25519_publickey_pt`, `syn_ed25519_sign_pt`, `syn_ed25519_verify_pt` | `syn_ed25519_wipe` |
| `crypto/syn_x509.h` | `SYN_X509_VerifyCtx` | `syn_x509_verify_signature_init`, `syn_x509_validate_chain_init` | `syn_x509_verify_pt` | `syn_x509_verify_wipe` |

Compose child protothreads inside scheduler tasks with `PT_SPAWN_IF_NEEDED` (`sched/syn_sched.h`). Uncontended dispatches run consecutive `PT_MORE` slices in one turn; contended dispatches (`syn_task_should_share(task)`) yield `PT_MORE` between slices:

```c
#include <syntropic/crypto/syn_x25519.h>
#include <syntropic/sched/syn_sched.h>

typedef struct {
    SYN_PT child_pt;
    SYN_X25519_Ctx x25519;
    uint8_t priv[32];
    uint8_t peer_pub[32];
    uint8_t shared[32];
} KeyExState;

SYN_PT_Status key_exchange_task(SYN_PT *pt, SYN_Task *task)
{
    KeyExState *st = (KeyExState *)task->user_data;

    PT_BEGIN(pt);

    if (!syn_x25519_init(&st->x25519, st->priv, st->peer_pub)) {
        PT_EXIT(pt);
    }
    PT_SPAWN_IF_NEEDED(pt, task, &st->child_pt,
                       syn_x25519_pt(&st->child_pt, &st->x25519, st->shared));

    PT_END(pt);
}
```

**Zeroization & Cancellation Cleanup**: Every `*_pt` function scrubs its working context via `volatile` `syn_secure_zero` before returning `PT_ENDED`. If a caller aborts, times out, or resets a task before `PT_ENDED`, call the matching `*_wipe` function (`syn_x25519_wipe`, `syn_p256_wipe`, `syn_ed25519_wipe`, `syn_x509_verify_wipe`) so `syn_secure_zero` clears clamped scalars, nonces, and intermediate field state.

