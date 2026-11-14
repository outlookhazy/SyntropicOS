# Cryptography & PKI Invariants (`src/syntropic/crypto/`)

Applies to `syn_aes`, `syn_aes_cmac`, `syn_chacha20poly1305`, `syn_sha256`, `syn_sha512`, `syn_blake2s`, `syn_hmac`, `syn_hmac_drbg`, `syn_hkdf`, `syn_p256`, `syn_x25519`, `syn_ed25519`, `syn_asn1`, `syn_x509`, and `syn_tpm2`.

## Cross-Directory Touchpoints

- **Hardware Acceleration, Entropy & Protothreads**: `src/syntropic/port/syn_port_aes.h` (`SYN_USE_PORT_AES`), `src/syntropic/util/syn_random.h`, `src/syntropic/pt/syn_pt.h`, and `src/syntropic/sched/syn_sched.h` (`PT_SPAWN_IF_NEEDED`).
- **Consumers**: `src/syntropic/net/` (`syn_tls`, `syn_dtls`, `syn_wg`), `src/syntropic/proto/` (`syn_cose`, `syn_lwm2m`), and `src/syntropic/system/` (`syn_fwupdate`, `syn_ota`, `syn_fwboot`).

## Cryptographic Safety Invariants

- **Constant-Time Execution**: Software AES in `syn_aes.c` operates with secret-independent control flow and memory addresses on all non-AVR targets via a 2-block `uint32_t` bitsliced Boyar–Peralta Boolean circuit covering encrypt, decrypt, and key expansion. On AVR (`__AVR__`), 256-byte flash lookup tables (`pgm_read_byte` in PROGMEM) are retained because AVR lacks data/flash caches (making table accesses constant-time) and Uno flash budgets are strictly constrained. Scalar multiplication (`syn_p256`, `syn_x25519`, `syn_ed25519`), 9-multiply Karatsuba GF(2^128) GHASH multiplication in `syn_aes.c` (`clmul32_ct` / `ghash_mult_reduce` / `syn_aes_ghash_mult` using branchless `0U - bit` masks), MAC/tag verification, and secret byte comparisons must execute with secret-independent control flow and memory indices. Never use `memcmp` or early-exit loops on tags, MACs, private scalars, or shared secrets.
- **Volatile-Safe Zeroization**: Scrub transient key schedules, nonces, ephemeral scalars, PRKs, GHASH accumulators, and intermediate digests on **all** return paths (including early validation/auth failures and cooperative `*_pt` completion) via `volatile` byte writes (`syn_secure_zero`) so dead-store elimination cannot strip cleanup. Callers aborting an in-flight cooperative context before `PT_ENDED` must invoke the matching `*_wipe` helper (`syn_x25519_wipe`, `syn_p256_wipe`, `syn_ed25519_wipe`, `syn_x509_verify_wipe`).
- **Canonical Curve & Shared-Secret Validation**: Enforce RFC 8032 canonical scalar `S < L` and canonical point encoding in Ed25519; reject RFC 7748 all-zero low-order shared secrets in X25519 callers; validate P-256 public points lie on curve and are not point-at-infinity.
- **AEAD Length, Counter & Tag-Before-Plaintext Rules (`syn_aes`, `syn_chacha20poly1305`)**: Enforce 64-bit RFC input/AAD length limits and block-counter overflow bounds; zero output buffers or withhold plaintext commitment if authentication tag verification fails.
- **X.509 & ASN.1 Fail-Closed Parsing (`syn_asn1`, `syn_x509`)**: Validate TLV length bounds strictly within parent container slices without pointer wraparound; bind signature verification directly to parsed `cert->sig_algo` and reject unknown/mismatched algorithm OIDs.
- **Stack Cap & Standalone Feature Gates**: Bignum/ECC static stack frames must stay `<= 1920B` (`<= 1024B` elsewhere). Guard optional hash/curve dependencies (e.g., `SYN_USE_SHA512` in `syn_hkdf`) so modules build standalone when peer algorithms are disabled.

## Cooperative Protothread Crypto API (`SYN_PT`)

Asymmetric and X.509 operations expose caller-owned, zero-heap contexts driven as child protothreads (`PT_MORE` per field/ladder/hash slice, `PT_ENDED` on completion after automatic `syn_secure_zero`, `PT_EXITED` on invalid arguments). Compose inside scheduler tasks via `PT_SPAWN_IF_NEEDED(pt, task, &child_pt, fn)` so uncontended dispatches run slices back-to-back while contended dispatches (`syn_task_should_share(task)`) yield `PT_MORE` between slices:

- **`syn_x25519`**: `SYN_X25519_Ctx`, `syn_x25519_init`, `syn_x25519_pubkey_init`, `syn_x25519_pt`, `syn_x25519_wipe`.
- **`syn_p256`**: `SYN_P256_Ctx`, `syn_p256_base_mul_init`, `syn_p256_point_mul_init`, `syn_p256_mul_pt`, `syn_p256_sign_init`, `syn_p256_sign_deterministic_init`, `syn_p256_sign_pt`, `syn_p256_verify_init`, `syn_p256_verify_pt`, `syn_p256_wipe`.
- **`syn_ed25519`**: `SYN_Ed25519_Ctx`, `syn_ed25519_publickey_init`, `syn_ed25519_publickey_pt`, `syn_ed25519_sign_init`, `syn_ed25519_sign_pt`, `syn_ed25519_verify_init`, `syn_ed25519_verify_hash_init`, `syn_ed25519_verify_pt`, `syn_ed25519_wipe`.
- **`syn_x509`**: `SYN_X509_VerifyCtx`, `syn_x509_verify_signature_init`, `syn_x509_validate_chain_init`, `syn_x509_verify_pt`, `syn_x509_verify_wipe` (DER bytes backing `cert->tbs_bytes` must remain live until Ed25519 verification completes).
