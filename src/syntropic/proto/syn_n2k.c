#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_N2K) || SYN_USE_N2K

/**
 * @file syn_n2k.c
 * @brief NMEA 2000 Marine CAN Protocol Stack implementation.
 */

#include "../util/syn_pack.h"
#include "syn_n2k.h"

#include <string.h>

SYN_Status syn_n2k_encode_position_rapid(uint8_t sa, const SYN_N2K_PositionRapid *pos,
                                         SYN_CAN_Frame *frame)
{
    if (!pos || !frame)
        return SYN_INVALID_PARAM;

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
    if (!frame || !pos || frame->dlc < 8)
        return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    pos->latitude_1e7 = (int32_t)syn_unpack_u32_le(frame->data, &pos_idx);
    pos->longitude_1e7 = (int32_t)syn_unpack_u32_le(frame->data, &pos_idx);

    return SYN_OK;
}

SYN_Status syn_n2k_encode_cog_sog_rapid(uint8_t sa, const SYN_N2K_CogSogRapid *cog_sog,
                                        SYN_CAN_Frame *frame)
{
    if (!cog_sog || !frame)
        return SYN_INVALID_PARAM;

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
    if (!frame || !cog_sog || frame->dlc < 6)
        return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    cog_sog->sid = syn_unpack_u8(frame->data, &pos_idx);
    cog_sog->cog_ref = syn_unpack_u8(frame->data, &pos_idx) & 0x03U;
    cog_sog->cog_rad_1e4 = syn_unpack_u16_le(frame->data, &pos_idx);
    cog_sog->sog_m_s_1e2 = syn_unpack_u16_le(frame->data, &pos_idx);

    return SYN_OK;
}

SYN_Status syn_n2k_encode_heading(uint8_t sa, const SYN_N2K_VesselHeading *heading,
                                  SYN_CAN_Frame *frame)
{
    if (!heading || !frame)
        return SYN_INVALID_PARAM;

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
    if (!frame || !heading || frame->dlc < 8)
        return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    heading->sid = syn_unpack_u8(frame->data, &pos_idx);
    heading->heading_rad_1e4 = syn_unpack_u16_le(frame->data, &pos_idx);
    heading->deviation_rad_1e4 = syn_unpack_i16_le(frame->data, &pos_idx);
    heading->variation_rad_1e4 = syn_unpack_i16_le(frame->data, &pos_idx);
    heading->heading_ref = syn_unpack_u8(frame->data, &pos_idx) & 0x03U;

    return SYN_OK;
}

SYN_Status syn_n2k_encode_battery(uint8_t sa, const SYN_N2K_BatteryStatus *battery,
                                  SYN_CAN_Frame *frame)
{
    if (!battery || !frame)
        return SYN_INVALID_PARAM;

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
    if (!frame || !battery || frame->dlc < 8)
        return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    battery->instance = syn_unpack_u8(frame->data, &pos_idx);
    battery->voltage_1e2 = syn_unpack_u16_le(frame->data, &pos_idx);
    battery->current_1e1 = syn_unpack_i16_le(frame->data, &pos_idx);
    battery->temperature_1e1 = syn_unpack_u16_le(frame->data, &pos_idx);
    battery->sid = syn_unpack_u8(frame->data, &pos_idx);

    return SYN_OK;
}

SYN_Status syn_n2k_encode_dc_detailed(uint8_t sa, const SYN_N2K_DcDetailedStatus *dc,
                                      SYN_CAN_Frame *frame)
{
    if (!dc || !frame) {
        return SYN_INVALID_PARAM;
    }

    frame->id = syn_j1939_id_pack(6, SYN_N2K_PGN_DC_DETAILED_STATUS, sa, SYN_J1939_ADDR_GLOBAL);
    frame->dlc = 8;
    frame->extended = true;

    size_t pos_idx = 0;
    uint8_t cap_u8 = (dc->capacity_ah_1e1 > 0xFEU) ? 0xFFU : (uint8_t)dc->capacity_ah_1e1;
    syn_pack_u8(frame->data, &pos_idx, dc->sid);
    syn_pack_u8(frame->data, &pos_idx, dc->instance);
    syn_pack_u8(frame->data, &pos_idx, dc->dc_type);
    syn_pack_u8(frame->data, &pos_idx, dc->state_of_charge);
    syn_pack_u8(frame->data, &pos_idx, dc->state_of_health);
    syn_pack_u16_le(frame->data, &pos_idx, dc->time_to_go_min);
    syn_pack_u8(frame->data, &pos_idx, cap_u8);

    return SYN_OK;
}

SYN_Status syn_n2k_decode_dc_detailed(const SYN_CAN_Frame *frame, SYN_N2K_DcDetailedStatus *dc)
{
    if (!frame || !dc || frame->dlc < 8) {
        return SYN_INVALID_PARAM;
    }

    size_t pos_idx = 0;
    dc->sid = syn_unpack_u8(frame->data, &pos_idx);
    dc->instance = syn_unpack_u8(frame->data, &pos_idx);
    dc->dc_type = syn_unpack_u8(frame->data, &pos_idx);
    dc->state_of_charge = syn_unpack_u8(frame->data, &pos_idx);
    dc->state_of_health = syn_unpack_u8(frame->data, &pos_idx);
    dc->time_to_go_min = syn_unpack_u16_le(frame->data, &pos_idx);
    dc->ripple_voltage = 0xFFFFU;
    dc->capacity_ah_1e1 = (uint16_t)syn_unpack_u8(frame->data, &pos_idx);

    return SYN_OK;
}

SYN_Status syn_n2k_encode_dc_detailed_fastpacket(uint8_t sa, uint8_t seq_id,
                                                 const SYN_N2K_DcDetailedStatus *dc,
                                                 SYN_CAN_Frame frames_out[2])
{
    if (dc == NULL || frames_out == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint8_t raw[11];
    size_t pos_idx = 0U;
    syn_pack_u8(raw, &pos_idx, dc->sid);
    syn_pack_u8(raw, &pos_idx, dc->instance);
    syn_pack_u8(raw, &pos_idx, dc->dc_type);
    syn_pack_u8(raw, &pos_idx, dc->state_of_charge);
    syn_pack_u8(raw, &pos_idx, dc->state_of_health);
    syn_pack_u16_le(raw, &pos_idx, dc->time_to_go_min);
    syn_pack_u16_le(raw, &pos_idx, dc->ripple_voltage);
    syn_pack_u16_le(raw, &pos_idx, dc->capacity_ah_1e1);

    uint32_t can_id =
        syn_j1939_id_pack(6U, SYN_N2K_PGN_DC_DETAILED_STATUS, sa, SYN_J1939_ADDR_GLOBAL);
    uint8_t seq_bits = (uint8_t)((seq_id & 0x07U) << 5U);

    memset(&frames_out[0], 0, sizeof(SYN_CAN_Frame));
    frames_out[0].id = can_id;
    frames_out[0].dlc = 8U;
    frames_out[0].extended = true;
    frames_out[0].data[0] = (uint8_t)(seq_bits | 0U);
    frames_out[0].data[1] = 11U;
    memcpy(&frames_out[0].data[2], &raw[0], 6U);

    memset(&frames_out[1], 0, sizeof(SYN_CAN_Frame));
    frames_out[1].id = can_id;
    frames_out[1].dlc = 8U;
    frames_out[1].extended = true;
    frames_out[1].data[0] = (uint8_t)(seq_bits | 1U);
    memcpy(&frames_out[1].data[1], &raw[6], 5U);
    frames_out[1].data[6] = 0xFFU;
    frames_out[1].data[7] = 0xFFU;

    return SYN_OK;
}

SYN_Status syn_n2k_decode_dc_detailed_payload(const uint8_t *payload, size_t len,
                                              SYN_N2K_DcDetailedStatus *dc)
{
    if (payload == NULL || dc == NULL || len < 11U) {
        return SYN_INVALID_PARAM;
    }

    size_t pos_idx = 0U;
    dc->sid = syn_unpack_u8(payload, &pos_idx);
    dc->instance = syn_unpack_u8(payload, &pos_idx);
    dc->dc_type = syn_unpack_u8(payload, &pos_idx);
    dc->state_of_charge = syn_unpack_u8(payload, &pos_idx);
    dc->state_of_health = syn_unpack_u8(payload, &pos_idx);
    dc->time_to_go_min = syn_unpack_u16_le(payload, &pos_idx);
    dc->ripple_voltage = syn_unpack_u16_le(payload, &pos_idx);
    dc->capacity_ah_1e1 = syn_unpack_u16_le(payload, &pos_idx);

    return SYN_OK;
}

SYN_Status syn_n2k_encode_environment(uint8_t sa, const SYN_N2K_EnvParams *env,
                                      SYN_CAN_Frame *frame)
{
    if (!env || !frame)
        return SYN_INVALID_PARAM;

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
    if (!frame || !env || frame->dlc < 7)
        return SYN_INVALID_PARAM;

    size_t pos_idx = 0;
    env->sid = syn_unpack_u8(frame->data, &pos_idx);
    env->water_temp_1e2 = syn_unpack_u16_le(frame->data, &pos_idx);
    env->air_temp_1e2 = syn_unpack_u16_le(frame->data, &pos_idx);
    env->pressure_pa_1e2 = syn_unpack_u16_le(frame->data, &pos_idx);

    return SYN_OK;
}

/**
 * @brief Process the initial frame (frame_idx == 0) of an NMEA 2000 Fast-Packet sequence.
 * @param rx Fast-Packet RX context.
 * @param frame Incoming 8-byte CAN frame.
 * @param sa Unpacked J1939 source address.
 * @param target_pgn Target PGN matched by caller.
 * @param seq_id 3-bit Fast-Packet sequence identifier.
 * @param out_payload Output pointer to reassembled payload when complete.
 * @param out_len Output payload length when complete.
 * @return SYN_OK if single-frame Fast-Packet completed, SYN_BUSY if multi-frame started, or
 *         SYN_INVALID_PARAM if total_bytes exceeds buffer capacity.
 */
static SYN_Status n2k_fp_first_frame(SYN_N2K_FastPacketRx *rx, const SYN_CAN_Frame *frame,
                                     uint8_t sa, uint32_t target_pgn, uint8_t seq_id,
                                     const uint8_t **out_payload, size_t *out_len)
{
    uint8_t total_bytes = frame->data[1];
    if (total_bytes > sizeof(rx->data)) {
        return SYN_INVALID_PARAM;
    }

    rx->active = true;
    rx->seq_id = seq_id;
    rx->expected_frame = 1U;
    rx->sa = sa;
    rx->pgn = target_pgn;
    rx->total_bytes = total_bytes;
    rx->received_bytes = 0U;

    size_t copy_len = (total_bytes > 6U) ? 6U : total_bytes;
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

/**
 * @brief Process a continuation frame (frame_idx > 0) of an NMEA 2000 Fast-Packet sequence.
 * @param rx Fast-Packet RX context.
 * @param frame Incoming 8-byte CAN frame.
 * @param sa Unpacked J1939 source address.
 * @param pgn Unpacked 24-bit PGN of the continuation frame.
 * @param seq_id 3-bit Fast-Packet sequence identifier.
 * @param frame_idx 5-bit Fast-Packet frame counter (> 0).
 * @param out_payload Output pointer to reassembled payload when complete.
 * @param out_len Output payload length when complete.
 * @return SYN_OK on completion, SYN_BUSY while reassembling, or SYN_INVALID_PARAM on mismatch.
 */
static SYN_Status n2k_fp_cont_frame(SYN_N2K_FastPacketRx *rx, const SYN_CAN_Frame *frame,
                                    uint8_t sa, uint32_t pgn, uint8_t seq_id, uint8_t frame_idx,
                                    const uint8_t **out_payload, size_t *out_len)
{
    if (!rx->active || (rx->pgn != pgn) || (rx->seq_id != seq_id) ||
        (frame_idx != rx->expected_frame) || (sa != rx->sa)) {
        return SYN_INVALID_PARAM;
    }

    size_t offset = 6U + ((size_t)(frame_idx - 1U) * 7U);
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

SYN_Status syn_n2k_fastpacket_process(SYN_N2K_FastPacketRx *rx, const SYN_CAN_Frame *frame,
                                      uint32_t target_pgn, const uint8_t **out_payload,
                                      size_t *out_len)
{
    if (!rx || !frame || !out_payload || !out_len || !frame->extended || frame->dlc < 8) {
        return SYN_INVALID_PARAM;
    }

    SYN_J1939_Header hdr;
    (void)syn_j1939_id_unpack(frame->id, &hdr);
    if (hdr.pgn != target_pgn) {
        return SYN_INVALID_PARAM;
    }

    uint8_t seq_frame = frame->data[0];
    uint8_t seq_id = (seq_frame >> 5) & 0x07U;
    uint8_t frame_idx = seq_frame & 0x1FU;

    if (frame_idx == 0U) {
        return n2k_fp_first_frame(rx, frame, hdr.sa, target_pgn, seq_id, out_payload, out_len);
    }
    return n2k_fp_cont_frame(rx, frame, hdr.sa, hdr.pgn, seq_id, frame_idx, out_payload, out_len);
}

#endif /* SYN_USE_N2K */
