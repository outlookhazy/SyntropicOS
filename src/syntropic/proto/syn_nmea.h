/**
 * @file syn_nmea.h
 * @brief Non-blocking streaming NMEA 0183 GPS sentence parser.
 * @ingroup syn_protocol
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

/**
 * @brief Parsed GPS position & fix status metadata.
 */
typedef struct {
    int32_t  lat_microdeg;   /**< Latitude in microdegrees (degrees * 1,000,000; positive = N, negative = S) */
    int32_t  lon_microdeg;   /**< Longitude in microdegrees (degrees * 1,000,000; positive = E, negative = W) */
    int32_t  alt_mm;         /**< Altitude above mean sea level in millimeters */
    uint32_t utc_time;       /**< UTC time in HHMMSS format (e.g. 123456 = 12:34:56) */
    uint32_t utc_date;       /**< UTC date in DDMMYY format (e.g. 230726 = 23 July 2026) */
    uint16_t speed_knots_x10;/**< Speed over ground in knots * 10 */
    uint8_t  satellites;     /**< Number of satellites in view / used in fix */
    uint8_t  fix_quality;    /**< Fix quality: 0 = Invalid, 1 = GPS fix, 2 = DGPS fix */
    bool     valid;          /**< True if valid navigation fix (RMC status 'A') */
} SYN_NmeaData;

/**
 * @brief Streaming NMEA parser context.
 */
typedef struct {
    char         line_buf[128]; /**< Sentence line buffer staging area */
    uint8_t      line_pos;      /**< Write cursor position in line_buf */
    bool         in_sentence;   /**< True if currently receiving inside '$' frame */
    SYN_NmeaData data;          /**< Parsed GPS metrics output */
    
    /**
     * @brief User callback invoked when a valid NMEA sentence is parsed.
     * @param data Current updated GPS data struct.
     * @param sentence_type Sentence identifier (e.g. "RMC" or "GGA").
     * @param ctx User context pointer.
     */
    void (*on_fix)(const SYN_NmeaData *data, const char *sentence_type, void *ctx);
    void *ctx;                  /**< Context pointer passed to callback */
} SYN_NmeaParser;

/**
 * @brief Initialize the NMEA streaming parser.
 * @param parser Pointer to parser context to initialize.
 * @param on_fix Callback triggered when a valid position fix sentence is parsed.
 * @param ctx User context pointer passed to callback.
 */
void syn_nmea_init(SYN_NmeaParser *parser,
                   void (*on_fix)(const SYN_NmeaData *data, const char *sentence_type, void *ctx),
                   void *ctx);

/**
 * @brief Feed a single received byte into the streaming parser.
 * @param parser Parser context.
 * @param ch Incoming character byte.
 * @return true if a complete valid sentence was processed.
 */
bool syn_nmea_feed_char(SYN_NmeaParser *parser, char ch);

/**
 * @brief Feed a string or buffer of bytes into the parser.
 * @param parser Parser context.
 * @param str Input string or byte sequence.
 * @param len Length of str in bytes.
 */
void syn_nmea_feed_buf(SYN_NmeaParser *parser, const char *str, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_NMEA_H */
