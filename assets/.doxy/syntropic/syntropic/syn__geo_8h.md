

# File syn\_geo.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_geo.h**](syn__geo_8h.md)

[Go to the source code of this file](syn__geo_8h_source.md)

_Geodetic coordinate transformations and distance calculation library._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../proto/syn_nmea.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_ENU**](structSYN__ENU.md) <br>_Local East-North-Up (ENU) 3D Cartesian coordinates in meters._  |
| struct | [**SYN\_GeoPos**](structSYN__GeoPos.md) <br>_Structured Geographic Position with accuracy bound and fix quality._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  double | [**syn\_geo\_3d\_distance\_m**](#function-syn_geo_3d_distance_m) (const [**SYN\_ENU**](structSYN__ENU.md) \* p1, const [**SYN\_ENU**](structSYN__ENU.md) \* p2) <br>_Compute 3D Euclidean Cartesian distance between two ENU points._  |
|  uint32\_t | [**syn\_geo\_distance\_fixed\_mm**](#function-syn_geo_distance_fixed_mm) (int32\_t lat1\_udeg, int32\_t lon1\_udeg, int32\_t lat2\_udeg, int32\_t lon2\_udeg) <br>_Compute 2D surface distance in millimeters using 32-bit Micro-Degree integers._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_geo\_ecef\_to\_enu**](#function-syn_geo_ecef_to_enu) (double x, double y, double z, double ref\_lat\_deg, double ref\_lon\_deg, double ref\_alt\_m, [**SYN\_ENU**](structSYN__ENU.md) \* out\_enu) <br>_Convert global ECEF coordinates to a Local East-North-Up (ENU) frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_geo\_ecef\_to\_wgs84**](#function-syn_geo_ecef_to_wgs84) (double x, double y, double z, double \* out\_lat, double \* out\_lon, double \* out\_alt) <br>_Convert Earth-Centered Earth-Fixed (ECEF) coordinates back to WGS-84 Geodetic._  |
|  double | [**syn\_geo\_haversine\_m**](#function-syn_geo_haversine_m) (double lat1\_deg, double lon1\_deg, double lat2\_deg, double lon2\_deg) <br>_Compute 2D surface geodesic distance using the Haversine formula._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_geo\_pos\_from\_gga**](#function-syn_geo_pos_from_gga) (const [**SYN\_NMEA\_GGA**](structSYN__NMEA__GGA.md) \* gga, [**SYN\_GeoPos**](structSYN__GeoPos.md) \* out\_pos) <br>_Populate a_ [_**SYN\_GeoPos**_](structSYN__GeoPos.md) _structure from a parsed NMEA GGA sentence._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_geo\_wgs84\_to\_ecef**](#function-syn_geo_wgs84_to_ecef) (double lat\_deg, double lon\_deg, double alt\_m, double \* out\_x, double \* out\_y, double \* out\_z) <br>_Convert WGS-84 Geodetic coordinates to Earth-Centered Earth-Fixed (ECEF)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_geo\_wgs84\_to\_enu**](#function-syn_geo_wgs84_to_enu) (double lat\_deg, double lon\_deg, double alt\_m, double ref\_lat\_deg, double ref\_lon\_deg, double ref\_alt\_m, [**SYN\_ENU**](structSYN__ENU.md) \* out\_enu) <br>_Direct conversion from WGS-84 Geodetic to Local East-North-Up (ENU)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_GEO\_USE\_FIXED\_POINT**](syn__geo_8h.md#define-syn_geo_use_fixed_point)  `[**SYN\_GNSS\_USE\_FIXED\_POINT**](syn__nmea_8h.md#define-syn_gnss_use_fixed_point)`<br> |
| define  | [**SYN\_GEO\_WGS84\_A**](syn__geo_8h.md#define-syn_geo_wgs84_a)  `6378137.0`<br> |
| define  | [**SYN\_GEO\_WGS84\_E2**](syn__geo_8h.md#define-syn_geo_wgs84_e2)  `0.0066943799901413165`<br> |
| define  | [**SYN\_GEO\_WGS84\_INV\_F**](syn__geo_8h.md#define-syn_geo_wgs84_inv_f)  `298.257223563`<br> |

## Detailed Description


Provides 64-bit precision WGS-84 ellipsoid coordinate transformations between spherical GNSS coordinates (Latitude, Longitude, Altitude) and 3D metric Cartesian frames:
* ECEF (Earth-Centered Earth-Fixed): Global (X, Y, Z) in meters.
* ENU (East-North-Up): Local (East, North, Up) in meters relative to an origin reference station (Lat0, Lon0, Alt0).




Preserves sub-millimeter mathematical precision, fully supporting 1 cm RTK Fixed GNSS positioning data parsed from NMEA sentences.


Usage: 
```C++
SYN_GeoPos base, rover;
// ... initialize base and rover positions ...
SYN_ENU local_enu;
syn_geo_wgs84_to_enu(rover.latitude, rover.longitude, rover.altitude_m,
                     base.latitude, base.longitude, base.altitude_m,
                     &local_enu);
// local_enu.east_m, local_enu.north_m, local_enu.up_m are in meters!
```
 


    
## Public Functions Documentation




### function syn\_geo\_3d\_distance\_m 

_Compute 3D Euclidean Cartesian distance between two ENU points._ 
```C++
double syn_geo_3d_distance_m (
    const SYN_ENU * p1,
    const SYN_ENU * p2
) 
```





**Parameters:**


* `p1` Point 1 ENU coordinates in meters. Must not be NULL. 
* `p2` Point 2 ENU coordinates in meters. Must not be NULL. 



**Returns:**

3D Euclidean distance in meters. 





        

<hr>



### function syn\_geo\_distance\_fixed\_mm 

_Compute 2D surface distance in millimeters using 32-bit Micro-Degree integers._ 
```C++
uint32_t syn_geo_distance_fixed_mm (
    int32_t lat1_udeg,
    int32_t lon1_udeg,
    int32_t lat2_udeg,
    int32_t lon2_udeg
) 
```



Executed in 100% pure integer math (zero floating point operations).




**Parameters:**


* `lat1_udeg` Point 1 Latitude in micro-degrees (1e-7 deg). 
* `lon1_udeg` Point 1 Longitude in micro-degrees (1e-7 deg). 
* `lat2_udeg` Point 2 Latitude in micro-degrees (1e-7 deg). 
* `lon2_udeg` Point 2 Longitude in micro-degrees (1e-7 deg). 



**Returns:**

2D Surface distance in millimeters (mm). 





        

<hr>



### function syn\_geo\_ecef\_to\_enu 

_Convert global ECEF coordinates to a Local East-North-Up (ENU) frame._ 
```C++
SYN_Status syn_geo_ecef_to_enu (
    double x,
    double y,
    double z,
    double ref_lat_deg,
    double ref_lon_deg,
    double ref_alt_m,
    SYN_ENU * out_enu
) 
```



Computes metric Cartesian offsets (East, North, Up) relative to a reference station origin (ref\_lat\_deg, ref\_lon\_deg, ref\_alt\_m).




**Parameters:**


* `x` Target ECEF X in meters. 
* `y` Target ECEF Y in meters. 
* `z` Target ECEF Z in meters. 
* `ref_lat_deg` Reference origin Latitude in decimal degrees. 
* `ref_lon_deg` Reference origin Longitude in decimal degrees. 
* `ref_alt_m` Reference origin Altitude in meters. 
* `out_enu` Output ENU Cartesian coordinates. Must not be NULL. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if out\_enu is NULL. 





        

<hr>



### function syn\_geo\_ecef\_to\_wgs84 

_Convert Earth-Centered Earth-Fixed (ECEF) coordinates back to WGS-84 Geodetic._ 
```C++
SYN_Status syn_geo_ecef_to_wgs84 (
    double x,
    double y,
    double z,
    double * out_lat,
    double * out_lon,
    double * out_alt
) 
```



Uses Bowring's closed-form algorithm for high precision.




**Parameters:**


* `x` ECEF X coordinate in meters. 
* `y` ECEF Y coordinate in meters. 
* `z` ECEF Z coordinate in meters. 
* `out_lat` Output Latitude in decimal degrees. Must not be NULL. 
* `out_lon` Output Longitude in decimal degrees. Must not be NULL. 
* `out_alt` Output Altitude in meters. Must not be NULL. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if outputs are NULL. 





        

<hr>



### function syn\_geo\_haversine\_m 

_Compute 2D surface geodesic distance using the Haversine formula._ 
```C++
double syn_geo_haversine_m (
    double lat1_deg,
    double lon1_deg,
    double lat2_deg,
    double lon2_deg
) 
```





**Parameters:**


* `lat1_deg` Point 1 Latitude in decimal degrees. 
* `lon1_deg` Point 1 Longitude in decimal degrees. 
* `lat2_deg` Point 2 Latitude in decimal degrees. 
* `lon2_deg` Point 2 Longitude in decimal degrees. 



**Returns:**

2D Surface distance in meters. 





        

<hr>



### function syn\_geo\_pos\_from\_gga 

_Populate a_ [_**SYN\_GeoPos**_](structSYN__GeoPos.md) _structure from a parsed NMEA GGA sentence._
```C++
SYN_Status syn_geo_pos_from_gga (
    const SYN_NMEA_GGA * gga,
    SYN_GeoPos * out_pos
) 
```



Assigns automatic accuracy estimate based on fix quality:
* RTK\_FIXED: 0.01 m (1 cm)
* RTK\_FLOAT: 0.20 m (20 cm)
* DGPS: 1.00 m
* GPS: 2.50 m
* INVALID/EST: 50.0 m






**Parameters:**


* `gga` Parsed NMEA GGA sentence. Must not be NULL. 
* `out_pos` Output [**SYN\_GeoPos**](structSYN__GeoPos.md) struct. Must not be NULL. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if args are NULL. 





        

<hr>



### function syn\_geo\_wgs84\_to\_ecef 

_Convert WGS-84 Geodetic coordinates to Earth-Centered Earth-Fixed (ECEF)._ 
```C++
SYN_Status syn_geo_wgs84_to_ecef (
    double lat_deg,
    double lon_deg,
    double alt_m,
    double * out_x,
    double * out_y,
    double * out_z
) 
```





**Parameters:**


* `lat_deg` Latitude in decimal degrees (-90 to +90). 
* `lon_deg` Longitude in decimal degrees (-180 to +180). 
* `alt_m` Altitude above ellipsoid/MSL in meters. 
* `out_x` Output ECEF X coordinate in meters. Must not be NULL. 
* `out_y` Output ECEF Y coordinate in meters. Must not be NULL. 
* `out_z` Output ECEF Z coordinate in meters. Must not be NULL. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if outputs are NULL. 





        

<hr>



### function syn\_geo\_wgs84\_to\_enu 

_Direct conversion from WGS-84 Geodetic to Local East-North-Up (ENU)._ 
```C++
SYN_Status syn_geo_wgs84_to_enu (
    double lat_deg,
    double lon_deg,
    double alt_m,
    double ref_lat_deg,
    double ref_lon_deg,
    double ref_alt_m,
    SYN_ENU * out_enu
) 
```



Convenience function combining WGS84-&gt;ECEF and ECEF-&gt;ENU.




**Parameters:**


* `lat_deg` Target Latitude in decimal degrees. 
* `lon_deg` Target Longitude in decimal degrees. 
* `alt_m` Target Altitude in meters. 
* `ref_lat_deg` Reference origin Latitude in decimal degrees. 
* `ref_lon_deg` Reference origin Longitude in decimal degrees. 
* `ref_alt_m` Reference origin Altitude in meters. 
* `out_enu` Output ENU Cartesian coordinates in meters. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if out\_enu is NULL. 





        

<hr>
## Macro Definition Documentation





### define SYN\_GEO\_USE\_FIXED\_POINT 

```C++
#define SYN_GEO_USE_FIXED_POINT `SYN_GNSS_USE_FIXED_POINT`
```



Geodesy fixed-point mode flag 


        

<hr>



### define SYN\_GEO\_WGS84\_A 

```C++
#define SYN_GEO_WGS84_A `6378137.0`
```



WGS-84 Semi-major axis in meters (a). 


        

<hr>



### define SYN\_GEO\_WGS84\_E2 

```C++
#define SYN_GEO_WGS84_E2 `0.0066943799901413165`
```



WGS-84 First eccentricity squared (e^2 = 2f - f^2). 


        

<hr>



### define SYN\_GEO\_WGS84\_INV\_F 

```C++
#define SYN_GEO_WGS84_INV_F `298.257223563`
```



WGS-84 Inverse flattening (1/f). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_geo.h`

