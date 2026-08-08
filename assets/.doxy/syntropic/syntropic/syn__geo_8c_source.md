

# File syn\_geo.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_geo.c**](syn__geo_8c.md)

[Go to the documentation of this file](syn__geo_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_GEO) || SYN_USE_GEO

#include "../util/syn_assert.h"
#include "syn_geo.h"

#include <math.h>
#include <stddef.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)
/* ── WGS84 to ECEF ─────────────────────────────────────────────────────── */

SYN_Status syn_geo_wgs84_to_ecef(double lat_deg, double lon_deg, double alt_m, double *out_x,
                                 double *out_y, double *out_z)
{
    if (out_x == NULL || out_y == NULL || out_z == NULL) {
        return SYN_INVALID_PARAM;
    }

    double lat_rad = lat_deg * DEG_TO_RAD;
    double lon_rad = lon_deg * DEG_TO_RAD;

    double sin_lat = sin(lat_rad);
    double cos_lat = cos(lat_rad);
    double sin_lon = sin(lon_rad);
    double cos_lon = cos(lon_rad);

    /* Prime vertical radius of curvature N(phi) */
    double N = SYN_GEO_WGS84_A / sqrt(1.0 - SYN_GEO_WGS84_E2 * sin_lat * sin_lat);

    *out_x = (N + alt_m) * cos_lat * cos_lon;
    *out_y = (N + alt_m) * cos_lat * sin_lon;
    *out_z = (N * (1.0 - SYN_GEO_WGS84_E2) + alt_m) * sin_lat;

    return SYN_OK;
}

/* ── ECEF to WGS84 (Bowring's Closed-Form Algorithm) ───────────────────── */

SYN_Status syn_geo_ecef_to_wgs84(double x, double y, double z, double *out_lat, double *out_lon,
                                 double *out_alt)
{
    if (out_lat == NULL || out_lon == NULL || out_alt == NULL) {
        return SYN_INVALID_PARAM;
    }

    double a = SYN_GEO_WGS84_A;
    double e2 = SYN_GEO_WGS84_E2;
    double b = a * sqrt(1.0 - e2);
    double ep2 = (a * a - b * b) / (b * b);

    double p = sqrt(x * x + y * y);
    if (p < 1e-12) {
        /* Polar region handling */
        *out_lat = (z >= 0.0) ? 90.0 : -90.0;
        *out_lon = 0.0;
        *out_alt = fabs(z) - b;
        return SYN_OK;
    }

    double theta = atan2(z * a, p * b);
    double sin_theta = sin(theta);
    double cos_theta = cos(theta);

    double lat_rad = atan2(z + ep2 * b * sin_theta * sin_theta * sin_theta,
                           p - e2 * a * cos_theta * cos_theta * cos_theta);

    double lon_rad = atan2(y, x);

    double sin_lat = sin(lat_rad);
    double N = a / sqrt(1.0 - e2 * sin_lat * sin_lat);

    *out_lat = lat_rad * RAD_TO_DEG;
    *out_lon = lon_rad * RAD_TO_DEG;
    *out_alt = (p / cos(lat_rad)) - N;

    return SYN_OK;
}

/* ── ECEF to ENU ───────────────────────────────────────────────────────── */

SYN_Status syn_geo_ecef_to_enu(double x, double y, double z, double ref_lat_deg, double ref_lon_deg,
                               double ref_alt_m, SYN_ENU *out_enu)
{
    if (out_enu == NULL) {
        return SYN_INVALID_PARAM;
    }

    double ref_x, ref_y, ref_z;
    SYN_Status st =
        syn_geo_wgs84_to_ecef(ref_lat_deg, ref_lon_deg, ref_alt_m, &ref_x, &ref_y, &ref_z);
    if (st != SYN_OK)
        return st; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */

    double dx = x - ref_x;
    double dy = y - ref_y;
    double dz = z - ref_z;

    double ref_lat_rad = ref_lat_deg * DEG_TO_RAD;
    double ref_lon_rad = ref_lon_deg * DEG_TO_RAD;

    double sin_lat = sin(ref_lat_rad);
    double cos_lat = cos(ref_lat_rad);
    double sin_lon = sin(ref_lon_rad);
    double cos_lon = cos(ref_lon_rad);

    /* Rotation matrix R_ENU */
    out_enu->east_m = -sin_lon * dx + cos_lon * dy;
    out_enu->north_m = -sin_lat * cos_lon * dx - sin_lat * sin_lon * dy + cos_lat * dz;
    out_enu->up_m = cos_lat * cos_lon * dx + cos_lat * sin_lon * dy + sin_lat * dz;

    return SYN_OK;
}

/* ── Direct WGS84 to ENU ───────────────────────────────────────────────── */

SYN_Status syn_geo_wgs84_to_enu(double lat_deg, double lon_deg, double alt_m, double ref_lat_deg,
                                double ref_lon_deg, double ref_alt_m, SYN_ENU *out_enu)
{
    if (out_enu == NULL) {
        return SYN_INVALID_PARAM;
    }

    double x, y, z;
    SYN_Status st = syn_geo_wgs84_to_ecef(lat_deg, lon_deg, alt_m, &x, &y, &z);
    if (st != SYN_OK)
        return st; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */

    return syn_geo_ecef_to_enu(x, y, z, ref_lat_deg, ref_lon_deg, ref_alt_m, out_enu);
}

/* ── Haversine 2D Distance ─────────────────────────────────────────────── */

double syn_geo_haversine_m(double lat1_deg, double lon1_deg, double lat2_deg, double lon2_deg)
{
    double dlat = (lat2_deg - lat1_deg) * DEG_TO_RAD;
    double dlon = (lon2_deg - lon1_deg) * DEG_TO_RAD;

    double lat1_rad = lat1_deg * DEG_TO_RAD;
    double lat2_rad = lat2_deg * DEG_TO_RAD;

    double a = sin(dlat / 2.0) * sin(dlat / 2.0) +
               sin(dlon / 2.0) * sin(dlon / 2.0) * cos(lat1_rad) * cos(lat2_rad);
    /* LCOV_EXCL_START: IEEE-754 floating-point underflow/overflow guards */
    if (a < 0.0)
        a = 0.0;
    if (a > 1.0)
        a = 1.0;
    /* LCOV_EXCL_STOP */
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    return SYN_GEO_WGS84_A * c;
}

/* ── 3D Euclidean Distance ─────────────────────────────────────────────── */

double syn_geo_3d_distance_m(const SYN_ENU *p1, const SYN_ENU *p2)
{
    if (p1 == NULL || p2 == NULL)
        return 0.0;

    double de = p2->east_m - p1->east_m;
    double dn = p2->north_m - p1->north_m;
    double du = p2->up_m - p1->up_m;

    return sqrt(de * de + dn * dn + du * du);
}

uint32_t syn_geo_distance_fixed_mm(int32_t lat1_udeg, int32_t lon1_udeg, int32_t lat2_udeg,
                                   int32_t lon2_udeg)
{
    int64_t dlat = (int64_t)lat2_udeg - (int64_t)lat1_udeg;
    int64_t dlon = (int64_t)lon2_udeg - (int64_t)lon1_udeg;

    int64_t dy_mm = (dlat * 111132LL) / 10000LL;
    int64_t dx_mm = ((dlon * 111132LL) / 10000LL * 786LL) >> 10;

    int64_t dist_sq_mm = (dx_mm * dx_mm) + (dy_mm * dy_mm);
    return (uint32_t)sqrt((double)dist_sq_mm);
}

/* ── NMEA GGA Bridge ───────────────────────────────────────────────────── */

SYN_Status syn_geo_pos_from_gga(const SYN_NMEA_GGA *gga, SYN_GeoPos *out_pos)
{
    if (gga == NULL || out_pos == NULL) {
        return SYN_INVALID_PARAM;
    }

#if SYN_GEO_USE_FIXED_POINT
    out_pos->lat_udeg = gga->lat_udeg;
    out_pos->lon_udeg = gga->lon_udeg;
    out_pos->altitude_mm = (int32_t)(gga->altitude_m * 1000.0f);
#else
    out_pos->latitude = gga->latitude;
    out_pos->longitude = gga->longitude;
    out_pos->altitude_m = (double)gga->altitude_m;
#endif
    out_pos->fix_type = gga->fix_quality;
    out_pos->valid = gga->valid;

    switch (gga->fix_quality) {
    case SYN_NMEA_FIX_RTK:
        out_pos->accuracy_m = 0.01f; /* 1 cm RTK Fixed accuracy */
        break;
    case SYN_NMEA_FIX_FLOAT_RTK:
        out_pos->accuracy_m = 0.20f; /* 20 cm RTK Float accuracy */
        break;
    case SYN_NMEA_FIX_DGPS:
        out_pos->accuracy_m = 1.00f; /* 1 meter DGPS accuracy */
        break;
    case SYN_NMEA_FIX_GPS:
    case SYN_NMEA_FIX_PPS:
        out_pos->accuracy_m = 2.50f; /* Standard GPS accuracy */
        break;
    default:
        out_pos->accuracy_m = 50.0f; /* Unverified / estimated accuracy */
        break;
    }

    return SYN_OK;
}

#endif /* SYN_USE_GEO */
```


