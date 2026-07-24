

# File syn\_dali.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dali.c**](syn__dali_8c.md)

[Go to the documentation of this file](syn__dali_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_DALI) || SYN_USE_DALI

#include "syn_dali.h"
#include "../util/syn_assert.h"

#include <string.h>

uint16_t syn_dali_encode_forward(uint8_t addr, uint8_t data_cmd)
{
    return (((uint16_t)addr) << 8) | (uint16_t)data_cmd;
}

bool syn_dali_decode_forward(uint16_t raw_16, SYN_DALI_ForwardFrame *frame)
{
    if (frame == NULL) return false;

    frame->raw      = raw_16;
    uint8_t addr    = (uint8_t)((raw_16 >> 8) & 0xFFU);
    frame->data_cmd = (uint8_t)(raw_16 & 0xFFU);

    /* Address decoding:
     * - Bit 7 = 0: Short Address (0a aaaa aS) -> short addr = a aaaa
     * - Bit 7..4 = 1000..1001: Group Address (100g gggS) -> group = g ggg
     * - Bit 7..1 = 1111 111: Broadcast (1111 111S)
     * - Otherwise: Special / Configuration
     */
    bool selector_bit = (addr & 0x01U) != 0U; /* Bit 0: 0 = Direct Arc Level, 1 = Command */
    frame->is_direct = !selector_bit;

    if ((addr & 0x80U) == 0x00U) {
        frame->addr_type = SYN_DALI_ADDR_SHORT;
        frame->address   = (addr >> 1) & 0x3FU;
    } else if ((addr & 0xE0U) == 0x80U) {
        frame->addr_type = SYN_DALI_ADDR_GROUP;
        frame->address   = (addr >> 1) & 0x0FU;
    } else if ((addr & 0xFEU) == 0xFEU) {
        frame->addr_type = SYN_DALI_ADDR_BROADCAST;
        frame->address   = 0xFFU;
    } else {
        frame->addr_type = SYN_DALI_ADDR_SPECIAL;
        frame->address   = addr;
    }

    return true;
}

uint8_t syn_dali_encode_backward(uint8_t data)
{
    return data;
}

bool syn_dali_decode_backward(uint8_t raw_8, SYN_DALI_BackwardFrame *frame)
{
    if (frame == NULL) return false;
    frame->data = raw_8;
    return true;
}

SYN_Status syn_dali_slave_init(SYN_DALI_SlaveState *slave, const SYN_DALI_SlaveConfig *cfg)
{
    if (slave == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }

    memset(slave, 0, sizeof(*slave));
    slave->cfg = *cfg;

    if (slave->cfg.min_level == 0) slave->cfg.min_level = 1;
    if (slave->cfg.max_level == 0) slave->cfg.max_level = 254;
    if (slave->cfg.power_on_level != SYN_DALI_MASK_LEVEL) {
        slave->actual_level = slave->cfg.power_on_level;
    } else {
        slave->actual_level = slave->cfg.max_level;
    }

    memset(slave->scenes, SYN_DALI_MASK_LEVEL, sizeof(slave->scenes));
    slave->lamp_on = (slave->actual_level > 0);

    return SYN_OK;
}

static bool check_address_match(const SYN_DALI_SlaveState *slave, const SYN_DALI_ForwardFrame *req)
{
    if (req->addr_type == SYN_DALI_ADDR_BROADCAST) {
        return true;
    }
    if (req->addr_type == SYN_DALI_ADDR_SHORT) {
        return (slave->cfg.short_address != SYN_DALI_SHORT_ADDR_UNASSIGNED &&
                slave->cfg.short_address == req->address);
    }
    if (req->addr_type == SYN_DALI_ADDR_GROUP) {
        if (req->address < 16) {
            return ((slave->cfg.group_mask & (1U << req->address)) != 0U);
        }
    }
    return false;
}

SYN_Status syn_dali_slave_process(SYN_DALI_SlaveState *slave,
                                  const SYN_DALI_ForwardFrame *req,
                                  uint8_t *resp_data,
                                  bool *has_resp)
{
    if (slave == NULL || req == NULL || resp_data == NULL || has_resp == NULL) {
        return SYN_INVALID_PARAM;
    }

    *has_resp = false;
    *resp_data = 0;

    bool is_match = check_address_match(slave, req);

    if (is_match) {
        if (req->is_direct) {
            /* Direct Arc Power Control */
            uint8_t target_level = req->data_cmd;
            if (target_level == SYN_DALI_MASK_LEVEL) {
                /* MASK level: do nothing */
            } else if (target_level == 0) {
                slave->actual_level = 0;
                slave->lamp_on = false;
            } else {
                if (target_level < slave->cfg.min_level) target_level = slave->cfg.min_level;
                if (target_level > slave->cfg.max_level) target_level = slave->cfg.max_level;
                slave->actual_level = target_level;
                slave->lamp_on = true;
            }
            return SYN_OK;
        }

        /* Indirect Standard Commands */
        uint8_t cmd = req->data_cmd;
        switch (cmd) {
        case SYN_DALI_CMD_OFF:
            slave->actual_level = 0;
            slave->lamp_on = false;
            break;

        case SYN_DALI_CMD_UP:
            if (slave->actual_level < slave->cfg.max_level) {
                slave->actual_level++;
                slave->lamp_on = true;
            }
            break;

        case SYN_DALI_CMD_DOWN:
            if (slave->actual_level > slave->cfg.min_level) {
                slave->actual_level--;
            }
            break;

        case SYN_DALI_CMD_STEP_UP:
            if (slave->actual_level < slave->cfg.max_level) {
                slave->actual_level++;
                slave->lamp_on = true;
            }
            break;

        case SYN_DALI_CMD_STEP_DOWN:
            if (slave->actual_level > slave->cfg.min_level) {
                slave->actual_level--;
            } else {
                slave->actual_level = 0;
                slave->lamp_on = false;
            }
            break;

        case SYN_DALI_CMD_RECALL_MAX:
            slave->actual_level = slave->cfg.max_level;
            slave->lamp_on = true;
            break;

        case SYN_DALI_CMD_RECALL_MIN:
            slave->actual_level = slave->cfg.min_level;
            slave->lamp_on = true;
            break;

        case SYN_DALI_CMD_RESET:
            slave->actual_level = slave->cfg.power_on_level;
            slave->lamp_on = (slave->actual_level > 0);
            break;

        case SYN_DALI_CMD_STORE_ACTUAL_LEVEL_IN_DTR:
            slave->dtr0 = slave->actual_level;
            break;

        case SYN_DALI_CMD_STORE_DTR_AS_MAX_LEVEL:
            slave->cfg.max_level = slave->dtr0;
            break;

        case SYN_DALI_CMD_STORE_DTR_AS_MIN_LEVEL:
            slave->cfg.min_level = slave->dtr0;
            break;

        case SYN_DALI_CMD_STORE_DTR_AS_SYS_FAIL_LEVEL:
            slave->cfg.system_failure_level = slave->dtr0;
            break;

        case SYN_DALI_CMD_STORE_DTR_AS_POWER_ON_LEVEL:
            slave->cfg.power_on_level = slave->dtr0;
            break;

        case SYN_DALI_CMD_STORE_DTR_AS_FADE_TIME:
            slave->cfg.fade_time = slave->dtr0;
            break;

        case SYN_DALI_CMD_STORE_DTR_AS_FADE_RATE:
            slave->cfg.fade_rate = slave->dtr0;
            break;

        case SYN_DALI_CMD_STORE_DTR_AS_SHORT_ADDR:
            slave->cfg.short_address = (slave->dtr0 >> 1) & 0x3FU;
            break;

        case SYN_DALI_CMD_ENABLE_WRITE_MEMORY:
            slave->write_mem_enabled = true;
            break;

        case SYN_DALI_CMD_QUERY_STATUS: {
            uint8_t status = 0;
            if (slave->control_gear_failure) status |= (1U << 0);
            if (slave->lamp_failure)         status |= (1U << 1);
            if (slave->lamp_on)              status |= (1U << 2);
            if (slave->cfg.short_address == SYN_DALI_SHORT_ADDR_UNASSIGNED) status |= (1U << 6);
            *resp_data = status;
            *has_resp = true;
            break;
        }

        case SYN_DALI_CMD_QUERY_CONTENT_DTR:
            *resp_data = slave->dtr0;
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_CONTENT_DTR1:
            *resp_data = slave->dtr1;
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_CONTENT_DTR2:
            *resp_data = slave->dtr2;
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_GROUPS_0_7:
            *resp_data = (uint8_t)(slave->cfg.group_mask & 0xFFU);
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_GROUPS_8_15:
            *resp_data = (uint8_t)((slave->cfg.group_mask >> 8) & 0xFFU);
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_RANDOM_ADDR_H:
            *resp_data = (uint8_t)((slave->random_address >> 16) & 0xFFU);
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_RANDOM_ADDR_M:
            *resp_data = (uint8_t)((slave->random_address >> 8) & 0xFFU);
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_RANDOM_ADDR_L:
            *resp_data = (uint8_t)(slave->random_address & 0xFFU);
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_ACTUAL_LEVEL:
            *resp_data = slave->actual_level;
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_MAX_LEVEL:
            *resp_data = slave->cfg.max_level;
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_MIN_LEVEL:
            *resp_data = slave->cfg.min_level;
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_POWER_ON_LEVEL:
            *resp_data = slave->cfg.power_on_level;
            *has_resp = true;
            break;

        case SYN_DALI_CMD_QUERY_SYS_FAIL_LEVEL:
            *resp_data = slave->cfg.system_failure_level;
            *has_resp = true;
            break;

        default:
            if (cmd >= SYN_DALI_CMD_GO_TO_SCENE_BASE && cmd <= (SYN_DALI_CMD_GO_TO_SCENE_BASE + 15U)) {
                uint8_t scene_idx = cmd - SYN_DALI_CMD_GO_TO_SCENE_BASE;
                uint8_t scene_lvl = slave->scenes[scene_idx];
                if (scene_lvl != SYN_DALI_MASK_LEVEL) {
                    slave->actual_level = scene_lvl;
                    slave->lamp_on = (scene_lvl > 0);
                }
            } else if (cmd >= SYN_DALI_CMD_STORE_DTR_AS_SCENE_BASE && cmd <= (SYN_DALI_CMD_STORE_DTR_AS_SCENE_BASE + 15U)) {
                slave->scenes[cmd - SYN_DALI_CMD_STORE_DTR_AS_SCENE_BASE] = slave->dtr0;
            } else if (cmd >= SYN_DALI_CMD_REMOVE_FROM_SCENE_BASE && cmd <= (SYN_DALI_CMD_REMOVE_FROM_SCENE_BASE + 15U)) {
                slave->scenes[cmd - SYN_DALI_CMD_REMOVE_FROM_SCENE_BASE] = SYN_DALI_MASK_LEVEL;
            } else if (cmd >= SYN_DALI_CMD_ADD_TO_GROUP_BASE && cmd <= (SYN_DALI_CMD_ADD_TO_GROUP_BASE + 15U)) {
                slave->cfg.group_mask |= (1U << (cmd - SYN_DALI_CMD_ADD_TO_GROUP_BASE));
            } else if (cmd >= SYN_DALI_CMD_REMOVE_FROM_GROUP_BASE && cmd <= (SYN_DALI_CMD_REMOVE_FROM_GROUP_BASE + 15U)) {
                slave->cfg.group_mask &= ~(1U << (cmd - SYN_DALI_CMD_REMOVE_FROM_GROUP_BASE));
            }
            break;
        }
    }

    /* Special / Configuration Commands */
    if (req->addr_type == SYN_DALI_ADDR_SPECIAL) {
        uint8_t spec_code = req->address;
        uint8_t spec_data = req->data_cmd;

        switch (spec_code) {
        case SYN_DALI_SPEC_TERMINATE:
            slave->initialise_active = false;
            break;

        case SYN_DALI_SPEC_DTR0:
            slave->dtr0 = spec_data;
            break;

        case SYN_DALI_SPEC_DTR1:
            slave->dtr1 = spec_data;
            break;

        case SYN_DALI_SPEC_DTR2:
            slave->dtr2 = spec_data;
            break;

        case SYN_DALI_SPEC_INITIALISE:
            slave->initialise_active = true;
            break;

        case SYN_DALI_SPEC_RANDOMISE:
            if (slave->initialise_active) {
                slave->random_address = ((uint32_t)spec_data << 16) | 0x1234U;
            }
            break;

        case SYN_DALI_SPEC_SEARCHADDRH:
            slave->search_address = (slave->search_address & 0x00FFFFU) | ((uint32_t)spec_data << 16);
            break;

        case SYN_DALI_SPEC_SEARCHADDRM:
            slave->search_address = (slave->search_address & 0xFF00FFU) | ((uint32_t)spec_data << 8);
            break;

        case SYN_DALI_SPEC_SEARCHADDRL:
            slave->search_address = (slave->search_address & 0xFFFF00U) | (uint32_t)spec_data;
            break;

        case SYN_DALI_SPEC_COMPARE:
            if (slave->initialise_active && slave->random_address <= slave->search_address) {
                *resp_data = 0xFFU;
                *has_resp = true;
            }
            break;

        case SYN_DALI_SPEC_PROGRAM_SHORT_ADDR:
            if (slave->initialise_active) {
                slave->cfg.short_address = (spec_data >> 1) & 0x3FU;
            }
            break;

        case SYN_DALI_SPEC_VERIFY_SHORT_ADDR:
            if (((spec_data >> 1) & 0x3FU) == slave->cfg.short_address) {
                *resp_data = 0xFFU;
                *has_resp = true;
            }
            break;

        case SYN_DALI_SPEC_QUERY_SHORT_ADDR:
            if (slave->cfg.short_address != SYN_DALI_SHORT_ADDR_UNASSIGNED) {
                *resp_data = (slave->cfg.short_address << 1) | 0x01U;
                *has_resp = true;
            } else {
                *resp_data = 0xFFU;
                *has_resp = true;
            }
            break;

        default:
            break;
        }
    }

    return SYN_OK;
}

size_t syn_dali_manchester_encode_byte(uint8_t val, uint8_t *bit_out)
{
    if (bit_out == NULL) return 0;

    size_t idx = 0;
    for (int i = 7; i >= 0; i--) {
        bool bit = (val & (1U << i)) != 0U;
        if (bit) {
            /* Logical 1: Low -> High */
            bit_out[idx++] = 0;
            bit_out[idx++] = 1;
        } else {
            /* Logical 0: High -> Low */
            bit_out[idx++] = 1;
            bit_out[idx++] = 0;
        }
    }
    return idx;
}

bool syn_dali_manchester_decode_byte(const uint8_t *bit_in, uint8_t *val_out)
{
    if (bit_in == NULL || val_out == NULL) return false;

    uint8_t val = 0;
    size_t idx = 0;

    for (int i = 7; i >= 0; i--) {
        uint8_t b0 = bit_in[idx++];
        uint8_t b1 = bit_in[idx++];

        if (b0 == 0 && b1 == 1) {
            val |= (1U << i); /* Logical 1 */
        } else if (b0 == 1 && b1 == 0) {
            /* Logical 0 */
        } else {
            return false; /* Invalid Manchester bi-phase transition */
        }
    }

    *val_out = val;
    return true;
}

#endif /* SYN_USE_DALI */
```


