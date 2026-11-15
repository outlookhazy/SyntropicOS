

# File syn\_nmea.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_nmea.c**](syn__nmea_8c.md)

[Go to the documentation of this file](syn__nmea_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_NMEA) || SYN_USE_NMEA


#include "../util/syn_assert.h"
#include "syn_nmea.h"

#include <stddef.h>
#include <string.h>

static inline bool nmea_is_digit(char c)
{
    return (c >= '0') && (c <= '9');
}

static bool nmea_digits_span_valid(const char *s, size_t len)
{
    for (size_t i = 0U; i < len; i++) {
        if (!nmea_is_digit(s[i])) {
            return false;
        }
    }
    return len > 0U;
}

static bool nmea_parse_u32_bounded(const char *s, uint32_t min_val, uint32_t max_val, uint32_t *out)
{
    if (!nmea_digits_span_valid(s, strlen(s))) {
        return false;
    }

    uint32_t val = 0U;
    for (const char *p = s; *p != '\0'; p++) {
        val = (val * 10U) + (uint32_t)(*p - '0');
        if (val > max_val) {
            return false;
        }
    }

    if (val < min_val) {
        return false;
    }

    *out = val;
    return true;
}

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

static inline bool is_nmea_end(char c)
{
    return (c == '\0') || (c == '*') || (c == '\r') || (c == '\n');
}

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

static const char *nmea_find_valid_star(const char *sentence)
{
    const char *star = strchr(sentence + 1, '*');
    if (star == NULL) {
        return NULL;
    }

    for (const char *p = sentence + 1; p < star; p++) {
        if ((*p == '\r') || (*p == '\n') || (*p == '$')) {
            return NULL;
        }
    }

    return star;
}

bool syn_nmea_validate(const char *sentence)
{
    if ((sentence == NULL) || (sentence[0] != '$')) {
        return false;
    }

    const char *star = nmea_find_valid_star(sentence);
    if (star == NULL) {
        return false;
    }

    uint8_t hi = 0U;
    uint8_t lo = 0U;
    if (!hex_nibble(star[1], &hi) || !hex_nibble(star[2], &lo)) {
        return false;
    }

    for (const char *tail = star + 3; *tail != '\0'; tail++) {
        if ((*tail != '\r') && (*tail != '\n')) {
            return false;
        }
    }

    uint8_t expected_crc = (uint8_t)((hi << 4U) | lo);
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

static bool nmea_parse_coord_parts(const char *nmea_coord, int32_t *out_deg, int32_t *out_min_whole,
                                   const char **out_frac)
{
    if (nmea_coord == NULL) {
        return false;
    }

    const char *s = nmea_coord;
    if (*s == '+') {
        s++;
    }

    const char *dot = strchr(s, '.');
    if (dot == NULL) {
        return false;
    }

    ptrdiff_t pre_len = dot - s;
    if ((pre_len < 3) || (pre_len > 5)) {
        return false;
    }

    if (!nmea_digits_span_valid(s, (size_t)pre_len) ||
        !nmea_digits_span_valid(dot + 1, strlen(dot + 1))) {
        return false;
    }

    ptrdiff_t deg_digits = pre_len - 2;
    int32_t deg = 0;
    for (ptrdiff_t i = 0; i < deg_digits; i++) {
        deg = (deg * 10) + (int32_t)(s[i] - '0');
    }

    int32_t min_whole = ((int32_t)(s[deg_digits] - '0') * 10) + (int32_t)(s[deg_digits + 1] - '0');
    if ((min_whole >= 60) || (deg > 180)) {
        return false;
    }

    *out_deg = deg;
    *out_min_whole = min_whole;
    *out_frac = dot + 1;
    return true;
}

double syn_nmea_parse_coord(const char *nmea_coord, char dir)
{
    int32_t deg = 0;
    int32_t min_whole = 0;
    const char *frac_str = NULL;
    if (!nmea_parse_coord_parts(nmea_coord, &deg, &min_whole, &frac_str)) {
        return 0.0;
    }

    double frac = 0.0;
    double scale = 0.1;
    for (const char *p = frac_str; *p != '\0'; p++) {
        frac += (double)(*p - '0') * scale;
        scale *= 0.1;
    }

    double decimal = (double)deg + (((double)min_whole + frac) / 60.0);
    if (decimal > 180.0) {
        return 0.0;
    }

    if ((dir == 'S') || (dir == 'W')) {
        decimal = -decimal;
    }

    return decimal;
}

#if SYN_NMEA_USE_FIXED_POINT
int32_t syn_nmea_parse_coord_udeg(const char *nmea_coord, char dir)
{
    int32_t deg = 0;
    int32_t min_int = 0;
    const char *p = NULL;
    if (!nmea_parse_coord_parts(nmea_coord, &deg, &min_int, &p)) {
        return 0;
    }

    int64_t min_frac = 0;
    for (int i = 0; i < 4; i++) {
        if (*p != '\0') {
            min_frac = (min_frac * 10LL) + (int64_t)(*p - '0');
            p++;
        } else {
            min_frac = min_frac * 10LL;
        }
    }

    int64_t total_min_scaled = ((int64_t)min_int * 10000LL) + min_frac;
    int64_t min_udeg = (total_min_scaled * 10000000LL) / (60LL * 10000LL);
    int64_t udeg64 = ((int64_t)deg * 10000000LL) + min_udeg;
    if (udeg64 > 1800000000LL) {
        return 0;
    }

    if ((dir == 'S') || (dir == 'W')) {
        udeg64 = -udeg64;
    }
    return (int32_t)udeg64;
}
#endif

static void parse_time(const char *field, uint8_t *h, uint8_t *m, uint8_t *s, uint16_t *ms)
{
    *h = 0U;
    *m = 0U;
    *s = 0U;
    *ms = 0U;

    size_t len = strlen(field);
    if ((len < 6U) || !nmea_digits_span_valid(field, 6U)) {
        return;
    }

    uint8_t hh = (uint8_t)(((field[0] - '0') * 10) + (field[1] - '0'));
    uint8_t mm = (uint8_t)(((field[2] - '0') * 10) + (field[3] - '0'));
    uint8_t ss = (uint8_t)(((field[4] - '0') * 10) + (field[5] - '0'));
    if ((hh >= 24U) || (mm >= 60U) || (ss > 60U)) {
        return;
    }

    uint16_t msec = 0U;
    if (field[6] != '\0') {
        if ((field[6] != '.') || !nmea_digits_span_valid(&field[7], len - 7U)) {
            return;
        }
        uint16_t scale = 100U;
        for (size_t i = 7U; (i < len) && (scale > 0U); i++) {
            msec = (uint16_t)(msec + ((uint16_t)(field[i] - '0') * scale));
            scale /= 10U;
        }
    }

    *h = hh;
    *m = mm;
    *s = ss;
    *ms = msec;
}

static void parse_rmc_date(const char *f, SYN_NMEA_RMC *rmc)
{
    if ((strlen(f) != 6U) || !nmea_digits_span_valid(f, 6U)) {
        return;
    }

    uint8_t dd = (uint8_t)(((f[0] - '0') * 10) + (f[1] - '0'));
    uint8_t mm = (uint8_t)(((f[2] - '0') * 10) + (f[3] - '0'));
    uint16_t yr = (uint16_t)(((f[4] - '0') * 10) + (f[5] - '0'));
    if ((dd < 1U) || (dd > 31U) || (mm < 1U) || (mm > 12U)) {
        return;
    }

    rmc->day = dd;
    rmc->month = mm;
    rmc->year = (yr >= 70U) ? (uint16_t)(1900U + yr) : (uint16_t)(2000U + yr);
}

static void nmea_gga_parse_metrics(const char *sentence, SYN_NMEA_GGA *gga)
{
    char f[16] = {0};
    uint32_t val = 0U;

    if (get_field(sentence, 6U, f, sizeof(f)) && nmea_parse_u32_bounded(f, 0U, 8U, &val)) {
        gga->fix_quality = (SYN_NMEA_FixQuality)val;
    }
    if (get_field(sentence, 7U, f, sizeof(f)) && nmea_parse_u32_bounded(f, 0U, 255U, &val)) {
        gga->num_satellites = (uint8_t)val;
    }
    if (get_field(sentence, 8U, f, sizeof(f))) {
        gga->hdop = (float)syn_atof(f);
    }
    if (get_field(sentence, 9U, f, sizeof(f))) {
        gga->altitude_m = (float)syn_atof(f);
    }
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

    nmea_gga_parse_metrics(sentence, gga);

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
    uint32_t val = 0U;
    if (get_field(sentence, 1U, f, sizeof(f))) {
        gsa->mode = f[0];
    }
    if (get_field(sentence, 2U, f, sizeof(f)) && nmea_parse_u32_bounded(f, 1U, 3U, &val)) {
        gsa->fix_type = (uint8_t)val;
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
    uint32_t val = 0U;
    if (get_field(sentence, 1U, f, sizeof(f))) {
        parse_time(f, &zda->hours, &zda->minutes, &zda->seconds, &zda->milliseconds);
    }
    if (get_field(sentence, 2U, f, sizeof(f)) && nmea_parse_u32_bounded(f, 1U, 31U, &val)) {
        zda->day = (uint8_t)val;
    }
    if (get_field(sentence, 3U, f, sizeof(f)) && nmea_parse_u32_bounded(f, 1U, 12U, &val)) {
        zda->month = (uint8_t)val;
    }
    if (get_field(sentence, 4U, f, sizeof(f)) && nmea_parse_u32_bounded(f, 1900U, 9999U, &val)) {
        zda->year = (uint16_t)val;
    }

    zda->valid = true;
    return true;
}

#endif /* SYN_USE_NMEA */
```


