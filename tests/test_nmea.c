#include "unity/unity.h"
#include "syntropic/proto/syn_nmea.h"

#include <string.h>
#include <math.h>

void test_nmea_checksum_and_validate(void)
{
    const char *gga = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    TEST_ASSERT_TRUE(syn_nmea_validate(gga));
    TEST_ASSERT_EQUAL_HEX8(0x47, syn_nmea_checksum(gga));

    /* Tampered string */
    const char *bad = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*48";
    TEST_ASSERT_FALSE(syn_nmea_validate(bad));
}

void test_nmea_coord_parsing(void)
{
    /* 48 deg 07.038 min N -> 48 + 7.038/60 = 48.1173 */
    double lat = syn_nmea_parse_coord("4807.038", 'N');
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 48.1173, lat);

    /* 011 deg 31.000 min W -> -(11 + 31/60) = -11.516666 */
    double lon = syn_nmea_parse_coord("01131.000", 'W');
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, -11.516666, lon);

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
    TEST_ASSERT_EQUAL_STRING("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47", out_sentence);
}

void test_nmea_edge_cases(void)
{
    TEST_ASSERT_EQUAL(0, syn_nmea_checksum(NULL));
    TEST_ASSERT_FALSE(syn_nmea_validate(NULL));
    TEST_ASSERT_FALSE(syn_nmea_validate("GARBAGE"));
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type(NULL));
    TEST_ASSERT_EQUAL(SYN_NMEA_SENTENCE_UNKNOWN, syn_nmea_get_type("$GPXYZ,1,2*34"));

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, (float)syn_nmea_parse_coord(NULL, 'N'));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, (float)syn_nmea_parse_coord("12", 'N'));

    SYN_NMEA_GGA gga;
    TEST_ASSERT_FALSE(syn_nmea_parse_gga("$GPRMC,1,2,3*00", &gga));

    SYN_NMEA_RMC rmc;
    TEST_ASSERT_FALSE(syn_nmea_parse_rmc("$GPGGA,1,2,3*00", &rmc));

    SYN_NMEA_VTG vtg;
    TEST_ASSERT_FALSE(syn_nmea_parse_vtg("$GPGGA,1,2,3*00", &vtg));

    SYN_NMEA_GSA gsa;
    TEST_ASSERT_FALSE(syn_nmea_parse_gsa("$GPGGA,1,2,3*00", &gsa));

    SYN_NMEA_ZDA zda;
    TEST_ASSERT_FALSE(syn_nmea_parse_zda("$GPGGA,1,2,3*00", &zda));
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
}
