/**
 * @file syn_cia418.c
 * @brief CANopen CiA 418 Device Profile for Batteries & Energy Management implementation.
 */

#include "syn_cia418.h"
#include <string.h>

SYN_Status syn_cia418_init(SYN_CiA418_State *state)
{
    if (!state) return SYN_INVALID_PARAM;
    memset(state, 0, sizeof(*state));
    return SYN_OK;
}

SYN_Status syn_cia418_od_read(const SYN_CiA418_State *state, uint16_t index, uint8_t subindex, uint8_t *out_data, size_t *out_len)
{
    if (!state || !out_data || !out_len || subindex != 0) return SYN_INVALID_PARAM;

    switch (index) {
        case SYN_CIA418_OD_SOC:
            out_data[0] = (uint8_t)(state->soc_0_1pct & 0xFFU);
            out_data[1] = (uint8_t)((state->soc_0_1pct >> 8) & 0xFFU);
            *out_len = 2;
            return SYN_OK;

        case SYN_CIA418_OD_SOH:
            out_data[0] = (uint8_t)(state->soh_0_1pct & 0xFFU);
            out_data[1] = (uint8_t)((state->soh_0_1pct >> 8) & 0xFFU);
            *out_len = 2;
            return SYN_OK;

        case SYN_CIA418_OD_VOLTAGE:
            out_data[0] = (uint8_t)(state->voltage_mv & 0xFFU);
            out_data[1] = (uint8_t)((state->voltage_mv >> 8) & 0xFFU);
            out_data[2] = (uint8_t)((state->voltage_mv >> 16) & 0xFFU);
            out_data[3] = (uint8_t)((state->voltage_mv >> 24) & 0xFFU);
            *out_len = 4;
            return SYN_OK;

        case SYN_CIA418_OD_CURRENT: {
            uint32_t val = (uint32_t)state->current_ma;
            out_data[0] = (uint8_t)(val & 0xFFU);
            out_data[1] = (uint8_t)((val >> 8) & 0xFFU);
            out_data[2] = (uint8_t)((val >> 16) & 0xFFU);
            out_data[3] = (uint8_t)((val >> 24) & 0xFFU);
            *out_len = 4;
            return SYN_OK;
        }

        case SYN_CIA418_OD_TEMPERATURE: {
            uint16_t val = (uint16_t)state->temperature_0_1c;
            out_data[0] = (uint8_t)(val & 0xFFU);
            out_data[1] = (uint8_t)((val >> 8) & 0xFFU);
            *out_len = 2;
            return SYN_OK;
        }

        case SYN_CIA418_OD_TIME_TO_GO:
            out_data[0] = (uint8_t)(state->time_to_go_sec & 0xFFU);
            out_data[1] = (uint8_t)((state->time_to_go_sec >> 8) & 0xFFU);
            out_data[2] = (uint8_t)((state->time_to_go_sec >> 16) & 0xFFU);
            out_data[3] = (uint8_t)((state->time_to_go_sec >> 24) & 0xFFU);
            *out_len = 4;
            return SYN_OK;

        case SYN_CIA418_OD_MIN_CELL_VOLTAGE:
            out_data[0] = (uint8_t)(state->min_cell_voltage_mv & 0xFFU);
            out_data[1] = (uint8_t)((state->min_cell_voltage_mv >> 8) & 0xFFU);
            *out_len = 2;
            return SYN_OK;

        case SYN_CIA418_OD_MAX_CELL_VOLTAGE:
            out_data[0] = (uint8_t)(state->max_cell_voltage_mv & 0xFFU);
            out_data[1] = (uint8_t)((state->max_cell_voltage_mv >> 8) & 0xFFU);
            *out_len = 2;
            return SYN_OK;

        case SYN_CIA418_OD_MIN_CELL_TEMP: {
            uint16_t val = (uint16_t)state->min_cell_temp_0_1c;
            out_data[0] = (uint8_t)(val & 0xFFU);
            out_data[1] = (uint8_t)((val >> 8) & 0xFFU);
            *out_len = 2;
            return SYN_OK;
        }

        case SYN_CIA418_OD_MAX_CELL_TEMP: {
            uint16_t val = (uint16_t)state->max_cell_temp_0_1c;
            out_data[0] = (uint8_t)(val & 0xFFU);
            out_data[1] = (uint8_t)((val >> 8) & 0xFFU);
            *out_len = 2;
            return SYN_OK;
        }

        default:
            break;
    }

    return SYN_ERROR;
}
