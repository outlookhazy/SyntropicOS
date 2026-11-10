# Cryptography & PKI Invariants (`src/syntropic/crypto/`)

Applies to `syn_aes`, `syn_aes_cmac`, `syn_chacha20poly1305`, `syn_sha256`, `syn_sha512`, `syn_blake2s`, `syn_hmac`, `syn_hmac_drbg`, `syn_hkdf`, `syn_p256`, `syn_x25519`, `syn_ed25519`, `syn_asn1`, `syn_x509`, and `syn_tpm2`.

## Cross-Directory Touchpoints

- **Hardware Acceleration & Entropy**: `src/syntropic/port/syn_port_aes.h` (`SYN_USE_PORT_AES`) and `src/syntropic/util/syn_random.h`.
- **Consumers**: `src/syntropic/net/` (`syn_tls`, `syn_dtls`, `syn_wg`), `src/syntropic/proto/syn_cose.*`, and `src/syntropic/system/` (`syn_fwupdate`, `syn_ota`, `syn_fwboot`).

## Cryptographic Safety Invariants

- **Constant-Time Execution**: Scalar multiplication (`syn_p256`, `syn_x25519`, `syn_ed25519`), MAC/tag verification, and secret byte comparisons must execute with secret-independent control flow and memory indices. Never use `memcmp` or early-exit loops on tags, MACs, private scalars, or shared secrets.
- **Volatile-Safe Zeroization**: Scrub transient key schedules, nonces, ephemeral scalars, PRKs, and intermediate digests on **all** return paths (including early validation/auth failures and resumable `SYN_X25519_Ctx` completion in `syn_x25519_step` / `syn_x25519_pt`) via `volatile` byte writes (`secure_zero`) so dead-store elimination cannot strip cleanup.
- **Canonical Curve & Shared-Secret Validation**: Enforce RFC 8032 canonical scalar `S < L` and canonical point encoding in Ed25519; reject RFC 7748 all-zero low-order shared secrets in X25519 callers; validate P-256 public points lie on curve and are not point-at-infinity.
- **AEAD Length, Counter & Tag-Before-Plaintext Rules (`syn_aes`, `syn_chacha20poly1305`)**: Enforce 64-bit RFC input/AAD length limits and block-counter overflow bounds; zero output buffers or withhold plaintext commitment if authentication tag verification fails.
- **X.509 & ASN.1 Fail-Closed Parsing (`syn_asn1`, `syn_x509`)**: Validate TLV length bounds strictly within parent container slices without pointer wraparound; bind signature verification directly to parsed `cert->sig_algo` and reject unknown/mismatched algorithm OIDs.
- **Stack Cap & Standalone Feature Gates**: Bignum/ECC static stack frames must stay `<= 1920B` (`<= 1024B` elsewhere). Guard optional hash/curve dependencies (e.g., `SYN_USE_SHA512` in `syn_hkdf`) so modules build standalone when peer algorithms are disabled.
