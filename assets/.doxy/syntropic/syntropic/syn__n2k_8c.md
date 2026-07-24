

# File syn\_n2k.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_n2k.c**](syn__n2k_8c.md)

[Go to the source code of this file](syn__n2k_8c_source.md)

_NMEA 2000 Marine CAN Protocol Stack implementation._ 

* `#include "syn_n2k.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_decode\_battery**](#function-syn_n2k_decode_battery) (const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, [**SYN\_N2K\_BatteryStatus**](structSYN__N2K__BatteryStatus.md) \* battery) <br>_Decode PGN 127508 (Battery Status) from an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_decode\_cog\_sog\_rapid**](#function-syn_n2k_decode_cog_sog_rapid) (const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, [**SYN\_N2K\_CogSogRapid**](structSYN__N2K__CogSogRapid.md) \* cog\_sog) <br>_Decode PGN 129026 (COG & SOG, Rapid Update) from an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_decode\_dc\_detailed**](#function-syn_n2k_decode_dc_detailed) (const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, [**SYN\_N2K\_DcDetailedStatus**](structSYN__N2K__DcDetailedStatus.md) \* dc) <br>_Decode PGN 127506 (DC Detailed Status) from an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_decode\_environment**](#function-syn_n2k_decode_environment) (const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, [**SYN\_N2K\_EnvParams**](structSYN__N2K__EnvParams.md) \* env) <br>_Decode PGN 130310 (Environmental Parameters) from an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_decode\_heading**](#function-syn_n2k_decode_heading) (const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, [**SYN\_N2K\_VesselHeading**](structSYN__N2K__VesselHeading.md) \* heading) <br>_Decode PGN 127250 (Vessel Heading) from an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_decode\_position\_rapid**](#function-syn_n2k_decode_position_rapid) (const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, [**SYN\_N2K\_PositionRapid**](structSYN__N2K__PositionRapid.md) \* pos) <br>_Decode PGN 129025 (Position, Rapid Update) from an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_encode\_battery**](#function-syn_n2k_encode_battery) (uint8\_t sa, const [**SYN\_N2K\_BatteryStatus**](structSYN__N2K__BatteryStatus.md) \* battery, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Encode PGN 127508 (Battery Status) into an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_encode\_cog\_sog\_rapid**](#function-syn_n2k_encode_cog_sog_rapid) (uint8\_t sa, const [**SYN\_N2K\_CogSogRapid**](structSYN__N2K__CogSogRapid.md) \* cog\_sog, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Encode PGN 129026 (COG & SOG, Rapid Update) into an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_encode\_dc\_detailed**](#function-syn_n2k_encode_dc_detailed) (uint8\_t sa, const [**SYN\_N2K\_DcDetailedStatus**](structSYN__N2K__DcDetailedStatus.md) \* dc, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Encode PGN 127506 (DC Detailed Status) into an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_encode\_environment**](#function-syn_n2k_encode_environment) (uint8\_t sa, const [**SYN\_N2K\_EnvParams**](structSYN__N2K__EnvParams.md) \* env, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Encode PGN 130310 (Environmental Parameters) into an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_encode\_heading**](#function-syn_n2k_encode_heading) (uint8\_t sa, const [**SYN\_N2K\_VesselHeading**](structSYN__N2K__VesselHeading.md) \* heading, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Encode PGN 127250 (Vessel Heading) into an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_encode\_position\_rapid**](#function-syn_n2k_encode_position_rapid) (uint8\_t sa, const [**SYN\_N2K\_PositionRapid**](structSYN__N2K__PositionRapid.md) \* pos, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Encode PGN 129025 (Position, Rapid Update) into an 8-byte CAN frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_n2k\_fastpacket\_process**](#function-syn_n2k_fastpacket_process) ([**SYN\_N2K\_FastPacketRx**](structSYN__N2K__FastPacketRx.md) \* rx, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, uint32\_t target\_pgn, const uint8\_t \*\* out\_payload, size\_t \* out\_len) <br>_Process an incoming CAN frame in the Fast-Packet multi-frame RX reassembler._  |




























## Public Functions Documentation




### function syn\_n2k\_decode\_battery 

_Decode PGN 127508 (Battery Status) from an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_decode_battery (
    const SYN_CAN_Frame * frame,
    SYN_N2K_BatteryStatus * battery
) 
```





**Parameters:**


* `frame` Source CAN frame. 
* `battery` Output Battery status structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_decode\_cog\_sog\_rapid 

_Decode PGN 129026 (COG & SOG, Rapid Update) from an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_decode_cog_sog_rapid (
    const SYN_CAN_Frame * frame,
    SYN_N2K_CogSogRapid * cog_sog
) 
```





**Parameters:**


* `frame` Source CAN frame. 
* `cog_sog` Output COG & SOG structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_decode\_dc\_detailed 

_Decode PGN 127506 (DC Detailed Status) from an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_decode_dc_detailed (
    const SYN_CAN_Frame * frame,
    SYN_N2K_DcDetailedStatus * dc
) 
```





**Parameters:**


* `frame` Source CAN frame. 
* `dc` Output DC detailed status structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_decode\_environment 

_Decode PGN 130310 (Environmental Parameters) from an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_decode_environment (
    const SYN_CAN_Frame * frame,
    SYN_N2K_EnvParams * env
) 
```





**Parameters:**


* `frame` Source CAN frame. 
* `env` Output Environmental parameters structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_decode\_heading 

_Decode PGN 127250 (Vessel Heading) from an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_decode_heading (
    const SYN_CAN_Frame * frame,
    SYN_N2K_VesselHeading * heading
) 
```





**Parameters:**


* `frame` Source CAN frame. 
* `heading` Output Heading structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_decode\_position\_rapid 

_Decode PGN 129025 (Position, Rapid Update) from an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_decode_position_rapid (
    const SYN_CAN_Frame * frame,
    SYN_N2K_PositionRapid * pos
) 
```





**Parameters:**


* `frame` Source CAN frame. 
* `pos` Output Position structure. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_encode\_battery 

_Encode PGN 127508 (Battery Status) into an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_encode_battery (
    uint8_t sa,
    const SYN_N2K_BatteryStatus * battery,
    SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `sa` Source address. 
* `battery` Battery status structure. 
* `frame` Output CAN frame. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_encode\_cog\_sog\_rapid 

_Encode PGN 129026 (COG & SOG, Rapid Update) into an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_encode_cog_sog_rapid (
    uint8_t sa,
    const SYN_N2K_CogSogRapid * cog_sog,
    SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `sa` Source address. 
* `cog_sog` COG & SOG structure. 
* `frame` Output CAN frame. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_encode\_dc\_detailed 

_Encode PGN 127506 (DC Detailed Status) into an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_encode_dc_detailed (
    uint8_t sa,
    const SYN_N2K_DcDetailedStatus * dc,
    SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `sa` Source address. 
* `dc` DC detailed status structure. 
* `frame` Output CAN frame. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_encode\_environment 

_Encode PGN 130310 (Environmental Parameters) into an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_encode_environment (
    uint8_t sa,
    const SYN_N2K_EnvParams * env,
    SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `sa` Source address. 
* `env` Environmental parameters structure. 
* `frame` Output CAN frame. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_encode\_heading 

_Encode PGN 127250 (Vessel Heading) into an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_encode_heading (
    uint8_t sa,
    const SYN_N2K_VesselHeading * heading,
    SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `sa` Source address. 
* `heading` Heading structure. 
* `frame` Output CAN frame. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_encode\_position\_rapid 

_Encode PGN 129025 (Position, Rapid Update) into an 8-byte CAN frame._ 
```C++
SYN_Status syn_n2k_encode_position_rapid (
    uint8_t sa,
    const SYN_N2K_PositionRapid * pos,
    SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `sa` Source address. 
* `pos` Position structure (Lat/Lon in 1e-7 degrees). 
* `frame` Output CAN frame. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_n2k\_fastpacket\_process 

_Process an incoming CAN frame in the Fast-Packet multi-frame RX reassembler._ 
```C++
SYN_Status syn_n2k_fastpacket_process (
    SYN_N2K_FastPacketRx * rx,
    const SYN_CAN_Frame * frame,
    uint32_t target_pgn,
    const uint8_t ** out_payload,
    size_t * out_len
) 
```





**Parameters:**


* `rx` Fast-Packet RX state context. 
* `frame` Incoming CAN frame. 
* `target_pgn` Expected target PGN for the Fast-Packet message. 
* `out_payload` Output pointer receiving reassembled payload buffer. 
* `out_len` Output length of payload in bytes. 



**Returns:**

SYN\_OK if payload complete, or SYN\_BUSY if Fast-Packet frames are still pending. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_n2k.c`

