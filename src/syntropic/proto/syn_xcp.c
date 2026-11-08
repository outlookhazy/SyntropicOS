/**
 * @file syn_xcp.c
 * @brief ASAM Universal Measurement and Calibration Protocol v1.x (XCP) Slave Implementation.
 */

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
    return true;
}

bool syn_xcp_register_mem_region(SYN_XCP_Slave *slave, uint32_t base_addr, void *host_ptr,
                                 uint32_t size, bool writable)
{
    if (slave == NULL || host_ptr == NULL || size == 0U ||
        slave->region_count >= SYN_XCP_MAX_MEM_REGIONS) {
        return false;
    }
    SYN_XCP_MemRegion *reg = &slave->mem_regions[slave->region_count];
    reg->base_addr = base_addr;
    reg->size = size;
    reg->host_ptr = (uint8_t *)host_ptr;
    reg->writable = writable;
    slave->region_count++;
    return true;
}

bool syn_xcp_set_mta(SYN_XCP_Slave *slave, uint8_t ext, uintptr_t addr)
{
    if (slave == NULL) {
        return false;
    }

    slave->mta_ext = ext;
    slave->mta_addr = addr;
    slave->mta_host_valid = (addr != 0U);
    return true;
}

/**
 * @brief Check whether an address range matches a registered XCP memory region.
 * @param reg Memory region descriptor.
 * @param addr Target protocol or host address.
 * @param size Requested access length in bytes.
 * @param is_write true for write access check, false for read access check.
 * @param out_ptr Output resolved host memory pointer on match.
 * @param err_code Output XCP error code when access fails bounds or write-protection check.
 * @return 1 if matched and permitted, 0 if outside region, -1 if matched but rejected.
 */
static int xcp_match_region(const SYN_XCP_MemRegion *reg, uintptr_t addr, uint32_t size,
                            bool is_write, uint8_t **out_ptr, uint8_t *err_code)
{
    uint64_t addr64 = (uint64_t)addr;
    uint64_t proto_base = (uint64_t)reg->base_addr;
    uint64_t host_base = (uint64_t)(uintptr_t)reg->host_ptr;
    uint64_t reg_size = (uint64_t)reg->size;
    uint32_t offset = 0U;
    if ((addr64 >= proto_base) && ((addr64 - proto_base) < reg_size)) {
        offset = (uint32_t)(addr64 - proto_base);
    } else if ((addr64 >= host_base) && ((addr64 - host_base) < reg_size)) {
        offset = (uint32_t)(addr64 - host_base);
    } else {
        return 0;
    }

    if ((uint64_t)size > (reg_size - (uint64_t)offset)) {
        *err_code = SYN_XCP_ERR_OUT_OF_RANGE;
        return -1;
    }
    if (is_write && !reg->writable) {
        *err_code = SYN_XCP_ERR_WRITE_PROTECTED;
        return -1;
    }
    *out_ptr = &reg->host_ptr[offset];
    return 1;
}

/**
 * @brief Resolve an XCP address and byte count to a validated host memory pointer.
 * @param slave XCP slave instance.
 * @param addr Target protocol or host address.
 * @param size Requested byte length.
 * @param is_write true for write access, false for read access.
 * @param host_valid true if direct host pointer fallback is allowed when no regions exist.
 * @param out_ptr Output resolved host memory pointer.
 * @param err_code Output XCP error code on failure.
 * @return true if memory access is permitted, false otherwise.
 */
static bool xcp_resolve_mem(const SYN_XCP_Slave *slave, uintptr_t addr, uint32_t size,
                            bool is_write, bool host_valid, uint8_t **out_ptr, uint8_t *err_code)
{
    if (size == 0U) {
        *out_ptr = NULL;
        return true;
    }
    for (uint8_t i = 0U; i < slave->region_count; i++) {
        int rc = xcp_match_region(&slave->mem_regions[i], addr, size, is_write, out_ptr, err_code);
        if (rc != 0) {
            return (rc > 0);
        }
    }
    if (slave->region_count == 0U && host_valid && addr != 0U) {
        *out_ptr = (uint8_t *)addr;
        return true;
    }
    *err_code = SYN_XCP_ERR_ACCESS_DENIED;
    return false;
}

/**
 * @brief Populate an XCP error response DTO packet (0xFE + error code).
 * @param dto_out 8-byte output DTO buffer.
 * @param err_code XCP error code byte.
 */
static void xcp_set_err(uint8_t dto_out[8], uint8_t err_code)
{
    dto_out[0] = SYN_XCP_PID_ERR;
    dto_out[1] = err_code;
}

/**
 * @brief Verify that the required XCP resource bitmask is unlocked, or format ERR_ACCESS_LOCKED.
 * @param slave XCP slave instance.
 * @param mask Required resource protection bitmask.
 * @param dto_out 8-byte output DTO buffer populated on lock error.
 * @return true if unlocked, false if locked.
 */
static bool xcp_require_resource(const SYN_XCP_Slave *slave, uint8_t mask, uint8_t dto_out[8])
{
    if ((slave->unlocked_resources & mask) == 0U) {
        xcp_set_err(dto_out, SYN_XCP_ERR_ACCESS_LOCKED);
        return false;
    }
    return true;
}

/**
 * @brief Handle standard XCP connection and status commands (CONNECT, DISCONNECT, GET_STATUS, etc).
 * @param slave XCP slave instance.
 * @param cmd XCP CTO command code.
 * @param dto_out 8-byte output DTO response buffer.
 */
static void xcp_handle_std_info(SYN_XCP_Slave *slave, uint8_t cmd, uint8_t dto_out[8])
{
    uint8_t all_res = SYN_XCP_RESOURCE_CAL_PAG | SYN_XCP_RESOURCE_DAQ | SYN_XCP_RESOURCE_STIM |
                      SYN_XCP_RESOURCE_PGM;
    if (cmd == SYN_XCP_CMD_CONNECT) {
        slave->connected = true;
        dto_out[1] = all_res;
        dto_out[3] = 0x08U;
        syn_poke_u16_le(0x0008U, dto_out, 4);
        dto_out[6] = 0x01U;
        dto_out[7] = 0x01U;
    } else if (cmd == SYN_XCP_CMD_DISCONNECT) {
        slave->connected = false;
        slave->pending_unlock_resource = 0U;
        slave->daq_tick_drained = false;
        for (size_t i = 0U; i < SYN_XCP_MAX_DAQ_LISTS; i++) {
            slave->daq_lists[i].running = false;
            slave->daq_lists[i].pending_tx = false;
            slave->daq_lists[i].current_odt_idx = 0U;
            slave->daq_lists[i].current_entry_idx = 0U;
            slave->daq_lists[i].cycle_counter = 0U;
        }
    } else if (cmd == SYN_XCP_CMD_GET_STATUS) {
        dto_out[1] = 0x00U;
        dto_out[2] = (uint8_t)(all_res & (uint8_t)~slave->unlocked_resources);
        syn_poke_u16_le(0x0000U, dto_out, 4);
    } else if (cmd == SYN_XCP_CMD_SYNCH) {
        xcp_set_err(dto_out, SYN_XCP_ERR_CMD_SYNTAX);
    } else if (cmd == SYN_XCP_CMD_GET_COMM_MODE_INFO) {
        dto_out[2] = 0x01U;
        dto_out[4] = 0x08U;
        dto_out[7] = 0x01U;
    } else {
        syn_poke_u32_le(0x00000004U, dto_out, 4);
    }
}

/**
 * @brief Handle XCP GET_SEED and UNLOCK resource protection commands.
 * @param slave XCP slave instance.
 * @param cmd XCP CTO command code (GET_SEED or UNLOCK).
 * @param cto 8-byte inbound CTO command buffer.
 * @param dto_out 8-byte output DTO response buffer.
 */
static void xcp_handle_security(SYN_XCP_Slave *slave, uint8_t cmd, const uint8_t cto[8],
                                uint8_t dto_out[8])
{
    if (cmd == SYN_XCP_CMD_GET_SEED) {
        uint8_t resource = cto[2];
        if (resource == 0U) {
            slave->pending_unlock_resource = 0U;
            xcp_set_err(dto_out, SYN_XCP_ERR_ACCESS_DENIED);
            return;
        }
        slave->pending_unlock_resource = resource;
        if (slave->seed_cb != NULL) {
            uint8_t seed_len = slave->seed_cb(resource, &dto_out[2], slave->seed_ctx);
            dto_out[1] = (seed_len > 6U) ? 6U : seed_len;
        } else {
            dto_out[1] = 0x04U;
            dto_out[2] = 0x12U;
            dto_out[3] = 0x34U;
            dto_out[4] = 0x56U;
            dto_out[5] = 0x78U;
        }
        return;
    }

    uint8_t key_len = cto[1];
    if (key_len == 0U || key_len > 6U) {
        xcp_set_err(dto_out, SYN_XCP_ERR_CMD_SYNTAX);
        return;
    }
    if (slave->pending_unlock_resource == 0U) {
        xcp_set_err(dto_out, SYN_XCP_ERR_ACCESS_DENIED);
        return;
    }
    uint8_t res = slave->pending_unlock_resource;
    slave->pending_unlock_resource = 0U;
    bool ok = false;
    if (slave->unlock_cb != NULL) {
        ok = slave->unlock_cb(res, &cto[2], key_len, slave->unlock_ctx);
    } else {
        ok = (key_len == 4U && cto[2] == 0x12U && cto[3] == 0x34U && cto[4] == 0x56U &&
              cto[5] == 0x78U);
    }
    if (!ok) {
        slave->unlocked_resources = (uint8_t)(slave->unlocked_resources & (uint8_t)~res);
        xcp_set_err(dto_out, SYN_XCP_ERR_ACCESS_DENIED);
        return;
    }
    uint8_t all_res = SYN_XCP_RESOURCE_CAL_PAG | SYN_XCP_RESOURCE_DAQ | SYN_XCP_RESOURCE_STIM |
                      SYN_XCP_RESOURCE_PGM;
    slave->unlocked_resources |= res;
    dto_out[1] = (uint8_t)(all_res & (uint8_t)~slave->unlocked_resources);
}

/**
 * @brief Handle XCP UPLOAD and SHORT_UPLOAD memory read commands.
 * @param slave XCP slave instance.
 * @param cmd XCP CTO command code (UPLOAD or SHORT_UPLOAD).
 * @param cto 8-byte inbound CTO command buffer.
 * @param dto_out 8-byte output DTO response buffer.
 */
static void xcp_handle_upload(SYN_XCP_Slave *slave, uint8_t cmd, const uint8_t cto[8],
                              uint8_t dto_out[8])
{
    uint8_t size = cto[1];
    if (size > 7U) {
        xcp_set_err(dto_out, SYN_XCP_ERR_OUT_OF_RANGE);
        return;
    }
    if (cmd == SYN_XCP_CMD_SHORT_UPLOAD) {
        uint32_t addr = syn_peek_u32_le(cto, 4);
        slave->mta_ext = cto[3];
        if (addr != 0U) {
            slave->mta_addr = (uintptr_t)addr;
            slave->mta_host_valid = false;
        }
    }
    uint8_t *ptr = NULL;
    uint8_t err_code = SYN_XCP_ERR_ACCESS_DENIED;
    if (!xcp_resolve_mem(slave, slave->mta_addr, size, false, slave->mta_host_valid, &ptr,
                         &err_code)) {
        xcp_set_err(dto_out, err_code);
        return;
    }
    if (ptr != NULL) {
        memcpy(&dto_out[1], ptr, size);
        slave->mta_addr += size;
    }
}

/**
 * @brief Handle XCP DOWNLOAD and DOWNLOAD_MAX calibration memory write commands.
 * @param slave XCP slave instance.
 * @param cmd XCP CTO command code (DOWNLOAD or DOWNLOAD_MAX).
 * @param cto 8-byte inbound CTO command buffer.
 * @param dto_out 8-byte output DTO response buffer.
 */
static void xcp_handle_download(SYN_XCP_Slave *slave, uint8_t cmd, const uint8_t cto[8],
                                uint8_t dto_out[8])
{
    if (!xcp_require_resource(slave, SYN_XCP_RESOURCE_CAL_PAG, dto_out)) {
        return;
    }
    uint8_t size = 7U;
    uint8_t src_off = 1U;
    if (cmd == SYN_XCP_CMD_DOWNLOAD) {
        size = cto[1];
        src_off = 2U;
        if (size > 6U) {
            xcp_set_err(dto_out, SYN_XCP_ERR_OUT_OF_RANGE);
            return;
        }
    }
    uint8_t *ptr = NULL;
    uint8_t err_code = SYN_XCP_ERR_ACCESS_DENIED;
    if (!xcp_resolve_mem(slave, slave->mta_addr, size, true, slave->mta_host_valid, &ptr,
                         &err_code)) {
        xcp_set_err(dto_out, err_code);
        return;
    }
    if (ptr != NULL) {
        memcpy(ptr, &cto[src_off], size);
        slave->mta_addr += size;
    }
}

/**
 * @brief Handle XCP SET_DAQ_PTR and WRITE_DAQ ODT entry configuration commands.
 * @param slave XCP slave instance.
 * @param cmd XCP CTO command code (SET_DAQ_PTR or WRITE_DAQ).
 * @param cto 8-byte inbound CTO command buffer.
 * @param dto_out 8-byte output DTO response buffer.
 */
static void xcp_handle_daq_ptr_write(SYN_XCP_Slave *slave, uint8_t cmd, const uint8_t cto[8],
                                     uint8_t dto_out[8])
{
    if (cmd == SYN_XCP_CMD_SET_DAQ_PTR) {
        uint16_t list = syn_peek_u16_le(cto, 2);
        uint8_t odt = cto[4];
        uint8_t idx = cto[5];
        if (list >= SYN_XCP_MAX_DAQ_LISTS || odt >= SYN_XCP_MAX_ODT_PER_DAQ ||
            idx >= SYN_XCP_MAX_ENTRIES_PER_ODT) {
            xcp_set_err(dto_out, SYN_XCP_ERR_OUT_OF_RANGE);
            return;
        }
        slave->current_daq_ptr_list = (uint8_t)list;
        slave->current_daq_ptr_odt = odt;
        slave->current_daq_ptr_idx = idx;
        return;
    }

    uint8_t list_idx = slave->current_daq_ptr_list;
    uint8_t odt_idx = slave->current_daq_ptr_odt;
    uint8_t entry_idx = slave->current_daq_ptr_idx;
    if (list_idx >= SYN_XCP_MAX_DAQ_LISTS || odt_idx >= SYN_XCP_MAX_ODT_PER_DAQ ||
        entry_idx >= SYN_XCP_MAX_ENTRIES_PER_ODT) {
        xcp_set_err(dto_out, SYN_XCP_ERR_OUT_OF_RANGE);
        return;
    }

    uint32_t addr = syn_peek_u32_le(cto, 4);
    SYN_XCP_DAQList *list = &slave->daq_lists[list_idx];
    SYN_XCP_ODT *odt = &list->odts[odt_idx];
    SYN_XCP_ODTEntry *entry = &odt->entries[entry_idx];
    if (addr != 0U) {
        entry->address = (uintptr_t)addr;
        entry->host_valid = false;
    } else {
        entry->address = slave->mta_addr;
        entry->host_valid = slave->mta_host_valid;
    }
    entry->size = cto[2];
    if ((uint8_t)(entry_idx + 1U) > odt->entry_count) {
        odt->entry_count = (uint8_t)(entry_idx + 1U);
    }
    if ((uint8_t)(odt_idx + 1U) > list->odt_count) {
        list->odt_count = (uint8_t)(odt_idx + 1U);
    }
    slave->current_daq_ptr_idx++;
}

/**
 * @brief Handle XCP DAQ mode and start/stop control commands.
 * @param slave XCP slave instance.
 * @param cmd XCP CTO command code (SET_DAQ_LIST_MODE, START_STOP_DAQ_LIST, START_STOP_SYNCH).
 * @param cto 8-byte inbound CTO command buffer.
 * @param dto_out 8-byte output DTO response buffer.
 */
static void xcp_handle_daq_ctrl(SYN_XCP_Slave *slave, uint8_t cmd, const uint8_t cto[8],
                                uint8_t dto_out[8])
{
    if (cmd == SYN_XCP_CMD_SET_DAQ_LIST_MODE) {
        uint16_t list_idx = syn_peek_u16_le(cto, 2);
        if (list_idx >= SYN_XCP_MAX_DAQ_LISTS) {
            xcp_set_err(dto_out, SYN_XCP_ERR_OUT_OF_RANGE);
            return;
        }
        SYN_XCP_DAQList *list = &slave->daq_lists[list_idx];
        list->mode = cto[1];
        list->event_channel = cto[4];
        list->prescaler = syn_peek_u16_le(cto, 5);
        if (list->prescaler == 0U) {
            list->prescaler = 1U;
        }
        return;
    }
    if (cmd == SYN_XCP_CMD_START_STOP_DAQ_LIST) {
        uint16_t list_idx = syn_peek_u16_le(cto, 2);
        if (list_idx >= SYN_XCP_MAX_DAQ_LISTS) {
            xcp_set_err(dto_out, SYN_XCP_ERR_OUT_OF_RANGE);
            return;
        }
        SYN_XCP_DAQList *list = &slave->daq_lists[list_idx];
        list->running = (cto[1] == 0x01U);
        list->pending_tx = false;
        list->current_odt_idx = 0U;
        list->current_entry_idx = 0U;
        list->cycle_counter = 0U;
        slave->daq_tick_drained = false;
        dto_out[1] = list->odt_count;
        return;
    }
    slave->daq_tick_drained = false;
    for (size_t i = 0U; i < SYN_XCP_MAX_DAQ_LISTS; i++) {
        slave->daq_lists[i].running = (cto[1] == 0x01U);
        slave->daq_lists[i].pending_tx = false;
        slave->daq_lists[i].current_odt_idx = 0U;
        slave->daq_lists[i].current_entry_idx = 0U;
        slave->daq_lists[i].cycle_counter = 0U;
    }
}

/**
 * @brief Dispatch XCP MTA, calibration upload/download, or DAQ configuration/control commands.
 * @param slave XCP slave instance.
 * @param cmd XCP CTO command code.
 * @param cto 8-byte inbound CTO command buffer.
 * @param dto_out 8-byte output DTO response buffer.
 */
static void xcp_dispatch_cal_or_daq(SYN_XCP_Slave *slave, uint8_t cmd, const uint8_t cto[8],
                                    uint8_t dto_out[8])
{
    if (cmd == SYN_XCP_CMD_SET_MTA) {
        uint32_t addr = syn_peek_u32_le(cto, 4);
        slave->mta_ext = cto[3];
        if (addr != 0U) {
            slave->mta_addr = (uintptr_t)addr;
            slave->mta_host_valid = false;
        }
        return;
    }
    if (cmd == SYN_XCP_CMD_UPLOAD || cmd == SYN_XCP_CMD_SHORT_UPLOAD) {
        xcp_handle_upload(slave, cmd, cto, dto_out);
        return;
    }
    if (cmd == SYN_XCP_CMD_DOWNLOAD || cmd == SYN_XCP_CMD_DOWNLOAD_MAX) {
        xcp_handle_download(slave, cmd, cto, dto_out);
        return;
    }
    if (cmd == SYN_XCP_CMD_SET_DAQ_PTR || cmd == SYN_XCP_CMD_WRITE_DAQ) {
        if (xcp_require_resource(slave, SYN_XCP_RESOURCE_DAQ, dto_out)) {
            xcp_handle_daq_ptr_write(slave, cmd, cto, dto_out);
        }
        return;
    }
    if (cmd == SYN_XCP_CMD_SET_DAQ_LIST_MODE || cmd == SYN_XCP_CMD_START_STOP_DAQ_LIST ||
        cmd == SYN_XCP_CMD_START_STOP_SYNCH) {
        if (xcp_require_resource(slave, SYN_XCP_RESOURCE_DAQ, dto_out)) {
            xcp_handle_daq_ctrl(slave, cmd, cto, dto_out);
        }
        return;
    }
    xcp_set_err(dto_out, SYN_XCP_ERR_CMD_UNKNOWN);
}

bool syn_xcp_process_cto(SYN_XCP_Slave *slave, const uint8_t cto[8], uint8_t dto_out[8])
{
    if ((slave == NULL) || (cto == NULL) || (dto_out == NULL)) {
        return false;
    }

    memset(dto_out, 0, 8U);
    uint8_t cmd = cto[0];

    if (!slave->connected && (cmd != SYN_XCP_CMD_CONNECT)) {
        xcp_set_err(dto_out, SYN_XCP_ERR_NOT_CONNECTED);
        return true;
    }

    dto_out[0] = SYN_XCP_PID_RES;

    switch (cmd) {
    case SYN_XCP_CMD_CONNECT:
    case SYN_XCP_CMD_DISCONNECT:
    case SYN_XCP_CMD_GET_STATUS:
    case SYN_XCP_CMD_SYNCH:
    case SYN_XCP_CMD_GET_COMM_MODE_INFO:
    case SYN_XCP_CMD_GET_ID:
        xcp_handle_std_info(slave, cmd, dto_out);
        break;
    case SYN_XCP_CMD_GET_SEED:
    case SYN_XCP_CMD_UNLOCK:
        xcp_handle_security(slave, cmd, cto, dto_out);
        break;
    default:
        xcp_dispatch_cal_or_daq(slave, cmd, cto, dto_out);
        break;
    }

    return true;
}

/**
 * @brief Check if any running DAQ list on the given event channel already has a pending ODT TX.
 * @param slave XCP slave instance.
 * @param event_channel Event channel index.
 * @return true if at least one matching DAQ list has pending_tx set, false otherwise.
 */
static bool xcp_has_pending_event(const SYN_XCP_Slave *slave, uint8_t event_channel)
{
    for (uint8_t l = 0U; l < SYN_XCP_MAX_DAQ_LISTS; l++) {
        const SYN_XCP_DAQList *list = &slave->daq_lists[l];
        if (list->running && list->event_channel == event_channel && list->pending_tx) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Advance prescaler cycle counters and arm pending_tx for running DAQ lists on an event.
 * @param slave XCP slave instance.
 * @param event_channel Event channel index.
 */
static void xcp_step_event_counters(SYN_XCP_Slave *slave, uint8_t event_channel)
{
    for (uint8_t l = 0U; l < SYN_XCP_MAX_DAQ_LISTS; l++) {
        SYN_XCP_DAQList *list = &slave->daq_lists[l];
        if (!list->running || list->event_channel != event_channel || list->odt_count == 0U) {
            continue;
        }
        uint16_t prescaler = (list->prescaler == 0U) ? 1U : list->prescaler;
        list->cycle_counter++;
        if (list->cycle_counter >= prescaler) {
            list->cycle_counter = 0U;
            list->pending_tx = true;
            if (list->current_odt_idx >= list->odt_count) {
                list->current_odt_idx = 0U;
            }
        }
    }
}

/**
 * @brief Sample all configured entries of an XCP ODT and pack them into an 8-byte DTO frame.
 * @param slave XCP slave instance.
 * @param odt ODT descriptor to sample.
 * @param pid Absolute ODT packet identifier byte.
 * @param dto_out 8-byte output DTO buffer.
 */
static void xcp_pack_odt(const SYN_XCP_Slave *slave, const SYN_XCP_ODT *odt, uint8_t pid,
                         uint8_t dto_out[8])
{
    memset(dto_out, 0, 8U);
    dto_out[0] = pid;
    uint8_t write_pos = 1U;
    for (uint8_t e = 0U; e < odt->entry_count; e++) {
        const SYN_XCP_ODTEntry *entry = &odt->entries[e];
        uint8_t *src = NULL;
        uint8_t err_code = 0U;
        if (entry->size > 0U && write_pos + entry->size <= 8U &&
            xcp_resolve_mem(slave, entry->address, entry->size, false, entry->host_valid, &src,
                            &err_code) &&
            src != NULL) {
            memcpy(&dto_out[write_pos], src, entry->size);
            write_pos = (uint8_t)(write_pos + entry->size);
        }
    }
}

void syn_xcp_daq_event(SYN_XCP_Slave *slave, uint8_t event_channel)
{
    if (slave == NULL || !slave->connected ||
        (slave->unlocked_resources & SYN_XCP_RESOURCE_DAQ) == 0U) {
        return;
    }
    xcp_step_event_counters(slave, event_channel);
    slave->daq_tick_drained = true;
    slave->daq_tick_ch = event_channel;
}

/**
 * @brief Step XCP DAQ event counters once per tick if not already stepped or draining.
 * @param slave XCP slave instance.
 * @param event_channel Event channel index.
 */
static void xcp_prepare_daq_event(SYN_XCP_Slave *slave, uint8_t event_channel)
{
    if (xcp_has_pending_event(slave, event_channel)) {
        return;
    }
    if (slave->daq_tick_drained && slave->daq_tick_ch == event_channel) {
        slave->daq_tick_drained = false;
        return;
    }
    slave->daq_tick_drained = false;
    xcp_step_event_counters(slave, event_channel);
}

bool syn_xcp_service_daq(SYN_XCP_Slave *slave, uint8_t event_channel, uint8_t dto_out[8],
                         uint8_t *list_idx, uint8_t *odt_idx)
{
    if ((slave == NULL) || (dto_out == NULL) || (list_idx == NULL) || (odt_idx == NULL)) {
        return false;
    }
    if (!slave->connected || (slave->unlocked_resources & SYN_XCP_RESOURCE_DAQ) == 0U) {
        return false;
    }

    xcp_prepare_daq_event(slave, event_channel);

    for (uint8_t k = 0U; k < SYN_XCP_MAX_DAQ_LISTS; k++) {
        uint8_t l = (uint8_t)((slave->rr_daq_idx + k) % SYN_XCP_MAX_DAQ_LISTS);
        SYN_XCP_DAQList *list = &slave->daq_lists[l];
        if (!list->running || list->event_channel != event_channel || !list->pending_tx ||
            list->odt_count == 0U) {
            continue;
        }

        uint8_t o = list->current_odt_idx;
        uint8_t pid = (uint8_t)(l * SYN_XCP_MAX_ODT_PER_DAQ + o);
        xcp_pack_odt(slave, &list->odts[o], pid, dto_out);

        *list_idx = l;
        *odt_idx = o;

        list->current_odt_idx++;
        if (list->current_odt_idx >= list->odt_count) {
            list->current_odt_idx = 0U;
            list->pending_tx = false;
            if (!xcp_has_pending_event(slave, event_channel)) {
                slave->daq_tick_drained = true;
                slave->daq_tick_ch = event_channel;
            }
        }
        slave->rr_daq_idx = (uint8_t)((l + 1U) % SYN_XCP_MAX_DAQ_LISTS);
        return true;
    }

    *list_idx = 0U;
    *odt_idx = 0U;
    return false;
}
