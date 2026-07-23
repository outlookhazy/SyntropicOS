#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_NMEA) || SYN_USE_NMEA

/**
 * @file syn_nmea.c
 * @brief Non-blocking streaming NMEA 0183 GPS sentence parser implementation.
 */

#include "syn_nmea.h"
#include "../util/syn_assert.h"
#include "../util/syn_str.h"
#include <string.h>
#include <stdlib.h>

/* ── Helpers ────────────────────────────────────────────────────────────── */

/**
 * @brief Convert NMEA coordinate format (DDMM.MMMM) to microdegrees.
 * @param str NMEA coordinate string.
 * @param dir Direction character ('N', 'S', 'E', 'W').
 * @return Microdegrees (signed: negative for S or W).
 */
static int32_t parse_coord_microdeg(const char *str, char dir)
{
    if (!str || !*str) return 0;

    const char *dot = strchr(str, '.');
    if (!dot) return 0;

    size_t len_before_dot = (size_t)(dot - str);
    size_t deg_digits = (len_before_dot == 5) ? 3 : 2; /* Longitude: DDDMM.MMMM (5 chars), Latitude: DDMM.MMMM (4 chars) */

    char deg_buf[4] = {0};
    if (deg_digits >= sizeof(deg_buf)) return 0;
    memcpy(deg_buf, str, deg_digits);
    int32_t degrees = (int32_t)atol(deg_buf);

    double minutes = atof(str + deg_digits);
    int32_t microdeg = (degrees * 1000000) + (int32_t)((minutes / 60.0) * 1000000.0);

    if (dir == 'S' || dir == 'W') {
        microdeg = -microdeg;
    }

    return microdeg;
}

/* ── Sentence Processors ────────────────────────────────────────────────── */

/**
 * @brief Parse NMEA RMC (Recommended Minimum Navigation Information) sentence.
 * @param parser NMEA parser context.
 * @param line   NMEA sentence string.
 */
static void parse_rmc(SYN_NmeaParser *parser, char *line)
{
    char *fields[16];
    size_t count = syn_str_split(line, ',', fields, 16);
    if (count < 10) return;

    /* Field 1: UTC Time (HHMMSS.ss) */
    if (fields[1][0] != '\0') {
        parser->data.utc_time = (uint32_t)atol(fields[1]);
    }

    /* Field 2: Status ('A' = Valid, 'V' = Warning) */
    parser->data.valid = (fields[2][0] == 'A');

    /* Field 3 & 4: Lat & N/S */
    if (fields[3][0] != '\0' && fields[4][0] != '\0') {
        parser->data.lat_microdeg = parse_coord_microdeg(fields[3], fields[4][0]);
    }

    /* Field 5 & 6: Lon & E/W */
    if (fields[5][0] != '\0' && fields[6][0] != '\0') {
        parser->data.lon_microdeg = parse_coord_microdeg(fields[5], fields[6][0]);
    }

    /* Field 7: Speed over ground in knots */
    if (fields[7][0] != '\0') {
        parser->data.speed_knots_x10 = (uint16_t)(atof(fields[7]) * 10.0);
    }

    /* Field 9: UTC Date (DDMMYY) */
    if (fields[9][0] != '\0') {
        parser->data.utc_date = (uint32_t)atol(fields[9]);
    }

    if (parser->on_fix != NULL) {
        parser->on_fix(&parser->data, "RMC", parser->ctx);
    }
}

/**
 * @brief Parse NMEA GGA (Global Positioning System Fix Data) sentence.
 * @param parser NMEA parser context.
 * @param line   NMEA sentence string.
 */
static void parse_gga(SYN_NmeaParser *parser, char *line)
{
    char *fields[16];
    size_t count = syn_str_split(line, ',', fields, 16);
    if (count < 10) return;

    /* Field 1: UTC Time */
    if (fields[1][0] != '\0') {
        parser->data.utc_time = (uint32_t)atol(fields[1]);
    }

    /* Field 2 & 3: Lat & N/S */
    if (fields[2][0] != '\0' && fields[3][0] != '\0') {
        parser->data.lat_microdeg = parse_coord_microdeg(fields[2], fields[3][0]);
    }

    /* Field 4 & 5: Lon & E/W */
    if (fields[4][0] != '\0' && fields[5][0] != '\0') {
        parser->data.lon_microdeg = parse_coord_microdeg(fields[4], fields[5][0]);
    }

    /* Field 6: Fix Quality (0 = Invalid, 1 = GPS fix, 2 = DGPS) */
    if (fields[6][0] != '\0') {
        parser->data.fix_quality = (uint8_t)atoi(fields[6]);
    }

    /* Field 7: Satellites in view */
    if (fields[7][0] != '\0') {
        parser->data.satellites = (uint8_t)atoi(fields[7]);
    }

    /* Field 9: Altitude in meters */
    if (fields[9][0] != '\0') {
        parser->data.alt_mm = (int32_t)(atof(fields[9]) * 1000.0);
    }

    if (parser->on_fix != NULL) {
        parser->on_fix(&parser->data, "GGA", parser->ctx);
    }
}

/**
 * @brief Dispatch validated NMEA sentence to appropriate parser function.
 * @param parser NMEA parser context.
 * @param line   NMEA sentence string.
 */
static void process_sentence(SYN_NmeaParser *parser, char *line)
{
    /* Check XOR checksum if '*' is present */
    char *star = strchr(line, '*');
    if (star != NULL) {
        uint8_t expected_crc = (uint8_t)strtoul(star + 1, NULL, 16);
        uint8_t computed_crc = 0;
        for (char *p = line; p < star; p++) {
            computed_crc ^= (uint8_t)(*p);
        }
        if (computed_crc != expected_crc) {
            return; /* Checksum mismatch */
        }
    }

    if (strncmp(line, "GPRMC", 5) == 0 || strncmp(line, "GNRMC", 5) == 0) {
        parse_rmc(parser, line);
    } else if (strncmp(line, "GPGGA", 5) == 0 || strncmp(line, "GNGGA", 5) == 0) {
        parse_gga(parser, line);
    }
}

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_nmea_init(SYN_NmeaParser *parser,
                   void (*on_fix)(const SYN_NmeaData *data, const char *sentence_type, void *ctx),
                   void *ctx)
{
    SYN_ASSERT(parser != NULL);
    memset(parser, 0, sizeof(*parser));
    parser->on_fix = on_fix;
    parser->ctx    = ctx;
}

bool syn_nmea_feed_char(SYN_NmeaParser *parser, char ch)
{
    SYN_ASSERT(parser != NULL);

    if (ch == '$') {
        parser->line_pos    = 0;
        parser->in_sentence = true;
        return false;
    }

    if (!parser->in_sentence) return false;

    if (ch == '\r' || ch == '\n') {
        parser->in_sentence = false;
        if (parser->line_pos > 0) {
            parser->line_buf[parser->line_pos] = '\0';
            process_sentence(parser, parser->line_buf);
            return true;
        }
        return false;
    }

    if (parser->line_pos < sizeof(parser->line_buf) - 1) {
        parser->line_buf[parser->line_pos++] = ch;
    }

    return false;
}

void syn_nmea_feed_buf(SYN_NmeaParser *parser, const char *str, size_t len)
{
    SYN_ASSERT(parser != NULL);
    if (!str) return;

    for (size_t i = 0; i < len; i++) {
        syn_nmea_feed_char(parser, str[i]);
    }
}

#endif /* SYN_USE_NMEA */
