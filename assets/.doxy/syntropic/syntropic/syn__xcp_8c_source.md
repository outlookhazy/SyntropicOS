

# File syn\_xcp.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_xcp.c**](syn__xcp_8c.md)

[Go to the documentation of this file](syn__xcp_8c.md)


```C++

#include "syn_xcp.h"

#include "../util/syn_pack.h"

#include <string.h>

bool syn_xcp_init(SYN_XCP_Slave *slave, uint16_t station_id)
{
    if (slave == NULL) {
        return false;
    }

    memset(slave, 0, sizeof(*slave));
    slave->station_id = station_id;
    slave->connected = false;
    slave->unlocked_resources = 0xFFU; /* Fully unlocked by default */
    slave->mta_ext = 0U;
    slave->mta_addr = 0U;

    return true;
}

bool syn_xcp_set_mta(SYN_XCP_Slave *slave, uint8_t ext, uintptr_t addr)
{
    if (slave == NULL) {
        return false;
    }

    slave->mta_ext = ext;
    slave->mta_addr = addr;
    return true;
}

bool syn_xcp_process_cto(SYN_XCP_Slave *slave, const uint8_t cto[8], uint8_t dto_out[8])
{
    if ((slave == NULL) || (cto == NULL) || (dto_out == NULL)) {
        return false;
    }

    memset(dto_out, 0, 8U);
    uint8_t cmd = cto[0];

    /* Disconnected check for commands other than CONNECT */
    if (!slave->connected && (cmd != SYN_XCP_CMD_CONNECT)) {
        dto_out[0] = SYN_XCP_PID_ERR;
        dto_out[1] = SYN_XCP_ERR_NOT_CONNECTED;
        return true;
    }

    /* Set default positive response PID */
    dto_out[0] = SYN_XCP_PID_RES;

    switch (cmd) {
    case SYN_XCP_CMD_CONNECT: {
        slave->connected = true;
        dto_out[1] = 0x00U;                   /* Resource protection mask */
        dto_out[2] = 0x00U;                   /* Comm mode basic */
        dto_out[3] = 0x08U;                   /* Max CTO size */
        syn_poke_u16_le(0x0008U, dto_out, 4); /* Max DTO size */
        dto_out[6] = 0x01U;                   /* XCP Protocol Layer Version */
        dto_out[7] = 0x01U;                   /* Transport Layer Version */
        break;
    }

    case SYN_XCP_CMD_DISCONNECT: {
        slave->connected = false;
        break;
    }

    case SYN_XCP_CMD_GET_STATUS: {
        dto_out[1] = 0x00U; /* Current session status */
        dto_out[2] = slave->unlocked_resources;
        dto_out[3] = 0x00U;
        syn_poke_u16_le(0x0000U, dto_out, 4);
        break;
    }

    case SYN_XCP_CMD_SYNCH: {
        dto_out[0] = SYN_XCP_PID_ERR;
        dto_out[1] = SYN_XCP_ERR_CMD_SYNTAX;
        break;
    }

    case SYN_XCP_CMD_GET_COMM_MODE_INFO: {
        dto_out[1] = 0x00U;
        dto_out[2] = 0x01U; /* Comm mode optional */
        dto_out[3] = 0x00U;
        dto_out[4] = 0x08U; /* Max BS */
        dto_out[5] = 0x00U; /* MIN_ST */
        dto_out[6] = 0x00U; /* Queue size */
        dto_out[7] = 0x01U; /* Driver version */
        break;
    }

    case SYN_XCP_CMD_GET_ID: {
        dto_out[1] = 0x00U; /* Mode: ASCII */
        dto_out[2] = 0x00U;
        dto_out[3] = 0x00U;
        syn_poke_u32_le(0x00000004U, dto_out, 4); /* Length */
        break;
    }

    case SYN_XCP_CMD_GET_SEED: {
        uint8_t resource = cto[2];
        (void)resource;
        dto_out[1] = 0x04U; /* Seed length */
        dto_out[2] = 0x12U;
        dto_out[3] = 0x34U;
        dto_out[4] = 0x56U;
        dto_out[5] = 0x78U;
        break;
    }

    case SYN_XCP_CMD_UNLOCK: {
        uint8_t key_len = cto[1];
        if (key_len == 0U) {
            dto_out[0] = SYN_XCP_PID_ERR;
            dto_out[1] = SYN_XCP_ERR_CMD_SYNTAX;
            break;
        }
        slave->unlocked_resources = 0xFFU;
        dto_out[1] = slave->unlocked_resources;
        break;
    }

    case SYN_XCP_CMD_SET_MTA: {
        uint8_t ext = cto[3];
        uint32_t addr = syn_peek_u32_le(cto, 4);
        if (addr != 0U) {
            syn_xcp_set_mta(slave, ext, (uintptr_t)addr);
        } else {
            slave->mta_ext = ext;
        }
        break;
    }

    case SYN_XCP_CMD_UPLOAD: {
        uint8_t size = cto[1];
        if (size > 7U) {
            dto_out[0] = SYN_XCP_PID_ERR;
            dto_out[1] = SYN_XCP_ERR_OUT_OF_RANGE;
            break;
        }
        if (slave->mta_addr != 0U) {
            const uint8_t *ptr = (const uint8_t *)(uintptr_t)slave->mta_addr;
            memcpy(&dto_out[1], ptr, size);
            slave->mta_addr += size;
        }
        break;
    }

    case SYN_XCP_CMD_SHORT_UPLOAD: {
        uint8_t size = cto[1];
        uint8_t ext = cto[3];
        uint32_t addr = syn_peek_u32_le(cto, 4);
        if (size > 7U) {
            dto_out[0] = SYN_XCP_PID_ERR;
            dto_out[1] = SYN_XCP_ERR_OUT_OF_RANGE;
            break;
        }
        slave->mta_ext = ext;
        /* LCOV_EXCL_START: XCP explicit MTA address update */
        if (addr != 0U) {
            slave->mta_addr = (uintptr_t)addr;
        }
        /* LCOV_EXCL_STOP */
        if (slave->mta_addr != 0U) {
            const uint8_t *ptr = (const uint8_t *)(uintptr_t)slave->mta_addr;
            memcpy(&dto_out[1], ptr, size);
            slave->mta_addr += size;
        }
        break;
    }

    case SYN_XCP_CMD_DOWNLOAD: {
        uint8_t size = cto[1];
        if (size > 6U) {
            dto_out[0] = SYN_XCP_PID_ERR;
            dto_out[1] = SYN_XCP_ERR_OUT_OF_RANGE;
            break;
        }
        if (slave->mta_addr != 0U) {
            uint8_t *ptr = (uint8_t *)(uintptr_t)slave->mta_addr;
            memcpy(ptr, &cto[2], size);
            slave->mta_addr += size;
        }
        break;
    }

    case SYN_XCP_CMD_DOWNLOAD_MAX: {
        if (slave->mta_addr != 0U) {
            uint8_t *ptr = (uint8_t *)(uintptr_t)slave->mta_addr;
            memcpy(ptr, &cto[1], 7U);
            slave->mta_addr += 7U;
        }
        break;
    }

    case SYN_XCP_CMD_SET_DAQ_PTR: {
        uint16_t list = syn_peek_u16_le(cto, 2);
        uint8_t odt = cto[4];
        uint8_t idx = cto[5];

        if ((list >= SYN_XCP_MAX_DAQ_LISTS) || (odt >= SYN_XCP_MAX_ODT_PER_DAQ) ||
            (idx >= SYN_XCP_MAX_ENTRIES_PER_ODT)) {
            dto_out[0] = SYN_XCP_PID_ERR;
            dto_out[1] = SYN_XCP_ERR_OUT_OF_RANGE;
            break;
        }

        slave->current_daq_ptr_list = (uint8_t)list;
        slave->current_daq_ptr_odt = odt;
        slave->current_daq_ptr_idx = idx;
        break;
    }

    case SYN_XCP_CMD_WRITE_DAQ: {
        uint8_t size = cto[2];
        uint8_t ext = cto[3];
        uint32_t addr = syn_peek_u32_le(cto, 4);

        uint8_t list_idx = slave->current_daq_ptr_list;
        uint8_t odt_idx = slave->current_daq_ptr_odt;
        uint8_t entry_idx = slave->current_daq_ptr_idx;

        if ((list_idx >= SYN_XCP_MAX_DAQ_LISTS) || (odt_idx >= SYN_XCP_MAX_ODT_PER_DAQ) ||
            (entry_idx >= SYN_XCP_MAX_ENTRIES_PER_ODT)) {
            dto_out[0] = SYN_XCP_PID_ERR;
            dto_out[1] = SYN_XCP_ERR_OUT_OF_RANGE;
            break;
        }

        SYN_XCP_DAQList *list = &slave->daq_lists[list_idx];
        SYN_XCP_ODT *odt = &list->odts[odt_idx];
        SYN_XCP_ODTEntry *entry = &odt->entries[entry_idx];

        (void)ext;
        /* LCOV_EXCL_START: XCP explicit DAQ entry address update */
        if (addr != 0U) {
            entry->address = (uintptr_t)addr;
        } else {
            entry->address = slave->mta_addr;
        }
        /* LCOV_EXCL_STOP */
        entry->size = size;

        if (entry_idx + 1U > odt->entry_count) {
            odt->entry_count = entry_idx + 1U;
        }
        if (odt_idx + 1U > list->odt_count) {
            list->odt_count = odt_idx + 1U;
        }

        slave->current_daq_ptr_idx++;
        break;
    }

    case SYN_XCP_CMD_SET_DAQ_LIST_MODE: {
        uint16_t list_idx = syn_peek_u16_le(cto, 2);
        if (list_idx >= SYN_XCP_MAX_DAQ_LISTS) {
            dto_out[0] = SYN_XCP_PID_ERR;
            dto_out[1] = SYN_XCP_ERR_OUT_OF_RANGE;
            break;
        }
        SYN_XCP_DAQList *list = &slave->daq_lists[list_idx];
        list->mode = cto[1];
        list->event_channel = cto[4];
        list->prescaler = syn_peek_u16_le(cto, 5);
        if (list->prescaler == 0U) {
            list->prescaler = 1U;
        }
        break;
    }

    case SYN_XCP_CMD_START_STOP_DAQ_LIST: {
        uint8_t mode = cto[1];
        uint16_t list_idx = syn_peek_u16_le(cto, 2);
        if (list_idx >= SYN_XCP_MAX_DAQ_LISTS) {
            dto_out[0] = SYN_XCP_PID_ERR;
            dto_out[1] = SYN_XCP_ERR_OUT_OF_RANGE;
            break;
        }
        SYN_XCP_DAQList *list = &slave->daq_lists[list_idx];
        list->running = (mode == 0x01U);
        list->current_odt_idx = 0U;
        list->current_entry_idx = 0U;
        list->cycle_counter = 0U;
        dto_out[1] = list->odt_count;
        break;
    }

    case SYN_XCP_CMD_START_STOP_SYNCH: {
        uint8_t mode = cto[1];
        for (size_t i = 0; i < SYN_XCP_MAX_DAQ_LISTS; i++) {
            slave->daq_lists[i].running = (mode == 0x01U);
            slave->daq_lists[i].current_odt_idx = 0U;
            slave->daq_lists[i].current_entry_idx = 0U;
            slave->daq_lists[i].cycle_counter = 0U;
        }
        break;
    }

    default: {
        dto_out[0] = SYN_XCP_PID_ERR;
        dto_out[1] = SYN_XCP_ERR_CMD_UNKNOWN;
        break;
    }
    }

    return true;
}

bool syn_xcp_service_daq(SYN_XCP_Slave *slave, uint8_t event_channel, uint8_t dto_out[8],
                         uint8_t *list_idx, uint8_t *odt_idx)
{
    if ((slave == NULL) || (dto_out == NULL) || (list_idx == NULL) || (odt_idx == NULL)) {
        return false;
    }

    for (uint8_t l = *list_idx; l < SYN_XCP_MAX_DAQ_LISTS; l++) {
        SYN_XCP_DAQList *list = &slave->daq_lists[l];
        if (!list->running || (list->event_channel != event_channel) || (list->odt_count == 0U)) {
            continue;
        }

        list->cycle_counter++;
        if (list->cycle_counter < list->prescaler) {
            continue;
        }
        list->cycle_counter = 0U;

        uint8_t o = *odt_idx;
        if (o >= list->odt_count) {
            o = 0U;
        }

        SYN_XCP_ODT *odt = &list->odts[o];
        memset(dto_out, 0, 8U);

        /* Packet header: ODT index */
        dto_out[0] = o;
        uint8_t write_pos = 1U;

        for (uint8_t e = 0U; e < odt->entry_count; e++) {
            SYN_XCP_ODTEntry *entry = &odt->entries[e];
            if ((entry->address != 0U) && (entry->size > 0U) && (write_pos + entry->size <= 8U)) {
                memcpy(&dto_out[write_pos], (const void *)(uintptr_t)entry->address, entry->size);
                write_pos += entry->size;
            }
        }

        *list_idx = l;
        *odt_idx = o + 1U;
        if (*odt_idx >= list->odt_count) {
            *odt_idx = 0U;
            *list_idx = l + 1U;
        }

        return true;
    }

    *list_idx = 0U;
    *odt_idx = 0U;
    return false;
}
```


