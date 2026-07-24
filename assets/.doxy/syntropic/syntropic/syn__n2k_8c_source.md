

# File syn\_n2k.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_n2k.c**](syn__n2k_8c.md)

[Go to the documentation of this file](syn__n2k_8c.md)


```C++

#include "syn_n2k.h"
#include "../util/syn_pack.h"
#include <string.h>

SYN_Status syn_n2k_encode_position_rapid(uint8_t sa, const SYN_N2K_PositionRapid *pos, SYN_CAN_Frame *frame)
{
    if (!pos || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(2, SYN_N2K_PGN_POS_RAPID, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    size_t pos_idx = 0;
    syn_pack_u32_le(frame->data, &pos_idx, (uint32_t)pos->latitude_1e7);
    syn_pack_u32_le(frame->data, &pos_idx, (uint32_t)pos->longitude_1e7);

    return SYN_OK;
}

SYN_Status syn_n2k_decode_position_rapid(const SYN_CAN_Frame *frame, SYN_N2K_PositionRapid *pos)
{
    if (!frame || !pos || frame->dlc < 8) return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    pos->latitude_1e7  = (int32_t)syn_unpack_u32_le(frame->data, &pos_idx);
    pos->longitude_1e7 = (int32_t)syn_unpack_u32_le(frame->data, &pos_idx);

    return SYN_OK;
}

SYN_Status syn_n2k_encode_cog_sog_rapid(uint8_t sa, const SYN_N2K_CogSogRapid *cog_sog, SYN_CAN_Frame *frame)
{
    if (!cog_sog || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(2, SYN_N2K_PGN_COG_SOG_RAPID, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    size_t pos_idx = 0;
    syn_pack_u8(frame->data, &pos_idx, cog_sog->sid);
    syn_pack_u8(frame->data, &pos_idx, (uint8_t)(cog_sog->cog_ref & 0x03U) | 0xFCU);
    syn_pack_u16_le(frame->data, &pos_idx, cog_sog->cog_rad_1e4);
    syn_pack_u16_le(frame->data, &pos_idx, cog_sog->sog_m_s_1e2);
    syn_pack_u8(frame->data, &pos_idx, 0xFFU); /* Reserved */
    syn_pack_u8(frame->data, &pos_idx, 0xFFU); /* Reserved */

    return SYN_OK;
}

SYN_Status syn_n2k_decode_cog_sog_rapid(const SYN_CAN_Frame *frame, SYN_N2K_CogSogRapid *cog_sog)
{
    if (!frame || !cog_sog || frame->dlc < 6) return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    cog_sog->sid           = syn_unpack_u8(frame->data, &pos_idx);
    cog_sog->cog_ref       = syn_unpack_u8(frame->data, &pos_idx) & 0x03U;
    cog_sog->cog_rad_1e4   = syn_unpack_u16_le(frame->data, &pos_idx);
    cog_sog->sog_m_s_1e2   = syn_unpack_u16_le(frame->data, &pos_idx);

    return SYN_OK;
}

SYN_Status syn_n2k_encode_heading(uint8_t sa, const SYN_N2K_VesselHeading *heading, SYN_CAN_Frame *frame)
{
    if (!heading || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(2, SYN_N2K_PGN_VESSEL_HEADING, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    size_t pos_idx = 0;
    syn_pack_u8(frame->data, &pos_idx, heading->sid);
    syn_pack_u16_le(frame->data, &pos_idx, heading->heading_rad_1e4);
    syn_pack_u16_le(frame->data, &pos_idx, (uint16_t)heading->deviation_rad_1e4);
    syn_pack_u16_le(frame->data, &pos_idx, (uint16_t)heading->variation_rad_1e4);
    syn_pack_u8(frame->data, &pos_idx, (uint8_t)(heading->heading_ref & 0x03U) | 0xFCU);

    return SYN_OK;
}

SYN_Status syn_n2k_decode_heading(const SYN_CAN_Frame *frame, SYN_N2K_VesselHeading *heading)
{
    if (!frame || !heading || frame->dlc < 8) return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    heading->sid               = syn_unpack_u8(frame->data, &pos_idx);
    heading->heading_rad_1e4   = syn_unpack_u16_le(frame->data, &pos_idx);
    heading->deviation_rad_1e4 = syn_unpack_i16_le(frame->data, &pos_idx);
    heading->variation_rad_1e4 = syn_unpack_i16_le(frame->data, &pos_idx);
    heading->heading_ref       = syn_unpack_u8(frame->data, &pos_idx) & 0x03U;

    return SYN_OK;
}

SYN_Status syn_n2k_encode_battery(uint8_t sa, const SYN_N2K_BatteryStatus *battery, SYN_CAN_Frame *frame)
{
    if (!battery || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(6, SYN_N2K_PGN_BATTERY_STATUS, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    size_t pos_idx = 0;
    syn_pack_u8(frame->data, &pos_idx, battery->instance);
    syn_pack_u16_le(frame->data, &pos_idx, battery->voltage_1e2);
    syn_pack_u16_le(frame->data, &pos_idx, (uint16_t)battery->current_1e1);
    syn_pack_u16_le(frame->data, &pos_idx, battery->temperature_1e1);
    syn_pack_u8(frame->data, &pos_idx, battery->sid);

    return SYN_OK;
}

SYN_Status syn_n2k_decode_battery(const SYN_CAN_Frame *frame, SYN_N2K_BatteryStatus *battery)
{
    if (!frame || !battery || frame->dlc < 8) return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    battery->instance        = syn_unpack_u8(frame->data, &pos_idx);
    battery->voltage_1e2     = syn_unpack_u16_le(frame->data, &pos_idx);
    battery->current_1e1     = syn_unpack_i16_le(frame->data, &pos_idx);
    battery->temperature_1e1 = syn_unpack_u16_le(frame->data, &pos_idx);
    battery->sid             = syn_unpack_u8(frame->data, &pos_idx);

    return SYN_OK;
}

SYN_Status syn_n2k_encode_dc_detailed(uint8_t sa, const SYN_N2K_DcDetailedStatus *dc, SYN_CAN_Frame *frame)
{
    if (!dc || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(6, SYN_N2K_PGN_DC_DETAILED_STATUS, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    size_t pos_idx = 0;
    syn_pack_u8(frame->data, &pos_idx, dc->sid);
    syn_pack_u8(frame->data, &pos_idx, dc->instance);
    syn_pack_u8(frame->data, &pos_idx, dc->dc_type);
    syn_pack_u8(frame->data, &pos_idx, dc->state_of_charge);
    syn_pack_u8(frame->data, &pos_idx, dc->state_of_health);
    syn_pack_u16_le(frame->data, &pos_idx, dc->time_to_go_min);
    syn_pack_u8(frame->data, &pos_idx, (uint8_t)dc->capacity_ah_1e1);

    return SYN_OK;
}

SYN_Status syn_n2k_decode_dc_detailed(const SYN_CAN_Frame *frame, SYN_N2K_DcDetailedStatus *dc)
{
    if (!frame || !dc || frame->dlc < 8) return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    dc->sid             = syn_unpack_u8(frame->data, &pos_idx);
    dc->instance        = syn_unpack_u8(frame->data, &pos_idx);
    dc->dc_type         = syn_unpack_u8(frame->data, &pos_idx);
    dc->state_of_charge = syn_unpack_u8(frame->data, &pos_idx);
    dc->state_of_health = syn_unpack_u8(frame->data, &pos_idx);
    dc->time_to_go_min  = syn_unpack_u16_le(frame->data, &pos_idx);
    dc->capacity_ah_1e1 = (uint16_t)syn_unpack_u8(frame->data, &pos_idx);

    return SYN_OK;
}

SYN_Status syn_n2k_encode_environment(uint8_t sa, const SYN_N2K_EnvParams *env, SYN_CAN_Frame *frame)
{
    if (!env || !frame) return SYN_INVALID_PARAM;

    frame->id = syn_j1939_id_pack(6, SYN_N2K_PGN_ENV_PARAMS, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    size_t pos_idx = 0;
    syn_pack_u8(frame->data, &pos_idx, env->sid);
    syn_pack_u16_le(frame->data, &pos_idx, env->water_temp_1e2);
    syn_pack_u16_le(frame->data, &pos_idx, env->air_temp_1e2);
    syn_pack_u16_le(frame->data, &pos_idx, env->pressure_pa_1e2);
    syn_pack_u8(frame->data, &pos_idx, 0xFFU); /* Reserved */

    return SYN_OK;
}

SYN_Status syn_n2k_decode_environment(const SYN_CAN_Frame *frame, SYN_N2K_EnvParams *env)
{
    if (!frame || !env || frame->dlc < 7) return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    env->sid            = syn_unpack_u8(frame->data, &pos_idx);
    env->water_temp_1e2 = syn_unpack_u16_le(frame->data, &pos_idx);
    env->air_temp_1e2   = syn_unpack_u16_le(frame->data, &pos_idx);
    env->pressure_pa_1e2= syn_unpack_u16_le(frame->data, &pos_idx);

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
```


