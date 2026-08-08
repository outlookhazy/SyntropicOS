

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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static double syn_atof(const char *s)
{
    /* LCOV_EXCL_START: Defensive NULL/empty check */
    if (s == NULL || *s == '\0')
        return 0.0;
    /* LCOV_EXCL_STOP */
    double res = 0.0;
    double sign = 1.0;
    if (*s == '-') {
        sign = -1.0;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while (*s >= '0' && *s <= '9') {
        res = res * 10.0 + (double)(*s - '0');
        s++;
    }
    if (*s == '.') {
        s++;
        double frac = 0.1;
        while (*s >= '0' && *s <= '9') {
            res += (double)(*s - '0') * frac;
            frac *= 0.1;
            s++;
        }
    }
    return res * sign;
}

static bool get_field(const char *sentence, uint8_t index, char *dest, size_t max_len)
{
    if (sentence == NULL || dest == NULL || max_len == 0)
        return false; /* LCOV_EXCL_LINE: Unreachable guard; callers pass non-NULL valid stack
                         buffers */
    dest[0] = '\0';

    const char *p = sentence;
    if (*p == '$')
        p++;

    uint8_t current_field = 0;
    while (*p && *p != '*' && *p != '\r' && *p != '\n') {
        if (*p == ',') {
            current_field++;
            p++;
            if (current_field > index)
                break; /* LCOV_EXCL_LINE: Early termination guard when field index is exceeded */
            continue;
        }

        if (current_field == index) {
            size_t len = 0;
            while (*p && *p != ',' && *p != '*' && *p != '\r' && *p != '\n' && len < max_len - 1) {
                dest[len++] = *p++;
            }
            dest[len] = '\0';
            return (len > 0);
        }

        p++;
    }

    return false; /* LCOV_EXCL_LINE: Fallback return when requested field index exceeds total
                     sentence comma count */
}

static uint8_t hex2dec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f')
        return 10 + (c - 'a');
    return 0;
}

void syn_nmea_parser_init(SYN_NMEA_Parser *parser)
{
    SYN_ASSERT(parser != NULL);
    parser->pos = 0;
    parser->in_sentence = false;
    parser->buf[0] = '\0';
}

bool syn_nmea_parser_feed(SYN_NMEA_Parser *parser, char byte, char *out_sentence)
{
    SYN_ASSERT(parser != NULL);

    if (byte == '$') {
        parser->pos = 0;
        parser->buf[parser->pos++] = '$';
        parser->in_sentence = true;
        return false;
    }

    if (!parser->in_sentence)
        return false;

    if (byte == '\n' || byte == '\r') {
        if (parser->pos > 6) {
            parser->buf[parser->pos] = '\0';
            if (syn_nmea_validate(parser->buf)) {
                if (out_sentence != NULL) {
                    size_t len = strlen(parser->buf);
                    memcpy(out_sentence, parser->buf, len + 1);
                }
                parser->in_sentence = false;
                parser->pos = 0;
                return true;
            }
        }
        parser->in_sentence = false;
        parser->pos = 0;
        return false;
    }

    if (parser->pos < SYN_NMEA_MAX_SENTENCE_LEN) {
        parser->buf[parser->pos++] = byte;
    } else {
        parser->in_sentence = false;
        parser->pos = 0;
    }

    return false;
}

uint8_t syn_nmea_checksum(const char *sentence)
{
    if (sentence == NULL)
        return 0;
    const char *p = sentence;
    if (*p == '$')
        p++;

    uint8_t checksum = 0;
    while (*p && *p != '*' && *p != '\r' && *p != '\n') {
        checksum ^= (uint8_t)(*p++);
    }

    return checksum;
}

bool syn_nmea_validate(const char *sentence)
{
    if (sentence == NULL || sentence[0] != '$')
        return false;

    const char *star = strchr(sentence, '*');
    if (star == NULL || strlen(star) < 3)
        return false;

    uint8_t expected_crc = (uint8_t)((hex2dec(star[1]) << 4) | hex2dec(star[2]));
    uint8_t calc_crc = syn_nmea_checksum(sentence);

    return (expected_crc == calc_crc);
}

SYN_NMEA_SentenceType syn_nmea_get_type(const char *sentence)
{
    if (!syn_nmea_validate(sentence))
        return SYN_NMEA_SENTENCE_UNKNOWN;

    char talker[10];
    if (!get_field(sentence, 0, talker, sizeof(talker)))
        return SYN_NMEA_SENTENCE_UNKNOWN; /* LCOV_EXCL_LINE: Unreachable guard; sentence was
                                             validated non-empty above */

    size_t len = strlen(talker);
    if (len < 3)
        return SYN_NMEA_SENTENCE_UNKNOWN;
    const char *type_str = &talker[len - 3];

    if (strcmp(type_str, "GGA") == 0)
        return SYN_NMEA_SENTENCE_GGA;
    if (strcmp(type_str, "RMC") == 0)
        return SYN_NMEA_SENTENCE_RMC;
    if (strcmp(type_str, "VTG") == 0)
        return SYN_NMEA_SENTENCE_VTG;
    if (strcmp(type_str, "GSA") == 0)
        return SYN_NMEA_SENTENCE_GSA;
    if (strcmp(type_str, "ZDA") == 0)
        return SYN_NMEA_SENTENCE_ZDA;

    return SYN_NMEA_SENTENCE_UNKNOWN;
}

double syn_nmea_parse_coord(const char *nmea_coord, char dir)
{
    if (nmea_coord == NULL || strlen(nmea_coord) < 4)
        return 0.0;

    double val = syn_atof(nmea_coord);
    double deg = (double)((int)(val / 100.0));
    double minutes = val - (deg * 100.0);
    double decimal = deg + (minutes / 60.0);

    if (dir == 'S' || dir == 'W') {
        decimal = -decimal;
    }

    return decimal;
}

#if SYN_NMEA_USE_FIXED_POINT
int32_t syn_nmea_parse_coord_udeg(const char *nmea_coord, char dir)
{
    if (nmea_coord == NULL || strlen(nmea_coord) < 4)
        return 0;

    const char *dot = strchr(nmea_coord, '.');
    if (!dot || (dot - nmea_coord) < 3)
        return 0;

    int deg_digits = (int)(dot - nmea_coord) - 2;
    int32_t deg = 0;
    for (int i = 0; i < deg_digits; i++) {
        deg = deg * 10 + (nmea_coord[i] - '0');
    }

    int32_t min_int = (nmea_coord[deg_digits] - '0') * 10 + (nmea_coord[deg_digits + 1] - '0');

    int32_t min_frac = 0;
    const char *p = dot + 1;
    for (int i = 0; i < 4; i++) {
        if (*p >= '0' && *p <= '9') {
            min_frac = min_frac * 10 + (*p - '0');
            p++;
        } else {
            min_frac = min_frac * 10;
        }
    }

    int64_t total_min_scaled = (int64_t)min_int * 10000LL + min_frac;
    int64_t min_udeg = (total_min_scaled * 10000000LL) / (60LL * 10000LL);

    int32_t udeg = (deg * 10000000) + (int32_t)min_udeg;
    if (dir == 'S' || dir == 'W') {
        udeg = -udeg;
    }
    return udeg;
}
#endif

static void parse_time(const char *field, uint8_t *h, uint8_t *m, uint8_t *s, uint16_t *ms)
{
    *h = 0;
    *m = 0;
    *s = 0;
    *ms = 0;
    if (field == NULL || field[0] == '\0' || field[1] == '\0' || field[2] == '\0' ||
        field[3] == '\0' || field[4] == '\0' || field[5] == '\0') {
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

bool syn_nmea_parse_gga(const char *sentence, SYN_NMEA_GGA *gga)
{
    SYN_ASSERT(gga != NULL);
    memset(gga, 0, sizeof(SYN_NMEA_GGA));

    if (syn_nmea_get_type(sentence) != SYN_NMEA_SENTENCE_GGA)
        return false;

    char f[16];
    if (get_field(sentence, 1, f, sizeof(f)))
        parse_time(f, &gga->hours, &gga->minutes, &gga->seconds, &gga->milliseconds);

    char lat_str[16], lat_dir[4], lon_str[16], lon_dir[4];
    bool has_lat = get_field(sentence, 2, lat_str, sizeof(lat_str)) &&
                   get_field(sentence, 3, lat_dir, sizeof(lat_dir));
    bool has_lon = get_field(sentence, 4, lon_str, sizeof(lon_str)) &&
                   get_field(sentence, 5, lon_dir, sizeof(lon_dir));

#if SYN_NMEA_USE_FIXED_POINT
    if (has_lat)
        gga->lat_udeg = syn_nmea_parse_coord_udeg(lat_str, lat_dir[0]);
    if (has_lon)
        gga->lon_udeg = syn_nmea_parse_coord_udeg(lon_str, lon_dir[0]);
#else
    if (has_lat)
        gga->latitude = syn_nmea_parse_coord(lat_str, lat_dir[0]);
    if (has_lon)
        gga->longitude = syn_nmea_parse_coord(lon_str, lon_dir[0]);
#endif

    if (get_field(sentence, 6, f, sizeof(f)))
        gga->fix_quality = (SYN_NMEA_FixQuality)atoi(f);
    if (get_field(sentence, 7, f, sizeof(f)))
        gga->num_satellites = (uint8_t)atoi(f);
    if (get_field(sentence, 8, f, sizeof(f)))
        gga->hdop = (float)syn_atof(f);
    if (get_field(sentence, 9, f, sizeof(f)))
        gga->altitude_m = (float)syn_atof(f);

    gga->valid = true;
    return true;
}

bool syn_nmea_parse_rmc(const char *sentence, SYN_NMEA_RMC *rmc)
{
    SYN_ASSERT(rmc != NULL);
    memset(rmc, 0, sizeof(SYN_NMEA_RMC));

    if (syn_nmea_get_type(sentence) != SYN_NMEA_SENTENCE_RMC)
        return false;

    char f[16];
    if (get_field(sentence, 1, f, sizeof(f)))
        parse_time(f, &rmc->hours, &rmc->minutes, &rmc->seconds, &rmc->milliseconds);

    if (get_field(sentence, 2, f, sizeof(f)))
        rmc->status_valid = (f[0] == 'A');

    char lat_str[16], lat_dir[4], lon_str[16], lon_dir[4];
    bool has_lat = get_field(sentence, 3, lat_str, sizeof(lat_str)) &&
                   get_field(sentence, 4, lat_dir, sizeof(lat_dir));
    bool has_lon = get_field(sentence, 5, lon_str, sizeof(lon_str)) &&
                   get_field(sentence, 6, lon_dir, sizeof(lon_dir));

#if SYN_NMEA_USE_FIXED_POINT
    if (has_lat)
        rmc->lat_udeg = syn_nmea_parse_coord_udeg(lat_str, lat_dir[0]);
    if (has_lon)
        rmc->lon_udeg = syn_nmea_parse_coord_udeg(lon_str, lon_dir[0]);
#else
    if (has_lat)
        rmc->latitude = syn_nmea_parse_coord(lat_str, lat_dir[0]);
    if (has_lon)
        rmc->longitude = syn_nmea_parse_coord(lon_str, lon_dir[0]);
#endif

    if (get_field(sentence, 7, f, sizeof(f)))
        rmc->speed_knots = (float)syn_atof(f);
    if (get_field(sentence, 8, f, sizeof(f)))
        rmc->course_deg = (float)syn_atof(f);

    if (get_field(sentence, 9, f, sizeof(f)) && strlen(f) >= 6) {
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
        rmc->year = (yr >= 70) ? (uint16_t)(1900 + yr) : (uint16_t)(2000 + yr);
    }

    rmc->valid = true;
    return true;
}

bool syn_nmea_parse_vtg(const char *sentence, SYN_NMEA_VTG *vtg)
{
    SYN_ASSERT(vtg != NULL);
    memset(vtg, 0, sizeof(SYN_NMEA_VTG));

    if (syn_nmea_get_type(sentence) != SYN_NMEA_SENTENCE_VTG)
        return false;

    char f[16];
    if (get_field(sentence, 1, f, sizeof(f)))
        vtg->course_true_deg = (float)syn_atof(f);
    if (get_field(sentence, 5, f, sizeof(f)))
        vtg->speed_knots = (float)syn_atof(f);
    if (get_field(sentence, 7, f, sizeof(f)))
        vtg->speed_kph = (float)syn_atof(f);

    vtg->valid = true;
    return true;
}

bool syn_nmea_parse_gsa(const char *sentence, SYN_NMEA_GSA *gsa)
{
    SYN_ASSERT(gsa != NULL);
    memset(gsa, 0, sizeof(SYN_NMEA_GSA));

    if (syn_nmea_get_type(sentence) != SYN_NMEA_SENTENCE_GSA)
        return false;

    char f[16];
    if (get_field(sentence, 1, f, sizeof(f)))
        gsa->mode = f[0];
    if (get_field(sentence, 2, f, sizeof(f)))
        gsa->fix_type = (uint8_t)atoi(f);
    if (get_field(sentence, 15, f, sizeof(f)))
        gsa->pdop = (float)syn_atof(f);
    if (get_field(sentence, 16, f, sizeof(f)))
        gsa->hdop = (float)syn_atof(f);
    if (get_field(sentence, 17, f, sizeof(f)))
        gsa->vdop = (float)syn_atof(f);

    gsa->valid = true;
    return true;
}

bool syn_nmea_parse_zda(const char *sentence, SYN_NMEA_ZDA *zda)
{
    SYN_ASSERT(zda != NULL);
    memset(zda, 0, sizeof(SYN_NMEA_ZDA));

    if (syn_nmea_get_type(sentence) != SYN_NMEA_SENTENCE_ZDA)
        return false;

    char f[16];
    if (get_field(sentence, 1, f, sizeof(f)))
        parse_time(f, &zda->hours, &zda->minutes, &zda->seconds, &zda->milliseconds);
    if (get_field(sentence, 2, f, sizeof(f)))
        zda->day = (uint8_t)atoi(f);
    if (get_field(sentence, 3, f, sizeof(f)))
        zda->month = (uint8_t)atoi(f);
    if (get_field(sentence, 4, f, sizeof(f)))
        zda->year = (uint16_t)atoi(f);

    zda->valid = true;
    return true;
}

#endif /* SYN_USE_NMEA */
```


