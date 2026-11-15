

# File syn\_dali.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dali.c**](syn__dali_8c.md)

[Go to the documentation of this file](syn__dali_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_DALI) || SYN_USE_DALI


#include "../util/syn_assert.h"
#include "syn_dali.h"

#include <string.h>

uint16_t syn_dali_encode_forward(uint8_t addr, uint8_t data_cmd)
{
    return (((uint16_t)addr) << 8) | (uint16_t)data_cmd;
}

bool syn_dali_decode_forward(uint16_t raw_16, SYN_DALI_ForwardFrame *frame)
{
    if (frame == NULL)
        return false;

    frame->raw = raw_16;
    uint8_t addr = (uint8_t)((raw_16 >> 8) & 0xFFU);
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
        frame->address = (addr >> 1) & 0x3FU;
    } else if ((addr & 0xE0U) == 0x80U) {
        frame->addr_type = SYN_DALI_ADDR_GROUP;
        frame->address = (addr >> 1) & 0x0FU;
    } else if ((addr & 0xFEU) == 0xFEU) {
        frame->addr_type = SYN_DALI_ADDR_BROADCAST;
        frame->address = 0xFFU;
    } else {
        frame->addr_type = SYN_DALI_ADDR_SPECIAL;
        frame->address = addr;
    }

    return true;
}

uint8_t syn_dali_encode_backward(uint8_t data)
{
    return data;
}

bool syn_dali_decode_backward(uint8_t raw_8, SYN_DALI_BackwardFrame *frame)
{
    if (frame == NULL)
        return false;
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

    if (slave->cfg.min_level == 0)
        slave->cfg.min_level = 1;
    if (slave->cfg.max_level == 0)
        slave->cfg.max_level = 254;
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

static void dali_reset_state(SYN_DALI_SlaveState *slave)
{
    slave->actual_level = 254U;
    slave->lamp_on = true;
    slave->cfg.power_on_level = 254U;
    slave->cfg.system_failure_level = 254U;
    slave->cfg.min_level = 1U;
    slave->cfg.max_level = 254U;
    slave->cfg.fade_time = 0U;
    slave->cfg.fade_rate = 7U;
    slave->cfg.group_mask = 0U;
    memset(slave->scenes, SYN_DALI_MASK_LEVEL, sizeof(slave->scenes));
    slave->search_address = 0xFFFFFFU;
    slave->random_address = 0xFFFFFFU;
    slave->dtr0 = 0U;
    slave->dtr1 = 0U;
    slave->dtr2 = 0U;
    slave->dapc_sequence_active = false;
    slave->write_mem_enabled = false;
    slave->initialise_active = false;
    slave->withdrawn = false;
}

static void dali_exec_direct_arc(SYN_DALI_SlaveState *slave, uint8_t target_level)
{
    if (target_level == SYN_DALI_MASK_LEVEL) {
        return;
    }
    if (target_level == 0U) {
        slave->actual_level = 0U;
        slave->lamp_on = false;
        return;
    }
    if (target_level < slave->cfg.min_level) {
        target_level = slave->cfg.min_level;
    }
    if (target_level > slave->cfg.max_level) {
        target_level = slave->cfg.max_level;
    }
    slave->actual_level = target_level;
    slave->lamp_on = true;
}

static bool dali_exec_step_cmd(SYN_DALI_SlaveState *slave, uint8_t cmd)
{
    switch (cmd) {
    case SYN_DALI_CMD_OFF:
        slave->actual_level = 0U;
        slave->lamp_on = false;
        return true;

    case SYN_DALI_CMD_UP:
    case SYN_DALI_CMD_STEP_UP:
        if (slave->lamp_on && slave->actual_level > 0U &&
            slave->actual_level < slave->cfg.max_level) {
            slave->actual_level++;
        }
        return true;

    case SYN_DALI_CMD_DOWN:
        if (slave->actual_level > slave->cfg.min_level) {
            slave->actual_level--;
        }
        return true;

    case SYN_DALI_CMD_STEP_DOWN:
    case SYN_DALI_CMD_STEP_DOWN_AND_OFF:
        if (slave->actual_level > slave->cfg.min_level) {
            slave->actual_level--;
        } else {
            slave->actual_level = 0U;
            slave->lamp_on = false;
        }
        return true;

    default:
        return false;
    }
}

static bool dali_exec_preset_cmd(SYN_DALI_SlaveState *slave, uint8_t cmd)
{
    switch (cmd) {
    case SYN_DALI_CMD_RECALL_MAX:
        slave->actual_level = slave->cfg.max_level;
        slave->lamp_on = true;
        return true;

    case SYN_DALI_CMD_RECALL_MIN:
        slave->actual_level = slave->cfg.min_level;
        slave->lamp_on = true;
        return true;

    case SYN_DALI_CMD_ON_AND_STEP_UP:
        if (!slave->lamp_on || slave->actual_level == 0U) {
            slave->actual_level = slave->cfg.min_level;
            slave->lamp_on = true;
        } else if (slave->actual_level < slave->cfg.max_level) {
            slave->actual_level++;
        }
        return true;

    case SYN_DALI_CMD_ENABLE_DAPC_SEQUENCE:
        slave->dapc_sequence_active = true;
        return true;

    case SYN_DALI_CMD_RESET:
        dali_reset_state(slave);
        return true;

    default:
        return false;
    }
}

static inline uint8_t dali_clamp_u8(uint8_t val, uint8_t lo, uint8_t hi)
{
    if (val < lo) {
        return lo;
    }
    if (val > hi) {
        return hi;
    }
    return val;
}

static inline uint8_t dali_decode_short_addr_arg(uint8_t raw)
{
    if (raw == 0xFFU) {
        return SYN_DALI_SHORT_ADDR_UNASSIGNED;
    }
    return (uint8_t)((raw >> 1U) & 0x3FU);
}

static bool dali_exec_dtr_store_cmd(SYN_DALI_SlaveState *slave, uint8_t cmd)
{
    switch (cmd) {
    case SYN_DALI_CMD_STORE_ACTUAL_LEVEL_IN_DTR:
        slave->dtr0 = slave->actual_level;
        return true;
    case SYN_DALI_CMD_STORE_DTR_AS_MAX_LEVEL:
        slave->cfg.max_level = dali_clamp_u8(slave->dtr0, slave->cfg.min_level, 254U);
        return true;
    case SYN_DALI_CMD_STORE_DTR_AS_MIN_LEVEL:
        slave->cfg.min_level = dali_clamp_u8(slave->dtr0, 1U, slave->cfg.max_level);
        return true;
    case SYN_DALI_CMD_STORE_DTR_AS_SYS_FAIL_LEVEL:
        slave->cfg.system_failure_level = slave->dtr0;
        return true;
    case SYN_DALI_CMD_STORE_DTR_AS_POWER_ON_LEVEL:
        slave->cfg.power_on_level = slave->dtr0;
        return true;
    case SYN_DALI_CMD_STORE_DTR_AS_FADE_TIME:
        slave->cfg.fade_time = slave->dtr0;
        return true;
    case SYN_DALI_CMD_STORE_DTR_AS_FADE_RATE:
        slave->cfg.fade_rate = slave->dtr0;
        return true;
    case SYN_DALI_CMD_STORE_DTR_AS_SHORT_ADDR:
        slave->cfg.short_address = dali_decode_short_addr_arg(slave->dtr0);
        return true;
    case SYN_DALI_CMD_ENABLE_WRITE_MEMORY:
        slave->write_mem_enabled = true;
        return true;
    default:
        return false;
    }
}

static bool dali_exec_query_basic(const SYN_DALI_SlaveState *slave, uint8_t cmd, uint8_t *resp_data)
{
    switch (cmd) {
    case SYN_DALI_CMD_QUERY_STATUS: {
        uint8_t status = 0U;
        if (slave->control_gear_failure) {
            status |= (1U << 0);
        }
        if (slave->lamp_failure) {
            status |= (1U << 1);
        }
        if (slave->lamp_on) {
            status |= (1U << 2);
        }
        if (slave->cfg.short_address == SYN_DALI_SHORT_ADDR_UNASSIGNED) {
            status |= (1U << 6);
        }
        *resp_data = status;
        return true;
    }
    case SYN_DALI_CMD_QUERY_CONTENT_DTR:
        *resp_data = slave->dtr0;
        return true;
    case SYN_DALI_CMD_QUERY_CONTENT_DTR1:
        *resp_data = slave->dtr1;
        return true;
    case SYN_DALI_CMD_QUERY_CONTENT_DTR2:
        *resp_data = slave->dtr2;
        return true;
    case SYN_DALI_CMD_QUERY_GROUPS_0_7:
        *resp_data = (uint8_t)(slave->cfg.group_mask & 0xFFU);
        return true;
    case SYN_DALI_CMD_QUERY_GROUPS_8_15:
        *resp_data = (uint8_t)((slave->cfg.group_mask >> 8) & 0xFFU);
        return true;
    default:
        return false;
    }
}

static bool dali_exec_query_addr_levels(const SYN_DALI_SlaveState *slave, uint8_t cmd,
                                        uint8_t *resp_data)
{
    switch (cmd) {
    case SYN_DALI_CMD_QUERY_RANDOM_ADDR_H:
        *resp_data = (uint8_t)((slave->random_address >> 16) & 0xFFU);
        return true;
    case SYN_DALI_CMD_QUERY_RANDOM_ADDR_M:
        *resp_data = (uint8_t)((slave->random_address >> 8) & 0xFFU);
        return true;
    case SYN_DALI_CMD_QUERY_RANDOM_ADDR_L:
        *resp_data = (uint8_t)(slave->random_address & 0xFFU);
        return true;
    case SYN_DALI_CMD_QUERY_ACTUAL_LEVEL:
        *resp_data = slave->actual_level;
        return true;
    case SYN_DALI_CMD_QUERY_MAX_LEVEL:
        *resp_data = slave->cfg.max_level;
        return true;
    case SYN_DALI_CMD_QUERY_MIN_LEVEL:
        *resp_data = slave->cfg.min_level;
        return true;
    case SYN_DALI_CMD_QUERY_POWER_ON_LEVEL:
        *resp_data = slave->cfg.power_on_level;
        return true;
    case SYN_DALI_CMD_QUERY_SYS_FAIL_LEVEL:
        *resp_data = slave->cfg.system_failure_level;
        return true;
    default:
        return false;
    }
}

static void dali_exec_scene_group_cmd(SYN_DALI_SlaveState *slave, uint8_t cmd)
{
    if (cmd >= SYN_DALI_CMD_GO_TO_SCENE_BASE && cmd <= (SYN_DALI_CMD_GO_TO_SCENE_BASE + 15U)) {
        uint8_t scene_lvl = slave->scenes[cmd - SYN_DALI_CMD_GO_TO_SCENE_BASE];
        if (scene_lvl != SYN_DALI_MASK_LEVEL) {
            slave->actual_level = scene_lvl;
            slave->lamp_on = (scene_lvl > 0U);
        }
    } else if (cmd >= SYN_DALI_CMD_STORE_DTR_AS_SCENE_BASE &&
               cmd <= (SYN_DALI_CMD_STORE_DTR_AS_SCENE_BASE + 15U)) {
        slave->scenes[cmd - SYN_DALI_CMD_STORE_DTR_AS_SCENE_BASE] = slave->dtr0;
    } else if (cmd >= SYN_DALI_CMD_REMOVE_FROM_SCENE_BASE &&
               cmd <= (SYN_DALI_CMD_REMOVE_FROM_SCENE_BASE + 15U)) {
        slave->scenes[cmd - SYN_DALI_CMD_REMOVE_FROM_SCENE_BASE] = SYN_DALI_MASK_LEVEL;
    } else if (cmd >= SYN_DALI_CMD_ADD_TO_GROUP_BASE &&
               cmd <= (SYN_DALI_CMD_ADD_TO_GROUP_BASE + 15U)) {
        slave->cfg.group_mask |= (uint16_t)(1U << (cmd - SYN_DALI_CMD_ADD_TO_GROUP_BASE));
    } else if (cmd >= SYN_DALI_CMD_REMOVE_FROM_GROUP_BASE &&
               cmd <= (SYN_DALI_CMD_REMOVE_FROM_GROUP_BASE + 15U)) {
        slave->cfg.group_mask &= (uint16_t)(~(1U << (cmd - SYN_DALI_CMD_REMOVE_FROM_GROUP_BASE)));
    }
}

static bool dali_exec_query_capabilities(const SYN_DALI_SlaveState *slave, uint8_t cmd,
                                         uint8_t *resp_data, bool *has_resp)
{
    switch (cmd) {
    case SYN_DALI_CMD_QUERY_CONTROL_GEAR:
        *resp_data = 0xFFU;
        *has_resp = true;
        return true;
    case SYN_DALI_CMD_QUERY_LAMP_FAILURE:
        if (slave->lamp_failure) {
            *resp_data = 0xFFU;
            *has_resp = true;
        }
        return true;
    case SYN_DALI_CMD_QUERY_LAMP_POWER_ON:
        if (slave->lamp_on) {
            *resp_data = 0xFFU;
            *has_resp = true;
        }
        return true;
    case SYN_DALI_CMD_QUERY_DEVICE_TYPE:
        *resp_data = 0U;
        *has_resp = true;
        return true;
    case SYN_DALI_CMD_QUERY_PHYSICAL_MIN_LEVEL:
        *resp_data = 1U;
        *has_resp = true;
        return true;
    default:
        return false;
    }
}

static void dali_exec_indirect_cmd(SYN_DALI_SlaveState *slave, uint8_t cmd, uint8_t *resp_data,
                                   bool *has_resp)
{
    if (dali_exec_step_cmd(slave, cmd) || dali_exec_preset_cmd(slave, cmd) ||
        dali_exec_dtr_store_cmd(slave, cmd)) {
        return;
    }
    if (dali_exec_query_basic(slave, cmd, resp_data) ||
        dali_exec_query_addr_levels(slave, cmd, resp_data)) {
        *has_resp = true;
        return;
    }
    if (dali_exec_query_capabilities(slave, cmd, resp_data, has_resp)) {
        return;
    }
    dali_exec_scene_group_cmd(slave, cmd);
}

static bool dali_exec_special_search(SYN_DALI_SlaveState *slave, uint8_t spec_code,
                                     uint8_t spec_data)
{
    switch (spec_code) {
    case SYN_DALI_SPEC_TERMINATE:
        slave->initialise_active = false;
        slave->withdrawn = false;
        return true;
    case SYN_DALI_SPEC_DTR0:
        slave->dtr0 = spec_data;
        return true;
    case SYN_DALI_SPEC_DTR1:
        slave->dtr1 = spec_data;
        return true;
    case SYN_DALI_SPEC_DTR2:
        slave->dtr2 = spec_data;
        return true;
    case SYN_DALI_SPEC_INITIALISE:
        slave->initialise_active = true;
        slave->withdrawn = false;
        return true;
    case SYN_DALI_SPEC_RANDOMISE:
        if (slave->initialise_active) {
            slave->random_address = ((uint32_t)spec_data << 16) | 0x1234U;
        }
        return true;
    case SYN_DALI_SPEC_SEARCHADDRH:
        slave->search_address = (slave->search_address & 0x00FFFFU) | ((uint32_t)spec_data << 16);
        return true;
    case SYN_DALI_SPEC_SEARCHADDRM:
        slave->search_address = (slave->search_address & 0xFF00FFU) | ((uint32_t)spec_data << 8);
        return true;
    case SYN_DALI_SPEC_SEARCHADDRL:
        slave->search_address = (slave->search_address & 0xFFFF00U) | (uint32_t)spec_data;
        return true;
    default:
        return false;
    }
}

static void dali_exec_special_addr(SYN_DALI_SlaveState *slave, uint8_t spec_code, uint8_t spec_data,
                                   uint8_t *resp_data, bool *has_resp)
{
    switch (spec_code) {
    case SYN_DALI_SPEC_COMPARE:
        if (slave->initialise_active && !slave->withdrawn &&
            (slave->random_address <= slave->search_address)) {
            *resp_data = 0xFFU;
            *has_resp = true;
        }
        break;
    case SYN_DALI_SPEC_WITHDRAW:
        if (slave->initialise_active && (slave->random_address == slave->search_address)) {
            slave->withdrawn = true;
        }
        break;
    case SYN_DALI_SPEC_PROGRAM_SHORT_ADDR:
        if (slave->initialise_active) {
            slave->cfg.short_address = dali_decode_short_addr_arg(spec_data);
        }
        break;
    case SYN_DALI_SPEC_VERIFY_SHORT_ADDR:
        if ((slave->cfg.short_address != SYN_DALI_SHORT_ADDR_UNASSIGNED) && (spec_data != 0xFFU) &&
            (((spec_data >> 1U) & 0x3FU) == slave->cfg.short_address)) {
            *resp_data = 0xFFU;
            *has_resp = true;
        }
        break;
    case SYN_DALI_SPEC_QUERY_SHORT_ADDR:
        if (slave->cfg.short_address != SYN_DALI_SHORT_ADDR_UNASSIGNED) {
            *resp_data = (uint8_t)((slave->cfg.short_address << 1) | 0x01U);
        } else {
            *resp_data = 0xFFU;
        }
        *has_resp = true;
        break;
    default:
        break;
    }
}

SYN_Status syn_dali_slave_process(SYN_DALI_SlaveState *slave, const SYN_DALI_ForwardFrame *req,
                                  uint8_t *resp_data, bool *has_resp)
{
    if (slave == NULL || req == NULL || resp_data == NULL || has_resp == NULL) {
        return SYN_INVALID_PARAM;
    }

    *has_resp = false;
    *resp_data = 0U;

    if (check_address_match(slave, req)) {
        if (req->is_direct) {
            dali_exec_direct_arc(slave, req->data_cmd);
            return SYN_OK;
        }
        dali_exec_indirect_cmd(slave, req->data_cmd, resp_data, has_resp);
    }

    if (req->addr_type == SYN_DALI_ADDR_SPECIAL) {
        if (!dali_exec_special_search(slave, req->address, req->data_cmd)) {
            dali_exec_special_addr(slave, req->address, req->data_cmd, resp_data, has_resp);
        }
    }

    return SYN_OK;
}

size_t syn_dali_manchester_encode_byte(uint8_t val, uint8_t *bit_out)
{
    if (bit_out == NULL)
        return 0;

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
    if (bit_in == NULL || val_out == NULL)
        return false;

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


