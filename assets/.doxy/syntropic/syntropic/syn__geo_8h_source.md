

# File syn\_geo.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_geo.h**](syn__geo_8h.md)

[Go to the documentation of this file](syn__geo_8h.md)


```C++

#ifndef SYN_GEO_H
#define SYN_GEO_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_GEO) || SYN_USE_GEO

#include "../common/syn_defs.h"
#include "../proto/syn_nmea.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_GEO_USE_FIXED_POINT
#define SYN_GEO_USE_FIXED_POINT SYN_GNSS_USE_FIXED_POINT 
#endif

#if (SYN_GNSS_USE_FIXED_POINT != SYN_GEO_USE_FIXED_POINT)
#error \
    "SyntropicOS Config Error: Mismatched GNSS numeric mode! SYN_GEO_USE_FIXED_POINT must match SYN_GNSS_USE_FIXED_POINT."
#endif

/* ── WGS-84 Ellipsoid Constants ────────────────────────────────────────── */

#define SYN_GEO_WGS84_A 6378137.0

#define SYN_GEO_WGS84_INV_F 298.257223563

#define SYN_GEO_WGS84_E2 0.0066943799901413165

/* ── Data Structures ───────────────────────────────────────────────────── */

typedef struct {
#if SYN_GEO_USE_FIXED_POINT
    int32_t lat_udeg;    
    int32_t lon_udeg;    
    int32_t altitude_mm; 
#else
    double latitude;   
    double longitude;  
    double altitude_m; 
#endif
    float accuracy_m;             
    SYN_NMEA_FixQuality fix_type; 
    bool valid;                   
} SYN_GeoPos;

typedef struct {
    double east_m;  
    double north_m; 
    double up_m;    
} SYN_ENU;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_geo_wgs84_to_ecef(double lat_deg, double lon_deg, double alt_m, double *out_x,
                                 double *out_y, double *out_z);

SYN_Status syn_geo_ecef_to_wgs84(double x, double y, double z, double *out_lat, double *out_lon,
                                 double *out_alt);

SYN_Status syn_geo_ecef_to_enu(double x, double y, double z, double ref_lat_deg, double ref_lon_deg,
                               double ref_alt_m, SYN_ENU *out_enu);

SYN_Status syn_geo_wgs84_to_enu(double lat_deg, double lon_deg, double alt_m, double ref_lat_deg,
                                double ref_lon_deg, double ref_alt_m, SYN_ENU *out_enu);

uint32_t syn_geo_distance_fixed_mm(int32_t lat1_udeg, int32_t lon1_udeg, int32_t lat2_udeg,
                                   int32_t lon2_udeg);

double syn_geo_haversine_m(double lat1_deg, double lon1_deg, double lat2_deg, double lon2_deg);

double syn_geo_3d_distance_m(const SYN_ENU *p1, const SYN_ENU *p2);

SYN_Status syn_geo_pos_from_gga(const SYN_NMEA_GGA *gga, SYN_GeoPos *out_pos);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_GEO */

#endif /* SYN_GEO_H */
```


