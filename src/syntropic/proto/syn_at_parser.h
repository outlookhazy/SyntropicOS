/**
 * @file syn_at_parser.h
 * @brief Zero-allocation, stream-oriented AT command parser for cellular & serial modems.
 * @ingroup syn_protocol
 *
 * Lightweight, non-blocking line and prompt parser for modems (SIM800, Quectel, ESP-AT, etc.).
 * Feeds from SYN_Stream or raw byte feeds, handles CRLF line framing, prompt '>' detection,
 * CME ERROR decoding, URC filtering, and parameter extraction without dynamic memory allocation.
 *
 * @par Usage
 * @code
 *   static char line_buf[128];
 *   static SYN_AtParser parser;
 *   syn_at_parser_init(&parser, line_buf, sizeof(line_buf));
 *
 *   // In protothread or main loop:
 *   SYN_AtRespType resp = syn_at_parser_feed_stream(&parser, &uart_rx_stream);
 *   if (resp == SYN_AT_RESP_OK) {
 *       // Command succeeded
 *   } else if (resp == SYN_AT_RESP_PROMPT) {
 *       // Modem sent '>' prompt, ready for data
 *   }
 * @endcode
 */

#ifndef SYN_AT_PARSER_H
#define SYN_AT_PARSER_H

#include "../common/syn_defs.h"
#include "../pt/syn_pt.h"
#include "../util/syn_stream.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Decoded AT response types.
 */
typedef enum {
    SYN_AT_RESP_NONE = 0,  /**< No complete response line or prompt decoded yet */
    SYN_AT_RESP_OK,        /**< Standard 'OK' response */
    SYN_AT_RESP_ERROR,     /**< Standard 'ERROR' response */
    SYN_AT_RESP_CME_ERROR, /**< Extended '+CME ERROR: err_code' or '+CMS ERROR: err_code' */
    SYN_AT_RESP_PROMPT,    /**< Modem data entry prompt '>' */
    SYN_AT_RESP_LINE,      /**< Response data line (e.g. "+CSQ: 20,0", "CONNECT OK", IP address) */
    SYN_AT_RESP_URC,       /**< Unsolicited Result Code line (e.g. "+RECEIVE,0,17", "CLOSED") */
} SYN_AtRespType;

/**
 * @brief AT Command Parser context.
 */
typedef struct {
    char *line_buf;           /**< Caller-owned line buffer */
    size_t line_buf_size;     /**< Capacity of line_buf */
    size_t line_len;          /**< Current accumulated line length */
    SYN_AtRespType last_resp; /**< Result of last decoded feed byte */
    int cme_error_code;   /**< Parsed CME/CMS error code (if last_resp == SYN_AT_RESP_CME_ERROR) */
    bool prompt_detected; /**< true if '>' prompt was encountered */
    bool skip_prompt_space; /**< true to consume a single ' ' immediately following '>' prompt */
} SYN_AtParser;

/**
 * @brief Initialize an AT command parser.
 *
 * @param parser   Parser instance.
 * @param buf      Caller-owned buffer for line accumulation.
 * @param buf_size Capacity of buffer in bytes.
 * @return SYN_OK on success, or SYN_INVALID_PARAM.
 */
SYN_Status syn_at_parser_init(SYN_AtParser *parser, char *buf, size_t buf_size);

/**
 * @brief Reset internal parser state.
 *
 * Clears accumulated line buffer and last decoded response.
 *
 * @param parser Parser instance.
 */
void syn_at_parser_reset(SYN_AtParser *parser);

/**
 * @brief Feed a single byte into the AT parser.
 *
 * Processes a single character. Returns response type if a complete line (CRLF)
 * or prompt ('>') has been parsed.
 *
 * @param parser Parser instance.
 * @param c      Byte to process.
 * @return Decoded response type, or SYN_AT_RESP_NONE if line incomplete.
 */
SYN_AtRespType syn_at_parser_feed_char(SYN_AtParser *parser, char c);

/**
 * @brief Feed available bytes from a SYN_Stream into the AT parser.
 *
 * Reads bytes non-blockingly from the stream until a complete response line/prompt
 * is parsed or the stream is empty.
 *
 * @param parser Parser instance.
 * @param stream Pointer to input stream.
 * @return Decoded response type, or SYN_AT_RESP_NONE if line incomplete.
 */
SYN_AtRespType syn_at_parser_feed_stream(SYN_AtParser *parser, SYN_Stream *stream);

/**
 * @brief Get the current accumulated response line string.
 *
 * @param parser Parser instance.
 * @return Null-terminated line string.
 */
const char *syn_at_parser_get_line(const SYN_AtParser *parser);

/**
 * @brief Get the parsed error code for SYN_AT_RESP_CME_ERROR.
 *
 * @param parser Parser instance.
 * @return Numerical CME/CMS error code, or -1 if unavailable.
 */
int syn_at_parser_get_cme_error(const SYN_AtParser *parser);

/**
 * @brief Helper: Extract an integer parameter from a comma-delimited AT response line.
 *
 * Example line: "+CSQ: 20,0" -> param 0 is 20, param 1 is 0.
 *
 * @param line      Response text line.
 * @param param_idx 0-based parameter index.
 * @param out_val   [out] Extracted integer value.
 * @return true if parameter was found and successfully parsed.
 */
bool syn_at_parser_get_param_int(const char *line, size_t param_idx, int *out_val);

/**
 * @brief Helper: Extract a string parameter from a comma-delimited AT response line.
 *
 * Handles quoted strings (e.g. "+CPIN: \"READY\"") and strips quotes.
 *
 * @param line      Response text line.
 * @param param_idx 0-based parameter index.
 * @param out_buf   Destination buffer.
 * @param max_len   Destination capacity.
 * @return true if parameter was found and extracted.
 */
bool syn_at_parser_get_param_str(const char *line, size_t param_idx, char *out_buf, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AT_PARSER_H */
