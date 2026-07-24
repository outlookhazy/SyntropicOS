/**
 * @file syn_n2k.c
 * @brief NMEA 2000 Marine CAN Protocol Stack implementation.
 */

#include "syn_n2k.h"
#include <string.h>

SYN_Status syn_n2k_encode_position_rapid(uint8_t sa, const SYN_N2K_PositionRapid *pos, SYN_CAN_Frame *frame)
{
    if (!pos || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(2, SYN_N2K_PGN_POS_RAPID, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    uint32_t lat = (uint32_t)pos->latitude_1e7;
    uint32_t lon = (uint32_t)pos->longitude_1e7;

    frame->data[0] = (uint8_t)(lat & 0xFFU);
    frame->data[1] = (uint8_t)((lat >> 8) & 0xFFU);
    frame->data[2] = (uint8_t)((lat >> 16) & 0xFFU);
    frame->data[3] = (uint8_t)((lat >> 24) & 0xFFU);

    frame->data[4] = (uint8_t)(lon & 0xFFU);
    frame->data[5] = (uint8_t)((lon >> 8) & 0xFFU);
    frame->data[6] = (uint8_t)((lon >> 16) & 0xFFU);
    frame->data[7] = (uint8_t)((lon >> 24) & 0xFFU);

    return SYN_OK;
}

SYN_Status syn_n2k_decode_position_rapid(const SYN_CAN_Frame *frame, SYN_N2K_PositionRapid *pos)
{
    if (!frame || !pos || frame->dlc < 8) return SYN_INVALID_PARAM;

    uint32_t lat = (uint32_t)frame->data[0] |
                  ((uint32_t)frame->data[1] << 8) |
                  ((uint32_t)frame->data[2] << 16) |
                  ((uint32_t)frame->data[3] << 24);

    uint32_t lon = (uint32_t)frame->data[4] |
                  ((uint32_t)frame->data[5] << 8) |
                  ((uint32_t)frame->data[6] << 16) |
                  ((uint32_t)frame->data[7] << 24);

    pos->latitude_1e7  = (int32_t)lat;
    pos->longitude_1e7 = (int32_t)lon;

    return SYN_OK;
}

SYN_Status syn_n2k_encode_cog_sog_rapid(uint8_t sa, const SYN_N2K_CogSogRapid *cog_sog, SYN_CAN_Frame *frame)
{
    if (!cog_sog || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(2, SYN_N2K_PGN_COG_SOG_RAPID, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    frame->data[0] = cog_sog->sid;
    frame->data[1] = (uint8_t)(cog_sog->cog_ref & 0x03U) | 0xFCU;
    frame->data[2] = (uint8_t)(cog_sog->cog_rad_1e4 & 0xFFU);
    frame->data[3] = (uint8_t)((cog_sog->cog_rad_1e4 >> 8) & 0xFFU);
    frame->data[4] = (uint8_t)(cog_sog->sog_m_s_1e2 & 0xFFU);
    frame->data[5] = (uint8_t)((cog_sog->sog_m_s_1e2 >> 8) & 0xFFU);
    frame->data[6] = 0xFFU; /* Reserved */
    frame->data[7] = 0xFFU; /* Reserved */

    return SYN_OK;
}

SYN_Status syn_n2k_decode_cog_sog_rapid(const SYN_CAN_Frame *frame, SYN_N2K_CogSogRapid *cog_sog)
{
    if (!frame || !cog_sog || frame->dlc < 6) return SYN_INVALID_PARAM;

    cog_sog->sid           = frame->data[0];
    cog_sog->cog_ref       = frame->data[1] & 0x03U;
    cog_sog->cog_rad_1e4   = (uint16_t)frame->data[2] | ((uint16_t)frame->data[3] << 8);
    cog_sog->sog_m_s_1e2   = (uint16_t)frame->data[4] | ((uint16_t)frame->data[5] << 8);

    return SYN_OK;
}

SYN_Status syn_n2k_encode_heading(uint8_t sa, const SYN_N2K_VesselHeading *heading, SYN_CAN_Frame *frame)
{
    if (!heading || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(2, SYN_N2K_PGN_VESSEL_HEADING, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    frame->data[0] = heading->sid;
    frame->data[1] = (uint8_t)(heading->heading_rad_1e4 & 0xFFU);
    frame->data[2] = (uint8_t)((heading->heading_rad_1e4 >> 8) & 0xFFU);
    frame->data[3] = (uint8_t)((uint16_t)heading->deviation_rad_1e4 & 0xFFU);
    frame->data[4] = (uint8_t)(((uint16_t)heading->deviation_rad_1e4 >> 8) & 0xFFU);
    frame->data[5] = (uint8_t)((uint16_t)heading->variation_rad_1e4 & 0xFFU);
    frame->data[6] = (uint8_t)(((uint16_t)heading->variation_rad_1e4 >> 8) & 0xFFU);
    frame->data[7] = (uint8_t)(heading->heading_ref & 0x03U) | 0xFCU;

    return SYN_OK;
}

SYN_Status syn_n2k_decode_heading(const SYN_CAN_Frame *frame, SYN_N2K_VesselHeading *heading)
{
    if (!frame || !heading || frame->dlc < 8) return SYN_INVALID_PARAM;

    heading->sid               = frame->data[0];
    heading->heading_rad_1e4   = (uint16_t)frame->data[1] | ((uint16_t)frame->data[2] << 8);
    heading->deviation_rad_1e4 = (int16_t)((uint16_t)frame->data[3] | ((uint16_t)frame->data[4] << 8));
    heading->variation_rad_1e4 = (int16_t)((uint16_t)frame->data[5] | ((uint16_t)frame->data[6] << 8));
    heading->heading_ref       = frame->data[7] & 0x03U;

    return SYN_OK;
}

SYN_Status syn_n2k_encode_battery(uint8_t sa, const SYN_N2K_BatteryStatus *battery, SYN_CAN_Frame *frame)
{
    if (!battery || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(6, SYN_N2K_PGN_BATTERY_STATUS, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    frame->data[0] = battery->instance;
    frame->data[1] = (uint8_t)(battery->voltage_1e2 & 0xFFU);
    frame->data[2] = (uint8_t)((battery->voltage_1e2 >> 8) & 0xFFU);
    frame->data[3] = (uint8_t)((uint16_t)battery->current_1e1 & 0xFFU);
    frame->data[4] = (uint8_t)(((uint16_t)battery->current_1e1 >> 8) & 0xFFU);
    frame->data[5] = (uint8_t)(battery->temperature_1e1 & 0xFFU);
    frame->data[6] = (uint8_t)((battery->temperature_1e1 >> 8) & 0xFFU);
    frame->data[7] = battery->sid;

    return SYN_OK;
}

SYN_Status syn_n2k_decode_battery(const SYN_CAN_Frame *frame, SYN_N2K_BatteryStatus *battery)
{
    if (!frame || !battery || frame->dlc < 8) return SYN_INVALID_PARAM;

    battery->instance        = frame->data[0];
    battery->voltage_1e2     = (uint16_t)frame->data[1] | ((uint16_t)frame->data[2] << 8);
    battery->current_1e1     = (int16_t)((uint16_t)frame->data[3] | ((uint16_t)frame->data[4] << 8));
    battery->temperature_1e1 = (uint16_t)frame->data[5] | ((uint16_t)frame->data[6] << 8);
    battery->sid             = frame->data[7];

    return SYN_OK;
}

SYN_Status syn_n2k_encode_dc_detailed(uint8_t sa, const SYN_N2K_DcDetailedStatus *dc, SYN_CAN_Frame *frame)
{
    if (!dc || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(6, SYN_N2K_PGN_DC_DETAILED_STATUS, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    frame->data[0] = dc->sid;
    frame->data[1] = dc->instance;
    frame->data[2] = dc->dc_type;
    frame->data[3] = dc->state_of_charge;
    frame->data[4] = dc->state_of_health;
    frame->data[5] = (uint8_t)(dc->time_to_go_min & 0xFFU);
    frame->data[6] = (uint8_t)((dc->time_to_go_min >> 8) & 0xFFU);
    frame->data[7] = (uint8_t)(dc->capacity_ah_1e1 & 0xFFU);

    return SYN_OK;
}

SYN_Status syn_n2k_decode_dc_detailed(const SYN_CAN_Frame *frame, SYN_N2K_DcDetailedStatus *dc)
{
    if (!frame || !dc || frame->dlc < 8) return SYN_INVALID_PARAM;

    dc->sid             = frame->data[0];
    dc->instance        = frame->data[1];
    dc->dc_type         = frame->data[2];
    dc->state_of_charge = frame->data[3];
    dc->state_of_health = frame->data[4];
    dc->time_to_go_min  = (uint16_t)frame->data[5] | ((uint16_t)frame->data[6] << 8);
    dc->capacity_ah_1e1 = (uint16_t)frame->data[7];

    return SYN_OK;
}

SYN_Status syn_n2k_encode_environment(uint8_t sa, const SYN_N2K_EnvParams *env, SYN_CAN_Frame *frame)
{
    if (!env || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(6, SYN_N2K_PGN_ENV_PARAMS, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    frame->data[0] = env->sid;
    frame->data[1] = (uint8_t)(env->water_temp_1e2 & 0xFFU);
    frame->data[2] = (uint8_t)((env->water_temp_1e2 >> 8) & 0xFFU);
    frame->data[3] = (uint8_t)(env->air_temp_1e2 & 0xFFU);
    frame->data[4] = (uint8_t)((env->air_temp_1e2 >> 8) & 0xFFU);
    frame->data[5] = (uint8_t)(env->pressure_pa_1e2 & 0xFFU);
    frame->data[6] = (uint8_t)((env->pressure_pa_1e2 >> 8) & 0xFFU);
    frame->data[7] = 0xFFU; /* Reserved */

    return SYN_OK;
}

SYN_Status syn_n2k_decode_environment(const SYN_CAN_Frame *frame, SYN_N2K_EnvParams *env)
{
    if (!frame || !env || frame->dlc < 7) return SYN_INVALID_PARAM;

    env->sid            = frame->data[0];
    env->water_temp_1e2 = (uint16_t)frame->data[1] | ((uint16_t)frame->data[2] << 8);
    env->air_temp_1e2   = (uint16_t)frame->data[3] | ((uint16_t)frame->data[4] << 8);
    env->pressure_pa_1e2= (uint16_t)frame->data[5] | ((uint16_t)frame->data[6] << 8);

    return SYN_OK;
}

SYN_Status syn_n2k_fastpacket_process(SYN_N2K_FastPacketRx *rx,
                                      const SYN_CAN_Frame *frame,
                                      uint32_t target_pgn,
                                      const uint8_t **out_payload,
                                      size_t *out_len)
{
    if (!rx || !frame || !out_payload || !out_len) return SYN_INVALID_PARAM;

    SYN_J1939_Header hdr;
    if (syn_j1939_id_unpack(frame->id, &hdr) != SYN_OK) return SYN_INVALID_PARAM;
    if (hdr.pgn != target_pgn) return SYN_INVALID_PARAM;

    uint8_t seq_frame = frame->data[0];
    uint8_t seq_id    = (seq_frame >> 5) & 0x07U;
    uint8_t frame_idx = seq_frame & 0x1FU;

    /* First frame of a Fast-Packet sequence */
    if (frame_idx == 0) {
        uint8_t total_bytes = frame->data[1];
        if (total_bytes > sizeof(rx->data)) return SYN_INVALID_PARAM;

        rx->active = true;
        rx->seq_id = seq_id;
        rx->expected_frame = 1;
        rx->sa = hdr.sa;
        rx->pgn = target_pgn;
        rx->total_bytes = total_bytes;
        rx->received_bytes = 0;

        size_t copy_len = (total_bytes > 6) ? 6 : total_bytes;
        memcpy(&rx->data[0], &frame->data[2], copy_len);
        rx->received_bytes = (uint8_t)copy_len;

        if (rx->received_bytes >= rx->total_bytes) {
            *out_payload = rx->data;
            *out_len = rx->total_bytes;
            rx->active = false;
            return SYN_OK;
        }
        return SYN_BUSY;
    }

    /* Subsequent Fast-Packet frame */
    if (rx->active && rx->seq_id == seq_id && frame_idx == rx->expected_frame && hdr.sa == rx->sa) {
        size_t offset = 6U + (size_t)(frame_idx - 1U) * 7U;
        if (offset < rx->total_bytes) {
            size_t remaining = rx->total_bytes - offset;
            size_t copy_len = (remaining > 7U) ? 7U : remaining;
            memcpy(&rx->data[offset], &frame->data[1], copy_len);
            rx->received_bytes = (uint8_t)(offset + copy_len);
            rx->expected_frame++;

            if (rx->received_bytes >= rx->total_bytes) {
                *out_payload = rx->data;
                *out_len = rx->total_bytes;
                rx->active = false;
                return SYN_OK;
            }
        }
        return SYN_BUSY;
    }

    return SYN_INVALID_PARAM;
}
