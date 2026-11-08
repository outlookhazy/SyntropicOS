#include "syntropic/proto/syn_nmea.h"
#include "unity/unity.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

void test_nmea_checksum_and_validate(void)
{
    const char *gga = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    TEST_ASSERT_TRUE(syn_nmea_validate(gga));
    TEST_ASSERT_EQUAL_HEX8(0x47, syn_nmea_checksum(gga));

    /* Tampered string */
    const char *bad = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*48";
    TEST_ASSERT_FALSE(syn_nmea_validate(bad));

    /* Test parser feed validation failure (lines 114-116 in syn_nmea.c) */
    SYN_NMEA_Parser parser;
    syn_nmea_parser_init(&parser);
    const char *invalid_feed = "$BAD*00\n";
    for (size_t i = 0; i < strlen(invalid_feed); i++) {
        TEST_ASSERT_FALSE(syn_nmea_parser_feed(&parser, invalid_feed[i], NULL));
    }

    /* Test unknown sentence type (line 185 in syn_nmea.c) */
    const char *gsv = "GPGSV,1,1,00";
    char gsv_str[64];
    snprintf(gsv_str, sizeof(gsv_str), "$%s*%02X", gsv, syn_nmea_checksum(gsv));
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type(gsv_str));

    /* Test short time string in parse_gga (line 221 in syn_nmea.c) */
    SYN_NMEA_GGA gga_short;
    const char *short_time = "GPGGA,12,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,";
    char short_time_str[128];
    snprintf(short_time_str, sizeof(short_time_str), "$%s*%02X", short_time,
             syn_nmea_checksum(short_time));
    TEST_ASSERT_TRUE(syn_nmea_parse_gga(short_time_str, &gga_short));
}

void test_nmea_coord_parsing(void)
{
    /* 48 deg 07.038 min N -> 48 + 7.038/60 = 48.1173 */
    double lat = syn_nmea_parse_coord("4807.038", 'N');
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 48.1173, lat);

    /* 011 deg 31.000 min W -> -(11 + 31/60) = -11.516666 */
    double lon = syn_nmea_parse_coord("01131.000", 'W');
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, -11.516666, lon);

    /* NULL and empty string test for syn_atof / syn_nmea_parse_coord (line 28) */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord(NULL, 'N'));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord("", 'N'));

    (void)lat;
    (void)lon;
}

void test_nmea_parse_gga(void)
{
    char gga_str[128];
    const char *payload = "GPGGA,123519.50,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,";
    snprintf(gga_str, sizeof(gga_str), "$%s*%02X", payload, syn_nmea_checksum(payload));

    SYN_NMEA_GGA gga;

    TEST_ASSERT_TRUE(syn_nmea_parse_gga(gga_str, &gga));
    TEST_ASSERT_TRUE(gga.valid);
    TEST_ASSERT_EQUAL(12, gga.hours);
    TEST_ASSERT_EQUAL(35, gga.minutes);
    TEST_ASSERT_EQUAL(19, gga.seconds);
    TEST_ASSERT_EQUAL(500, gga.milliseconds);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 48.1173, gga.latitude);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 11.5166, gga.longitude);
    TEST_ASSERT_EQUAL(SYN_NMEA_FIX_GPS, gga.fix_quality);
    TEST_ASSERT_EQUAL(8, gga.num_satellites);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.9f, gga.hdop);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 545.4f, gga.altitude_m);

    /* Positive (+, line 35), negative (-, lines 31-33), and empty (line 28) altitude */
    const char *payload_pos = "GPGGA,123519.50,4807.038,N,01131.000,E,1,08,0.9,+545.4,M,46.9,M,,";
    snprintf(gga_str, sizeof(gga_str), "$%s*%02X", payload_pos, syn_nmea_checksum(payload_pos));
    TEST_ASSERT_TRUE(syn_nmea_parse_gga(gga_str, &gga));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 545.4f, gga.altitude_m);

    const char *payload_neg = "GPGGA,123519.50,4807.038,N,01131.000,E,1,08,0.9,-545.4,M,46.9,M,,";
    snprintf(gga_str, sizeof(gga_str), "$%s*%02X", payload_neg, syn_nmea_checksum(payload_neg));
    TEST_ASSERT_TRUE(syn_nmea_parse_gga(gga_str, &gga));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, -545.4f, gga.altitude_m);

    const char *payload_empty_alt = "GPGGA,123519.50,4807.038,N,01131.000,E,1,08,,,M,46.9,M,,";
    uint8_t cs = syn_nmea_checksum(payload_empty_alt);
    snprintf(gga_str, sizeof(gga_str), "$%s*%02X", payload_empty_alt, cs);
    TEST_ASSERT_TRUE(syn_nmea_parse_gga(gga_str, &gga));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, gga.hdop);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, gga.altitude_m);
}

void test_nmea_parse_rmc(void)
{
    char rmc_str[128];
    const char *payload = "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W";
    snprintf(rmc_str, sizeof(rmc_str), "$%s*%02X", payload, syn_nmea_checksum(payload));

    SYN_NMEA_RMC rmc;

    TEST_ASSERT_TRUE(syn_nmea_parse_rmc(rmc_str, &rmc));
    TEST_ASSERT_TRUE(rmc.valid);
    TEST_ASSERT_TRUE(rmc.status_valid);
    TEST_ASSERT_EQUAL(12, rmc.hours);
    TEST_ASSERT_EQUAL(35, rmc.minutes);
    TEST_ASSERT_EQUAL(19, rmc.seconds);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 22.4f, rmc.speed_knots);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 84.4f, rmc.course_deg);
    TEST_ASSERT_EQUAL(23, rmc.day);
    TEST_ASSERT_EQUAL(3, rmc.month);
    TEST_ASSERT_EQUAL(1994, rmc.year);
}

void test_nmea_parse_vtg_gsa_zda(void)
{
    char str[128];
    const char *vtg_p = "GPVTG,054.7,T,034.4,M,005.5,N,010.2,K";
    snprintf(str, sizeof(str), "$%s*%02X", vtg_p, syn_nmea_checksum(vtg_p));
    SYN_NMEA_VTG vtg;
    TEST_ASSERT_TRUE(syn_nmea_parse_vtg(str, &vtg));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 54.7f, vtg.course_true_deg);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 5.5f, vtg.speed_knots);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.2f, vtg.speed_kph);

    const char *gsa_p = "GPGSA,A,3,04,05,09,12,14,17,20,24,,,,,2.5,1.3,2.1";
    snprintf(str, sizeof(str), "$%s*%02X", gsa_p, syn_nmea_checksum(gsa_p));
    SYN_NMEA_GSA gsa;
    TEST_ASSERT_TRUE(syn_nmea_parse_gsa(str, &gsa));
    TEST_ASSERT_EQUAL('A', gsa.mode);
    TEST_ASSERT_EQUAL(3, gsa.fix_type);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 2.5f, gsa.pdop);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.3f, gsa.hdop);

    const char *zda_p = "GPZDA,201530.00,04,07,2026,00,00";
    snprintf(str, sizeof(str), "$%s*%02X", zda_p, syn_nmea_checksum(zda_p));
    SYN_NMEA_ZDA zda;
    TEST_ASSERT_TRUE(syn_nmea_parse_zda(str, &zda));
    TEST_ASSERT_EQUAL(20, zda.hours);
    TEST_ASSERT_EQUAL(15, zda.minutes);
    TEST_ASSERT_EQUAL(4, zda.day);
    TEST_ASSERT_EQUAL(7, zda.month);
    TEST_ASSERT_EQUAL(2026, zda.year);
}

void test_nmea_streaming_parser(void)
{
    SYN_NMEA_Parser parser;
    syn_nmea_parser_init(&parser);

    const char *sentence = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    char out_sentence[128];
    bool got_frame = false;

    for (size_t i = 0; i < strlen(sentence); i++) {
        if (syn_nmea_parser_feed(&parser, sentence[i], out_sentence)) {
            got_frame = true;
        }
    }

    TEST_ASSERT_TRUE(got_frame);
    TEST_ASSERT_EQUAL_STRING("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47",
                             out_sentence);
}

static void format_nmea(const char *payload, char *out, size_t out_size)
{
    uint8_t crc = syn_nmea_checksum(payload);
    snprintf(out, out_size, "%s*%02X\r\n", payload, crc);
}

void test_nmea_edge_cases(void)
{
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type(NULL));
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type("invalid"));
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type("$GPXYZ,1,2,3*00"));

    SYN_NMEA_GGA gga;
    SYN_NMEA_RMC rmc;

    TEST_ASSERT_FALSE(syn_nmea_parse_gga(NULL, &gga));
    TEST_ASSERT_FALSE(syn_nmea_parse_rmc(NULL, &rmc));

    TEST_ASSERT_FALSE(syn_nmea_parse_gga("$GPRMC,1,2,3*00", &gga));
    TEST_ASSERT_FALSE(syn_nmea_parse_rmc("$GPGGA,1,2,3*00", &rmc));

    char sentence_buf[128];

    SYN_NMEA_VTG vtg;
    TEST_ASSERT_FALSE(syn_nmea_parse_vtg("$GPGGA,1,2,3*00", &vtg));
    format_nmea("$GPVTG,054.7,T,,M,005.5,N,010.2,K", sentence_buf, sizeof(sentence_buf));
    TEST_ASSERT_TRUE(syn_nmea_parse_vtg(sentence_buf, &vtg));
    TEST_ASSERT_TRUE(vtg.valid);

    SYN_NMEA_GSA gsa;
    TEST_ASSERT_FALSE(syn_nmea_parse_gsa("$GPGGA,1,2,3*00", &gsa));
    format_nmea("$GPGSA,A,3,01,02,03,,,,,,,,,,2.5,1.3,2.1", sentence_buf, sizeof(sentence_buf));
    TEST_ASSERT_TRUE(syn_nmea_parse_gsa(sentence_buf, &gsa));
    TEST_ASSERT_TRUE(gsa.valid);

    SYN_NMEA_ZDA zda;
    TEST_ASSERT_FALSE(syn_nmea_parse_zda("$GPGGA,1,2,3*00", &zda));
    format_nmea("$GPZDA,160012.00,09,03,2026,00,00", sentence_buf, sizeof(sentence_buf));
    TEST_ASSERT_TRUE(syn_nmea_parse_zda(sentence_buf, &zda));
    TEST_ASSERT_TRUE(zda.valid);
}

static void test_nmea_lowercase_checksum_and_overflow(void)
{
    /* Lowercase checksum hex 'a'-'f' */
    const char *gga_lc = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*4a";
    TEST_ASSERT_FALSE(syn_nmea_validate(gga_lc));

    /* Sentence buffer overflow */
    SYN_NMEA_Parser parser;
    syn_nmea_parser_init(&parser);
    syn_nmea_parser_feed(&parser, '$', NULL);
    for (int i = 0; i < 150; i++) {
        syn_nmea_parser_feed(&parser, 'A', NULL);
    }
    TEST_ASSERT_FALSE(parser.in_sentence);
}

static void test_nmea_checksum_null_and_short_stars(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, syn_nmea_checksum(NULL));
    TEST_ASSERT_FALSE(syn_nmea_validate(NULL));
    TEST_ASSERT_FALSE(syn_nmea_validate("GPGGA,123456*47")); /* Missing leading $ */
    TEST_ASSERT_FALSE(syn_nmea_validate("$GPGGA,123456*4")); /* Short star string < 3 chars */

    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type(NULL));
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN,
                      syn_nmea_get_type("$XX*00")); /* Length < 3 after talker */

    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord(NULL, 'N'));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord("12", 'N')); /* Length < 4 */
}

static void test_nmea_sentence_checksum_and_field_truncation(void)
{
    /* 1. Valid sentence with lowercase hex checksum */
    const char *valid_lc = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6a";
    TEST_ASSERT_TRUE(syn_nmea_validate(valid_lc));

    /* 2. get_type on short talker (<3 chars) */
    const char *short_talker = "$AB*07";
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type(short_talker));

    /* 3. Non-hex char in checksum */
    const char *non_hex = "$GPGGA*ZZ";
    TEST_ASSERT_FALSE(syn_nmea_validate(non_hex));

    /* 4. Parse coordinate with South direction and invalid direction fallback */
    double lat_s = syn_nmea_parse_coord("4807.038", 'S');
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, -48.1173, lat_s);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 12.576, syn_nmea_parse_coord("1234.56", 'X'));

    /* 5. Streaming parser feed with NULL out_sentence pointer */
    SYN_NMEA_Parser parser;
    syn_nmea_parser_init(&parser);
    const char *frame = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    bool parsed = false;
    for (size_t i = 0; i < strlen(frame); i++) {
        if (syn_nmea_parser_feed(&parser, frame[i], NULL)) {
            parsed = true;
        }
    }
    TEST_ASSERT_TRUE(parsed);
}

static void test_nmea_date_time_parsing_and_null_coordinates(void)
{
    char sentence_buf[128];

    /* 1. Feed parser invalid checksum sentence with \r\n and pos > 6 */
    SYN_NMEA_Parser parser;
    syn_nmea_parser_init(&parser);
    const char *bad_cs_sentence =
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*00\r\n";
    for (size_t i = 0; i < strlen(bad_cs_sentence); i++) {
        syn_nmea_parser_feed(&parser, bad_cs_sentence[i], sentence_buf);
    }
    TEST_ASSERT_FALSE(parser.in_sentence);

    /* 2. Feed parser short sentence with \r\n (pos <= 6) */
    syn_nmea_parser_init(&parser);
    const char *short_sentence = "$GP*00\r\n";
    for (size_t i = 0; i < strlen(short_sentence); i++) {
        syn_nmea_parser_feed(&parser, short_sentence[i], sentence_buf);
    }

    /* 3. Feed parser '$' while already in sentence */
    syn_nmea_parser_init(&parser);
    syn_nmea_parser_feed(&parser, '$', NULL);
    syn_nmea_parser_feed(&parser, 'G', NULL);
    syn_nmea_parser_feed(&parser, '$', NULL); /* Resets pos */
    TEST_ASSERT_EQUAL(1, parser.pos);

    /* 4. RMC with status 'V' (Invalid) and 2-digit year < 70 (e.g. 23 -> 2023) */
    SYN_NMEA_RMC rmc;
    const char *rmc_v_p = "GPRMC,123519,V,,,,,022.4,084.4,230323,003.1,W";
    snprintf(sentence_buf, sizeof(sentence_buf), "$%s*%02X", rmc_v_p, syn_nmea_checksum(rmc_v_p));
    TEST_ASSERT_TRUE(syn_nmea_parse_rmc(sentence_buf, &rmc));
    TEST_ASSERT_FALSE(rmc.status_valid);
    TEST_ASSERT_EQUAL(2023, rmc.year);

    /* 5. GSA with vdop (field 17) */
    SYN_NMEA_GSA gsa;
    const char *gsa_vdop_p = "GPGSA,A,3,01,02,03,,,,,,,,,,2.5,1.3,2.1";
    snprintf(sentence_buf, sizeof(sentence_buf), "$%s*%02X", gsa_vdop_p,
             syn_nmea_checksum(gsa_vdop_p));
    TEST_ASSERT_TRUE(syn_nmea_parse_gsa(sentence_buf, &gsa));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 2.1f, gsa.vdop);

    /* 6. GGA without latitude and longitude fields */
    SYN_NMEA_GGA gga_no_ll;
    const char *gga_no_ll_p = "GPGGA,123519,,,,,,0,08,0.9,545.4,M,46.9,M,,";
    snprintf(sentence_buf, sizeof(sentence_buf), "$%s*%02X", gga_no_ll_p,
             syn_nmea_checksum(gga_no_ll_p));
    TEST_ASSERT_TRUE(syn_nmea_parse_gga(sentence_buf, &gga_no_ll));

    /* 7. RMC without latitude/longitude and short date field (<6 chars) */
    SYN_NMEA_RMC rmc_short_date;
    const char *rmc_sd_p = "GPRMC,123519,A,,,,,,022.4,084.4,123,003.1,W";
    snprintf(sentence_buf, sizeof(sentence_buf), "$%s*%02X", rmc_sd_p, syn_nmea_checksum(rmc_sd_p));
    TEST_ASSERT_TRUE(syn_nmea_parse_rmc(sentence_buf, &rmc_short_date));
}

void test_nmea_branch_coverage(void)
{
    /* 1. RMC year parsing: yr < 70 -> 20xx, yr >= 70 -> 19xx */
    SYN_NMEA_RMC rmc;
    const char *payload_2000 = "GPRMC,123519.50,A,4807.038,N,01131.000,E,022.4,084.4,230324,,,";
    char rmc_str[128];
    snprintf(rmc_str, sizeof(rmc_str), "$%s*%02X", payload_2000, syn_nmea_checksum(payload_2000));
    TEST_ASSERT_TRUE(syn_nmea_parse_rmc(rmc_str, &rmc));
    TEST_ASSERT_EQUAL_UINT16(2024, rmc.year);

    const char *payload_1990 = "GPRMC,123519.50,A,4807.038,N,01131.000,E,022.4,084.4,230395,,,";
    snprintf(rmc_str, sizeof(rmc_str), "$%s*%02X", payload_1990, syn_nmea_checksum(payload_1990));
    TEST_ASSERT_TRUE(syn_nmea_parse_rmc(rmc_str, &rmc));
    TEST_ASSERT_EQUAL_UINT16(1995, rmc.year);

    /* 2. Parser buffer overflow test (exceeding SYN_NMEA_MAX_SENTENCE_LEN = 128) */
    SYN_NMEA_Parser parser;
    syn_nmea_parser_init(&parser);
    TEST_ASSERT_FALSE(syn_nmea_parser_feed(&parser, '$', NULL));
    for (int i = 0; i < 140; i++) {
        TEST_ASSERT_FALSE(syn_nmea_parser_feed(&parser, 'A', NULL));
    }

    /* 3. Short talker ID (< 3 chars) in validate/get_type */
    const char *short_talker = "AB,1,2";
    char short_str[64];
    snprintf(short_str, sizeof(short_str), "$%s*%02X", short_talker,
             syn_nmea_checksum(short_talker));
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type(short_str));

    /* 4. Lowercase hex characters in checksum validation ('a'-'f') */
    const char *hex_lower = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    char hex_lower_str[128];
    snprintf(hex_lower_str, sizeof(hex_lower_str), "%s", hex_lower);
    /* Replace uppercase hex with lowercase '4' '7' */
    hex_lower_str[strlen(hex_lower_str) - 2] = '4';
    hex_lower_str[strlen(hex_lower_str) - 1] = '7';
    TEST_ASSERT_TRUE(syn_nmea_validate(hex_lower_str));

    /* 5. Positive sign '+' prefixed numerical fields in syn_atof */
    SYN_NMEA_GGA gga_plus;
    const char *payload_plus = "GPGGA,123519,+4807.038,N,01131.000,E,1,08,+0.9,+545.4,M,46.9,M,,";
    char gga_plus_str[128];
    snprintf(gga_plus_str, sizeof(gga_plus_str), "$%s*%02X", payload_plus,
             syn_nmea_checksum(payload_plus));
    TEST_ASSERT_TRUE(syn_nmea_parse_gga(gga_plus_str, &gga_plus));
    TEST_ASSERT_FLOAT_WITHIN(0.01, 545.4, gga_plus.altitude_m);

    /* 6. Checksum NULL and invalid star length */
    TEST_ASSERT_EQUAL_UINT8(0, syn_nmea_checksum(NULL));
    TEST_ASSERT_FALSE(syn_nmea_validate(NULL));
    TEST_ASSERT_FALSE(syn_nmea_validate("GPGGA,123519*47"));
    TEST_ASSERT_FALSE(syn_nmea_validate("$GPGGA,123519"));
    TEST_ASSERT_FALSE(syn_nmea_validate("$GPGGA,123519*4"));

    /* 7. RMC 1900s year parsing (year >= 70) */
    SYN_NMEA_RMC rmc_1995;
    const char *payload_1995 = "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230395,003.1,W";
    char rmc_1995_str[128];
    snprintf(rmc_1995_str, sizeof(rmc_1995_str), "$%s*%02X", payload_1995,
             syn_nmea_checksum(payload_1995));
    TEST_ASSERT_TRUE(syn_nmea_parse_rmc(rmc_1995_str, &rmc_1995));
    TEST_ASSERT_EQUAL_UINT16(1995, rmc_1995.year);

    /* 8. Sentence parser type mismatch checks */
    SYN_NMEA_VTG vtg_dummy;
    SYN_NMEA_GSA gsa_dummy;
    SYN_NMEA_ZDA zda_dummy;
    TEST_ASSERT_FALSE(syn_nmea_parse_vtg(gga_plus_str, &vtg_dummy));
    TEST_ASSERT_FALSE(syn_nmea_parse_gsa(gga_plus_str, &gsa_dummy));
    TEST_ASSERT_FALSE(syn_nmea_parse_zda(gga_plus_str, &zda_dummy));

    /* 9. Coordinate parsing 'S' and 'W' direction and short string */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord("12", 'N'));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, -48.1173, syn_nmea_parse_coord("4807.038", 'W'));

    /* 11. Empty field sentence tests for VTG, GSA, ZDA, GGA, RMC */
    SYN_NMEA_GGA gga_empty;
    SYN_NMEA_RMC rmc_empty;
    SYN_NMEA_VTG vtg_empty;
    SYN_NMEA_GSA gsa_empty;
    SYN_NMEA_ZDA zda_empty;

    const char *p_gga_e = "GPGGA,,,,,,,,,,";
    char str_gga_e[64];
    snprintf(str_gga_e, sizeof(str_gga_e), "$%s*%02X", p_gga_e, syn_nmea_checksum(p_gga_e));
    TEST_ASSERT_TRUE(syn_nmea_parse_gga(str_gga_e, &gga_empty));

    const char *p_rmc_e = "GPRMC,,,,,,,,,,";
    char str_rmc_e[64];
    snprintf(str_rmc_e, sizeof(str_rmc_e), "$%s*%02X", p_rmc_e, syn_nmea_checksum(p_rmc_e));
    TEST_ASSERT_TRUE(syn_nmea_parse_rmc(str_rmc_e, &rmc_empty));

    const char *p_vtg_e = "GPVTG,,,,,,,,";
    char str_vtg_e[64];
    snprintf(str_vtg_e, sizeof(str_vtg_e), "$%s*%02X", p_vtg_e, syn_nmea_checksum(p_vtg_e));
    TEST_ASSERT_TRUE(syn_nmea_parse_vtg(str_vtg_e, &vtg_empty));

    const char *p_gsa_e = "GPGSA,,,,,,,,,,,,,,,,";
    char str_gsa_e[64];
    snprintf(str_gsa_e, sizeof(str_gsa_e), "$%s*%02X", p_gsa_e, syn_nmea_checksum(p_gsa_e));
    TEST_ASSERT_TRUE(syn_nmea_parse_gsa(str_gsa_e, &gsa_empty));

    const char *p_zda_e = "GPZDA,,,,,";
    char str_zda_e[64];
    snprintf(str_zda_e, sizeof(str_zda_e), "$%s*%02X", p_zda_e, syn_nmea_checksum(p_zda_e));
    TEST_ASSERT_TRUE(syn_nmea_parse_zda(str_zda_e, &zda_empty));

    /* 12. Short talker (len < 3) and coord_udeg short inputs */
    const char *p_short = "AB,123";
    char str_short[64];
    snprintf(str_short, sizeof(str_short), "$%s*%02X", p_short, syn_nmea_checksum(p_short));
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type(str_short));

    /* RMC date with year >= 70 (1998, line 413) */
    SYN_NMEA_RMC rmc_1998;
    const char *p_rmc_98 = "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,150398,,,A";
    char str_rmc_98[128];
    snprintf(str_rmc_98, sizeof(str_rmc_98), "$%s*%02X", p_rmc_98, syn_nmea_checksum(p_rmc_98));
    TEST_ASSERT_TRUE(syn_nmea_parse_rmc(str_rmc_98, &rmc_1998));
    TEST_ASSERT_EQUAL_UINT16(1998, rmc_1998.year);

#if SYN_NMEA_USE_FIXED_POINT
    TEST_ASSERT_EQUAL_INT32(0, syn_nmea_parse_coord_udeg(NULL, 'N'));
    TEST_ASSERT_EQUAL_INT32(0, syn_nmea_parse_coord_udeg("12", 'N'));
    TEST_ASSERT_EQUAL_INT32(0, syn_nmea_parse_coord_udeg("12345", 'N'));
    TEST_ASSERT_EQUAL_INT32(0, syn_nmea_parse_coord_udeg("12.3", 'N'));
    TEST_ASSERT_EQUAL_INT32(-48117300, syn_nmea_parse_coord_udeg("04807.0380", 'S'));
    TEST_ASSERT_EQUAL_INT32(-48117300, syn_nmea_parse_coord_udeg("04807.0380", 'W'));
#endif
}

static void test_r5_nmea_strict_hex_and_empty_talker(void)
{
    /* R5-S15: Zero-XOR payloads with non-hex '*ZZ' / '*0Z' / '*Z0' must return false! */
    TEST_ASSERT_FALSE(syn_nmea_validate("$*ZZ"));
    TEST_ASSERT_FALSE(syn_nmea_validate("$AA*ZZ"));
    TEST_ASSERT_FALSE(syn_nmea_validate("$AA*0Z"));
    TEST_ASSERT_FALSE(syn_nmea_validate("$AA*G0"));
    TEST_ASSERT_FALSE(syn_nmea_validate("$AA*0g"));
    TEST_ASSERT_TRUE(syn_nmea_validate("$*00"));
    TEST_ASSERT_TRUE(syn_nmea_validate("$AA*00"));

    /* Empty talker field "$*00" returns UNKNOWN cleanly */
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type("$*00"));

    /* Inline static conversion helpers in syn_nmea.h */
    TEST_ASSERT_DOUBLE_WITHIN(0.000001, 48.1173, syn_udeg_to_deg(481173000));
    TEST_ASSERT_EQUAL_INT32(481173000, syn_deg_to_udeg(48.1173));
    TEST_ASSERT_EQUAL_INT32(-481173000, syn_deg_to_udeg(-48.1173));

    /* Truncated GGA with fewer commas than expected fields */
    SYN_NMEA_GGA gga;
    TEST_ASSERT_TRUE(syn_nmea_parse_gga("$GPGGA*56", &gga));
    TEST_ASSERT_TRUE(gga.valid);

    /* Non-sentence byte ignored when parser is idle */
    SYN_NMEA_Parser parser;
    syn_nmea_parser_init(&parser);
    TEST_ASSERT_FALSE(syn_nmea_parser_feed(&parser, 'X', NULL));
}

static void test_tier_a_nmea_framing_coord_and_field_bounds(void)
{
    /* 1. syn_nmea_validate: reject embedded CR/LF/'$', multiple '*', and trailing garbage */
    TEST_ASSERT_FALSE(syn_nmea_validate("$AA\nBB*33"));
    TEST_ASSERT_FALSE(syn_nmea_validate("$AA\rBB*33"));
    TEST_ASSERT_FALSE(syn_nmea_validate("$AA$BB*24"));
    TEST_ASSERT_FALSE(syn_nmea_validate("$AA*00*00"));
    TEST_ASSERT_FALSE(syn_nmea_validate("$AA*00XYZ"));
    TEST_ASSERT_TRUE(syn_nmea_validate("$AA*00\r\n"));

    /* 2. Coordinate validation: leading '+', non-digits, min >= 60, deg > 180, > 180.0 deg */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 48.1173, syn_nmea_parse_coord("+4807.038", 'N'));
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord("12345", 'N'));    /* No dot */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord("12.34", 'N'));    /* pre_len < 3 */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord("123456.7", 'N')); /* pre_len > 5 */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0,
                              syn_nmea_parse_coord("48A7.038", 'N'));           /* Bad pre digit */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord("4807.", 'N')); /* Empty frac */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord("4807.03X", 'N'));  /* Bad frac */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord("4860.000", 'N'));  /* min >= 60 */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0, syn_nmea_parse_coord("18100.000", 'E')); /* deg > 180 */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 0.0,
                              syn_nmea_parse_coord("18000.100", 'E')); /* > 180.0 deg */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, -180.0, syn_nmea_parse_coord("18000.000", 'W'));

    /* 3. Bounded integer validation in GGA, RMC, GSA, ZDA (no atoi wrap!) */
    char buf[128];
    SYN_NMEA_GGA gga;
    const char *bad_gga1 = "GPGGA,250000,4807.038,N,01131.000,E,-1,300,0.9,10.0,M,0.0,M,,";
    snprintf(buf, sizeof(buf), "$%s*%02X", bad_gga1, syn_nmea_checksum(bad_gga1));
    TEST_ASSERT_TRUE(syn_nmea_parse_gga(buf, &gga));
    TEST_ASSERT_EQUAL_UINT8(0U, gga.hours);
    TEST_ASSERT_EQUAL(SYN_NMEA_FIX_INVALID, gga.fix_quality);
    TEST_ASSERT_EQUAL_UINT8(0U, gga.num_satellites);

    const char *bad_gga2 = "GPGGA,123519.5X,4807.038,N,01131.000,E,9,12,0.9,10.0,M,0.0,M,,";
    snprintf(buf, sizeof(buf), "$%s*%02X", bad_gga2, syn_nmea_checksum(bad_gga2));
    TEST_ASSERT_TRUE(syn_nmea_parse_gga(buf, &gga));
    TEST_ASSERT_EQUAL_UINT8(0U, gga.hours);
    TEST_ASSERT_EQUAL_UINT8(12U, gga.num_satellites);

    const char *leap_gga = "GPGGA,235960.1234,4807.038,N,01131.000,E,1,08,0.9,10.0,M,0.0,M,,";
    snprintf(buf, sizeof(buf), "$%s*%02X", leap_gga, syn_nmea_checksum(leap_gga));
    TEST_ASSERT_TRUE(syn_nmea_parse_gga(buf, &gga));
    TEST_ASSERT_EQUAL_UINT8(23U, gga.hours);
    TEST_ASSERT_EQUAL_UINT8(60U, gga.seconds);
    TEST_ASSERT_EQUAL_UINT16(123U, gga.milliseconds);

    SYN_NMEA_RMC rmc;
    const char *bad_rmc = "GPRMC,126000,A,4807.038,N,01131.000,E,1.0,2.0,320324,,,";
    snprintf(buf, sizeof(buf), "$%s*%02X", bad_rmc, syn_nmea_checksum(bad_rmc));
    TEST_ASSERT_TRUE(syn_nmea_parse_rmc(buf, &rmc));
    TEST_ASSERT_EQUAL_UINT8(0U, rmc.minutes);
    TEST_ASSERT_EQUAL_UINT8(0U, rmc.day);
    TEST_ASSERT_EQUAL_UINT16(0U, rmc.year);

    SYN_NMEA_GSA gsa;
    const char *bad_gsa = "GPGSA,A,0,01,02,03,,,,,,,,,,2.5,1.3,2.1";
    snprintf(buf, sizeof(buf), "$%s*%02X", bad_gsa, syn_nmea_checksum(bad_gsa));
    TEST_ASSERT_TRUE(syn_nmea_parse_gsa(buf, &gsa));
    TEST_ASSERT_EQUAL_UINT8(0U, gsa.fix_type);

    SYN_NMEA_ZDA zda;
    const char *bad_zda = "GPZDA,120061,00,13,1899,00,00";
    snprintf(buf, sizeof(buf), "$%s*%02X", bad_zda, syn_nmea_checksum(bad_zda));
    TEST_ASSERT_TRUE(syn_nmea_parse_zda(buf, &zda));
    TEST_ASSERT_EQUAL_UINT8(0U, zda.seconds);
    TEST_ASSERT_EQUAL_UINT8(0U, zda.day);
    TEST_ASSERT_EQUAL_UINT8(0U, zda.month);
    TEST_ASSERT_EQUAL_UINT16(0U, zda.year);
}

void run_nmea_tests(void)
{
    RUN_TEST(test_nmea_checksum_and_validate);
    RUN_TEST(test_nmea_coord_parsing);
    RUN_TEST(test_nmea_parse_gga);
    RUN_TEST(test_nmea_parse_rmc);
    RUN_TEST(test_nmea_parse_vtg_gsa_zda);
    RUN_TEST(test_nmea_streaming_parser);
    RUN_TEST(test_nmea_edge_cases);
    RUN_TEST(test_nmea_lowercase_checksum_and_overflow);
    RUN_TEST(test_nmea_checksum_null_and_short_stars);
    RUN_TEST(test_nmea_sentence_checksum_and_field_truncation);
    RUN_TEST(test_nmea_date_time_parsing_and_null_coordinates);
    RUN_TEST(test_nmea_branch_coverage);
    RUN_TEST(test_r5_nmea_strict_hex_and_empty_talker);
    RUN_TEST(test_tier_a_nmea_framing_coord_and_field_bounds);
}
