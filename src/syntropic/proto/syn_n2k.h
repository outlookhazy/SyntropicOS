/**
 * @file syn_n2k.h
 * @brief NMEA 2000 Marine CAN Protocol Stack.
 * @ingroup syn_protocol
 *
 * Implements NMEA 2000 (IEC 61162-3) marine CAN bus PGN encoders, decoders,
 * and Fast-Packet multi-frame transport protocol reassembly.
 */

#ifndef SYN_N2K_H
#define SYN_N2K_H

#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"
#include "syn_j1939.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup n2k_pgns Standard NMEA 2000 Parameter Group Numbers
  * @{ */
/* Network Management */
#define SYN_N2K_PGN_ISO_ACKNOWLEDGEMENT 59392U  /**< ISO Acknowledgement (0x0E800) */
#define SYN_N2K_PGN_ISO_REQUEST         59904U  /**< ISO Request (0x0EA00) */
#define SYN_N2K_PGN_ISO_ADDRESS_CLAIM   60928U  /**< ISO Address Claim (0x0EE00) */
#define SYN_N2K_PGN_SYSTEM_TIME         126992U /**< System Time (0x1F010) */
#define SYN_N2K_PGN_PRODUCT_INFO        126996U /**< Product Information (0x1F014) */

/* Steering, Heading & Navigation */
#define SYN_N2K_PGN_RUDDER               127245U /**< Rudder Angle (0x1F10D) */
#define SYN_N2K_PGN_VESSEL_HEADING      127250U /**< Vessel Heading (0x1F112) */
#define SYN_N2K_PGN_RATE_OF_TURN        127251U /**< Rate of Turn (0x1F113) */
#define SYN_N2K_PGN_ATTITUDE            127257U /**< Attitude (Pitch/Roll/Yaw) (0x1F119) */
#define SYN_N2K_PGN_MAGNETIC_VARIATION  127258U /**< Magnetic Variation (0x1F11A) */
#define SYN_N2K_PGN_SPEED_WATER         128259U /**< Speed Through Water (0x1F503) */
#define SYN_N2K_PGN_WATER_DEPTH         128267U /**< Water Depth (0x1F50B) */
#define SYN_N2K_PGN_DISTANCE_LOG        128275U /**< Distance Log (0x1F513) */
#define SYN_N2K_PGN_POS_RAPID           129025U /**< Position, Rapid Update (0x1F801) */
#define SYN_N2K_PGN_COG_SOG_RAPID       129026U /**< COG & SOG, Rapid Update (0x1F802) */
#define SYN_N2K_PGN_GNSS_POS            129029U /**< GNSS Position Data (0x1F805) */

/* Power & Battery Systems */
#define SYN_N2K_PGN_INVERTER_STATUS     127501U /**< Inverter Status (0x1F20D) */
#define SYN_N2K_PGN_AC_INPUT_STATUS     127503U /**< AC Input Status (0x1F20F) */
#define SYN_N2K_PGN_AC_OUTPUT_STATUS    127504U /**< AC Output Status (0x1F210) */
#define SYN_N2K_PGN_FLUID_LEVEL         127505U /**< Fluid Level (Fuel/Water/Waste) (0x1F211) */
#define SYN_N2K_PGN_DC_DETAILED_STATUS  127506U /**< DC Detailed Status (0x1F212) */
#define SYN_N2K_PGN_CHARGER_STATUS      127507U /**< Charger Status (0x1F213) */
#define SYN_N2K_PGN_BATTERY_STATUS      127508U /**< Battery Status (0x1F214) */
#define SYN_N2K_PGN_CHARGER_CONFIG      127509U /**< Charger Configuration (0x1F215) */

/* Engine & Transmission */
#define SYN_N2K_PGN_ENGINE_RAPID        127488U /**< Engine Parameters, Rapid Update (0x1F200) */
#define SYN_N2K_PGN_ENGINE_DYNAMIC      127489U /**< Engine Parameters, Dynamic (0x1F201) */
#define SYN_N2K_PGN_TRANSMISSION_PARAMS 127493U /**< Transmission Parameters (0x1F205) */
#define SYN_N2K_PGN_TRIM_TAB            127497U /**< Trim Tab Position (0x1F209) */

/* Environment */
#define SYN_N2K_PGN_WIND_DATA           130306U /**< Wind Data (0x1FD02) */
#define SYN_N2K_PGN_ENV_PARAMS          130310U /**< Environmental Parameters (0x1FD06) */
#define SYN_N2K_PGN_ENVIRONMENTAL_ACT   130311U /**< Environmental Parameters (Actual) (0x1FD07) */
/** @} */

/**
 * @brief Rapid Update Position (PGN 129025).
 * Coordinates stored in 1e-7 degrees (signed 32-bit integer).
 */
typedef struct {
    int32_t latitude_1e7;  /**< Latitude in 1e-7 degrees (-90.0000000 to +90.0000000) */
    int32_t longitude_1e7; /**< Longitude in 1e-7 degrees (-180.0000000 to +180.0000000) */
} SYN_N2K_PositionRapid;

/**
 * @brief Rapid Update COG & SOG (PGN 129026).
 */
typedef struct {
    uint8_t  sid;             /**< Sequence ID / Measurement counter */
    uint8_t  cog_ref;         /**< Course Reference (0=True, 1=Magnetic) */
    uint16_t cog_rad_1e4;     /**< Course Over Ground in 0.0001 radians (0 to 62831) */
    uint16_t sog_m_s_1e2;     /**< Speed Over Ground in 0.01 m/s (0 to 65532) */
} SYN_N2K_CogSogRapid;

/**
 * @brief Vessel Heading (PGN 127250).
 */
typedef struct {
    uint8_t  sid;             /**< Sequence ID */
    uint16_t heading_rad_1e4; /**< Heading angle in 0.0001 radians */
    int16_t  deviation_rad_1e4;/**< Magnetic Deviation in 0.0001 radians */
    int16_t  variation_rad_1e4;/**< Magnetic Variation in 0.0001 radians */
    uint8_t  heading_ref;     /**< Heading Reference (0=True, 1=Magnetic) */
} SYN_N2K_VesselHeading;

/**
 * @brief Battery Status (PGN 127508).
 */
typedef struct {
    uint8_t  sid;             /**< Sequence ID */
    uint8_t  instance;        /**< Battery Instance ID (0-254) */
    uint16_t voltage_1e2;     /**< Battery Voltage in 0.01 Volts (e.g. 1250 = 12.50V) */
    int16_t  current_1e1;     /**< Battery Current in 0.1 Amperes (e.g. 150 = 15.0A) */
    uint16_t temperature_1e1; /**< Battery Temperature in 0.1 Kelvin (e.g. 2982 = 298.2K / 25C) */
} SYN_N2K_BatteryStatus;

/**
 * @brief DC Detailed Status (PGN 127506).
 */
typedef struct {
    uint8_t  sid;             /**< Sequence ID */
    uint8_t  instance;        /**< DC Instance ID (0-254) */
    uint8_t  dc_type;         /**< DC Source Type (0=Battery, 1=Alternator, 2=Convertor, 3=Solar, 4=Wind) */
    uint8_t  state_of_charge; /**< State of Charge in 1% (0 to 100%) */
    uint8_t  state_of_health; /**< State of Health in 1% (0 to 100%) */
    uint16_t time_to_go_min;  /**< Time Remaining in minutes (0 to 65532 min) */
    uint16_t capacity_ah_1e1; /**< Capacity / Ripple Voltage in 0.1 AH */
} SYN_N2K_DcDetailedStatus;

/**
 * @brief BMS Cell Extreme Status (Fast-Packet PGN 127508 / 127513 Extension).
 */
typedef struct {
    uint8_t  sid;                  /**< Sequence ID */
    uint8_t  instance;             /**< Battery Instance ID */
    uint16_t cell_min_voltage_mv;  /**< Lowest cell voltage in pack in mV (e.g. 3250 = 3.250V) */
    uint16_t cell_max_voltage_mv;  /**< Highest cell voltage in pack in mV (e.g. 3420 = 3.420V) */
    uint16_t cell_min_temp_1e1;    /**< Lowest cell temperature in 0.1K (e.g. 2932 = 20.0C) */
    uint16_t cell_max_temp_1e1;    /**< Highest cell temperature in 0.1K (e.g. 3082 = 35.0C) */
} SYN_N2K_BmsCellStatus;

/**
 * @brief Environmental Parameters (PGN 130310).
 */
typedef struct {
    uint8_t  sid;             /**< Sequence ID */
    uint16_t water_temp_1e2;  /**< Water Temperature in 0.01 Kelvin */
    uint16_t air_temp_1e2;    /**< Outside Air Temperature in 0.01 Kelvin */
    uint16_t pressure_pa_1e2; /**< Atmospheric Pressure in 100 Pa (hPa / mbar) */
} SYN_N2K_EnvParams;

/**
 * @brief NMEA 2000 Fast-Packet RX Re-assembly Context.
 */
typedef struct {
    bool     active;           /**< Active Fast-Packet session flag */
    uint8_t  seq_id;           /**< Fast-Packet Sequence Identifier (0-7) */
    uint8_t  expected_frame;   /**< Next expected frame index (0..31) */
    uint8_t  sa;               /**< Sender Source Address */
    uint32_t pgn;              /**< Target PGN */
    uint8_t  total_bytes;      /**< Total payload length in bytes (max 223) */
    uint8_t  received_bytes;   /**< Currently received byte count */
    uint8_t  data[223];        /**< Re-assembled payload buffer */
} SYN_N2K_FastPacketRx;

/**
 * @brief Encode PGN 129025 (Position, Rapid Update) into an 8-byte CAN frame.
 * @param sa Source address.
 * @param pos Position structure (Lat/Lon in 1e-7 degrees).
 * @param frame Output CAN frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_encode_position_rapid(uint8_t sa, const SYN_N2K_PositionRapid *pos, SYN_CAN_Frame *frame);

/**
 * @brief Decode PGN 129025 (Position, Rapid Update) from an 8-byte CAN frame.
 * @param frame Source CAN frame.
 * @param pos Output Position structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_decode_position_rapid(const SYN_CAN_Frame *frame, SYN_N2K_PositionRapid *pos);

/**
 * @brief Encode PGN 129026 (COG & SOG, Rapid Update) into an 8-byte CAN frame.
 * @param sa Source address.
 * @param cog_sog COG & SOG structure.
 * @param frame Output CAN frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_encode_cog_sog_rapid(uint8_t sa, const SYN_N2K_CogSogRapid *cog_sog, SYN_CAN_Frame *frame);

/**
 * @brief Decode PGN 129026 (COG & SOG, Rapid Update) from an 8-byte CAN frame.
 * @param frame Source CAN frame.
 * @param cog_sog Output COG & SOG structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_decode_cog_sog_rapid(const SYN_CAN_Frame *frame, SYN_N2K_CogSogRapid *cog_sog);

/**
 * @brief Encode PGN 127250 (Vessel Heading) into an 8-byte CAN frame.
 * @param sa Source address.
 * @param heading Heading structure.
 * @param frame Output CAN frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_encode_heading(uint8_t sa, const SYN_N2K_VesselHeading *heading, SYN_CAN_Frame *frame);

/**
 * @brief Decode PGN 127250 (Vessel Heading) from an 8-byte CAN frame.
 * @param frame Source CAN frame.
 * @param heading Output Heading structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_decode_heading(const SYN_CAN_Frame *frame, SYN_N2K_VesselHeading *heading);

/**
 * @brief Encode PGN 127508 (Battery Status) into an 8-byte CAN frame.
 * @param sa Source address.
 * @param battery Battery status structure.
 * @param frame Output CAN frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_encode_battery(uint8_t sa, const SYN_N2K_BatteryStatus *battery, SYN_CAN_Frame *frame);

/**
 * @brief Decode PGN 127508 (Battery Status) from an 8-byte CAN frame.
 * @param frame Source CAN frame.
 * @param battery Output Battery status structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_decode_battery(const SYN_CAN_Frame *frame, SYN_N2K_BatteryStatus *battery);

/**
 * @brief Encode PGN 127506 (DC Detailed Status) into an 8-byte CAN frame.
 * @param sa Source address.
 * @param dc DC detailed status structure.
 * @param frame Output CAN frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_encode_dc_detailed(uint8_t sa, const SYN_N2K_DcDetailedStatus *dc, SYN_CAN_Frame *frame);

/**
 * @brief Decode PGN 127506 (DC Detailed Status) from an 8-byte CAN frame.
 * @param frame Source CAN frame.
 * @param dc Output DC detailed status structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_decode_dc_detailed(const SYN_CAN_Frame *frame, SYN_N2K_DcDetailedStatus *dc);

/**
 * @brief Encode BMS Cell Extreme Status into a Fast-Packet payload buffer.
 * @param bms BMS cell status structure.
 * @param out_payload Output buffer pointer (must be >= 10 bytes).
 * @param out_len Output payload length.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_encode_bms_cell_status(const SYN_N2K_BmsCellStatus *bms, uint8_t *out_payload, size_t *out_len);

/**
 * @brief Decode BMS Cell Extreme Status from a Fast-Packet payload buffer.
 * @param payload Received Fast-Packet payload buffer.
 * @param len Payload length in bytes.
 * @param bms Output BMS cell status structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_decode_bms_cell_status(const uint8_t *payload, size_t len, SYN_N2K_BmsCellStatus *bms);

/**
 * @brief Encode PGN 130310 (Environmental Parameters) into an 8-byte CAN frame.
 * @param sa Source address.
 * @param env Environmental parameters structure.
 * @param frame Output CAN frame.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_encode_environment(uint8_t sa, const SYN_N2K_EnvParams *env, SYN_CAN_Frame *frame);

/**
 * @brief Decode PGN 130310 (Environmental Parameters) from an 8-byte CAN frame.
 * @param frame Source CAN frame.
 * @param env Output Environmental parameters structure.
 * @return SYN_OK on success.
 */
SYN_Status syn_n2k_decode_environment(const SYN_CAN_Frame *frame, SYN_N2K_EnvParams *env);

/**
 * @brief Process an incoming CAN frame in the Fast-Packet multi-frame RX reassembler.
 * @param rx Fast-Packet RX state context.
 * @param frame Incoming CAN frame.
 * @param target_pgn Expected target PGN for the Fast-Packet message.
 * @param out_payload Output pointer receiving reassembled payload buffer.
 * @param out_len Output length of payload in bytes.
 * @return SYN_OK if payload complete, or SYN_BUSY if Fast-Packet frames are still pending.
 */
SYN_Status syn_n2k_fastpacket_process(SYN_N2K_FastPacketRx *rx,
                                      const SYN_CAN_Frame *frame,
                                      uint32_t target_pgn,
                                      const uint8_t **out_payload,
                                      size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_N2K_H */
