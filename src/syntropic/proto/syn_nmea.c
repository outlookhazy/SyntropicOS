#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_NMEA) || SYN_USE_NMEA

/**
 * @file syn_nmea.c
 * @brief NMEA 0183 / GNSS sentence parser and encoder implementation.
 */

#include "../util/syn_assert.h"
#include "syn_nmea.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Lightweight ASCII string to double conversion for non-empty NMEA numerical fields.
 * @param s Non-NULL NMEA field string.
 * @return Converted double value.
 */
static double syn_atof(const char *s)
{
    double res = 0.0;
    double sign = 1.0;
    if (*s == '-') {
        sign = -1.0;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while ((*s >= '0') && (*s <= '9')) {
        res = (res * 10.0) + (double)(*s - '0');
        s++;
    }
    if (*s == '.') {
        s++;
        double frac = 0.1;
        while ((*s >= '0') && (*s <= '9')) {
            res += (double)(*s - '0') * frac;
            frac *= 0.1;
            s++;
        }
    }
    return res * sign;
}

/**
 * @brief Check whether character terminates NMEA sentence payload.
 * @param c Character to inspect.
 * @return true if end-of-payload character.
 */
static inline bool is_nmea_end(char c)
{
    return (c == '\0') || (c == '*') || (c == '\r') || (c == '\n');
}

/**
 * @brief Extract comma-separated NMEA field from sentence.
 * @param sentence Input NMEA sentence string (non-NULL).
 * @param index    Field index (0-based).
 * @param dest     Destination string buffer (non-NULL, max_len >= 2).
 * @param max_len  Maximum capacity of destination buffer.
 * @return true if non-empty field extracted, false otherwise.
 */
static bool get_field(const char *sentence, uint8_t index, char *dest, size_t max_len)
{
    dest[0] = '\0';

    const char *p = sentence;
    if (*p == '$') {
        p++;
    }

    uint8_t current_field = 0U;
    while (!is_nmea_end(*p)) {
        if (*p == ',') {
            current_field++;
            p++;
            if (current_field > index) {
                break;
            }
            continue;
        }

        if (current_field == index) {
            size_t len = 0U;
            while (!is_nmea_end(*p) && (*p != ',') && ((len + 1U) < max_len)) {
                dest[len++] = *p++;
            }
            dest[len] = '\0';
            return len > 0U;
        }

        p++;
    }

    return false;
}

/**
 * @brief Decode a single hexadecimal ASCII digit into its 4-bit nibble value.
 * @param c   Input character.
 * @param out [out] Decoded nibble (0..15).
 * @return true if c is a valid hexadecimal digit, false otherwise.
 */
static bool hex_nibble(char c, uint8_t *out)
{
    if ((c >= '0') && (c <= '9')) {
        *out = (uint8_t)(c - '0');
        return true;
    }
    if ((c >= 'A') && (c <= 'F')) {
        *out = (uint8_t)(10 + (c - 'A'));
        return true;
    }
    if ((c >= 'a') && (c <= 'f')) {
        *out = (uint8_t)(10 + (c - 'a'));
        return true;
    }
    return false;
}

void syn_nmea_parser_init(SYN_NMEA_Parser *parser)
{
    SYN_ASSERT(parser != NULL);
    parser->pos = 0U;
    parser->in_sentence = false;
    parser->buf[0] = '\0';
}

bool syn_nmea_parser_feed(SYN_NMEA_Parser *parser, char byte, char *out_sentence)
{
    SYN_ASSERT(parser != NULL);

    if (byte == '$') {
        parser->pos = 0U;
        parser->buf[parser->pos++] = '$';
        parser->in_sentence = true;
        return false;
    }

    if (!parser->in_sentence) {
        return false;
    }

    if ((byte == '\n') || (byte == '\r')) {
        if (parser->pos > 6U) {
            parser->buf[parser->pos] = '\0';
            if (syn_nmea_validate(parser->buf)) {
                if (out_sentence != NULL) {
                    size_t len = strlen(parser->buf);
                    memcpy(out_sentence, parser->buf, len + 1U);
                }
                parser->in_sentence = false;
                parser->pos = 0U;
                return true;
            }
        }
        parser->in_sentence = false;
        parser->pos = 0U;
        return false;
    }

    if (parser->pos < SYN_NMEA_MAX_SENTENCE_LEN) {
        parser->buf[parser->pos++] = byte;
    } else {
        parser->in_sentence = false;
        parser->pos = 0U;
    }

    return false;
}

uint8_t syn_nmea_checksum(const char *sentence)
{
    if (sentence == NULL) {
        return 0U;
    }
    const char *p = sentence;
    if (*p == '$') {
        p++;
    }

    uint8_t checksum = 0U;
    while (!is_nmea_end(*p)) {
        checksum ^= (uint8_t)(*p++);
    }

    return checksum;
}

bool syn_nmea_validate(const char *sentence)
{
    if ((sentence == NULL) || (sentence[0] != '$')) {
        return false;
    }

    const char *star = strchr(sentence, '*');
    if ((star == NULL) || (star[1] == '\0') || (star[2] == '\0')) {
        return false;
    }

    uint8_t hi = 0U;
    uint8_t lo = 0U;
    if (!hex_nibble(star[1], &hi) || !hex_nibble(star[2], &lo)) {
        return false;
    }

    uint8_t expected_crc = (uint8_t)((hi << 4) | lo);
    uint8_t calc_crc = syn_nmea_checksum(sentence);

    return expected_crc == calc_crc;
}

SYN_NMEA_SentenceType syn_nmea_get_type(const char *sentence)
{
    if (!syn_nmea_validate(sentence)) {
        return SYN_NMEA_SENTENCE_UNKNOWN;
    }

    char talker[10];
    if (!get_field(sentence, 0U, talker, sizeof(talker))) {
        return SYN_NMEA_SENTENCE_UNKNOWN;
    }

    size_t len = strlen(talker);
    if (len < 3U) {
        return SYN_NMEA_SENTENCE_UNKNOWN;
    }
    const char *type_str = &talker[len - 3U];

    if (strcmp(type_str, "GGA") == 0) {
        return SYN_NMEA_SENTENCE_GGA;
    }
    if (strcmp(type_str, "RMC") == 0) {
        return SYN_NMEA_SENTENCE_RMC;
    }
    if (strcmp(type_str, "VTG") == 0) {
        return SYN_NMEA_SENTENCE_VTG;
    }
    if (strcmp(type_str, "GSA") == 0) {
        return SYN_NMEA_SENTENCE_GSA;
    }
    if (strcmp(type_str, "ZDA") == 0) {
        return SYN_NMEA_SENTENCE_ZDA;
    }

    return SYN_NMEA_SENTENCE_UNKNOWN;
}

double syn_nmea_parse_coord(const char *nmea_coord, char dir)
{
    if ((nmea_coord == NULL) || (strlen(nmea_coord) < 4U)) {
        return 0.0;
    }

    double val = syn_atof(nmea_coord);
    double deg = (double)((int)(val / 100.0));
    double minutes = val - (deg * 100.0);
    double decimal = deg + (minutes / 60.0);

    if ((dir == 'S') || (dir == 'W')) {
        decimal = -decimal;
    }

    return decimal;
}

#if SYN_NMEA_USE_FIXED_POINT
int32_t syn_nmea_parse_coord_udeg(const char *nmea_coord, char dir)
{
    if ((nmea_coord == NULL) || (strlen(nmea_coord) < 4U)) {
        return 0;
    }

    const char *dot = strchr(nmea_coord, '.');
    if ((dot == NULL) || ((dot - nmea_coord) < 3)) {
        return 0;
    }

    int deg_digits = (int)(dot - nmea_coord) - 2;
    int32_t deg = 0;
    for (int i = 0; i < deg_digits; i++) {
        deg = (deg * 10) + (nmea_coord[i] - '0');
    }

    int32_t min_int = ((nmea_coord[deg_digits] - '0') * 10) + (nmea_coord[deg_digits + 1] - '0');

    int32_t min_frac = 0;
    const char *p = dot + 1;
    for (int i = 0; i < 4; i++) {
        if ((*p >= '0') && (*p <= '9')) {
            min_frac = (min_frac * 10) + (*p - '0');
            p++;
        } else {
            min_frac = min_frac * 10;
        }
    }

    int64_t total_min_scaled = ((int64_t)min_int * 10000LL) + min_frac;
    int64_t min_udeg = (total_min_scaled * 10000000LL) / (60LL * 10000LL);

    int32_t udeg = (deg * 10000000) + (int32_t)min_udeg;
    if ((dir == 'S') || (dir == 'W')) {
        udeg = -udeg;
    }
    return udeg;
}
#endif

/**
 * @brief Parse NMEA UTC time field (hhmmss.ss) into components.
 * @param field NMEA time field string.
 * @param h     Pointer to receive hours (0-23).
 * @param m     Pointer to receive minutes (0-59).
 * @param s     Pointer to receive seconds (0-59).
 * @param ms    Pointer to receive milliseconds (0-999).
 */
static void parse_time(const char *field, uint8_t *h, uint8_t *m, uint8_t *s, uint16_t *ms)
{
    *h = 0U;
    *m = 0U;
    *s = 0U;
    *ms = 0U;
    if (strlen(field) < 6U) {
        return;
    }

    char tmp[3] = {0};
    tmp[0] = field[0];
    tmp[1] = field[1];
    *h = (uint8_t)atoi(tmp);

    tmp[0] = field[2];
    tmp[1] = field[3];
    *m = (uint8_t)atoi(tmp);

    tmp[0] = field[4];
    tmp[1] = field[5];
    *s = (uint8_t)atoi(tmp);

    const char *dot = strchr(field, '.');
    if (dot != NULL) {
        *ms = (uint16_t)(syn_atof(dot) * 1000.0);
    }
}

/**
 * @brief Parse NMEA 6-character date field (ddmmyy) into day, month, and full year.
 * @param f   Date field string.
 * @param rmc Destination RMC structure.
 */
static void parse_rmc_date(const char *f, SYN_NMEA_RMC *rmc)
{
    if (strlen(f) < 6U) {
        return;
    }
    char tmp[3] = {0};
    tmp[0] = f[0];
    tmp[1] = f[1];
    rmc->day = (uint8_t)atoi(tmp);
    tmp[0] = f[2];
    tmp[1] = f[3];
    rmc->month = (uint8_t)atoi(tmp);
    tmp[0] = f[4];
    tmp[1] = f[5];
    uint16_t yr = (uint16_t)atoi(tmp);
    rmc->year = (yr >= 70U) ? (uint16_t)(1900U + yr) : (uint16_t)(2000U + yr);
}

bool syn_nmea_parse_gga(const char *sentence, SYN_NMEA_GGA *gga)
{
    SYN_ASSERT(gga != NULL);
    memset(gga, 0, sizeof(SYN_NMEA_GGA));

    if (syn_nmea_get_type(sentence) != SYN_NMEA_SENTENCE_GGA) {
        return false;
    }

    char f[16] = {0};
    if (get_field(sentence, 1U, f, sizeof(f))) {
        parse_time(f, &gga->hours, &gga->minutes, &gga->seconds, &gga->milliseconds);
    }

    char lat_str[16] = {0}, lat_dir[4] = {0}, lon_str[16] = {0}, lon_dir[4] = {0};
    bool has_lat = get_field(sentence, 2U, lat_str, sizeof(lat_str)) &&
                   get_field(sentence, 3U, lat_dir, sizeof(lat_dir));
    bool has_lon = get_field(sentence, 4U, lon_str, sizeof(lon_str)) &&
                   get_field(sentence, 5U, lon_dir, sizeof(lon_dir));

#if SYN_NMEA_USE_FIXED_POINT
    if (has_lat) {
        gga->lat_udeg = syn_nmea_parse_coord_udeg(lat_str, lat_dir[0]);
    }
    if (has_lon) {
        gga->lon_udeg = syn_nmea_parse_coord_udeg(lon_str, lon_dir[0]);
    }
#else
    if (has_lat) {
        gga->latitude = syn_nmea_parse_coord(lat_str, lat_dir[0]);
    }
    if (has_lon) {
        gga->longitude = syn_nmea_parse_coord(lon_str, lon_dir[0]);
    }
#endif

    if (get_field(sentence, 6U, f, sizeof(f))) {
        gga->fix_quality = (SYN_NMEA_FixQuality)atoi(f);
    }
    if (get_field(sentence, 7U, f, sizeof(f))) {
        gga->num_satellites = (uint8_t)atoi(f);
    }
    if (get_field(sentence, 8U, f, sizeof(f))) {
        gga->hdop = (float)syn_atof(f);
    }
    if (get_field(sentence, 9U, f, sizeof(f))) {
        gga->altitude_m = (float)syn_atof(f);
    }

    gga->valid = true;
    return true;
}

bool syn_nmea_parse_rmc(const char *sentence, SYN_NMEA_RMC *rmc)
{
    SYN_ASSERT(rmc != NULL);
    memset(rmc, 0, sizeof(SYN_NMEA_RMC));

    if (syn_nmea_get_type(sentence) != SYN_NMEA_SENTENCE_RMC) {
        return false;
    }

    char f[16] = {0};
    if (get_field(sentence, 1U, f, sizeof(f))) {
        parse_time(f, &rmc->hours, &rmc->minutes, &rmc->seconds, &rmc->milliseconds);
    }

    if (get_field(sentence, 2U, f, sizeof(f))) {
        rmc->status_valid = (f[0] == 'A');
    }

    char lat_str[16] = {0}, lat_dir[4] = {0}, lon_str[16] = {0}, lon_dir[4] = {0};
    bool has_lat = get_field(sentence, 3U, lat_str, sizeof(lat_str)) &&
                   get_field(sentence, 4U, lat_dir, sizeof(lat_dir));
    bool has_lon = get_field(sentence, 5U, lon_str, sizeof(lon_str)) &&
                   get_field(sentence, 6U, lon_dir, sizeof(lon_dir));

#if SYN_NMEA_USE_FIXED_POINT
    if (has_lat) {
        rmc->lat_udeg = syn_nmea_parse_coord_udeg(lat_str, lat_dir[0]);
    }
    if (has_lon) {
        rmc->lon_udeg = syn_nmea_parse_coord_udeg(lon_str, lon_dir[0]);
    }
#else
    if (has_lat) {
        rmc->latitude = syn_nmea_parse_coord(lat_str, lat_dir[0]);
    }
    if (has_lon) {
        rmc->longitude = syn_nmea_parse_coord(lon_str, lon_dir[0]);
    }
#endif

    if (get_field(sentence, 7U, f, sizeof(f))) {
        rmc->speed_knots = (float)syn_atof(f);
    }
    if (get_field(sentence, 8U, f, sizeof(f))) {
        rmc->course_deg = (float)syn_atof(f);
    }
    if (get_field(sentence, 9U, f, sizeof(f))) {
        parse_rmc_date(f, rmc);
    }

    rmc->valid = true;
    return true;
}

bool syn_nmea_parse_vtg(const char *sentence, SYN_NMEA_VTG *vtg)
{
    SYN_ASSERT(vtg != NULL);
    memset(vtg, 0, sizeof(SYN_NMEA_VTG));

    if (syn_nmea_get_type(sentence) != SYN_NMEA_SENTENCE_VTG) {
        return false;
    }

    char f[16] = {0};
    if (get_field(sentence, 1U, f, sizeof(f))) {
        vtg->course_true_deg = (float)syn_atof(f);
    }
    if (get_field(sentence, 5U, f, sizeof(f))) {
        vtg->speed_knots = (float)syn_atof(f);
    }
    if (get_field(sentence, 7U, f, sizeof(f))) {
        vtg->speed_kph = (float)syn_atof(f);
    }

    vtg->valid = true;
    return true;
}

bool syn_nmea_parse_gsa(const char *sentence, SYN_NMEA_GSA *gsa)
{
    SYN_ASSERT(gsa != NULL);
    memset(gsa, 0, sizeof(SYN_NMEA_GSA));

    if (syn_nmea_get_type(sentence) != SYN_NMEA_SENTENCE_GSA) {
        return false;
    }

    char f[16] = {0};
    if (get_field(sentence, 1U, f, sizeof(f))) {
        gsa->mode = f[0];
    }
    if (get_field(sentence, 2U, f, sizeof(f))) {
        gsa->fix_type = (uint8_t)atoi(f);
    }
    if (get_field(sentence, 15U, f, sizeof(f))) {
        gsa->pdop = (float)syn_atof(f);
    }
    if (get_field(sentence, 16U, f, sizeof(f))) {
        gsa->hdop = (float)syn_atof(f);
    }
    if (get_field(sentence, 17U, f, sizeof(f))) {
        gsa->vdop = (float)syn_atof(f);
    }

    gsa->valid = true;
    return true;
}

bool syn_nmea_parse_zda(const char *sentence, SYN_NMEA_ZDA *zda)
{
    SYN_ASSERT(zda != NULL);
    memset(zda, 0, sizeof(SYN_NMEA_ZDA));

    if (syn_nmea_get_type(sentence) != SYN_NMEA_SENTENCE_ZDA) {
        return false;
    }

    char f[16] = {0};
    if (get_field(sentence, 1U, f, sizeof(f))) {
        parse_time(f, &zda->hours, &zda->minutes, &zda->seconds, &zda->milliseconds);
    }
    if (get_field(sentence, 2U, f, sizeof(f))) {
        zda->day = (uint8_t)atoi(f);
    }
    if (get_field(sentence, 3U, f, sizeof(f))) {
        zda->month = (uint8_t)atoi(f);
    }
    if (get_field(sentence, 4U, f, sizeof(f))) {
        zda->year = (uint16_t)atoi(f);
    }

    zda->valid = true;
    return true;
}

#endif /* SYN_USE_NMEA */
