/**
 * @file syn_uds_util.h
 * @brief SAE J2012 / ISO 15031-6 UDS Diagnostic Trouble Code (DTC) Utilities.
 *
 * Provides standalone, zero-heap conversion functions between human-readable
 * SAE J2012 DTC strings (e.g., "P010500", "B111717", "U013100") and 24-bit
 * UDS wire integers (e.g., 0x010500, 0x911717, 0xC13100).
 *
 * @ingroup syn_protocols
 */

#ifndef SYN_UDS_UTIL_H
#define SYN_UDS_UTIL_H

#include "syntropic/common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parse a 5, 6, or 7-character SAE J2012 DTC string into a 24-bit UDS integer.
 *
 * Accepts strings like:
 * - "P0105" or "P010500" -> 0x010500
 * - "B111717"            -> 0x911717
 * - "U013100"            -> 0xC13100
 * - "C101000"            -> 0x501000
 *
 * Case-insensitive for prefix (p/c/b/u or P/C/B/U) and hex digits.
 *
 * @param str      Null-terminated SAE J2012 DTC string.
 * @param dtc_out  [out] Destination for parsed 24-bit DTC value.
 * @return true if successfully parsed, false on invalid format.
 */
bool syn_uds_dtc_from_str(const char *str, uint32_t *dtc_out);

/**
 * @brief Format a 24-bit UDS integer into a standard SAE J2012 DTC display string.
 *
 * Formats values like:
 * - 0x010500 -> "P010500"
 * - 0x911717 -> "B111717"
 * - 0xC13100 -> "U013100"
 *
 * Buffer must have a capacity of at least 8 bytes (7 chars + null terminator).
 *
 * @param dtc       24-bit UDS DTC integer value.
 * @param out_buf   Destination string buffer.
 * @param buf_size  Capacity of output buffer (minimum 8 bytes).
 * @return true if formatted successfully, false if buffer too small or NULL.
 */
bool syn_uds_dtc_to_str(uint32_t dtc, char *out_buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif /* SYN_UDS_UTIL_H */
