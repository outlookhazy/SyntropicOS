

# File syn\_ccp.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ccp.c**](syn__ccp_8c.md)

[Go to the documentation of this file](syn__ccp_8c.md)


```C++

#include "syn_ccp.h"

#include "../util/syn_assert.h"
#include "../util/syn_pack.h"

#include <string.h>

void syn_ccp_init(SYN_CCP_Slave *slave, uint16_t station_addr)
{
    SYN_ASSERT(slave != NULL);

    memset(slave, 0, sizeof(*slave));
    slave->station_addr = station_addr;
    slave->connected = false;
    slave->unlocked_resources = SYN_CCP_RESOURCE_CAL | SYN_CCP_RESOURCE_DAQ | SYN_CCP_RESOURCE_PGM;
}

void syn_ccp_set_mta(SYN_CCP_Slave *slave, uint8_t mta_num, uint8_t ext, uintptr_t addr)
{
    if (slave == NULL) {
        return;
    }
    if (mta_num == 0) {
        slave->mta0_ext = ext;
        slave->mta0_addr = addr;
    } else if (mta_num == 1) {
        slave->mta1_ext = ext;
        slave->mta1_addr = addr;
    }
}

bool syn_ccp_process_cro(SYN_CCP_Slave *slave, const uint8_t cro[8], uint8_t dto_out[8])
{
    if (slave == NULL || cro == NULL || dto_out == NULL) {
        return false;
    }

    uint8_t cmd = cro[0];
    uint8_t ctr = cro[1];

    /* Prepare base CRM payload */
    memset(dto_out, 0, 8);
    dto_out[0] = SYN_CCP_PID_CRM;
    dto_out[1] = SYN_CCP_ERR_SUCCESS;
    dto_out[2] = ctr;

    /* Handle CONNECT */
    if (cmd == SYN_CCP_CMD_CONNECT) {
        uint16_t target_addr = syn_peek_u16_le(cro, 2);
        if (target_addr == slave->station_addr || target_addr == 0x0000U) {
            slave->connected = true;
            dto_out[1] = SYN_CCP_ERR_SUCCESS;
        } else {
            /* Station address mismatch */
            return false;
        }
        return true;
    }

    /* All commands after CONNECT require connected status */
    if (!slave->connected) {
        dto_out[1] = SYN_CCP_ERR_NOT_CONNECTED;
        return true;
    }

    switch (cmd) {
    case SYN_CCP_CMD_DISCONNECT: {
        uint8_t mode = cro[2];
        uint16_t target_addr = syn_peek_u16_le(cro, 3);
        if (target_addr == slave->station_addr || target_addr == 0x0000U) {
            if (mode == 0x00U || mode == 0x01U) {
                slave->connected = false;
            }
            dto_out[1] = SYN_CCP_ERR_SUCCESS;
        }
        break;
    }

    case SYN_CCP_CMD_GET_CCP_VERSION: {
        dto_out[3] = 0x02U; /* Main version 2 */
        dto_out[4] = 0x01U; /* Release version 1 */
        break;
    }

    case SYN_CCP_CMD_EXCHANGE_ID: {
        dto_out[3] = 0x08U;                     /* Slave device ID length */
        dto_out[4] = 0x00U;                     /* DataType */
        dto_out[5] = slave->unlocked_resources; /* Resource availability */
        dto_out[6] = slave->unlocked_resources; /* Protection status */
        break;
    }

    case SYN_CCP_CMD_GET_SEED: {
        uint8_t req_resource = cro[2];
        bool is_unlocked = (slave->unlocked_resources & req_resource) != 0U;
        dto_out[3] = is_unlocked ? 0x00U : 0x01U; /* 0: Unlocked, 1: Seed needed */
        uint32_t seed = 0U;
        if (slave->seed_cb != NULL) {
            seed = slave->seed_cb(req_resource, slave->seed_ctx);
        } else {
            seed = 0x5A5A5A5AU;
        }
        syn_poke_u32_le(seed, dto_out, 4);
        break;
    }

    case SYN_CCP_CMD_UNLOCK: {
        uint8_t resource = cro[2];
        uint32_t key = syn_peek_u32_le(cro, 4);
        bool valid = false;
        if (slave->unlock_cb != NULL) {
            valid = slave->unlock_cb(resource, key, slave->unlock_ctx);
        } else {
            valid = true;
        }
        if (valid) {
            slave->unlocked_resources |= resource;
        } else {
            dto_out[1] = SYN_CCP_ERR_ACCESS_DENIED;
        }
        dto_out[3] = slave->unlocked_resources;
        break;
    }

    case SYN_CCP_CMD_SET_MTA: {
        uint8_t mta_num = cro[2];
        uint8_t ext = cro[3];
        uint32_t addr = syn_peek_u32_le(cro, 4);
        if (addr != 0) {
            syn_ccp_set_mta(slave, mta_num, ext, (uintptr_t)addr);
        } else if (mta_num == 0) {
            slave->mta0_ext = ext;
        } else {
            slave->mta1_ext = ext;
        }
        break;
    }

    case SYN_CCP_CMD_DNLOAD: {
        uint8_t size = cro[2];
        if (size > 5) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            break;
        }
        if (slave->mta0_addr != 0) {
            uint8_t *ptr = (uint8_t *)(uintptr_t)slave->mta0_addr;
            memcpy(ptr, &cro[3], size);
            slave->mta0_addr += size;
        }
        dto_out[3] = slave->mta0_ext;
        syn_poke_u32_le((uint32_t)slave->mta0_addr, dto_out, 4);
        break;
    }

    case SYN_CCP_CMD_DNLOAD_6: {
        if (slave->mta0_addr != 0) {
            uint8_t *ptr = (uint8_t *)(uintptr_t)slave->mta0_addr;
            memcpy(ptr, &cro[2], 6);
            slave->mta0_addr += 6;
        }
        dto_out[3] = slave->mta0_ext;
        syn_poke_u32_le((uint32_t)slave->mta0_addr, dto_out, 4);
        break;
    }

    case SYN_CCP_CMD_UPLOAD: {
        uint8_t size = cro[2];
        if (size > 5) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            break;
        }
        if (slave->mta0_addr != 0) {
            const uint8_t *ptr = (const uint8_t *)(uintptr_t)slave->mta0_addr;
            memcpy(&dto_out[3], ptr, size);
            slave->mta0_addr += size;
        }
        break;
    }

    case SYN_CCP_CMD_SHORT_UP: {
        uint8_t size = cro[2];
        uint8_t ext = cro[3];
        uint32_t addr = syn_peek_u32_le(cro, 4);
        if (size > 5) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            break;
        }
        slave->mta0_ext = ext;
        if (addr != 0) {
            slave->mta0_addr = (uintptr_t)addr;
        }
        if (slave->mta0_addr != 0) {
            const uint8_t *ptr = (const uint8_t *)(uintptr_t)slave->mta0_addr;
            memcpy(&dto_out[3], ptr, size);
            slave->mta0_addr += size;
        }
        break;
    }

    case SYN_CCP_CMD_SET_CAL_PAGE: {
        uint32_t page_addr = syn_peek_u32_le(cro, 2);
        slave->active_cal_page = (uint8_t)(page_addr & 0xFFU);
        break;
    }

    case SYN_CCP_CMD_GET_ACTIVE_CAL_PAGE: {
        dto_out[3] = 0x00U;
        syn_poke_u32_le((uint32_t)slave->active_cal_page, dto_out, 4);
        break;
    }

    case SYN_CCP_CMD_CLEAR_MEMORY: {
        uint32_t size = syn_peek_u32_le(cro, 2);
        if (slave->erase_cb != NULL) {
            if (!slave->erase_cb((uint32_t)slave->mta0_addr, size, slave->erase_ctx)) {
                dto_out[1] = SYN_CCP_ERR_ACCESS_DENIED;
            }
        }
        break;
    }

    case SYN_CCP_CMD_PROGRAM: {
        uint8_t size = cro[2];
        if (size > 5) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            break;
        }
        if (slave->mta0_addr != 0) {
            uint8_t *ptr = (uint8_t *)(uintptr_t)slave->mta0_addr;
            memcpy(ptr, &cro[3], size);
            slave->mta0_addr += size;
        }
        dto_out[3] = slave->mta0_ext;
        syn_poke_u32_le(slave->mta0_addr, dto_out, 4);
        break;
    }

    case SYN_CCP_CMD_GET_DAQ_SIZE: {
        uint8_t daq_num = cro[2];
        if (daq_num >= SYN_CCP_MAX_DAQ_LISTS) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            break;
        }
        slave->selected_daq = daq_num;
        dto_out[3] = SYN_CCP_MAX_ODT_PER_DAQ;           /* Number of ODTs */
        dto_out[4] = daq_num * SYN_CCP_MAX_ODT_PER_DAQ; /* First PID */
        break;
    }

    case SYN_CCP_CMD_SET_DAQ_SIZE: {
        uint8_t daq_num = cro[2];
        uint8_t count = cro[4];
        if (daq_num >= SYN_CCP_MAX_DAQ_LISTS || count > SYN_CCP_MAX_ODT_PER_DAQ) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            break;
        }
        slave->daq_lists[daq_num].odt_count = count;
        slave->daq_lists[daq_num].running = false;
        dto_out[3] = count;
        dto_out[4] = daq_num * SYN_CCP_MAX_ODT_PER_DAQ;
        break;
    }

    case SYN_CCP_CMD_BUILD_PKT: {
        uint8_t daq_num = cro[2];
        uint8_t odt_num = cro[3];
        uint8_t elem_idx = cro[4];
        uint8_t elem_size = cro[5];
        if (daq_num >= SYN_CCP_MAX_DAQ_LISTS || odt_num >= SYN_CCP_MAX_ODT_PER_DAQ ||
            elem_idx >= SYN_CCP_MAX_ENTRIES_PER_ODT) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            break;
        }
        SYN_CCP_ODT *odt = &slave->daq_lists[daq_num].odts[odt_num];
        odt->entries[elem_idx].address = slave->mta0_addr;
        odt->entries[elem_idx].size = elem_size;
        if (elem_idx + 1 > odt->entry_count) {
            odt->entry_count = elem_idx + 1;
        }
        break;
    }

    case SYN_CCP_CMD_START_STOP: {
        uint8_t mode = cro[2];
        uint8_t daq_num = cro[3];
        uint8_t event_ch = cro[5];
        uint16_t prescaler = syn_peek_u16_le(cro, 6);
        if (daq_num >= SYN_CCP_MAX_DAQ_LISTS) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            break;
        }
        SYN_CCP_DAQList *daq = &slave->daq_lists[daq_num];
        daq->event_channel = event_ch;
        daq->prescaler = (prescaler == 0) ? 1 : prescaler;
        daq->cycle_counter = 0;
        daq->current_odt_idx = 0;
        daq->running = (mode == 0x01U);
        break;
    }

    case SYN_CCP_CMD_START_STOP_ALL: {
        uint8_t mode = cro[2];
        for (uint8_t i = 0; i < SYN_CCP_MAX_DAQ_LISTS; i++) {
            slave->daq_lists[i].running = (mode == 0x01U);
        }
        break;
    }

    default:
        dto_out[1] = SYN_CCP_ERR_CMD_UNKNOWN;
        break;
    }

    return true;
}

bool syn_ccp_service_daq(SYN_CCP_Slave *slave, uint8_t event_channel, uint8_t dto_out[8],
                         uint8_t *list_idx, uint8_t *odt_idx)
{
    if (slave == NULL || dto_out == NULL || list_idx == NULL || odt_idx == NULL) {
        return false;
    }

    if (!slave->connected) {
        return false;
    }

    for (uint8_t i = 0; i < SYN_CCP_MAX_DAQ_LISTS; i++) {
        SYN_CCP_DAQList *daq = &slave->daq_lists[i];
        if (!daq->running || daq->event_channel != event_channel) {
            continue;
        }

        daq->cycle_counter++;
        if (daq->cycle_counter < daq->prescaler) {
            continue;
        }
        daq->cycle_counter = 0;

        uint8_t current_odt = daq->current_odt_idx;
        if (current_odt >= daq->odt_count) {
            daq->current_odt_idx = 0;
            current_odt = 0;
        }

        if (daq->odt_count == 0) {
            continue;
        }

        SYN_CCP_ODT *odt = &daq->odts[current_odt];

        memset(dto_out, 0, 8);
        dto_out[0] = i * SYN_CCP_MAX_ODT_PER_DAQ + current_odt; /* Packet ID = ODT index */

        uint8_t payload_pos = 1;
        for (uint8_t e = 0; e < odt->entry_count; e++) {
            SYN_CCP_ODTEntry *entry = &odt->entries[e];
            if (entry->address != 0 && entry->size > 0) {
                const uint8_t *src = (const uint8_t *)(uintptr_t)entry->address;
                for (uint8_t b = 0; b < entry->size && payload_pos < 8; b++) {
                    dto_out[payload_pos++] = src[b];
                }
            }
        }

        *list_idx = i;
        *odt_idx = current_odt;

        daq->current_odt_idx++;
        if (daq->current_odt_idx >= daq->odt_count) {
            daq->current_odt_idx = 0;
        }

        return true;
    }

    return false;
}
```


