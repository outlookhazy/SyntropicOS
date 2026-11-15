

# File syn\_at\_parser.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_at\_parser.c**](syn__at__parser_8c.md)

[Go to the documentation of this file](syn__at__parser_8c.md)


```C++


#include "syn_at_parser.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

SYN_Status syn_at_parser_init(SYN_AtParser *parser, char *buf, size_t buf_size)
{
    if (parser == NULL || buf == NULL || buf_size == 0) {
        return SYN_INVALID_PARAM;
    }

    parser->line_buf = buf;
    parser->line_buf_size = buf_size;
    syn_at_parser_reset(parser);

    return SYN_OK;
}

void syn_at_parser_reset(SYN_AtParser *parser)
{
    if (parser == NULL) {
        return;
    }

    parser->line_len = 0;
    parser->last_resp = SYN_AT_RESP_NONE;
    parser->cme_error_code = -1;
    parser->prompt_detected = false;
    parser->skip_prompt_space = false;
    if (parser->line_buf != NULL && parser->line_buf_size > 0) {
        parser->line_buf[0] = '\0';
    }
}

static bool parse_at_int(const char *p, int *out_val);

static bool starts_with(const char *str, const char *prefix)
{
    /* LCOV_EXCL_START: Defensive NULL check */
    if (str == NULL || prefix == NULL)
        return false;
    /* LCOV_EXCL_STOP */
    while (*prefix) {
        if (*str != *prefix)
            return false;
        str++;
        prefix++;
    }
    return true;
}

static bool is_urc_line(const char *line)
{
    return starts_with(line, "+RECEIVE,") || starts_with(line, "+IPD,") ||
           starts_with(line, "CLOSED") || starts_with(line, "SHUT OK") ||
           starts_with(line, "RING") || starts_with(line, "+CMTI:");
}

static SYN_AtRespType classify_at_line(SYN_AtParser *parser, const char *line)
{
    if (strcmp(line, "OK") == 0) {
        parser->last_resp = SYN_AT_RESP_OK;
    } else if (strcmp(line, "ERROR") == 0) {
        parser->last_resp = SYN_AT_RESP_ERROR;
    } else if (starts_with(line, "+CME ERROR:") || starts_with(line, "+CMS ERROR:")) {
        parser->last_resp = SYN_AT_RESP_CME_ERROR;
        const char *colon = strchr(line, ':');
        int code = 0;
        parser->cme_error_code = parse_at_int(colon + 1, &code) ? code : 0;
    } else if (is_urc_line(line)) {
        parser->last_resp = SYN_AT_RESP_URC;
    } else {
        parser->last_resp = SYN_AT_RESP_LINE;
    }
    return parser->last_resp;
}

SYN_AtRespType syn_at_parser_feed_char(SYN_AtParser *parser, char c)
{
    if (parser == NULL || parser->line_buf == NULL) {
        return SYN_AT_RESP_NONE;
    }

    /* Ignore carriage return characters (\r) in CRLF sequences */
    if (c == '\r') {
        return SYN_AT_RESP_NONE;
    }

    /* Ignore a single space immediately following '>' prompt at start of line */
    if (parser->skip_prompt_space) {
        parser->skip_prompt_space = false;
        if ((c == ' ') && (parser->line_len == 0U)) {
            return SYN_AT_RESP_NONE;
        }
    }

    /* Prompt detection: '>' as first non-whitespace character */
    if (c == '>' && parser->line_len == 0) {
        parser->prompt_detected = true;
        parser->skip_prompt_space = true;
        parser->last_resp = SYN_AT_RESP_PROMPT;
        return SYN_AT_RESP_PROMPT;
    }

    if (c == '\n') {
        if (parser->line_len == 0) {
            return SYN_AT_RESP_NONE;
        }

        parser->line_buf[parser->line_len] = '\0';
        parser->line_len = 0;
        return classify_at_line(parser, parser->line_buf);
    }

    /* Accumulate character */
    if (parser->line_len < parser->line_buf_size - 1) {
        parser->line_buf[parser->line_len++] = c;
    }

    return SYN_AT_RESP_NONE;
}

SYN_AtRespType syn_at_parser_feed_stream(SYN_AtParser *parser, SYN_Stream *stream)
{
    if (parser == NULL || stream == NULL) {
        return SYN_AT_RESP_NONE;
    }

    uint8_t byte;
    while (syn_stream_read(stream, &byte, 1) == 1) {
        SYN_AtRespType resp = syn_at_parser_feed_char(parser, (char)byte);
        if (resp != SYN_AT_RESP_NONE) {
            return resp;
        }
    }

    return SYN_AT_RESP_NONE;
}

const char *syn_at_parser_get_line(const SYN_AtParser *parser)
{
    if (parser == NULL || parser->line_buf == NULL) {
        return "";
    }
    return parser->line_buf;
}

int syn_at_parser_get_cme_error(const SYN_AtParser *parser)
{
    if (parser == NULL) {
        return -1;
    }
    return parser->cme_error_code;
}

static bool is_param_delim(char c)
{
    return (c == '\0' || c == ',' || c == '\r' || c == '\n');
}

static bool parse_at_int(const char *p, int *out_val)
{
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (is_param_delim(*p)) {
        return false;
    }

    errno = 0;
    char *endptr = NULL;
    long val = strtol(p, &endptr, 10);
    if (endptr == p || errno == ERANGE || val < (long)INT_MIN || val > (long)INT_MAX) {
        return false;
    }

    while (*endptr == ' ' || *endptr == '\t') {
        endptr++;
    }
    if (!is_param_delim(*endptr)) {
        return false;
    }

    *out_val = (int)val;
    return true;
}

static const char *skip_at_header(const char *line)
{
    bool in_quote = false;
    for (const char *s = line; *s != '\0'; s++) {
        if (*s == '"') {
            in_quote = !in_quote;
        } else if (!in_quote && *s == ',') {
            break;
        } else if (!in_quote && *s == ':') {
            return s + 1;
        }
    }
    return line;
}

static const char *next_unquoted_comma(const char *p)
{
    bool in_quote = false;
    while (*p != '\0') {
        if (*p == '"') {
            in_quote = !in_quote;
        } else if (!in_quote && *p == ',') {
            return p;
        }
        p++;
    }
    return NULL;
}

static const char *find_param_start(const char *line, size_t param_idx)
{
    if (line == NULL) {
        return NULL;
    }

    const char *p = skip_at_header(line);
    while (*p == ' ') {
        p++;
    }

    size_t curr_idx = 0U;
    while (curr_idx < param_idx) {
        p = next_unquoted_comma(p);
        if (p == NULL) {
            return NULL;
        }
        p++;
        while (*p == ' ') {
            p++;
        }
        curr_idx++;
    }

    return p;
}

bool syn_at_parser_get_param_int(const char *line, size_t param_idx, int *out_val)
{
    if (out_val == NULL) {
        return false;
    }

    const char *p = find_param_start(line, param_idx);
    if (p == NULL) {
        return false;
    }

    return parse_at_int(p, out_val);
}

static bool copy_param_token(const char *p, bool quoted, char *out_buf, size_t max_len)
{
    size_t out_idx = 0U;
    while ((*p != '\0') && (*p != '\r') && (*p != '\n')) {
        if (quoted && (*p == '"')) {
            break;
        }
        if (!quoted && (*p == ',')) {
            break;
        }
        if (out_idx >= (max_len - 1U)) {
            out_buf[out_idx] = '\0';
            return false;
        }
        out_buf[out_idx++] = *p++;
    }

    out_buf[out_idx] = '\0';
    if (quoted && (*p != '"')) {
        return false;
    }
    return true;
}

bool syn_at_parser_get_param_str(const char *line, size_t param_idx, char *out_buf, size_t max_len)
{
    if (out_buf == NULL || max_len == 0U) {
        return false;
    }

    const char *p = find_param_start(line, param_idx);
    if (p == NULL) {
        return false;
    }

    bool quoted = (*p == '"');
    if (quoted) {
        p++;
    }

    return copy_param_token(p, quoted, out_buf, max_len);
}
```


