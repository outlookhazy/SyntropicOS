/**
 * @file syn_nmea.h
 * @brief NMEA 0183 / GNSS sentence parser and encoder.
 * @ingroup syn_protocol
 *
 * Provides a zero-allocation, streaming byte-at-a-time NMEA 0183 parser,
 * sentence checksum verification, and structured data decoders for standard
 * GNSS sentences:
 *   - GGA: Global Positioning System Fix Data
 *   - RMC: Recommended Minimum Specific GNSS Data
 *   - VTG: Course Over Ground & Ground Speed
 *   - GSA: GNSS DOP & Active Satellites
 *   - ZDA: UTC Time & Date
 */

#ifndef SYN_NMEA_H
#define SYN_NMEA_H

#include "../common/syn_defs.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_NMEA_MAX_SENTENCE_LEN   82   /**< Max NMEA 0183 sentence length */

/** @brief NMEA Sentence Type Enum */
typedef enum {
    SYN_NMEA_SENTENCE_UNKNOWN = 0,
    SYN_NMEA_SENTENCE_GGA,
    SYN_NMEA_SENTENCE_RMC,
    SYN_NMEA_SENTENCE_VTG,
    SYN_NMEA_SENTENCE_GSA,
    SYN_NMEA_SENTENCE_ZDA,
} SYN_NMEA_SentenceType;

/** @brief GPS Fix Quality Enum (from GGA) */
typedef enum {
    SYN_NMEA_FIX_INVALID = 0,
    SYN_NMEA_FIX_GPS     = 1,
    SYN_NMEA_FIX_DGPS    = 2,
    SYN_NMEA_FIX_PPS     = 3,
    SYN_NMEA_FIX_RTK     = 4,
    SYN_NMEA_FIX_FLOAT_RTK = 5,
    SYN_NMEA_FIX_ESTIMATED = 6,
} SYN_NMEA_FixQuality;

/** @brief Parsed NMEA GGA (Fix Data) Structure */
typedef struct {
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
    uint16_t milliseconds;
    double   latitude;       /**< Decimal degrees (+N, -S) */
    double   longitude;      /**< Decimal degrees (+E, -W) */
    SYN_NMEA_FixQuality fix_quality;
    uint8_t  num_satellites;
    float    hdop;
    float    altitude_m;
    bool     valid;
} SYN_NMEA_GGA;

/** @brief Parsed NMEA RMC (Recommended Minimum Data) Structure */
typedef struct {
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
    uint16_t milliseconds;
    bool     status_valid;   /**< 'A' = valid, 'V' = receiver warning */
    double   latitude;       /**< Decimal degrees (+N, -S) */
    double   longitude;      /**< Decimal degrees (+E, -W) */
    float    speed_knots;    /**< Speed over ground in knots */
    float    course_deg;     /**< Course over ground in true degrees */
    uint8_t  day;
    uint8_t  month;
    uint16_t year;
    bool     valid;
} SYN_NMEA_RMC;

/** @brief Parsed NMEA VTG (Velocity & Course) Structure */
typedef struct {
    float    course_true_deg;
    float    speed_knots;
    float    speed_kph;
    bool     valid;
} SYN_NMEA_VTG;

/** @brief Parsed NMEA GSA (DOP & Active Satellites) Structure */
typedef struct {
    char     mode;           /**< 'M' = Manual, 'A' = Automatic */
    uint8_t  fix_type;       /**< 1 = No fix, 2 = 2D fix, 3 = 3D fix */
    float    pdop;
    float    hdop;
    float    vdop;
    bool     valid;
} SYN_NMEA_GSA;

/** @brief Parsed NMEA ZDA (UTC Date & Time) Structure */
typedef struct {
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
    uint16_t milliseconds;
    uint8_t  day;
    uint8_t  month;
    uint16_t year;
    bool     valid;
} SYN_NMEA_ZDA;

/** @brief Streaming NMEA Parser State Machine */
typedef struct {
    char     buf[SYN_NMEA_MAX_SENTENCE_LEN + 1];
    uint8_t  pos;
    bool     in_sentence;
} SYN_NMEA_Parser;

/**
 * @brief Initialize an NMEA streaming parser.
 * @param parser Pointer to parser state machine.
 */
void syn_nmea_parser_init(SYN_NMEA_Parser *parser);

/**
 * @brief Feed a byte into the NMEA streaming parser.
 *
 * @param parser Pointer to parser instance.
 * @param byte   Received character.
 * @param out_sentence Output buffer for completed NMEA sentence string (if returns true).
 * @return true if a complete, valid NMEA sentence was received.
 */
bool syn_nmea_parser_feed(SYN_NMEA_Parser *parser, char byte, char *out_sentence);

/**
 * @brief Calculate XOR checksum of an NMEA sentence payload (between '$' and '*').
 * @param sentence  NMEA sentence string.
 * @return 8-bit XOR checksum value.
 */
uint8_t syn_nmea_checksum(const char *sentence);

/**
 * @brief Validate an NMEA sentence string (checks start '$', '*XX' checksum, and CRLF).
 * @param sentence NMEA sentence string.
 * @return true if valid sentence and matching checksum.
 */
bool syn_nmea_validate(const char *sentence);

/**
 * @brief Identify sentence type from NMEA string.
 * @param sentence NMEA sentence string.
 * @return SYN_NMEA_SentenceType enum.
 */
SYN_NMEA_SentenceType syn_nmea_get_type(const char *sentence);

/**
 * @brief Parse NMEA DDMM.MMMM coordinate string and direction indicator.
 * @param nmea_coord  Coordinate string (e.g. "4807.038").
 * @param dir         Direction char ('N', 'S', 'E', 'W').
 * @return Decimal degrees (+N/+E, -S/-W).
 */
double syn_nmea_parse_coord(const char *nmea_coord, char dir);

/**
 * @brief Parse a $GPGGA / $GNGGA sentence.
 * @param sentence NMEA sentence string.
 * @param gga      Pointer to destination GGA struct.
 * @return true on success.
 */
bool syn_nmea_parse_gga(const char *sentence, SYN_NMEA_GGA *gga);

/**
 * @brief Parse a $GPRMC / $GNRMC sentence.
 * @param sentence NMEA sentence string.
 * @param rmc      Pointer to destination RMC struct.
 * @return true on success.
 */
bool syn_nmea_parse_rmc(const char *sentence, SYN_NMEA_RMC *rmc);

/**
 * @brief Parse a $GPVTG / $GNVTG sentence.
 * @param sentence NMEA sentence string.
 * @param vtg      Pointer to destination VTG struct.
 * @return true on success.
 */
bool syn_nmea_parse_vtg(const char *sentence, SYN_NMEA_VTG *vtg);

/**
 * @brief Parse a $GPGSA / $GNGSA sentence.
 * @param sentence NMEA sentence string.
 * @param gsa      Pointer to destination GSA struct.
 * @return true on success.
 */
bool syn_nmea_parse_gsa(const char *sentence, SYN_NMEA_GSA *gsa);

/**
 * @brief Parse a $GPZDA / $GNZDA sentence.
 * @param sentence NMEA sentence string.
 * @param zda      Pointer to destination ZDA struct.
 * @return true on success.
 */
bool syn_nmea_parse_zda(const char *sentence, SYN_NMEA_ZDA *zda);

#ifdef __cplusplus
}
#endif

#endif /* SYN_NMEA_H */
