# Security Policy

## Reporting a Vulnerability

**Do not open public GitHub issues or pull requests for suspected security vulnerabilities.**

Report vulnerabilities privately through **[GitHub Private Vulnerability Reports](https://github.com/outlookhazy/SyntropicOS/security/advisories/new)**.

Please include:
- Affected module(s) and file path(s) (e.g., `src/syntropic/net/syn_tls.c`).
- Commit SHA or release tag tested.
- Minimal reproduction input, unit test, or `libFuzzer` crash artifact demonstrating the issue.
- Impact assessment (memory corruption, authentication bypass, cryptographic key/nonce misuse, or protocol state-machine denial of service).

Reports are reviewed on a best-effort basis by the maintainer. Once a fix and regression test are verified, a coordinated advisory and patch commit will be published. There is no paid bug-bounty program.

---

## Supported Versions

Security fixes apply to the `main` branch and are included in the next Calendar Versioning (`YYYY.M.MINOR`) release:

| Version | Supported |
| :--- | :--- |
| `main` (HEAD) | Yes |
| Latest `2026.x` release | Yes |
| Older releases | No (upgrade to latest) |

---

## Threat Model & Scope

SyntropicOS is a C99 bare-metal library linked directly into single-address-space microcontroller firmware without an MMU or kernel/user privilege boundary.

### In Scope
- **Untrusted Wire & File Parsers**: Out-of-bounds reads/writes, integer overflows, uninitialized memory disclosure, or unbounded loops triggered by malformed network packets, fieldbus frames, BLE/USB PDUs, filesystems (`syn_fat`, `syn_lfs`), or Wasm modules (`syn_wasm`).
- **Cryptographic & Protocol Guarantees**: Flaws in constant-time scalar/MAC comparisons, AEAD/cipher implementations (`syn_aes`, `syn_chacha20poly1305`), ECC/PKI validation (`syn_p256`, `syn_ed25519`, `syn_x25519`, `syn_x509`), session/handshake state machines (`syn_tls`, `syn_dtls`, `syn_wg`), or TPM 2.0 session handling (`syn_tpm2`).
- **Firmware Update Integrity**: Signature, AEAD tag, rollback/version, or slot-validation bypasses in `syn_fwimage`, `syn_fwupdate`, `syn_fwboot`, and `syn_ota`.

### Out of Scope
- **Caller API Misuse**: Passing invalid pointers with mismatched buffer capacities (`buf` smaller than `buf_size`), mutating context structs concurrently from ISRs without documented synchronization, or disabling assertions (`SYN_DISABLE_ASSERT`).
- **Hardware & Physical Access**: SWD/JTAG debug port readout, power/EM fault injection, unencrypted external flash without hardware secure boot, or weak hardware RNG entropy supplied by a user `syn_port_*` implementation.
- **Unauthenticated / Cleartext Standards**: Protocol specs that lack built-in encryption or authentication by design (e.g., standard Modbus RTU/TCP, HTTP, CoAP without DTLS, CANopen, J1939, NMEA) when deployed without an encapsulating secure transport.
- **Host / Test Tooling Only**: Issues confined to `tests/`, `examples/`, or `tools/` build scripts that do not affect production code under `src/`.
