

# File syn\_nmea.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_nmea.h**](syn__nmea_8h.md)

[Go to the documentation of this file](syn__nmea_8h.md)


```C++

#ifndef SYN_NMEA_H
#define SYN_NMEA_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_NMEA) || SYN_USE_NMEA

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_GNSS_USE_FIXED_POINT
#define SYN_GNSS_USE_FIXED_POINT 0 
#endif

#ifndef SYN_NMEA_USE_FIXED_POINT
#define SYN_NMEA_USE_FIXED_POINT SYN_GNSS_USE_FIXED_POINT 
#endif

#if (SYN_GNSS_USE_FIXED_POINT != SYN_NMEA_USE_FIXED_POINT)
#error \
    "SyntropicOS Config Error: Mismatched GNSS numeric mode! SYN_NMEA_USE_FIXED_POINT must match SYN_GNSS_USE_FIXED_POINT."
#endif

static inline double syn_udeg_to_deg(int32_t udeg)
{
    return (double)udeg / 10000000.0;
}

static inline int32_t syn_deg_to_udeg(double deg)
{
    return (int32_t)(deg * 10000000.0 + (deg >= 0.0 ? 0.5 : -0.5));
}

#define SYN_NMEA_MAX_SENTENCE_LEN 82 
typedef enum {
    SYN_NMEA_SENTENCE_UNKNOWN = 0,
    SYN_NMEA_SENTENCE_GGA,
    SYN_NMEA_SENTENCE_RMC,
    SYN_NMEA_SENTENCE_VTG,
    SYN_NMEA_SENTENCE_GSA,
    SYN_NMEA_SENTENCE_ZDA,
} SYN_NMEA_SentenceType;

typedef enum {
    SYN_NMEA_FIX_INVALID = 0,
    SYN_NMEA_FIX_GPS = 1,
    SYN_NMEA_FIX_DGPS = 2,
    SYN_NMEA_FIX_PPS = 3,
    SYN_NMEA_FIX_RTK = 4,
    SYN_NMEA_FIX_FLOAT_RTK = 5,
    SYN_NMEA_FIX_ESTIMATED = 6,
} SYN_NMEA_FixQuality;

typedef struct {
    uint8_t hours;         
    uint8_t minutes;       
    uint8_t seconds;       
    uint16_t milliseconds; 
#if SYN_NMEA_USE_FIXED_POINT
    int32_t lat_udeg; 
    int32_t lon_udeg; 
#else
    double latitude;  
    double longitude; 
#endif
    SYN_NMEA_FixQuality fix_quality; 
    uint8_t num_satellites;          
    float hdop;                      
    float altitude_m;                
    bool valid;                      
} SYN_NMEA_GGA;

typedef struct {
    uint8_t hours;         
    uint8_t minutes;       
    uint8_t seconds;       
    uint16_t milliseconds; 
    bool status_valid;     
#if SYN_NMEA_USE_FIXED_POINT
    int32_t lat_udeg; 
    int32_t lon_udeg; 
#else
    double latitude;  
    double longitude; 
#endif
    float speed_knots; 
    float course_deg;  
    uint8_t day;       
    uint8_t month;     
    uint16_t year;     
    bool valid;        
} SYN_NMEA_RMC;

typedef struct {
    float course_true_deg; 
    float speed_knots;     
    float speed_kph;       
    bool valid;            
} SYN_NMEA_VTG;

typedef struct {
    char mode;        
    uint8_t fix_type; 
    float pdop;       
    float hdop;       
    float vdop;       
    bool valid;       
} SYN_NMEA_GSA;

typedef struct {
    uint8_t hours;         
    uint8_t minutes;       
    uint8_t seconds;       
    uint16_t milliseconds; 
    uint8_t day;           
    uint8_t month;         
    uint16_t year;         
    bool valid;            
} SYN_NMEA_ZDA;

typedef struct {
    char buf[SYN_NMEA_MAX_SENTENCE_LEN + 1]; 
    uint8_t pos;                             
    bool in_sentence;                        
} SYN_NMEA_Parser;

void syn_nmea_parser_init(SYN_NMEA_Parser *parser);

bool syn_nmea_parser_feed(SYN_NMEA_Parser *parser, char byte, char *out_sentence);

uint8_t syn_nmea_checksum(const char *sentence);

bool syn_nmea_validate(const char *sentence);

SYN_NMEA_SentenceType syn_nmea_get_type(const char *sentence);

double syn_nmea_parse_coord(const char *nmea_coord, char dir);

bool syn_nmea_parse_gga(const char *sentence, SYN_NMEA_GGA *gga);

bool syn_nmea_parse_rmc(const char *sentence, SYN_NMEA_RMC *rmc);

bool syn_nmea_parse_vtg(const char *sentence, SYN_NMEA_VTG *vtg);

bool syn_nmea_parse_gsa(const char *sentence, SYN_NMEA_GSA *gsa);

bool syn_nmea_parse_zda(const char *sentence, SYN_NMEA_ZDA *zda);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_NMEA */

#endif /* SYN_NMEA_H */
```


