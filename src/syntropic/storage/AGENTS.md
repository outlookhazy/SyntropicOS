# File Systems, Key-Value & Storage Invariants (`src/syntropic/storage/`)

Applies to `syn_vfs`, `syn_fat`, `syn_lfs`, `syn_settings`, and `syn_param`.

## Cross-Directory Touchpoints

- **Block & Flash Media**: `src/syntropic/port/syn_port_flash.h`, `src/syntropic/drivers/syn_sd.h`, and `src/syntropic/util/syn_crc.h`.
- **Consumers**: `src/syntropic/log/syn_datalog.h` and `src/syntropic/system/` (`syn_coredump`, `syn_ota`, `syn_fwupdate`).

## Media & Persistence Invariants

- **Untrusted On-Disk Geometry Validation (`syn_fat`, `syn_lfs`)**: Treat all superblock/BPB/MBR/directory bytes read from flash or SD media as untrusted input. Validate non-zero sector/cluster sizes, power-of-two invariants, FAT table bounds, root-directory extents, and `count * size` multiplication overflow before division, allocation, or buffer indexing.
- **Bounded Chain Traversal & Cycle Detection**: Corrupted FAT cluster chains or LFS metadata pairs can form loops or point out of range. Bound every chain/tree walk by total valid cluster/block count and reject premature-EOF, reserved, or out-of-bounds indices with explicit error codes instead of spinning forever.
- **Open-Handle Directory/Cluster Coherency (`syn_fat`)**: Pooled open file contexts cache `start_cluster`, `size`, and root directory slot offsets in RAM; reject conflicting concurrent `open` calls (whenever existing or new handle requests write/truncate) as well as `unlink` and `rename` (source or destination) with `SYN_BUSY` while a matching 8.3 entry is open in `g_fat_files`.
- **Signed Seek & Offset Bounds (`syn_vfs`, `syn_fat`, `syn_lfs`)**: Validate `SEEK_SET`/`SEEK_CUR`/`SEEK_END` arithmetic before casting signed `int32_t`/`off_t` offsets to unsigned positions; reject negative resulting offsets and integer wraparound past file/volume limits.
- **Crash-Atomic Dual-Bank & Monotonic Sequence Rules (`syn_settings`, `syn_param`)**: Write payload + CRC before committing magic/valid markers; when falling back to an older valid bank after an interrupted erase/write, preserve monotonic `next_seq` above all observed sequence numbers so subsequent writes unambiguously supersede prior banks.
- **Erase/Program Alignment & Error Propagation**: Respect flash erase-sector and program-word alignment contracts from `syn_port_flash.h`, and propagate underlying read/program/erase errors immediately without marking in-RAM cache state clean.
